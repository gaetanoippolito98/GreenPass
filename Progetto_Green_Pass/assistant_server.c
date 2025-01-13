/**
 * @file    central_server.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   Il seguente programma ha lo scopo di realizzare un server "assistente", il quale avrà lo scopo di gestire le richieste dei client "revisore" ed "amministratore"
 * 
 * @type    Eseguibile
 * @version 1.0
 */

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "lib/thread_utility.h"
#include "lib/sockets_utility.h"

#define LOG TRUE         /* I server implementati nel progetto fornito, sono stati ingegnerizzati per effettuare dei log di sistema ad ogni azione di rilievo eseguita. La costante LOG
                            è stata definita in quanto permette alle direttive precompilatore, implementate nel codice di esecuzione, di attivare le stampe a video */
#define SERVER_PORT 6465 /* Definiamo come costante la porta del server "assistente" in ascolto ed in attesa di accettare nuove connessioni */

int main()
{
    /* Rende STDOUT non bufferizzato */
    (void)setvbuf(stdout, NULL, _IONBF, 0);

    /* ==========================
     * =       VARIABLES        =
     * ==========================
     * */
    int                listen_file_descriptor;                   /* File descriptor del socket in ascolto sul server */

    int                connection_file_descriptor;               /* File descriptor del socket che si occuperà di gestire nuove connessioni al server */

    int                is_address_reusable = 1;                  /* Valore intero che permette di configurare il server per il riutilizzo di un indirizzo
                                                                  * su cui è già stata effettuata la "@bind()" */

    struct sockaddr_in server_address;                           /* Struttura utile a rappresentare un Endpoint, in particolare l'indirizzo del server */

    struct sockaddr_in client_address;                           /* Struttura utile a rappresentare un Endpoint, in particolare l'indirizzo del client */

    socklen_t          client_size = sizeof(client_address);     /* Grandezza espressa in termini di byte dell'Endpoint client */

    int                i = 0;                                    /* Variabile utilizzata da indice per i costrutti iterativi */

    pthread_t          threads_id[MAX_THREADS];                  /* Array contenente i descrittori dei threads utilizzati dal server */

    Args               thread_arguments;                         /* Struttura definita per permettere alla funzione handler del thread, di accettare in input molteplici argomenti.
                                                                    In particolare, sarà composta da una strttura @sockaddr_in per permettere alla funzione handler di eseguire i 
                                                                    log delle istruzioni rilevanti eseguite. Inoltre viene utilizzato il campo @file_descriptor, in quanto ci
                                                                    permetterà di eseguire operazioni di comunicazione sul socket dal thread creato, aperto durante l'accept 
                                                                    del server */

    /* ==========================
     * =    SOCKET CREATION     =
     * ==========================
     * */

    /*
     * Inizializziamo il valore del file descriptor, che verrà configurato in modalità ascolto, sfruttando la funzione definita
     * nella libreria "@sockets_utilities.h". In questo modo, associamo il file descriptor a una socket
     * */
    listen_file_descriptor = SocketIPV4();

    /* ==========================
     * =    ZEROING  ARRAYS     =
     * ==========================
     * */

    /* Azzeriamo i byte che compongono l'array "@thread_id" per evitare di avere valori raw all'interno di quest'ultimo */
    bzero(threads_id, sizeof(threads_id));

    /*
     * ==================================
     * =        SERVER CREATION         =
     * ==================================
     * */

    /*
     * Inizializziamo i campi della struttura "@sockaddr_in" del server in modo tale da costruire un Endpoint identificabile sulla rete.
     * La struttura "@sockaddr_in" è composta dai seguenti campi:
     *      @sin_family:      Famiglia degli indirizzi utilizzati (AF_INET - AF_INET6 - ecc...)
     *      @sin_port:        Porta in Network order
     *      @sin_addr.s_addr: Indirizzo IP in Network order
     * */

    /*
     * Inizializziamo il campo famiglia della struttura "@sockaddr_in" del server con il valore "@AF_INET". In questo modo, specifichiamo
     * che il nostro server utilizzerà un indirizzo del tipo IPv4
     * */
    server_address.sin_family = AF_INET;

    /*
     * Inizializziamo il campo indirizzo della struttura "@sockaddr_in" del server attraverso il valore di ritorno della funzione
     * "@htonl()" la quale accetterà come argomento la costante "@INADDR_ANY". La funzione utilizzata permette di trasformare un
     * indirizzo in formato host (ad esempio: 127.0.0.1) in formato Network order. Tale azione si rende necessaria in quanto le
     * informazioni sulla rete viaggiano in formato BIG ENDIAN, mentre alcuni host memorizzano le informazioni in formato
     * LITTLE ENDIAN, ad esempio la seguente sequenza di byte viene memorizzata su un host LITTLE ENDIAN come segue:
     *      00000001 10000000 00000000 00000000
     * Inversamente sulla rete la precedente sequenza di byte viene memorizzata in BIG ENDIAN come segue:
     *      00000000 00000000 10000000 00000001
     * Inoltre, viene passato come argomento "@INADDR_ANY". Tale costante ci permetterà di configurare il server in ascolto per
     * qualsiasi interfaccia di rete presente sull'host server.
     * Di seguito, facciamo un esempio di conversione di un indirizzo memorizzato in LITTLE ENDIAN e successivamente convertito
     * in BIG ENDIAN, supponiamo di avere il seguente indirizzo IP:
     * 127.0.0.1
     * Convertiamo l'indirizzo in gruppi di 8 bit:
     * 01111111 00000000 00000000 00000001
     * Invertiamo l'ordine dei byte secondo le specifiche del BIG ENDIAN:
     * 00000001 00000000 00000000 01111111
     * Otteniamo il valore decimale BIG ENDIAN:
     * 16777343
     * */
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    /*
     * Inizializziamo il campo porta della struttura "@sockaddr_in" del server attraverso il valore di ritorno della funzione
     * "@htons()" la quale accetterà come argomento un intero rappresentante la porta desiderata su cui il server deve rimanere
     * in ascolto. Le porte sono interi a 16 bit da 0 a 65535, raggruppate nel seguente modo:
     *      - da 0 a 1023, porte riservate ai processi root;
     *      - da 1024 a 49151, porte registrate;
     *      - da 49152 a 65535, porte effimere, per i client, ai quali non interessa scegliere una porta specifica.
     * Per il progetto si è deciso di utilizzare una porta registrata "6465"
     * */
    server_address.sin_port = htons(6465);

    /*
     * ==================================
     * =         SOCKET OPTION          =
     * ==================================
     * */

    /*
     * La funzione "@setsockopt" ci permette di configurare l'opzione specificata dal parametro "@option_name" (il terzo argomento) al livello protocollo
     * specificato dall'argomento "@level" (secondo argomento) al valore puntato dall'argomento "@option_value" (quarto argomento) per il socket
     * associato con il file descriptor specificato dall'argomento "@socket" (primo argomento).
     * In particolare, attraverso la seguente funzione stiamo configurando al livello socket che il server in questione dovrà riutilizzare l'indirizzo
     * locale durante una nuova esecuzione del processo server
     * */
    setsockopt(listen_file_descriptor, SOL_SOCKET, SO_REUSEADDR, &is_address_reusable, sizeof(is_address_reusable));

    /*
     * ==================================
     * =              BIND              =
     * ==================================
     * */

    /*
     * Attraverso la seguente funzione associamo il file descriptor che vogliamo configurare in modalità ascolto con l'Endpoint
     * costruito precedentemente
     * */
    BindIPV4(listen_file_descriptor, &server_address);

    /*
     * ==================================
     * =             LISTEN             =
     * ==================================
     * */

    /*
     * Attraverso la seguente funzione configuriamo il file descriptor del socket passato in input in modalità ascolto, con
     * una coda delle connessioni in pendenza pari al valore di default, definito nella libreria "@socket_utility.h"
     * */
    Listen(listen_file_descriptor, DEFAULT_BACKLOG_SIZE);

    /*
     * ==================================
     * =            ENDLESS             =
     * ==================================
     * */
    for(;;)
    {
        /*
         * =================================
         * =            ACCEPT             =
         * =================================
         * */

        /* Attraverso la seguente funzione andiamo a eseguire la Three way Handshake con il client facente richiesta di connessione */
        connection_file_descriptor = AcceptIPV4(listen_file_descriptor, &client_address, &client_size);

        /* Le direttive precompilatore utilizzate nell'implementazione dei server, vengono utilizzate per attivare e disattivare i log dei processi server. */
        #ifdef LOG
        LogHostIPV4(&client_address, "Connected to", NULL);
        #endif

        /*
         * =================================
         * =       THREAD  CREATION        =
         * =================================
         * */

        /* Popoliamo la struttura di tipo @Args, utile come argomento nella chiamata a @pthread_create()*/
        thread_arguments.file_descriptor = connection_file_descriptor;
        thread_arguments.endpoint = &client_address;


        /*
         * Sfruttiamo la funzione "@pthread_create" per eseguire un nuovo thread nel processo chiamante. Quest'ultima accetterà in
         * input l'indirizzo ID del thread, attributi di default, la funzione handler e l'indirizzo del parametro di input. Inoltre,
         * tale funzione, ritornerà 0 in caso di successo e un valore diverso da 0 corrispondente al numero dell'errore, per tal
         * motivo si è deciso di assegnare ad "@errno" il valore di ritorno della funzione.
         * A ogni nuova iterazione il server creerà un thread per servire il client connesso, in questo modo evitiamo di bloccare
         * il server nel caso in cui il client invii una sequenza di byte malevoli, in quanto l'unico processo che verrà bloccato
         * sarà quello del thread, permettendo agli altri client di essere serviti
         * */
        if((errno = pthread_create(&threads_id[i++], NULL, assistant_server_handler, &thread_arguments)) != 0)
        {
            /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
            perror("Thread creation error");
            break;
        }

        /*
         * =================================
         * =       THREAD  CREATION        =
         * =================================
         * */

        /*
         * Tale blocco di codice servirà a permettere al thread master di aspettare che gli altri thread generati, attraverso una "@pthread_create()",
         * che abbiano finito di eseguire le loro istruzioni
         * */
        if(i >= DEFAULT_BACKLOG_SIZE)
        {
            i = 0;

            while(i < DEFAULT_BACKLOG_SIZE)
            {
                /*
                 * La funzione "@pthread_join()" ritornerà 0 in caso di successo e un valore diverso da 0 corrispondente al numero dell'errore, per tal
                 * motivo si è deciso di assegnare ad "@errno" il valore di ritorno della funzione
                 * */
                if((errno = pthread_join(threads_id[i++], NULL)) != 0)
                {
                    /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
                    perror("Failed to join thread");
                    break;
                }
            }

            i = 0;
        }
    }

    /* Chiudiamo il file descriptor in ascolto */
    close(listen_file_descriptor);
    /* Interrompiamo l'esecuzione del programma con un errore */
    exit(EXIT_FAILURE);
}
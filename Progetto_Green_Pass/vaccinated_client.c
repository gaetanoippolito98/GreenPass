/**
 * @file    vaccinated_client.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   Il seguente programma ha lo scopo di realizzare un client, volto a permettere, ad un utente (una volta effettuata la vaccinazione), di collegarsi 
 *          ad un server, "centro vaccinale" e di comunicargli la data di vaccinazione ed il codice di tessera sanitaria, per ottenere il documento "Green Pass". 
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
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "lib/sockets_utility.h"
#include "lib/menu_utility.h"
#include "lib/package_utility.h"
#include "lib/encryption_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */
#define IP_ADDRESS "localhost" /* Il client "vaccinato" ha lo scopo di collegarsi ad un server "centro vaccinale" e non ad altri server. Pertanto, attraverso la seguente
                                  costante, andiamo a definire l'indirizzo IP del server con cui andremo a connetterci */
#define SERVER_PORT 6462       /* Definiamo come costante la porta del server "centro vaccinale" in ascolto ed in attesa di accettare nuove connessioni */

/* Punto di accesso per un qualsiasi programma eseguibile */
int main()
{
    /* 
     * ==========================
     * =       Variables        =
     * ==========================
     */
    int                client_file_descriptor;                                       /* File descriptor associato al socket del client */

    struct sockaddr_in server_address;                                               /* Struttura utile a rappresentare un Endpoint, in
                                                                                        particolare l'indirizzo del server */

    struct hostent*    server_dns;                                                   /* Struttura contenente varie informazioni sull'host server,
                                                                                        utile ad effettuare un'operazione DNS (Risoluzione diretta) */

    char               command_writer_buffer[CMD_BUFFER_LEN];                        /* Il server "centro vaccinale" può gestire molteplici richiste, pertanto
                                                                                        per effettuare una qualsiasi operazione, sarà necessario inviare un comando
                                                                                        per eseguire un set di istruzioni predefinite */

    struct tm*         server_daytime = (struct tm*)malloc(sizeof(struct tm));      /* Il nostro protocollo livello applicazione prevede che un utente possa registrarsi 
                                                                                       ed ottenere il documento "Green Pass" solo se ha effettuato una vaccinazione in una data
                                                                                       non inferiore ad un mese dalla data odierna. Per verificare tale condizione, viene richiesto
                                                                                       un daytime al server, in modo da deresponsabilizzare il client da tale verifica ed aggiungere 
                                                                                       maggiore sicurezza al software sviluppato. La response del server verrà memorizzata 
                                                                                       attraverso una struttura di tipo tm, contenuta in @time.h */

    struct tm*         client_daytime = (struct tm*)malloc(sizeof(struct tm));      /* Il nostro protocollo livello applicazione prevede che un utente inserisca la propria data di 
                                                                                       vaccinazione. Quest'ultima viene memorizzata attraverso una struttura di tipo tm, contenuta 
                                                                                       in time.h */
                                                                                        
    char*              verification_code = (char *)malloc(MAX_CODE_LEN);            /* Il nostro protocollo livello applicazione prevede che un utente inserisca un codice di
                                                                                       tessera sanitaria, VALIDO e non precedentemente caricato, per iscriversi alla piattaforma.
                                                                                       Pertanto utilizzeremo un vettore di caratteri, allocato dinamicamente e di dimensione CL, 
                                                                                       per memorizzare il codice di tessera sanitaria */

    Vaccinated_package vaccinated_request_package;                                  /* Il nostro protocollo livello applicazione, prevede, dopo la richiesta daytime, che il client 
                                                                                       effettui un nuova richiesta al server "centro vaccinale" per effettuare la registrazione 
                                                                                       al sistema "Green pass". Pertanto, abbiamo definito un pacchetto da comunicare al 
                                                                                       "server centro vaccinale", contente le informazioni inerenti all'identità e alla data di 
                                                                                       vaccinazione dell'utente, come: 
                                                                                       - codice di tessera sanitaria 
                                                                                       - Data di vaccinazione */
                                                                                       
    File_result        is_green_pass_obtained;                                      /* Il nostro protocollo livello applicazione, prevede, dopo la richiesta di iscrizione al 
                                                                                       sistema "Green pass", che il client ottenga una response dal server "centro vaccinale",
                                                                                       contenente, l'esito della sosscrizione al sistema ed eventuali errori avvenuti durante 
                                                                                       tale processo */


    /* Controlliamo che le variabili allocate dinamicamente in heap ed i puntatori ritornati dalle funzioni non siano nulli */
    if(server_daytime == NULL || client_daytime == NULL || verification_code == NULL)
    {
        fprintf(stderr, "Errore durante l'allocazione\n");
        exit(EXIT_FAILURE);
    }

    /* ==========================
     * =    ZEROING  ARRAYS     =
     * ==========================
     * */

    /* Azzeriamo i byte che compongono i seguenti array e struct per evitare di avere valori raw all'interno di quest'ultimo */
    bzero(command_writer_buffer, CMD_BUFFER_LEN);
    bzero(client_daytime, sizeof(*client_daytime));
    bzero(server_daytime, sizeof(*server_daytime));
    bzero(verification_code, MAX_CODE_LEN);
    bzero(&vaccinated_request_package, sizeof(vaccinated_request_package));

    /*
     * ==========================
     * =          DNS           =
     * ==========================
     * */

    /*
     * La funzione "@gethostbyname()" ritorna una struttura di tipo "@hostent" in base al nome o all'indirizzo IPv4 dell'host fornito in input.
     * Inoltre, se la funzione non è andata a buon fine, allora verrà ritornato un puntatore a "@NULL" e assegnato il numero dell'errore alla
     * variabile "@h_errno" intercettata successivamente dal "@herror()"
     * */
    if((server_dns = gethostbyname(IP_ADDRESS)) == NULL)
    {
        /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
        herror("DNS error: ");

        /* Interrompiamo l'esecuzione del programma con un errore */
        exit(EXIT_FAILURE);
    }

    /*
     * ==================================
     * =        SOCKET CREATION         =
     * ==================================
     * */

    /*
     * Inizializziamo il valore del file descriptor sfruttando la funzione definita nella libreria "@sockets_utility.h".
     * In questo modo, associamo il file descriptor ad una socket
     * */
    client_file_descriptor = SocketIPV4();

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
     * Inizializziamo il campo famiglia della struttura "@sockaddr_in" del server con il valore "@server_dns->h_addrtype",
     * reperito attraverso un azione di risoluzione diretta del DNS. In questo modo, specifichiamo che il nostro server
     * utilizzerà un indirizzo del tipo IPv4
     * */
     server_address.sin_family = server_dns->h_addrtype;

     /*
      * Inizializziamo il campo della struttura "@sockaddr_in" del server attraverso il valore "@server_dns->h_addr_list[0]"
      * Sfruttiamo un cast a puntatore della struttura "@in_addr" in modo da leggere i singoli byte che compongono "@h_addr_list[0]"
      * come un intero definito nella struttura "@sin_addr.s_addr". In questo modo si memorizza l'indirizzo IP ottenuto
      * attraverso un'operazione diretta DNS in formato Network order
      * */
     server_address.sin_addr = *((struct in_addr *)server_dns->h_addr_list[0]);

    /*
     * Inizializziamo il campo porta della struttura "@sockaddr_in" del server attraverso il valore di ritorno della funzione
     * "@htons()" la quale accetterà come argomento un intero rappresentante la porta desiderata.
     * Le porte sono interi a 16 bit da 0 a 65535, raggruppate nel seguente modo:
     *      - da 0 a 1023, porte riservate ai processi root;
     *      - da 1024 a 49151, porte registrate;
     *      - da 49152 a 65535, porte effimere, per i client, ai quali non interessa scegliere una porta specifica.
     * Per il progetto si è deciso di utilizzare una porta registrata "6462" per il server "centro vaccinale"
     * */
     server_address.sin_port = htons(SERVER_PORT);

    /*
     * ==================================
     * =           CONNECTION           =
     * ==================================
     * */

    /* Eseguiamo una richiesta di Three way Handshake alla struttura "@sockaddr_in" del server precedentemente generata */
    ConnectIPV4(client_file_descriptor, &server_address);

    /*
     * ==================================
     * =        DAYTIME  REQUEST        =
     * ==================================
     * */

    /* 
    Il nostro protocollo livello applicazione prevede che un utente possa registrarsi 
    ed ottenere il documento "Green Pass" solo se ha effettuato una vaccinazione in una data
    non inferiore ad un mese dalla data odierna, oppure, se non ha inserito una data superiore a 
    quella odierna. Per verificare tale condizione, viene richiesto un daytime al server, in modo
    da deresponsabilizzare il client da tale verifica ed aggiungere maggiore sicurezza al software
    sviluppato. La response del server verrà memorizzata attraverso una struttura di tipo tm, contenuta
    in @time.h */

    /* 
     * Il server "centro vaccinale" è stato ingegnerizzato ed implementato, per gestire molteplici tipologie di richieste. In particolare,
     * vengono utilizzati dei comandi per permettere al server di interpretare la tipologia di richiesta ed eseguire un set predefinito di istruzioni.
     * Per inviare la richiesta con il comando, viene quindi preparato un buffer di scrittura @command_writer_buffer, attraverso la funzione strcpy della
     * libreria @string.h */
    strcpy(command_writer_buffer, "CMD_DTM");

    /*                                CMD_DTM
     * |Client vaccinato|--------------------------------->|Server centro vaccianle|
     */

    /* Effettuiamo una richiesta daytime comunicando al server "centro vaccinale", attraverso una @FullWrite, il comando specificandone la tipologia (CMD_DTM) */
    FullWrite(client_file_descriptor, command_writer_buffer, CMD_BUFFER_LEN);

    /*
     * ==================================
     * =       DAYTIME   RESPONSE       =
     * ==================================
     * */

    /*                                Daytime
     * |Client vaccinato|<----------------------------------|Server centro vaccinale|
     */

    /* Eseguiamo una "@FullRead()" per ottenere una response Daytime dal server connesso */
    if(FullRead(client_file_descriptor, server_daytime, sizeof(*server_daytime)) < 0)
    {
        /* Caso in cui il server si sia disconnesso */

        fprintf(stderr, "Server disconnesso\n");

        /* Liberiamcharo la memoria precedentemente allocata dinamicamente nella memoria heap tramite una "@malloc" */
        free(server_daytime);
        free(client_daytime);
        free(verification_code);

        /* Chiusura del socket file descriptor connesso al server */
        close(client_file_descriptor);
        /* Terminiamo con successo il processo client */
        exit(EXIT_FAILURE);
    }

    /*
     * ==================================
     * =     SUBSCRIPTION  REQUEST      =
     * ==================================
     * */

    /* Eseguiamo il menu del client vaccinato, la cui funzione @run_vaccinated_menu si occuperà di popolare i parametri di output @client_daytime e @verification_code */
    if(!run_vaccinated_menu(client_daytime, server_daytime, verification_code))
    {
        /* Liberiamo la memoria precedentemente allocata dinamicamente nella memoria heap tramite una "@malloc" */
        free(server_daytime);
        free(client_daytime);
        free(verification_code);
        /* Chiusura del socket file descriptor connesso al server */
        close(client_file_descriptor);
        /* Terminiamo con successo il processo client */
        exit(EXIT_FAILURE);
    }

    /*  
        Inizializziamo i campi della struttuta di tipo @Vaccinated Package in moodo da inviare al server "centro vaccinale" una request contenente le informazioni dell'utente 
        iscritto alla piattaforma "Green pass"
    */
    vaccinated_request_package.vaccination_date = *client_daytime;
    strcpy(vaccinated_request_package.card_code, verification_code);

    /* Copiamo la stringa "CMD_SUB" all'interno dell'array di caratteri "@command_writer_buffer" */
    strcpy(command_writer_buffer, "CMD_SUB");

    /*                                CMD_SUB
     * |Client vaccinato|----------------------------------->|Server centro vaccinale|
     */

    /* Effettuiamo una richiesta al server con le informazioni contenute nel "@command_writer_buffer" */
    FullWrite(client_file_descriptor, command_writer_buffer, CMD_BUFFER_LEN);

    /*                              Vaccinated_package
     * |Client vaccinato|----------------------------------->|Server centro vaccinale|
     */

    /* Criptiamo il codice da inviare al server centro vaccinale */
    xor_crypt(vaccinated_request_package.card_code, 14);
    /* Effettuiamo una richiesta di iscrizione al server con le informazioni contenute nel @vaccinated_request_package */
    FullWrite(client_file_descriptor, &vaccinated_request_package, sizeof(vaccinated_request_package));

    /*                               File_result
     * |Client vaccinato|<---------------------------------|Server centro vaccinale|
     */
    if(FullRead(client_file_descriptor, &is_green_pass_obtained, sizeof(File_result)) > 0)
    {
        /* Liberiamo la memoria precedentemente allocata dinamicamente nella memoria heap tramite una "@malloc" */
        free(server_daytime);
        free(client_daytime);
        free(verification_code);
        /* Chiusura del socket file descriptor connesso al server */
        close(client_file_descriptor);
        /* Terminiamo con successo il processo client */
        exit(EXIT_FAILURE);
    }

    /* Stampiamo i risultati dell'operazione di iscrizione alla piattaforma "Green Pass" */
    if(is_green_pass_obtained.file_flags.write_file_flag || is_green_pass_obtained.file_flags.open_file_flag)
    {
        /* Caso di errore di scrittura o apertura file nel server centrale */
        fprintf(stderr,"Anomalia durante l'operazione del server\n");
    }
    else if(is_green_pass_obtained.result_flag)
    {
        /* Caso di avvenuta iscrizione alla piattaforma "Green Pass"*/
        printf("Caricato con successo\n");
    }
    else
    {
        /* Caso di iscrizione alla piattaforma "Green Pass" fallita in quanto l'utente è già presente */
        fprintf(stderr,"Errore nel caricamento\n");
    }

    /* Liberiamo la memoria precedentemente allocata dinamicamente nella memoria heap tramite una "@malloc" */
    free(server_daytime);
    free(client_daytime);
    free(verification_code);

    /* Chiusura del socket file descriptor connesso al server */
    close(client_file_descriptor);
    /* Terminiamo con successo il processo client */
    exit(EXIT_SUCCESS);
}
/**
 * @file    thread_utility.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   I server sono stati ingegnerizzati ed implementati sfruttando la logica dei thread. Tale scelta è stata presa, prendendo in considerazione la sicurezza del servizio
 *          offerto, in quanto, senza thread un client malintenzionato potrebbe comunicare uno stream di byte che comporterebbe il blocco del server e la conseguente 
 *          interruzione del servizio. Invece, attraverso l'utilizzo dei thread, un client malintenzionato potrebbe bloccare l'esecuzione di un thread, permettendo agli altri 
 *          client di essere serviti in qualsiasi caso. In tale libreria si è proceduto con il definire il set di istruzioni che ogni server deve eseguire in corrispondenza
 *          della creazione di un thread quando una nuova connessione viene accettata. Pertanto, vengono definite le funzioni handler di ogni server.
 * 
 * @type    Implementazione libreria @thread_utility.h
 * @version 1.0
 */

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "sockets_utility.h"
#include "thread_utility.h"
#include "package_utility.h"
#include "file_utility.h"
#include "date_utility.h"
#include "green_pass_utility.h"
#include "encryption_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */
#define LOG TRUE /* I server implementati nel progetto fornito, sono stati ingegnerizzati per effettuare dei log di sistema ad ogni azione di rilievo eseguita. La costante LOG
                    è stata definita in quanto permette alle direttive precompilatore, implementate nel codice di esecuzione, di attivare le stampe a video */

/* 
 * ==========================
 * =          Macro         =
 * ==========================
 */

/* 
 * La seguente direttiva precompilatore è utile a vericare la presenza della macro @SYS_gettid, utilizzata nella macro per definire @gettid(). In particolare, se quest'ultima
 * non è presente nel sistema operativvo di compilazione, verrà lanciato un errore con la stampa corrispondente alla stringa inserita 
*/
#ifndef SYS_gettid
#error "SYS_gettid unavailable on this system"
#endif

/* 
 * Macro volta a definire un'operazione di reperimento dell'id thread in esecuzione, attraverso la chiamata di sistema @syscall(), la quale accetterà in input la macro 
 * @SYS_gettid */
#define gettid() ((pid_t)syscall(SYS_gettid))

/**
 * @brief  Le funzioni di creazione socket, connessione, scrittura e lettura su socket possono avvenire sul thread master, oppure, su un thread differente. Pertanto si è deciso 
 *         di rendere tali funzioni thread safe, attraverso la seguente funzione, il quale ci permetterà di comprendere se la funzione viene eseguita sul thread principale oppure 
 *         su un thread secondario, prevedendo un set di istruzioni divverso in base all'ambiente di esecuzione.
 * 
 * @return true - il processo è in esecuzione sul thread master
 * @return false - il processo non è in esecuzione sul thread master
 */
bool is_main_thread(void)
{
    /* Se il process id è uguale al thread id, allora il processo sarà in esecuzione sul thread master */
    return getpid() == gettid();
}

/**
 * @brief Funzione handler per gestire il set di istruzioni da eseguire alla creazione del thread, successivamente all'@Accept() del server "centro vaccinale". Il server 
 *        in questione è stato ingegnerizzato ed implementato, per gestire molteplici tipologie di richieste. In particolare, vengono utilizzati dei comandi per permettere
 *        al server di interpretare la tipologia di richiesta ed eseguire un set predefinito di istruzioni. Di seguito viene mostrato l'elenco di comandi e le corrispettive 
 *        funzionalità:
 * 
 *        - CMD_DTM: Richiesta ricevuta dal client "vaccinato" per richiedere un daytime locale del server
 *        - CMD_SUB: Richiesta ricevuta dal client "vaccinato" per effettuare l'iscrizione di uno specifico utente al sistema "Green Pass"
 * 
 * @return void* - tutte le funzioni handler hanno un particolare tipo di sintassi, tra cui un tipo di ritorno specificato come puntatore a void
 */
void* vaccination_center_handler(void* args)
{
    /* Rende STDOUT non bufferizzato */
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    
    /* ==========================
     * =       VARIABLES        =
     * ==========================
     * */
    int                connection_file_descriptor = ((Args*)args)->file_descriptor;   /* File descriptor del socket che si occuperà di gestire nuove connessioni al server */

    struct sockaddr_in client_address = *(((Args*)args)->endpoint);                   /* Struttura utile a rappresentare un Endpoint, in particolare l'indirizzo del client */

    char               command_reader_buffer[CMD_BUFFER_LEN];                         /* Buffer utile all'operazione di lettura del comando inviato sul file descriptor del socket */

    char               command_writer_buffer[CMD_BUFFER_LEN];                         /* Buffer utile all'operazione di scrittura del comando da inviare sul file descriptor 
                                                                                         del socket */

    time_t             server_daytime;                                                /* Variabile utile a contenere un valore intero con lo scopo di rappresentare il numero di 
                                                                                         secondi da 00:00, 1 gennaio 1970 */

    struct tm*         local_daytime;                                                 /* Struttura utile a memorizzare data e tempo locale suddivisi in campi */

    Vaccinated_package vaccinated_response_package;                                   /* Il nostro protocollo livello applicazione, prevede, dopo la richiesta daytime, che 
                                                                                         il client effettui un nuova richiesta al server "centro vaccinale" per effettuare la
                                                                                         registrazione al sistema "Green pass". Tale richiesta viene effettuata dal client verso il server 
                                                                                         "centro veccinale", attraverso un package di tipo Vaccinated Package, contente le 
                                                                                         informazioni inerenti all'identità e alla data di vaccinazione dell'utente, come: 
                                                                                         - codice di tessera sanitaria 
                                                                                         - Data di vaccinazione */

    while(1)
    {
        /* =========================
         * =        ZEROING        =
         * =========================
         * */

        /*
         * Azzeriamo i byte che compongono i seguenti array e strutture per evitare di avere valori raw all'interno di questi ultimi */
        bzero(command_reader_buffer, CMD_BUFFER_LEN);
        bzero(command_writer_buffer, CMD_BUFFER_LEN);
        bzero(&vaccinated_response_package, sizeof(vaccinated_response_package));

        /*
         * ==================================
         * =           FULL READ            =
         * ==================================
         * */

        /*                                   CMD
         * |Client vaccinato|--------------------------------->|Server centro vaccinale|
         */

        /* Eseguiamo un'operazione di lettura sul socket per leggere il comando inviato dal client */
        if(FullRead(connection_file_descriptor, &command_reader_buffer, CMD_BUFFER_LEN) > 0)
        {
            /*
             * ==================================
             * =         CLOSE  THREAD          =
             * ==================================
             * */

            /* Caso in cui il client si sia disconnesso */

            /* Le direttive precompilatore utilizzate nell'implementazione dei server, vengono utilizzate per attivare e disattivare i log dei processi server. */
            #ifdef LOG
            /* Stampa della disconnesione con il client */
            LogHostIPV4(&client_address, "Client disconnected:", NULL);
            #endif

            /* Chiusura del socket file descriptor connesso al client */
            close(connection_file_descriptor);
            /*
             * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
             * reperire l'informazione relativa al prossimo thread disponibile
             * */
            pthread_exit(NULL);
        }
        
        #ifdef LOG
        LogHostIPV4(&client_address, "Request received from", "Command request");
        #endif

        /*
         * ================================
         * =       COMMAND  COMPARE       =
         * ================================
         * */

         /* 
          * Il server "centro vaccinale" può gestire molteplici richiste, pertanto per effettuare una qualsiasi operazione, sarà necessario inviare un comando
          * per eseguire un set di istruzioni predefinite. Per implementare un menu di comandi, ci avvaliamo della funzione @strcmp() definita in @string.h 
          */
        if(!strcmp(command_reader_buffer, "CMD_DTM"))
        {
            /*
             * =================================
             * =       DAYTIME  RESPONSE       =
             * =================================
             * */

            /* Sezione che gestisce il comando "@CMD_DTM" mandato dal client per servire la richiesta del daytime al client */

            /* Tale funzione ritorna un valore intero rappresentante il numero di secondi da 00:00, 1 gennaio 1970 */
            server_daytime = time(NULL);
            /* Tale funzione ci permette di convertire un tipo "@time_t" in una struttura utile a memorizzare data e tempo locale suddivisi in campi */
            if((local_daytime = localtime(&server_daytime)) == NULL)
            {
                /* Caso in cui la funzione @localtime non sia andata a buon fine */

                /* Stampiamo l'errore su standard error */
                fprintf(stderr, "local day time error\n");

                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }

            /*                                Daytime
             * |Client vaccinato|<---------------------------------|Server centro vaccinale|
             */

            /* Ci avvaliamo della funzione "@FullWrite" per scrivere sul socket i bytes che compongono la struttura di tipo "@tm" */
            FullWrite(connection_file_descriptor, local_daytime, sizeof(*local_daytime));
            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif
        }
        else if(!strcmp(command_reader_buffer, "CMD_SUB"))
        {
            /*
             * =================================
             * =     SUBSCRIPTION  RESPONSE    =
             * =================================
             * */
            
            
            int                subscription_socket;               /* File descriptor relativo alla nuova connessione con il server centrale */
            
            struct sockaddr_in central_server_address;            /* Struttura @sockaddr_in relativa all'endpoint del server centrale*/
            
            const char         central_server_ip[] = "127.0.0.1"; /* IP relativo al server centrale */
           
            File_result        is_card_code_written;              /* Struttura utile a memorizzare la presenza di uno specifico codice di tessera sanitaria all'interno del 
                                                                     file di memorizzazione degli utenti iscritti.*/

            /*                            Vaccinated_package
             * |Client vaccinato|--------------------------------->|Server centro vaccinale|
             */

            if(FullRead(connection_file_descriptor, &vaccinated_response_package, sizeof(vaccinated_response_package)) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&client_address, "Client disconnected:", NULL);
                #endif
                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&client_address, "Request received from", command_reader_buffer);
            #endif

            /*
             * Successivamente andremo ad instaurare una connessione con il server centrale, una volta che quest'ultimo avrà servito il server centro vaccinale, la connessione
             * verrà chiusa. La connessione tra le due entità avverrà su un altro socket, creato esplicitamente per permettere una nuova comunicazione mentre il server centro
             * vaccinale rimane connesso con il corrispettivo client
             */

            /* ==========================
             * =    SOCKET CREATION     =
             * ==========================
             * */

            subscription_socket = SocketIPV4();

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
            central_server_address.sin_family = AF_INET;

            /*
             *
             * */
            if(inet_pton(AF_INET, central_server_ip, &central_server_address.sin_addr) <= 0)
            {
                /*  */
                fprintf(stderr, "inet_pton() error for %s\n", central_server_ip);
                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                close(subscription_socket);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }

            /*
             * Inizializziamo il campo porta della struttura "@sockaddr_in" del server attraverso il valore di ritorno della funzione
             * "@htons()" la quale accetterà come argomento un intero rappresentante la porta desiderata su cui il server deve rimanere
             * in ascolto. Le porte sono interi a 16 bit da 0 a 65535, raggruppate nel seguente modo:
             *      - da 0 a 1023, porte riservate ai processi root;
             *      - da 1024 a 49151, porte registrate;
             *      - da 49152 a 65535, porte effimere, per i client, ai quali non interessa scegliere una porta specifica.
             * Per il progetto si è deciso di utilizzare una porta registrata "6464"
             * */
            central_server_address.sin_port = htons(6464);

            /*
             * ==================================
             * =           CONNECTION           =
             * ==================================
             * */

            /* Eseguiamo una richiesta di Three way Handshake alla struttura "@sockaddr_in" del server precedentemente generata */
            ConnectIPV4(subscription_socket, &central_server_address);

            strcpy(command_writer_buffer, "CMD_CTR");

            /*
             * Prima di calcolare la data di scadenza del "Green Pass", verifichiamo se l'utente è già iscritto alla piattaforma. Se è già iscritto l'utente non potrà iscriversi
             * nuovamente. Pertanto manderemo prima una richiesta al client per verificare tale condizione. Il server "centrale" risponderà:
             * - 0 se non è già iscritto 
             * - 1 se già iscritto
            */

            /*                                        CMD_CTR
             * |Server centro vaccinale|---------------------------------->|Server centrale|
             */
            FullWrite(subscription_socket, command_writer_buffer, CMD_BUFFER_LEN);
            #ifdef LOG
            LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
            #endif

            /*                                    card_code[21]
             * |Server centro vaccinale|---------------------------------->|Server centrale|
             */
            FullWrite(subscription_socket, vaccinated_response_package.card_code, sizeof(vaccinated_response_package.card_code));
            #ifdef LOG
            LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
            #endif

            /*                                       File_Result
             * |Server centro vaccinale|<----------------------------------|Server centrale|
             */
            if(FullRead(subscription_socket, &is_card_code_written, sizeof(File_result)) > 0)
            {
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Server disconnected from:", NULL);
                #endif
                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                close(subscription_socket);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&central_server_address, "Response received from", command_reader_buffer);
            #endif

            /* 
             * Attraverso il seguente costrutto di controllo verifichiamo che il server centrale non abbia riscontrato errori duurante l'apertura del file, ciò è possibile, 
             * in quanto @file_flags.open_file_flag sarà inizializzato ad:
             * 
             * - 1: Nel caso in cui vi sia stato un errore
             * - 0: Nel caso in cui non vi sia stato un errore
            */
            if(is_card_code_written.file_flags.open_file_flag)
            {
                /* Caso di errore durante l'apertura file sul server "centrale" */
                /* Chiudiamo la connessione con il server centrale */
                close(subscription_socket);

                /*                               File_result
                 * |Client vaccinato|<---------------------------------|Server centro vaccinale|
                 */
                FullWrite(connection_file_descriptor, &is_card_code_written, sizeof(File_result));
                #ifdef LOG
                LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
                #endif

                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            else if(!is_card_code_written.result_flag)
            {
                /* Caso in cui l'utente non si sia già iscritto @result_flag = 0 */

                /* Variabile dichiarata per comunicare al client l'esito dell'operazione di iscrizione alla piattaforma */
                File_result is_subscribed;

                /* Calcoliamo la data di scadenza del "Green Pass" (6 mesi dalla data di vaccinazione) */
                struct tm expiration_date = add_month_to_date(vaccinated_response_package.vaccination_date, EXPIRATION_DURATION_MONTH);

                /* Popoliamo la struttura @subscribe package con i dati ricevuti dal client e la data di scadenza "Green Pass" calcolata nell'istruzione precedente */
                Subscribe_package sub_client_vaccination = {.vaccinated_package.vaccination_date = vaccinated_response_package.vaccination_date,
                        .expiration_date = expiration_date};
                strcpy(sub_client_vaccination.vaccinated_package.card_code, vaccinated_response_package.card_code);

                /* Prepariamo il comando da inviare al "Server Centrale"*/
                strcpy(command_writer_buffer, "CMD_MEM");

                /*                                       File_Result
                 * |Server centro vaccinale|----------------------------------->|Server centrale|
                 */
                FullWrite(subscription_socket, command_writer_buffer, CMD_BUFFER_LEN);
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
                #endif

                /*                                       File_result
                 * |Server centro vaccinale|------------------------------------>|Server centrale|
                 */
                FullWrite(subscription_socket, &sub_client_vaccination, sizeof(sub_client_vaccination));
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
                #endif

                /*                                       File_result
                 * |Server centro vaccinale|<-----------------------------------|Server centrale|
                 */
                FullRead(subscription_socket, &is_subscribed, sizeof(File_result));
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Response received from", command_reader_buffer);
                #endif

                /* Chiudiamo la connessione con il server centrale */
                close(subscription_socket);

                /*                               File_result
                 * |Client vaccinato|<---------------------------------|Server centro vaccinale|
                 */
                FullWrite(connection_file_descriptor, &is_subscribed, sizeof(File_result));
                #ifdef LOG
                LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
                #endif
            }
            else
            {
                /* Caso in cui l'utente sia già presente nel file @result_flag = 1 */

                /* Chiudiamo la connessione con il server "centrale "*/
                close(subscription_socket);

                /* 
                 * In questo caso il @result_flag ricevuto, sarà uguale ad 1. L'operazione pertanto, non è andata a buon fine, quindi si prevede di inviare uno 0
                 * negando tale variabile
                 */
                is_card_code_written.result_flag = !is_card_code_written.result_flag;

                /*                                File_result
                 * |Client vaccinato|<----------------------------------|Server centro vaccinale|
                 */
                FullWrite(connection_file_descriptor, &is_card_code_written, sizeof(File_result));
                #ifdef LOG
                LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
                #endif
            }
        }
    }
}

/**
 * @brief Funzione handler per gestire il set di istruzioni da eseguire alla creazione del thread, successivamente all'@Accept() del server "centrale". Il server 
 *        in questione è stato ingegnerizzato ed implementato, per gestire molteplici tipologie di richieste. In particolare, vengono utilizzati dei comandi per permettere
 *        al server di interpretare le tipologie di richieste ed eseguire un set predefinito di istruzioni. Di seguito viene mostrato l'elenco di comandi e le corrispettive 
 *        funzionalità:
 * 
 *        - CMD_CTR: Richiesta ricevuta dal server "centro vaccinale" per verificare l'esistenza dell'utente sul file di memorizzazione
 *        - CMD_MEM: Richiesta ricevuta dal server "centro vaccinale" per effettuare la memorizzazione dell'utente vaccinato sul file
 *        - CMD_REV: Richiesta ricevuta dal server "assistente" qper fornire le informazioni del "Green pass" di uno specifico utente
 *        - CMD_LST: Richiesta ricevuta dal server "assistente" per fornire la lista degli utenti iscritti alla piattaforma "Green pass"
 *        - CMD_MOD: Richiesta ricevuta dal server "assistente" per modificare alcune informazioni inerenti ad uno specifico utente iscritto alla piattaforma "Green pass"
 * 
 * @return void* - tutte le funzioni handler hanno un particolare tipo di sintassi, tra cui un tipo di ritorno specificato come puntatore a void
 */
void* central_server_handler(void* args)
{
    /* Attraverso la seguente istruzione rendiamo lo standard output non bufferizzato, in modo da permettere ai log del server di essere stampati a video */
    (void)setvbuf(stdout, NULL, _IONBF, 0);

    
    int                connection_file_descriptor = ((Args*)args)->file_descriptor; /* File descriptor del socket che si occuperà di gestire nuove connessioni al server */

    struct sockaddr_in client_address = *(((Args*)args)->endpoint);                 /* Struttura utile a rappresentare un Endpoint, in particolare l'indirizzo del client */

    char               command_reader_buffer[CMD_BUFFER_LEN];                       /* Buffer utile all'operazione di lettura del comando inviato sul file descriptor del socket */

    char               reader_buffer[21];                                           /* Buffer utile all'operazione di lettura del codice di tessera sanitaria sul file 
                                                                                       descriptor del socket */

    File_result        is_card_code_written;                                        /* File_result struttura contenente informazioni relative agli errori di apertura, lettura e 
                                                                                       scrittura sul file, con l'aggiunta di un valore booleano che ha lo scopo di interpretare la
                                                                                       presenza di una determinata stringa in un file. */

    while(1)
    {
        /* =========================
         * =        ZEROING        =
         * =========================
         * */

        /*
         * Azzeriamo i byte che compongono l'array "@command_reader_buffer" e "@command_writer_buffer" per evitare di avere
         * valori raw all'interno di questi ultimi
         * */
        bzero(command_reader_buffer, CMD_BUFFER_LEN);
        bzero(reader_buffer, 21);

        /*                                        CMD_CTR
         * |Server centro vaccinale|---------------------------------->|Server centrale|
         */
        if(FullRead(connection_file_descriptor, &command_reader_buffer, CMD_BUFFER_LEN) > 0)
        {
            /*
             * ==================================
             * =         CLOSE  THREAD          =
             * ==================================
             * */

            /* Caso in cui il client si sia disconnesso */
            #ifdef LOG
            LogHostIPV4(&client_address, "Client disconnected:", NULL);
            #endif
            /* Chiusura del socket file descriptor connesso al client */
            close(connection_file_descriptor);
            /*
             * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
             * reperire l'informazione relativa al prossimo thread disponibile
             * */
            pthread_exit(NULL);
        }
        #ifdef LOG
        LogHostIPV4(&client_address, "Request received from", "Command request");
        #endif

        /*
         * ================================
         * =       COMMAND  COMPARE       =
         * ================================
         * */

        /* 
         * Il server "centrale" può gestire molteplici richiste, pertanto per effettuare una qualsiasi operazione, sarà necessario inviare un comando
         * per eseguire un set di istruzioni predefinite. Per implementare un menu di comandi, ci avvaliamo della funzione @strcmp() definita in @string.h 
         */
        if(!strcmp(command_reader_buffer, "CMD_CTR"))
        {
            /*                                      card_code[21]
             * |Server centro vaccinale|---------------------------------->|Server centrale|
             */
            if(FullRead(connection_file_descriptor, &reader_buffer, 21) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&client_address, "Client disconnected:", NULL);
                #endif
                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&client_address, "Request received from", command_reader_buffer);
            #endif

            /* Verifichiamo che il codice inviato dal server "centro vaccinale" sia presente nel file */
            is_card_code_written = is_code_written_in_file(VACCINATED_FILE_NAME, reader_buffer, MAX_FILE_LINE_SIZE);

            /*                                      File_result
             * |Server centro vaccinale|<----------------------------------|Server centrale|
             */

            FullWrite(connection_file_descriptor, &is_card_code_written, sizeof(File_result));
            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif
        }
        else if(!strcmp(command_reader_buffer, "CMD_MEM"))
        {
            Subscribe_package sub_client_vaccination;                  /* Pacchetto da ricevere dal Server "Centro vaccinale", contenente le informazioni utente per iscrivere
                                                                          procedere all'iscrizione di quest'ultimo alla piattaforma "Green Pass" */

            char              file_writer_buffer[MAX_FILE_LINE_SIZE];  /* Variabile dichiarata per creare una stringa formattata contenente le informazione dell'utente per 
                                                                          l'iscrizione */

            char              expiration_date_buffer[MAX_DATE_LEN];    /* Variabile dichiarata per creare una stringa formattata contenente la data di scadenza del "Green Pass" */

            char              last_update_date_buffer[MAX_DATE_LEN];   /* Variabile dichiarata per creare una stringa formattata contenente la data di ultimo aggiornamento
                                                                          del "Green Pass" */

            File_result       is_subscribed;                           /* Variabile dichiarata per comunicare al server "centro vaccinale" l'esito dell'operazione di iscrizione di 
                                                                          un utente*/

            /*                                    Subscribe_package
             * |Server centro vaccinale|----------------------------------->|Server centrale|
             */
            if(FullRead(connection_file_descriptor, &sub_client_vaccination, sizeof(sub_client_vaccination)) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&client_address, "Client disconnected:", NULL);
                #endif
                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&client_address, "Request received from", command_reader_buffer);
            #endif

            /* Formattiamo le date attraverso la funzione @strftime() */
            strftime(expiration_date_buffer, MAX_DATE_LEN, "%d/%m/%Y", &(sub_client_vaccination.expiration_date));
            strftime(last_update_date_buffer, MAX_DATE_LEN, "%d/%m/%Y", &(sub_client_vaccination.vaccinated_package.vaccination_date));

            /* Formattiamo la stringa da memorizzare su file attraverso la funzione @snprintf() */
            snprintf(file_writer_buffer, MAX_FILE_LINE_SIZE, "%s %s Vaccinazione %s", sub_client_vaccination.vaccinated_package.card_code,
                                                                                                       expiration_date_buffer,
                                                                                                       last_update_date_buffer);
            /* Iscriviamo l'utente alla piattaforma con le informazioni contenute nel @file_writer_buffer */
            is_subscribed = subscribe_vaccinated_client(file_writer_buffer);

            /*                                      File_result
             * |Server centro vaccinale|<----------------------------------|Server centrale|
             */
            FullWrite(connection_file_descriptor, &is_subscribed, sizeof(File_result));
            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif
        }
        else if(!strcmp(command_reader_buffer, "CMD_REV"))
        {
            Reviser_package reviser_package; /* Variabile di tipo @Reviser_package, dichiarata per inviare al server "assistente" le informarzioni sul "Green Pass" di 
                                                un utente */

            /*                                Card_code[21]
             * |Server assistente|---------------------------------->|Server centrale|
             */

            if(FullRead(connection_file_descriptor, reader_buffer, 21) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */

                #ifdef LOG
                LogHostIPV4(&client_address, "Client disconnected:", NULL);
                #endif

                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&client_address, "Request received from", command_reader_buffer);
            #endif

            /* Controlliamo che l'utente sia iscritto alla piattaforma "Green Pass" */
            if(is_code_written_in_file(VACCINATED_FILE_NAME, reader_buffer, MAX_FILE_LINE_SIZE).result_flag)
            {
                /* 
                 * Popoliamo la struttura @reviser_package del tipo @Reviser_package attraverso la funzione @generate_reviser_package, reperendo le informazioni contenute 
                 * nel file
                 */
                if(generate_reviser_response(&reviser_package, reader_buffer) != NULL)
                {
                    /* Chiusura del socket file descriptor connesso al client */
                    close(connection_file_descriptor);
                    /*
                     * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                     * reperire l'informazione relativa al prossimo thread disponibile
                     * */
                    pthread_exit(NULL);
                }
            }
            else
            {
                /* Caso di errore di lettura del file */
                bzero(&reviser_package, sizeof(reviser_package));

                /* Impostiamo l'errore di lettura del file*/
                reviser_package.file_flags.read_file_flag = TRUE;
            }

            /*                               Reviser_package
             * |Server assistente|<----------------------------------|Server centrale|
             */
            FullWrite(connection_file_descriptor, &reviser_package, sizeof(reviser_package));
            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif
        }
        else if(!strcmp(command_reader_buffer, "CMD_LST"))
        {
            char* vaccinated_array_list; /* Lista degli utenti iscritti alla piattaforma "Green Pass" */

            int   size_vaccinated_list;  /* Numero di utenti iscritti alla piattaforma "Green Pass" */

            /* Recuperiamo la lista di utenti iscritti alla piattaforma e il numero di utenti, attraverso la funzione @get_file_list() */
            if((size_vaccinated_list = get_file_list(VACCINATED_FILE_NAME, &vaccinated_array_list)) == -1)
            {
                #ifdef LOG
                LogHostIPV4(&client_address, "Disconnected from client:", NULL);
                #endif

                free(vaccinated_array_list);
                close(connection_file_descriptor);
                pthread_exit(NULL);
            }

            /*                             size_vaccinated_list
             * |Server assistente|<----------------------------------|Server centrale|
             */
            FullWrite(connection_file_descriptor, &size_vaccinated_list, sizeof(int));
            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif

            /*                            vaccinated_array_list
             * |Server assistente|<----------------------------------|Server centrale|
             */
            FullWrite(connection_file_descriptor, vaccinated_array_list, size_vaccinated_list * 21);
            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif

            /* Liberiamo la memoria allocata dinamicamente */
            free(vaccinated_array_list);
        }
        else if(!strcmp(command_reader_buffer, "CMD_MOD"))
        {
            Administrator_request_package  update_information_package;     /* Variabile contenente le informazioni aggiornate dell'utente da modificare*/
            Administrator_response_package administrator_response_package; /* Variabile contenente le informazioni relative all'utente aggiornato */

            /*                       Administrator_request_package
             * |Server assistente|---------------------------------->|Server centrale|
             */
            if(FullRead(connection_file_descriptor, &update_information_package, sizeof(update_information_package)) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&client_address, "Client disconnected:", NULL);
                #endif

                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&client_address, "Request received from", command_reader_buffer);
            #endif

            /* Modifichiamo le informazioni utente */
            if(!change_information_in_file(&update_information_package, &administrator_response_package, VACCINATED_FILE_NAME))
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }

            /*                       Administrator_response_package
             * |Server assistente|<----------------------------------|Server centrale|
             */
            FullWrite(connection_file_descriptor, &administrator_response_package, sizeof(administrator_response_package));
            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif
        }
    }
}

/**
 * @brief Funzione handler per gestire il set di istruzioni da eseguire alla creazione del thread, successivamente all'@Accept() del server "assistente". Il server 
 *        in questione è stato ingegnerizzato ed implementato, per gestire molteplici tipologie di richieste. In particolare, vengono utilizzati dei comandi per permettere
 *        al server di interpretare la tipologia di richiesta ed eseguire un set predefinito di istruzioni. Di seguito viene mostrato l'elenco di comandi e le corrispettive 
 *        funzionalità:
 * 
 *        - CMD_REV: Richiesta ricevuta per fornire le informazioni del "Green pass" di uno specifico utente
 *        - CMD_LST: Richiesta ricevuta per fornire la lista degli utenti iscritti alla piattaforma "Green pass"
 *        - CMD_MOD: Richiesta ricevuta per modificare alcune informazioni inerenti ad uno specifico utente iscritto alla piattaforma "Green pass"
 * 
 * @return void* - tutte le funzioni handler hanno un particolare tipo di sintassi, tra cui un tipo di ritorno specificato come puntatore a void
 */
void* assistant_server_handler(void* args)
{
    /* Rende STDOUT non bufferizzato */
    (void)setvbuf(stdout, NULL, _IONBF, 0);

    /* ==========================
     * =       Variables        =
     * ==========================
     */

    int                connection_file_descriptor = ((Args*)args)->file_descriptor;   /* File descriptor del socket che si occuperà di gestire nuove connessioni al server */

    struct sockaddr_in client_address = *(((Args*)args)->endpoint);                   /* Struttura utile a rappresentare un Endpoint, in particolare l'indirizzo del client */

    char               command_reader_buffer[CMD_BUFFER_LEN];                         /* Buffer utile all'operazione di lettura del comando inviato sul file descriptor del 
                                                                                         socket */

    char               code_reader_buffer[21];                                        /* Variabile dichiarata per memorizzare il codice di tessera sanitaria di un utente 
                                                                                         iscritto alla piattaforma */

    while(1)
    {

        /* ==========================
         * =    ZEROING  ARRAYS     =
         * ==========================
         * */

        /* Azzeriamo i byte che compongono i seguenti array per evitare di avere valori raw all'interno di quest'ultimo */
        bzero(command_reader_buffer, CMD_BUFFER_LEN);
        bzero(code_reader_buffer, 21);

        /*
         * ==================================
         * =           FULL READ            =
         * ==================================
         * */
        /*                             card_code[21]
         * |Client revisore|----------------------------------->|Server Assistente|
         */
        if (FullRead(connection_file_descriptor, command_reader_buffer, CMD_BUFFER_LEN) > 0) {
            /*
             * ==================================
             * =         CLOSE  THREAD          =
             * ==================================
             * */

            /* Caso in cui il client si sia disconnesso */

            #ifdef LOG
            LogHostIPV4(&client_address, "Client disconnected:", NULL);
            #endif

            /* Chiusura del socket file descriptor connesso al client */
            close(connection_file_descriptor);

            /*
             * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
             * reperire l'informazione relativa al prossimo thread disponibile
             * */
            pthread_exit(NULL);
        }
        #ifdef LOG
        LogHostIPV4(&client_address, "Request received from", "Command request");
        #endif

        /*
         * ================================
         * =       COMMAND  COMPARE       =
         * ================================
         * */

        /* 
         * Il server "assistente" può gestire molteplici richiste, pertanto per effettuare una qualsiasi operazione, sarà necessario inviare un comando
         * per eseguire un set di istruzioni predefinite. Per implementare un menu di comandi, ci avvaliamo della funzione @strcmp() definita in @string.h 
         */
        if (!strcmp(command_reader_buffer, "CMD_REV")) {

            /*                             card_code[21]
             * |Client revisore|----------------------------------->|Server Assistente|
             */
            if (FullRead(connection_file_descriptor, code_reader_buffer, 21) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */

                #ifdef LOG
                LogHostIPV4(&client_address, "Client disconnected:", NULL);
                #endif

                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&client_address, "Request received from", command_reader_buffer);
            #endif

            /*
             * =========================
             * =       VARIABLES       =
             * =========================
             * */
            int                reviser_socket; /* File descriptor relativo alla nuova connessione con il server centrale */

            struct sockaddr_in central_server_address; /* Struttura @sockaddr_in relativa all'endpoint del server centrale*/

            const char         central_server_ip[] = "127.0.0.1"; /* IP relativo al server centrale */

            Reviser_package    reviser_package; /* Variabile di tipo @Reviser_package, dichiarata per ricevere in risposta dal server centrale le informarzioni sul "Green Pass" di 
                                                   un utente ed inviarla al client "revisore"*/

            /*
             * Successivamente andremo ad instaurare una connessione con il server centrale, una volta che quest'ultimo avrà servito il server assistente, la connessione
             * verrà chiusa. La connessione tra le due entità avverrà su un altro socket, creato esplicitamente per permettere una nuova comunicazione mentre il server assistente
             * rimane connesso con il corrispettivo client
             */

            /* ==========================
             * =    SOCKET CREATION     =
             * ==========================
             * */

            reviser_socket = SocketIPV4();

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
            central_server_address.sin_family = AF_INET;

            /*
             *
             * */
            if(inet_pton(AF_INET, central_server_ip, &central_server_address.sin_addr) <= 0)
            {
                /*  */
                fprintf(stderr, "inet_pton() error for %s\n", central_server_ip);
                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                close(reviser_socket);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }

            /*
             * Inizializziamo il campo porta della struttura "@sockaddr_in" del server attraverso il valore di ritorno della funzione
             * "@htons()" la quale accetterà come argomento un intero rappresentante la porta desiderata su cui il server deve rimanere
             * in ascolto. Le porte sono interi a 16 bit da 0 a 65535, raggruppate nel seguente modo:
             *      - da 0 a 1023, porte riservate ai processi root;
             *      - da 1024 a 49151, porte registrate;
             *      - da 49152 a 65535, porte effimere, per i client, ai quali non interessa scegliere una porta specifica.
             * Per il progetto si è deciso di utilizzare una porta registrata "6464"
             * */
            central_server_address.sin_port = htons(6464);

            /*
             * ==================================
             * =           CONNECTION           =
             * ==================================
             * */

            /* Eseguiamo una richiesta di Three way Handshake alla struttura "@sockaddr_in" del server precedentemente generata */
            ConnectIPV4(reviser_socket, &central_server_address);

            /*                                 CMD_REV
             * |Server Assistente|----------------------------------->|Server Centrale|
             */
            FullWrite(reviser_socket, command_reader_buffer, CMD_BUFFER_LEN);

            #ifdef LOG
            LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
            #endif

            /*                                 Card_code[21]
             * |Server Assistente|----------------------------------->|Server Centrale|
             */
            FullWrite(reviser_socket, code_reader_buffer, 21);

            #ifdef LOG
            LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
            #endif

            /*                               Reviser_package
             * |Server assistente|<----------------------------------|Server centrale|
             */
            if(FullRead(reviser_socket, &reviser_package, sizeof(reviser_package)) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Server disconnected:", NULL);
                #endif

                /* Chiusura dei socket file descriptor */
                close(reviser_socket);
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&central_server_address, "Response received from", command_reader_buffer);
            #endif

            /* Chiusura della connessione con il server centrale */
            close(reviser_socket);

            /*                             Reviser_package
             * |client revisore|<----------------------------------|Server assistente|
             */
            FullWrite(connection_file_descriptor, &reviser_package, sizeof(reviser_package));

            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif
        }
        else if(!strcmp(command_reader_buffer, "CMD_LST"))
        {
            /*
             * =========================
             * =       VARIABLES       =
             * =========================
             * */

            int                administrator_socket;               /* File descriptor relativo alla nuova connessione con il server centrale */

            struct sockaddr_in central_server_address;             /* Struttura @sockaddr_in relativa all'endpoint del server centrale */

            const char         central_server_ip[] = "127.0.0.1";  /* IP relativo al server centrale */

            int                size_codes_list;                    /* Lista degli utenti iscritti alla piattaforma "Green Pass" */

            char*              codes_list;                         /* Numero di utenti iscritti alla piattaforma "Green Pass" */

            /* ==========================
             * =    SOCKET CREATION     =
             * ==========================
             * */

            administrator_socket = SocketIPV4();

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
            central_server_address.sin_family = AF_INET;

            /*
             *
             * */
            if(inet_pton(AF_INET, central_server_ip, &central_server_address.sin_addr) <= 0)
            {
                /*  */
                fprintf(stderr, "inet_pton() error for %s\n", central_server_ip);
                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                close(administrator_socket);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }

            /*
             * Inizializziamo il campo porta della struttura "@sockaddr_in" del server attraverso il valore di ritorno della funzione
             * "@htons()" la quale accetterà come argomento un intero rappresentante la porta desiderata su cui il server deve rimanere
             * in ascolto. Le porte sono interi a 16 bit da 0 a 65535, raggruppate nel seguente modo:
             *      - da 0 a 1023, porte riservate ai processi root;
             *      - da 1024 a 49151, porte registrate;
             *      - da 49152 a 65535, porte effimere, per i client, ai quali non interessa scegliere una porta specifica.
             * Per il progetto si è deciso di utilizzare una porta registrata "6464"
             * */
            central_server_address.sin_port = htons(6464);

            /*
             * ==================================
             * =           CONNECTION           =
             * ==================================
             * */

            /* Eseguiamo una richiesta di Three way Handshake alla struttura "@sockaddr_in" del server precedentemente generata */
            ConnectIPV4(administrator_socket, &central_server_address);

            /*                                 CMD_LST
             * |server assistente|----------------------------------->|Server centrale|
             */
            FullWrite(administrator_socket, command_reader_buffer, CMD_BUFFER_LEN);
            #ifdef LOG
            LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
            #endif

            /*                                 size_codes_list
             * |server assistente|<----------------------------------|Server centrale|
             */
            if(FullRead(administrator_socket, &size_codes_list, sizeof(int)) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Server disconnected:", NULL);
                #endif
                
                /* Chiusura del socket file descriptor */
                close(administrator_socket);
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&central_server_address, "Response received from", command_reader_buffer);
            #endif

            /* Allochiamo dinamicamente spazio per l'array di utenti */
            if((codes_list = (char*)malloc(size_codes_list * 21)) == NULL)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Server disconnected:", NULL);
                #endif

                /* Liberiamo dalla memoria le variabili allocate dinamicamente */
                free(codes_list);

                /* Chiusura del socket file descriptor */
                close(administrator_socket);
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }

            /*                                 code_list
             * |server assistente|<----------------------------------|Server centrale|
             */
            if(FullRead(administrator_socket, codes_list, size_codes_list * 21) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Server disconnected:", NULL);
                #endif

                /* Liberiamo dalla memoria le variabili allocate dinamicamente */
                free(codes_list);

                /* Chiusura del socket file descriptor connesso al client */
                close(administrator_socket);
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&central_server_address, "Response received from", command_reader_buffer);
            #endif
            
            /* Chiudiamo la connessione con il server centrale */
            close(administrator_socket);

            int i;
            for(i = 0; i < size_codes_list; i++)
            {
                /* decriptiamo ogni elemento contenuto nella lista */
                xor_crypt(codes_list + (i * 21), 14);
            }

            /*                                 size_codes_list
             * |client Amministratore|<----------------------------------|Server assistente|
             */
            FullWrite(connection_file_descriptor, &size_codes_list, sizeof(int));

            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif

            /*                                     codes_list
             * |client Amministratore|<----------------------------------|Server assistente|
             */
            FullWrite(connection_file_descriptor, codes_list, size_codes_list * 21);

            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif

            /* Liberiamo la memoria dalle variabili allocate dinamicamente */
            free(codes_list);
        }
        else if(!strcmp(command_reader_buffer, "CMD_MOD"))
        {
            /*
             * =========================
             * =       VARIABLES       =
             * =========================
             * */

            int                            administrator_socket;              /* File descriptor relativo alla nuova connessione con il server centrale */

            struct sockaddr_in             central_server_address;            /* Struttura @sockaddr_in relativa all'endpoint del server centrale */

            const char                     central_server_ip[] = "127.0.0.1"; /* IP relativo al server centrale */

            Administrator_request_package  update_information_package;        /* Variabile contenente le informazioni aggiornate dell'utente da modificare*/

            Administrator_response_package response_from_server;              /* Variabile contenente le informazioni relative all'utente aggiornato */

            /*                           Administrator_request_package
             * |client Amministratore|---------------------------------->|Server assistente|
             */
            if(FullRead(connection_file_descriptor, &update_information_package, sizeof(update_information_package)) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&client_address, "Client disconnected:", NULL);
                #endif

                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&client_address, "Request received from", command_reader_buffer);
            #endif


            /* ==========================
             * =    SOCKET CREATION     =
             * ==========================
             * */

            administrator_socket = SocketIPV4();

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
            central_server_address.sin_family = AF_INET;

            /*
             *
             * */
            if(inet_pton(AF_INET, central_server_ip, &central_server_address.sin_addr) <= 0)
            {
                /*  */
                fprintf(stderr, "inet_pton() error for %s\n", central_server_ip);
                /* Chiusura del socket file descriptor connesso al client */
                close(connection_file_descriptor);
                close(administrator_socket);
                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }

            /*
             * Inizializziamo il campo porta della struttura "@sockaddr_in" del server attraverso il valore di ritorno della funzione
             * "@htons()" la quale accetterà come argomento un intero rappresentante la porta desiderata su cui il server deve rimanere
             * in ascolto. Le porte sono interi a 16 bit da 0 a 65535, raggruppate nel seguente modo:
             *      - da 0 a 1023, porte riservate ai processi root;
             *      - da 1024 a 49151, porte registrate;
             *      - da 49152 a 65535, porte effimere, per i client, ai quali non interessa scegliere una porta specifica.
             * Per il progetto si è deciso di utilizzare una porta registrata "6464"
             * */
            central_server_address.sin_port = htons(6464);

            /*
             * ==================================
             * =           CONNECTION           =
             * ==================================
             * */

            /* Eseguiamo una richiesta di Three way Handshake alla struttura "@sockaddr_in" del server precedentemente generata */
            ConnectIPV4(administrator_socket, &central_server_address);

            /*                                  CMD_MOD
             * |Server Assistente|---------------------------------->|Server centrale|
             */
            FullWrite(administrator_socket, command_reader_buffer, CMD_BUFFER_LEN);

            #ifdef LOG
            LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
            #endif

            /*                      Administrator_request_package
             * |Server Assistente|---------------------------------->|Server centrale|
             */
            FullWrite(administrator_socket, &update_information_package, sizeof(update_information_package));

            #ifdef LOG
            LogHostIPV4(&central_server_address, "Request sent to", command_reader_buffer);
            #endif

            /*                       Administrator_response_package
             * |Server assistente|<----------------------------------|Server centrale|
             */
            if(FullRead(administrator_socket, &response_from_server, sizeof(response_from_server)) > 0)
            {
                /*
                 * ==================================
                 * =         CLOSE  THREAD          =
                 * ==================================
                 * */

                /* Caso in cui il client si sia disconnesso */
                #ifdef LOG
                LogHostIPV4(&central_server_address, "Server disconnected:", NULL);
                #endif

                /* Chiusura del socket file descriptor */
                close(administrator_socket);
                close(connection_file_descriptor);

                /*
                 * Tale funzione ci permette di terminare il thread chiamante. Viene passato "@NULL" come argomento in quanto non si vuole
                 * reperire l'informazione relativa al prossimo thread disponibile
                 * */
                pthread_exit(NULL);
            }
            #ifdef LOG
            LogHostIPV4(&central_server_address, "Response received from", command_reader_buffer);
            #endif

            /* Chiudiamo la connessione con il server "centrale" */
            close(administrator_socket);

            /*                          Administrator_response_package
             * |Client amministratore|<----------------------------------|Server assistente|
             */
            FullWrite(connection_file_descriptor, &response_from_server, sizeof(response_from_server));

            #ifdef LOG
            LogHostIPV4(&client_address, "Response sent to", command_reader_buffer);
            #endif
        }
    }
}
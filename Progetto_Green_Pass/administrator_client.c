/**
 * @file    vaccinated_client.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   Il seguente programma ha lo scopo di realizzare un client, volto a permettere, ad un utente (amministratore), di collegarsi 
 *          ad un server, "assistente" e di comunicargli le modifiche da effettuare ad un particolare utente iscritto e già presente sulla piattaforma "Green Pass"
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
#include <unistd.h>
#include <string.h>
#include "lib/sockets_utility.h"
#include "lib/menu_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */
#define IP_ADDRESS "localhost" /* Il client "amministratore" ha lo scopo di collegarsi ad un server "assistente" e non ad altri server. Pertanto, attraverso la seguente
                                  costante, andiamo a definire l'indirizzo IP del server con cui andremo a connetterci */
#define SERVER_PORT 6465       /* Definiamo come costante la porta del server "assistente" in ascolto ed in attesa di accettare nuove connessioni */

int main()
{
    /* ==========================
     * =       Variables        =
     * ==========================
     */

    int                            client_file_descriptor;                /* File descriptor associato al socket del client */

    struct sockaddr_in             server_address;                        /* Struttura utile a rappresentare un Endpoint, in particolare l'indirizzo del server */

    struct hostent*                server_dns;                            /* Struttura contenente varie informazioni sull'host server,
                                                                             utile ad effettuare un'operazione DNS (Risoluzione diretta) */

    char                           command_writer_buffer[CMD_BUFFER_LEN]; /* Il server "centro vaccinale" può gestire molteplici richiste, pertanto
                                                                             per effettuare una qualsiasi operazione, sarà necessario inviare un comando
                                                                             per eseguire un set di istruzioni predefinite */

    Administrator_request_package  administrator_package;                 /* La seguente variabile viene dichiarata in quanto contiene i campi necessari per aggiornare 
                                                                             le informazioni di uno specifico utente iscritto alla piattaforma "Green Pass" */

    char*                          code_list;                             /* Lista degli utenti iscritti alla piattaforma "Green Pass" */

    int                            size_list;                             /* Numero di utenti iscritti alla piattaforma "Green Pass" */

    Administrator_response_package administrator_response_package;        /* Variabile dichiarata per memorizzare e visualizzare la risposta del server assistente con le informazioni
                                                                             aggiornate */

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
        exit(EXIT_FAILURE);
    }

    /*
     * ==================================
     * =        SOCKET CREATION         =
     * ==================================
     * */

    /*
    * Inizializziamo il valore del file descriptor sfruttando la funzione definita nella libreria "@sockets_utilities.h".
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
     * Per il progetto si è deciso di utilizzare una porta registrata "6463"
     * */
    server_address.sin_port = htons(6465);

    /*
     * ==================================
     * =           CONNECTION           =
     * ==================================
     * */

    /* Eseguiamo una richiesta di Three way Handshake alla struttura "@sockaddr_in" del server precedentemente generata */
    ConnectIPV4(client_file_descriptor, &server_address);

    /* Prepariamo il comando da inviare al server "assistente" */
    strcpy(command_writer_buffer, "CMD_LST");

    /*                                CMD_LST
     * |Client amministratore|--------------------------------->|Server assistente|
     */
    FullWrite(client_file_descriptor, command_writer_buffer, CMD_BUFFER_LEN);

    /*                                 size_codes_list
     * |client Amministratore|<----------------------------------|Server assistente|
     */
    if(FullRead(client_file_descriptor, &size_list, sizeof(int)) > 0)
    {
        fprintf(stderr,"Anomalia durante l'operazione del server\n");
        /* Chiusura del socket file descriptor connesso al server */
        close(client_file_descriptor);
        /* Terminiamo con successo il processo client */
        exit(EXIT_FAILURE);
    }

    /* Allochiamo dinamicamente spazio per l'array di utenti */
    if((code_list = (char*)malloc(size_list * 21)) == NULL)
    {
        fprintf(stderr,"Errore durante l'allocazione\n");
        /* Chiusura del socket file descriptor connesso al server */
        close(client_file_descriptor);
        /* Terminiamo con successo il processo client */
        exit(EXIT_FAILURE);
    }

    /*                                 code_list
     * |client Amministratore|<----------------------------------|Server assistente|
     */
    if(FullRead(client_file_descriptor, code_list, size_list * 21) > 0)
    {
        fprintf(stderr,"Errore di lettura\n");
        /* Chiusura del socket file descriptor connesso al server */
        close(client_file_descriptor);
        /* Terminiamo con successo il processo client */
        exit(EXIT_FAILURE);
    }

    /*
     * ====================
     * =       MENU       =
     * ====================
     * */

    /* 
     * Una volta reperita la lista relativa agli utenti iscritti alla piattaforma, l'amministratore dovrà selezionare quali degli utenti aggiornare e con quale motivazione.
     * La funzione @run_administrator_menu si occuperà di interfacciarsi conl'utente amministratore e di popolare un pacchetto di richiesta @administrator_package del tipo 
     * @Administrator_request_package
    */
    if(!run_administrator_menu(&administrator_package, code_list, size_list))
    {
        /* Chiusura del socket file descriptor connesso al server */
        close(client_file_descriptor);
        /* Terminiamo con successo il processo client */
        exit(EXIT_SUCCESS);
    }

    /* Azzeriamo il @command writer buffer */
    bzero(command_writer_buffer, CMD_BUFFER_LEN);

    /* Prepariamo il comando da inviare al servere assistente */
    strcpy(command_writer_buffer, "CMD_MOD");

    /*                                 command_writer_buffer
     * |client Amministratore|---------------------------------->|Server assistente|
     */
    FullWrite(client_file_descriptor, command_writer_buffer, CMD_BUFFER_LEN);

    /*                           Administrator_request_package
     * |client Amministratore|---------------------------------->|Server assistente|
     */
    FullWrite(client_file_descriptor, &administrator_package, sizeof(administrator_package));

    /*                       Administrator_response_package
     * |client Amministratore|<----------------------------------|Server assistente|
     */
    if(FullRead(client_file_descriptor, &administrator_response_package, sizeof(administrator_response_package)) > 0)
    {
        fprintf(stderr,"Errore di lettura\n");
        /* Chiusura del socket file descriptor connesso al server */
        close(client_file_descriptor);
        /* Terminiamo con successo il processo client */
        exit(EXIT_FAILURE);
    }

    /* Stampiamo i risultati della richiesta effettuata al server assistente*/
    if(administrator_response_package.reviser_package.file_flags.read_file_flag || administrator_response_package.reviser_package.file_flags.open_file_flag || administrator_response_package.reviser_package.file_flags.write_file_flag)
    {
        /* Caso di errore di lettura, scrittura o apertura file sul server centrale */
        fprintf(stderr,"Anomalia durante l'operazione del server\n");
    }
    else if(administrator_response_package.not_updatable)
    {
        /* Caso utente non aggiornabile */
        fprintf(stderr,"Impossibile aggiornare l'utente selezionato con i campi scelti\n");
    }
    else
    {
        /* Informazioni aggiornate con successo */
        printf("Le informazioni sono state aggiornate come segue:\n");
        /* Stampiamo i risultati ottenuti dalla richiesta */
        output_user_information(&administrator_response_package.reviser_package, 45);
    }


    /* Chiusura del socket file descriptor connesso al server */
    close(client_file_descriptor);
    /* Terminiamo con successo il processo client */
    exit(EXIT_SUCCESS);
}
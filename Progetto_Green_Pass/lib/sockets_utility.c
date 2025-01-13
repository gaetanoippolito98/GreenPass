/**
 * @file    sockets_utility.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare alcune funzioni che avvengono sui sockets, in modo da nascondere la complessità di implementazione ed
 *          evitare istruzioni ripetute.
 * 
 * @type    Implementazione libreria @sockets_utility.h
 * @version 1.0
 */

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "sockets_utility.h"
#include "date_utility.h"
#include "thread_utility.h"

/* 
 * ================================
 * =   Function Implementation    =
 * ================================
 */

/**
 * @brief Funzione che permette la creazione di un file descriptor associato ad una socket
 *
 * @param address_family Famiglia degli indirizzi utilizzati (IPv4 - IPv6 - ecc...)
 * @param transport_type Protocollo di trasporto utilizzato (TCP - UDP)
 * @param transport_subtype Sottotipo del protocollo di trasporto (solitamente configurato a 0)
 *
 * @return File descriptor associato alla socket creata
 * */
int Socket(int address_family, int transport_type, int transport_subtype)
{
    /* File descriptor da inizializzare al valore di ritorno della chiamata "@socket()" */
    int file_descriptor;

    /*
     * In caso di successo, la funzione "@socket()" ritornerà un valore maggiore o uguale di 0, ovvero il valore del
     * file descriptor associato alla socket creata. In caso di errore viene ritornato -1 ed errno (numero dell'ultimo errore)
     * viene configurato per indicare quest'ultimo
     * */
    if((file_descriptor = socket(address_family, transport_type, transport_subtype)) < 0)
    {
        /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
        perror("Creation socket error");

        /* Il seguente controllo rende la funzione thread safe, in quanto può essere eseguita sul main thread oppure su thread secondari */
        if(is_main_thread())
        {
            /* Il programma viene interrotto con un errore */
            exit(EXIT_FAILURE);
        }
        else
        {
            /* Terminiamo l'esecuzione del thread creato */
            pthread_exit(NULL);
        }
    }

    /* Ritorniamo il file descriptor inizializzato, precedentemente, con il valore di ritorno della chiamata "@socket()"*/
    return file_descriptor;
}

/**
 * @brief Funzione che permette la creazione di un file descriptor associato ad una socket con una famiglia di indirizzi
 *        IPv4 ed un protocollo trasporto TCP
 *
 * @return File descriptor associato al socket creato
 * */
int SocketIPV4(void)
{
    /*
     * Sfruttiamo la funzione "@Socket()" definita, precedentemente, nella stessa libreria, per ritornare un file descriptor associato
     * ad una socket
     * */
    return Socket(AF_INET, SOCK_STREAM, 0);
}

/**
 * @brief La seguente funzione ha lo scopo di asscoiare un Endpoint ad un file descriptor specifico "@file_descriptor_to_bind" e "@endpoint"
 *
 * @param file_descriptor_to_bind File descrpitor del socket da associare ad una struttura di tipo "@sockaddr_in"
 * @param endpoint Indirizzo da associare al socket "@file_descriptor_to_bind"
 * */
void BindIPV4(int file_descriptor_to_bind, struct sockaddr_in* endpoint)
{
    /*
     * Attraverso la funzione "@bind()" associamo un file descriptor a un endpoint di tipo "@sockaddr", in modo tale da ereditare
     * le caratteristiche dell'indirizzo rappresentato dalla struttura passata come secondo argomento. In particolare sfruttiamo
     * il valore di ritorno della "@bind()" per la gestione degli errori, in quanto, quest'ultima ritornerà 0 in caso di successo
     * oppure -1 in caso di errore, configurando errno (numero dell'ultimo errore) per indicare l'errore stesso
     * */
    if(bind(file_descriptor_to_bind, (struct sockaddr*)endpoint, sizeof(*endpoint)) < 0)
    {
        /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) la quale descrive la natura dell'errore */
        perror("Bind to endpoint error");
        /* Interrompiamo l'esecuzione del programma con un errore */
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief La funzione configura il "@file_descriptor", passato in input, in modalità ascolto, con una coda delle richieste pari a "@backlog_size"
 *
 * @param file_descriptor File descriptor del socket da configurare in modalità ascolto
 * @param backlog_size Coda delle connessioni in pendenza
 * */
void Listen(int file_descriptor, int backlog_size)
{
    /*
     * Se la dimensione della coda delle connessioni in pendenza è negativa, allora procediamo riassegnando la "@backlog_size"
     * con la dimensione di default prevista dalla libreria
     * */
    if(backlog_size < 0)
    {
        backlog_size = DEFAULT_BACKLOG_SIZE;
    }

    /*
     * Attraverso la funzione "@listen()" configuriamo il socket, identificato dal "@file_descriptor" passato in input, in modalità
     * ascolto. In questo modo il socket sarà pronto per accettare nuove connessioni. In particolare sfruttiamo
     * il valore di ritorno della "@listen()" per la gestione degli errori, in quanto, quest'ultima ritornerà 0 in caso di successo
     * oppure -1 in caso di errore, configurando errno (numero dell'ultimo errore) per indicare l'errore
     * */
    if(listen(file_descriptor, backlog_size) < 0)
    {
        /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
        perror("Error while configuring the socket in listening");
        /* Interrompiamo l'esecuzione del programma con un errore */
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief La seguente funzione permette di connettere due socket "@file_descriptor_to_connect" ed il socket associato alla struttura "@destination_endpoint"
 *
 * @param file_descriptor_to_connect File descrittore sorgente da connettere con il socket associato alla struttura "@destination_endpoint"
 * @param destination_endpoint Endopoint della destinazione da connettere con il file descrittore sorgente "@file_descriptor_to_connect"
 * */
void ConnectIPV4(int file_descriptor_to_connect, struct sockaddr_in* destination_endpoint)
{
    /*
     * Attraverso la seguente funzione effettuiamo una richiesta al server di una Three way Handshake, ovvero richiediamo una
     * connessione con il server. Inoltre, quest'ultima ritorna 0 in caso di successo, oppure -1 in caso di errore,
     * configurando errno (numero dell'ultimo errore) per indicare l'errore
     * */
    if(connect(file_descriptor_to_connect, (struct sockaddr*)destination_endpoint, sizeof(*destination_endpoint)) < 0)
    {
        /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
        perror("Connect IPv4 error");

        /* Il seguente controllo rende la funzione thread safe, in quanto può essere eseguita sul main thread oppure su thread secondari */
        if(is_main_thread())
        {
            /* Il programma viene interrotto con un errore */
            exit(EXIT_FAILURE);
        }
        else
        {
            /* Terminiamo l'esecuzione del thread creato */
            pthread_exit(NULL);
        }
    }
}

/**
 * @brief La funzione esegue la Three way Handshake con il client che ne fa espressamente richiesta
 *
 * @param listen_file_descriptor File descriptor configurato in modalità ascolto sul quale arriverà la nuova richiesta di connessione
 * @param client_address Parametro di output, utile ad identificare l'indirizzo del client
 * @param client_size Parametro di output utile a rappresentante la dimensione della struttura utilizzata per rappresentare il client
 *
 * @return File descriptor avente le stesse proprietà del "@listen_file_descriptor" utile a gestire la nuova connessione e a servire il client
 * */
int AcceptIPV4(int listen_file_descriptor, struct sockaddr_in* client_address, socklen_t* client_size)
{
    /* File descriptor da inizializzare al valore di ritorno della chiamata "@accept()" */
    int connection_file_descriptor;

    /*
     * Attraverso la seguente funzione procediamo con l'eseguire e finalizzare la procedura Three way Handshake. In questo modo
     * andiamo a stabilire una connessione con il client che ne ha fatto richiesta. Inoltre, sfruttiamo il secondo e il terzo parametro
     * per ritornare le informazioni inerenti al client. Ritornerà un valore maggiore o uguale di 0, ovvero il valore del
     * file descriptor associato alla socket. In caso di errore viene ritornato -1 ed errno (numero dell'ultimo errore)
     * viene configurato per indicare quest'ultimo
     * */
    if((connection_file_descriptor = accept(listen_file_descriptor, (struct sockaddr *)client_address, client_size)) < 0)
    {
        /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
        perror("Error while trying to accept a new connection");
        /* Interrompiamo l'esecuzione del programma con un errore */
        exit(EXIT_FAILURE);
    }

    /* Ritorniamo il file descriptor inizializzato, precedentemente, con il valore di ritorno della chiamata "@accept()"*/
    return connection_file_descriptor;
}

/**
 * @brief La funzione permette la lettura dei byte presenti in un "@buffer" anche in caso di interruzioni da parte di una
 *        System call. In questo modo, gestiamo le interruzioni permettendo alla "@read()" di leggere tutti i byte presenti
 *        nel "@buffer"
 *
 * @param file_descriptor File descriptor del socket su cui eseguire l'operazione di lettura attraverso una "@read()"
 * @param buffer Buffer contenente i byte da leggere
 * @param n_bytes Numero di bytes presenti nel "@buffer"
 *
 * @return Numero di byte rimanenti da leggere. Se il valore ritornato è maggiore di zero, la funzione indicherà al processo in esecuzione che il 
 *         corrispettivo client/server si è disconnesso inviando un EOF
 * */
size_t FullRead(int file_descriptor, void* buffer, size_t n_bytes)
{
    /* Bytes rimanenti da leggere */
    size_t n_left = n_bytes;
    /* Bytes letti */
    ssize_t n_read;

    /* Costrutto iterativo, che si ripeterà fino a quando ci saranno bytes da leggere */
    while(n_left > 0)
    {
        /*
         * La "@read()" in caso di successo ritorna il numero di byte letti, 0 in caso di "@EOF", e un valore minore di 0
         * in caso di errore, configurando errno (numero dell'ultimo errore) per indicare l'errore. Sfruttiamo tale valore
         * di ritorno per gestire gli errori e casi speciali
         * */
        if((n_read = read(file_descriptor, buffer, n_bytes)) < 0)
        {
            /*
             * Verifichiamo che l'errore configurato dalla "@read()" sia uguale ad "@EINTR". Attraverso la verifica di tale condizione
             * rileviamo l'interruzione di una System call
             * */
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                /* In un qualsiasi altro caso, usciremo con dal programma con un errore */
                perror("Reading error");
                /* Il seguente controllo rende la funzione thread safe, in quanto può essere eseguita sul main thread oppure su thread secondari */
                if(is_main_thread())
                {
                    /* Il programma viene interrotto con un errore */
                    exit(EXIT_FAILURE);
                }
                else
                {
                    /* Terminiamo l'esecuzione del thread creato */
                    pthread_exit(NULL);
                }
            }
        }
        else if(n_read == 0) /* Caso in cui viene raggiunto "@EOF" */
        {
            /* Se abbiamo raggiunto la fine del file in lettura, interrompiamo il ciclo */
            break;
        }

        /* Aggiorniamo i bytes rimanenti da leggere */
        n_left -= n_read;

        /* Aggiorniamo l'offset del buffer */
        buffer += n_read;
    }

    /* Ritorniamo in output il numero di byte rimanenti da leggere */
    return n_left;
}

/**
 * @brief La funzione permette la scrittura dei byte presenti in un "@buffer" anche in caso di interruzioni da parte di una
 *        System call. In questo modo, gestiamo le interruzioni permettendo alla "@write()" di scrivere tutti i byte presenti
 *        nel "@buffer"
 *
 * @param file_descriptor File descriptor del socket su cui eseguire l'operazione di scrittura attraverso una "@write()"
 * @param buffer Buffer contenente i byte da scrivere
 * @param n_bytes Numero di bytes presenti nel "@buffer"
 *
 * @return Numero di byte rimanenti da scrivere
 * */
size_t FullWrite(int file_descriptor, void* buffer, size_t n_bytes)
{
    /* Bytes rimanenti da scrivere */
    size_t n_left = n_bytes;
    /* Bytes scritti */
    ssize_t n_written;

    /* Costrutto iterativo, che si ripeterà fino a quando ci saranno bytes da scrivere */
    while(n_left > 0)
    {
        /*
         * La "@write()" in caso di successo ritorna il numero di byte scritti. In caso di errore, ritorna un valore minore di 0,
         * configurando errno (numero dell'ultimo errore) per indicare l'errore. Sfruttiamo tale valore
         * di ritorno per gestire gli errori e casi speciali
         * */
        if((n_written = write(file_descriptor, buffer, n_bytes)) < 0)
        {
            /*
             * Verifichiamo che l'errore configurato dalla "@write()" sia uguale ad "@EINTR". Attraverso la verifica di tale condizione
             * rileviamo l'interruzione di una System call
             * */
            if(errno == EINTR)
            {
                continue;
            }
            else
            {
                /* In un qualsiasi altro caso, usciremo con dal programma con un errore */
                perror("Writing error");
                /* Il seguente controllo rende la funzione thread safe, in quanto può essere eseguita sul main thread oppure su thread secondari */
                if(is_main_thread())
                {
                    /* Il programma viene interrotto con un errore */
                    exit(EXIT_FAILURE);
                }
                else
                {
                    /* Terminiamo l'esecuzione del thread creato */
                    pthread_exit(NULL);
                }
            }
        }

        /* Aggiorniamo i bytes rimanenti da scrivere */
        n_left -= n_written;

        /* Aggiorniamo l'offset del buffer */
        buffer += n_written;
    }

    /* Ritorniamo in output il numero di byte rimanenti da scrivere */
    return n_left;
}

/**
 * @brief I server implementati nel progetto fornito, sono stati ingegnerizzati per effettuare dei log di sistema ad ogni azione di rilievo eseguita. Per tal motivo
 *        la seguente funzione, è stata implementata per effettuare una stampa a video costruita seguendo il seguente pattern:
 *  
 *        (timestamp) (comando eseguito dal server) - (azione) host (indirizzo IP), port (porta)
 * 
 * @param client_address Indirizzo del client con cui, il processso in esecuzione, esegue una determinata azione @action
 * @param action Tipologia di azione eseguita durante una tipica comunicazione con architettura client/server 
 * @param command Comando inviato dal client al server, per eseguire un set di istruzioni predefinito. Tale parametro di input può essere espresso con valore @NULL, in quanto
 *                la stampa a video, verrà scritta sullo standard output dinamicamente, non stampandola, in base al valore passato. 
 */
void LogHostIPV4(struct sockaddr_in* client_address, char* action, char* command)
{
    /* Indirizzo ip del client con cui, il processso in esecuzione, esegue una determinata azione */
    char ip_address[INET6_ADDRSTRLEN];
    /* Data e orario in cui l'azione @action è avvenuta. Il valore della seguente variabile viene inizializzato attraverso la funzione @get_timestamp() definita in @date_utility.h*/
    char* timestamp = get_timestamp();

    /* 
     * Attraverso la seguente funzione, convertiamo l'indirizzo IP passato in input (secondo argomento) in formato network order e lo memorizziamo all'interno del buffer
     * @ip_address in formato dotted notation 
     * */
    if(inet_ntop(AF_INET, &(client_address->sin_addr), ip_address, INET6_ADDRSTRLEN) == NULL)
    {
        /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
        perror("IP network to presentation error (inet_ntop)");

        /* Il seguente controllo rende la funzione thread safe, in quanto può essere eseguita sul main thread oppure su thread secondari */
        if(is_main_thread())
        {
            /* Il programma viene interrotto con un errore */
            exit(EXIT_FAILURE);
        }
        else
        {
            /* Terminiamo l'esecuzione del thread creato */
            pthread_exit(NULL);
        }
    }

    /* Stampa formattata del log desiderato */
    printf("%s%s%s%s - %s host %s, port %d\n", (timestamp != NULL) ? timestamp : "",
                                                     (command != NULL) ? " (" : "",
                                                     (command != NULL) ? command : "",
                                                     (command != NULL) ? ")" : "",
                                                     action, ip_address, ntohs(client_address->sin_port));
}

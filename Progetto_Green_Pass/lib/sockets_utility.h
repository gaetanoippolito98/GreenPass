/**
 * @file    sockets_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare alcune funzioni che avvengono sui sockets, in modo da nascondere la complessità di implementazione ed
 *          evitare istruzioni ripetute.
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @sockets_utility.h */
#ifndef SOCKETS_UTILITY_H
#define SOCKETS_UTILITY_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <arpa/inet.h>

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */
#define CMD_BUFFER_LEN        8   /* Costante volta a definire la dimensione massima dei comandi inviati */
#define DEFAULT_BACKLOG_SIZE 50   /* Costante volta a definire la dimensione di default massima delle backlog dei servers definiti nel sistema GreenPass */

/* 
 * ================================
 * =     Function Definition      =
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
int Socket(int, int, int);

/**
 * @brief Funzione che permette la creazione di un file descriptor associato ad una socket con una famiglia di indirizzi
 *        IPv4 ed un protocollo trasporto TCP
 *
 * @return File descriptor associato al socket creato
 * */
int SocketIPV4(void);

/**
 * @brief Questa funzione serve ad asscoiare un Endpoint ad un file descriptor specifico "@file_descriptor_to_bind" e "@endpoint"
 *
 * @param file_descriptor_to_bind File descrpitor del socket da associare ad una struttura di tipo "@sockaddr_in"
 * @param endpoint Indirizzo da associare al socket "@file_descriptor_to_bind"
 * */
void BindIPV4(int, struct sockaddr_in*);

/**
 * @brief La funzione configura "@file_descriptor", passato in input, in ascolto con una coda delle richieste pari a "@backlog_size"
 *
 * @param file_descriptor File descriptor del socket da configurare in modalità ascolto
 * @param backlog_size Coda delle connessioni in pendenza
 * */
void Listen(int, int);

/**
 * @brief Tale funzione permette di connettere due socket "@file_descriptor_to_connect" ed il socket associato alla struttura "@destination_endpoint"
 *
 * @param file_descriptor_to_connect File descrittore sorgente da connettere con il socket associato alla struttura "@destination_endpoint"
 * @param destination_endpoint Endopoint della destinazione da connettere con il file descrittore sorgente "@file_descriptor_to_connect"
 * */
void ConnectIPV4(int, struct sockaddr_in*);

/**
 * @brief La funzione esegue la Three way Handshake con il client che ne fa espressamente richiesta
 *
 * @param listen_file_descriptor File descriptor configurato in modalità ascolto sul quale arriverà la nuova richiesta di connessione
 * @param client_address Parametro di output, utile ad identificare l'indirizzo del client
 * @param client_size Parametro di output rappresentante la dimensione della struttura utilizzata per rappresentare il client
 *
 * @return File descriptor avente le stesse proprietà del "@listen_file_descriptor" utile a gestire la nuova connessione e a servire il client
 * */
int AcceptIPV4(int, struct sockaddr_in*, socklen_t*);

/**
 * @brief La funzione permette la lettura dei byte presenti in un "@buffer" anche in caso di interruzioni da parte di una
 *        System call. In questo modo, gestiamo le interruzioni permettendo alla "@read()" di leggere tutti i byte presenti
 *        nel "@buffer"
 *
 * @param file_descriptor File descriptor del socket su cui eseguire l'operazione di lettura attraverso una "@read()"
 * @param buffer Buffer contenente i byte da leggere
 * @param n_bytes Numero di bytes presenti nel "@buffer"
 *
 * @return Numero di byte rimanenti da leggere
 * */
size_t FullRead(int, void*, size_t);

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
size_t FullWrite(int, void*, size_t);

/**
 * @brief I server implementati nel progetto fornito, sono stati ingegnerizzati per effettuare dei log di sistema ad ogni azione di rilievo eseguita. Per tal motivo
 *        la seguente funzione, è stata implementata per effettuare una stampa a video costruita seguendo il seguente pattern:
 *  
 *        (timestamp) (comando eseguito dal server) - (motivazione) host (indirizzo IP), port (porta)
 * 
 * @param client_address Indirizzo del client con cui, il processso in esecuzione, esegue una determinata azione @action
 * @param action Tipologia di azione eseguita durante una tipica comunicazione con architettura client/server 
 * @param command Comando inviato dal client al server, per eseguire un set di istruzioni predefinito. Tale parametro di input può essere espresso con valore @NULL, in quanto
 *                la stampa a video, verrà scritta sullo standard output dinamicamente, non stampandola, in base al valore passato. 
 */
void LogHostIPV4(struct sockaddr_in *, char *, char *);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @sockets_utility.h */
#endif // SOCKETS_UTILITY_H

/**
 * @file    thread_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   I server sono stati ingegnerizzati ed implementati sfruttando la logica dei thread. Tale scelta è stata presa, prendendo in considerazione la sicurezza del servizio
 *          offerto, in quanto, senza thread un client malintenzionato potrebbe comunicare uno stream di byte che comporterebbe il blocco del server e la conseguente 
 *          interruzione del servizio. Invece, attraverso l'utilizzo dei thread, un client malintenzionato potrebbe bloccare l'esecuzione di un thread, permettendo agli altri 
 *          client di essere serviti in qualsiasi caso. In tale libreria si è proceduto con il definire il set di istruzioni che ogni server deve eseguire in corrispondenza
 *          della creazione di un thread quando una nuova connessione viene accettata. Pertanto, vengono definite le funzioni handler di ogni server.
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @thread_utility.h */
#ifndef THREAD_UTILITY_H
#define THREAD_UTILITY_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <pthread.h>
#include "bool_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */
#define MAX_THREADS 64 /* Costante definita per indicare il numero massimo di thread da utilizzare */

/* 
 * ================================
 * =      Type Definition         =
 * ================================
 */

/**
 * @brief Struttura definita per permettere alle funzioni handler dei thread, di accettare in input molteplici argomenti. In particolare, vi troviamo una strttura @sockaddr_in
 *        per permettere alle funzioni handler di eseguire i log delle istruzioni rilevanti eseguite. Inoltre viene definito come campo un file descriptor, il quale ci permetterà 
 *        di eseguire operazioni di comunicazione sul socket aperto durante l'accept del server
 * 
 * @property file_descriptor - descrittore del socket creato durante l'accept del server corrispondente
 * @property endpoint - struttura sockaddr_in del client, popolata durante l'esecuzione della funzione @Accept() da parte del server
 */
typedef struct
{
    int file_descriptor;
    struct sockaddr_in* endpoint;
} Args;

/* 
 * ================================
 * =     Function Definition      =
 * ================================
 */

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
void* vaccination_center_handler(void*);

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
void* central_server_handler(void*);

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
void* assistant_server_handler(void*);

/**
 * @brief Le funzioni di creazione socket, connessione, scrittura e lettura su socket possono avvenire sul thread master, oppure, su un thread differente. Pertanto si è deciso 
 *        di rendere tali funzioni thread safe, attraverso la seguente funzione, il quale ci permetterà di comprendere se la funzione viene eseguita sul thread principale oppure 
 *        su un thread secondario, prevedendo un set di istruzioni divverso in base all'ambiente di esecuzione.
 * 
 * @return true - il processo è in esecuzione sul thread master
 * @return false - il processo non è in esecuzione sul thread master
 */
bool is_main_thread(void);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @thread_utility.h */
#endif // THREAD_UTILITY_H

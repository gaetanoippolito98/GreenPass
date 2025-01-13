/**
 * @file    package_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   Il seguente header file, viene utilizzato per definire i pacchetti di livello applicazione che verranno comunicati sulla rete tra client/server
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @package_utility.h */
#ifndef PACKAGE_UTILITY_H
#define PACKAGE_UTILITY_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <time.h>
#include <netinet/in.h>
#include "bool_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */

#define MAX_CODE_LEN 21 /* Lunghezza massima, espressa in termini di caratteri, del codice di tessera sanitaria */

/* 
 * ================================
 * =      Type Definition         =
 * ================================
 */

/**
 * @brief    Struttura utile a rappresentare il pacchetto da comunicare al "server centro vaccinale", per procedere alla sottoscrizione del sistema "Green pass" ed ottenere il
 *           documento di valità.
 * 
 * @property card_code - campo utile a contenere il codice di tessera sanitaria, formata da 20 caratteri decimali. Viene aggiunto un carattere in più
 *           per memorizzare il carattere di terminazione stringa.
 *
 * @property vaccination_date - Data di vaccinazione dell'utente, memorizzata attraverso una struttura struct tm, definita in @time.h
 * 
 * @struct   <--------------------Vaccinated_package------------------------>
 *           <-------- card_code --------> <-------- vaccination date ------>
 *           ________________________________________________________________
 *          |     21 byte (char[21])     |       56 byte (struct tm)        | 
 *          ----------------------------------------------------------------
 */
typedef struct
{
    char card_code[MAX_CODE_LEN];
    struct tm vaccination_date;
} Vaccinated_package;

/**
 * @brief Struttura utile a rappresentare le informazioni relative alla vaccinazione di un utente
 * 
 * @property vaccinated_package informazioni relative al codice di tessera sanitaria e data di vaccinazione dell'utente
 * @property expiration_date informazione relativa alla data di scadenza del Green Pass dell'utente
 * 
 * @struct   <------------------------------Subscribe_package----------------------------->
 *           <------------ vaccinated_package -----------> <------- expiration_date ------->
 *           ______________________________________________________________________________
 *          |     77 byte (struct Vaccinated_package)     |       56 byte (struct tm)     |
 *          ------------------------------------------------------------------------------
 */
typedef struct
{
    Vaccinated_package vaccinated_package;
    struct tm expiration_date;
} Subscribe_package;

/**
 * @brief Struttura utile a rappresentare e ad indicare la presenza di errori sul server durante le operazioni di lettura, scrittura e apertura file
 * 
 * @property open_file_flag - Valore booleano utile a verificare la presenza di errore durante l'operazione di apertura file
 * @property write_file_flag - Valore booleano utile a verificare la presenza di errore durante l'operazione di scrittura file
 * @property read_file_flag - Valore booleano utile a verificare la presenza di errore durante l'operazione di lettura file
 * 
 * @struct   <------------------------------------File_flags----------------------------------->
 *           <----- open_file_flag -----> <---- write_file_flag ----> <----- open_file_flag ----->
 *           _____________________________________________________________________________________
 *          |        4 byte (int)        |       4 byte (int)        |       4 byte (int)        |
 *          -------------------------------------------------------------------------------------
 */
typedef struct
{
    bool open_file_flag;
    bool write_file_flag;
    bool read_file_flag;
} File_flags;

/**
 * @brief    Struttura utile a rappresentare la response del server "centrale"/"centro vaccinale" al client "vaccinato". In tale pacchetto, previsto dal protocollo a 
 *           livello applicazione, sono contenute le informazioni inerenti all'esito della sottoscrizione al sistema "Green pass" e ad errori che possono avvenire durante 
 *           tale processo. In questo modo, il client può essere notificato dell'avvenuta iscrizione e di eventuali errori che possono intercorrere durante la registrazione 
 *           al sistema.
 * 
 * @property result_flag - tale valore booleano è utile ad interpretare l'esito dell'avvenuta registrazione presso il sistema "Green pass" 
 * @property file_flag - struttura utile a rappresentare gli errori che possono intercorrere durante la registrazione al sistema. In particolare, abbiamo progettato 
 *           il sistema "Green pass", in modo che salvi permanentemente le informazioni all'interno di un file organizzato in campi, con analogia alle tabelle di un 
 *           database relazionale. Per tal motivo si è deciso di notificare il client sui possibili errori, che possono avvenire durante l'apertura, la scrittura e 
 *           la lettura su fiel
 * 
 * @struct   <------------------------------File_result----------------------------->
 *           <--------- result_flag ---------> <------------ file_flags ------------>
 *           ________________________________________________________________________
 *          |          4 byte (int)          |       12 byte (struct File_flags)     |
 *          --------------------------------------------------------------------------
 */
typedef struct
{
    bool result_flag;
    File_flags file_flags;
} File_result;

/**
 * @brief Il nostro protocollo livello applicazione prevede che un utente "revisore" una volta inviata la richiest al client "assistente" riceva da quest'ultimo un pacchetto in 
 * response contenente diverse proprietà che indicano lo stato e la validità del documento "Green Pass" Corrispondente al codice di tessera sanitaria dell'utente
 * 
 * @property is_green_pass_valid - variabile booleana utile ad indicare visivamente la validità del "Green Pass"
 * @property expiration_date - struct @tm utile a rappresentare la data di scadenza del "Green Pass"
 * @property last_update - struct @tm utile a rappresentare la data dell'ultimo aggiornamento di stato del documento "Green Pass"
 * @property motivation - Stringa indicante la motivazione (vaccinazione, covid o guarigione) dell'ultimo aggiornamento di stato del documento "Green Pass"
 * @property file_flags - struct @File_flags utile a rappresentare e ad indicare la presenza di errori sul server durante le operazioni di lettura, scrittura e apertura file
 * 
 * @struct   <-------------------------------Reviser_package------------------------------->
 *           <-------- is_green_pass_valid --------> <-------------- expiration_date -------------->
 *           ______________________________________________________________________________________
 *          |              4 byte (int)             |              56 byte (struct tm)             |
 *          ---------------------------------------------------------------------------------------
 *           <------ last_update ------> <------ motivation ------> <--------- file_flags --------->
 *           ______________________________________________________________________________________
 *          |   56 byte (struct tm)    |    13 byte (char[13])    |  12 byte (struct File_flags)   |
 *          ---------------------------------------------------------------------------------------
 */
typedef struct
{
    bool is_green_pass_valid;
    struct tm expiration_date;
    struct tm last_update;
    char motivation[13];
    File_flags file_flags;
} Reviser_package;

/**
 * @brief Il nostro protocollo livello applicazione prevede che il client "amministratore" invii al server "assistente" un pacchetto contenente l'indice di riga corrispondente 
 *        al codice di tessera sanitaria presente nel file ed una motivazione per la modifica che si vuole applicare (covid o guarigione). 
 * 
 * @property index_list - indice di riga corrispondente al codice di tessera sanitaria presente nel file
 * @property motivation - Motivazione per la modifica che si vuole applicare (covid o guarigione)
 * 
  * @struct   <----------------Administrator_request_package--------------->
 *           <--------- index_list ---------> <-------- motivation -------->
 *           ________________________________________________________________
 *          |          4 byte (int)          |       13 byte (char[13])     |
 *          ----------------------------------------------------------------
 */
typedef struct
{
    int index_list;
    char motivation[13];
} Administrator_request_package;

/**
 * @brief Struttura utile a rappresentare la risposta da consegnare al client amministratore contenente le informazioni relative alla modifica del
 *        documento Green Pass
 * 
 * @property not_updatable Flag utile ad identificare se le informazioni utente sono modificabili
 * @property code codice di tessera sanitaria dell'utente modificato
 * @property reviser_package informazioni relative alla modifica del documento Green Pass
 * 
 * @struct   <----------Administrator_response_package---------->
 *           <----- not_updatable ----> <-------- code ---------->
 *           _____________________________________________________
 *          |       4 byte (int)       |    21 byte (char[21])    |
 *          -------------------------------------------------------
 *           <----------------- reviser_package ----------------->
 *           _____________________________________________________
 *          |   176 byte (struct Administrator_response_package)  |
 *          ------------------------------------------------------
 */
typedef struct
{
    bool not_updatable;
    char code[MAX_CODE_LEN];
    Reviser_package reviser_package;
} Administrator_response_package;

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @package_utility.h */
#endif //PACKAGE_UTILITY_H

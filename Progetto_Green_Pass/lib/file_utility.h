/**
 * @file file_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare funzioni legate alla gestione dei file
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @file_utility.h */
#ifndef FILE_UTILITY_H
#define FILE_UTILITY_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include "bool_utility.h"
#include "package_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */
#define REGION_FILE_NAME     "../file/client/tessera_sanitaria_codici_regione"
#define VACCINATED_FILE_NAME "../file/server/vaccinated"
#define MAX_FILE_LINE_SIZE 56

/* 
 * ================================
 * =     Function Definition      =
 * ================================
 */

/**
 * @brief Tale funzione ha lo scopo di verificare che una data stringa sia presente in un determinato file
 * 
 * @param file_name File in cui si vuole verificare la presenza della stringa passata in input @code
 * @param code Stringa di cui si vuole verificare la presenza nel file passato in input @file_name
 * @param buffer_size Durante verifica della presenza della stringa nel file, viene effettuata un'operazione di lettura. Pertanto, sarà necessario utilizzare
 *                    un buffer per memorizzare i caratteri letti. Attraverso tale parametro di input, sarà possibile decide la dimensione del buffer di lettura
 * 
 * @return File_result struttura contenente informazioni relative agli errori di apertura, lettura e scrittura sul file, con l'aggiunta di un valore booleano che ha lo scopo
 *         di interpretare la presenza di una determinata stringa in un file.
 */
File_result is_code_written_in_file(char*, char*, int);

/**
 * @brief Funzione utile a memorizzare un utente vaccinato nel file @../file/server/vaccinated
 * 
 * @param vaccinated_client_info Stringa corrispondente alle informazioni dell'utente vaccinato
 * 
 * @return struttura contenente informazioni relative agli errori di apertura, lettura e scrittura sul file, con l'aggiunta di un valore booleano che ha lo scopo
 *         di interpretare la presenza di una determinata stringa in un file.  
 */
File_result subscribe_vaccinated_client(char*);

/**
 * @brief La seguente funzione ha lo scopo di reperire la lista degli utenti iscritti alla piattaforma "Green Pass"
 * 
 * @param file_name Nome del file dal quale si vuole reperire la lista di utenti
 * @param list_codes Parametro di output utile a memorizzare la lista di utenti
 * 
 * @return int Il numero di utenti iscritti alla piattaforma
 */
int get_file_list(char*, char**);

/**
 * @brief La seguente funzione ha lo scopo di sostituire le vecchie informazioni relative ad un utente con le nuove informazioni passate in input
 * 
 * @param update_package struct @Administrator_request_package utile a rappresentare le informazioni che si vogliono aggiornare nel @file_name indicato, alla riga corrispondente
 *                       all'utente
 * @param administrator_response Parametro di output, utile a rappresentare le modifiche effettuate nel @file_name indicato, alla riga corrispondente a quella indicata 
 *                               nell' @update_package nel file
 *    
 * @param file_name stringa corrispondente al nome del file che si vuole aggiornare con le nuove informazioni
 * 
 * @return true Se l'operazione di aggiornamento è andata a buon fine
 * @return false Se l'operazione di aggiornamento non è andata a buon fine
 */
bool change_information_in_file(Administrator_request_package*, Administrator_response_package*, char*);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @file_utility.h */
#endif //FILE_UTILITY_H

/**
 * @file code_verification.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare funzioni legate al controllo e verifica dei codici di tessera sanitaria inviati dall'utente attraverso il client
 *          vaccinato al sistema "Green Pass"
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @code_verification.h */
#ifndef CODE_VERIFICATION_H
#define CODE_VERIFICATION_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include "bool_utility.h"
#include "package_utility.h"

/* 
 * ================================
 * =     Function Definition      =
 * ================================
 */

/**
 * @brief La seguente funzione ha lo scopo di verificare la validità del codice di tessera sanitaria inserito dall'utente. In questa funzione, il codice 
 *        di tessera sanitaria viene suddiviso in tre sottostringhe ed effettuati i seguenti controlli per verificarne la validità:
 * 
 *        1. Codice di tessera sanitaria composto da soli valori numerici
 *        2. Codice di tessera sanitaria di dimensione 20
 *        3. Primi 5 caratteri del codice di tessera sanitaria equivalenti a 80380
 *        4. Successivi 5 caratteri del codice di tessera sanitaria equivalenti ad uno dei codici presenti nel file @../file/client/tessera_sanitaria_codice_regione
 * 
 * @param code Codice di tessera sanitaria di cui si vuole verificare la validità
 * 
 * @return true Se il codice di tessera sanitaria è valido 
 * @return false Se il codice di tessera sanitaria non è valido
 */
bool verify_card_code(char*);

/**
 * @brief La seguente funzione ha lo scopo di verificare la validità del codice regione del numero di tessera sanitaria passato in input
 * 
 * @param code Numero di tessera sanitaria di cui si vuole verificare la validità del codice regione
 * 
 * @return File_result struttura contenente informazioni relative agli errori di apertura, lettura e scrittura sul file, con l'aggiunta di un valore booleano che ha lo scopo
 *         di interpretare la presenza di una determinata stringa in un file.
 */
File_result check_region_code(char*);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @code_verification.h */
#endif //CODE_VERIFICATION_H

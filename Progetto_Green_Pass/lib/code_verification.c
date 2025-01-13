/**
 * @file code_verification.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare funzioni legate al controllo e verifica dei codici di tessera sanitaria inviati dall'utente attraverso il client
 *          vaccinato al sistema "Green Pass"
 * 
 * @type    Implementazione libreria @code_verification.h
 * @version 1.0
 */

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <stdio.h>
#include <string.h>
#include "digits_utility.h"
#include "code_verification.h"
#include "file_utility.h"

/* 
 * ================================
 * =   Function Implementation    =
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
bool verify_card_code(char* code)
{
    /* Calcoliamo la lunghezza del codice tessera sanitaria passato in input */
    size_t code_len = strlen(code);

    /* Dato che il codice di tessera sanitaria è composto da soli valori numerici, andiamo a controllare che il codice inserito verifichi tale condizione */
    if(is_digits_only(code))
    {
        /* Un codice di tessera sanitaria è composto da 20 cifre, pertanto controlleremo che @code_len non sia minore o maggiore di tale dimensione */
        if (code_len < (MAX_CODE_LEN - 1) || code_len > (MAX_CODE_LEN - 1))
        {
            /* Caso in cui la lunghezza del codice sia piu lunga o più corta*/
            fprintf(stderr,"Lunghezza codice errata\n");
            return FALSE;
        }
        else
        {
            /* Caso in cui la lughezza sia giusta*/
            
            /**
             * Il codice di tessera sanitaria gode del seguente pattern:
             * 
             *                   xxxxx-xxxxx-xxxxxxxxxx
             * 
             * 1. I primi 5 caratteri, equivalenti a 80380, riguardano l'istituzione sanitaria specifica, pertanto, tale codice sarà uguale per tutti;
             * 2. I successivi 5 caratteri indicano il codice della regione, contenuto in un file nella folder @../file/client/tessera_sanitaria_codice_regione;
             * 3. I successivi 10 caratteri sono casuali
             */

            /* Variabile dichiarata allo scopo di verificare la presenza del codice regione nel file @../file/client/tessera_sanitaria_codice_regione */
            File_result file_result;

            /* Verifichiamo che l'apertura del file @../file/client/tessera_sanitaria_codice_regione non restituisca un errore */
            if((file_result = check_region_code(code)).file_flags.open_file_flag)
            {
                /* Caso in cui ci sia stato un errore durante l'apertura del file contenente la lista dei codici regione */
                return FALSE;
            }

            /* 
             * Verifichiamo la presenza del codice regione del codice tessera sanitaria, nel file @../file/client/tessera_sanitaria_codice_regione e che il codice
             * di istituzione sanitaria corrisponda ad 80380 
             */
            if(!strncmp(code, "80380", 5) && file_result.result_flag)
            {
                /* Caso in cui i primi 5 caratteri o i successivi 5 caratteri siano validi */
                return TRUE;
            }
            else
            {
                /* Caso in cui i primi 5 caratteri o i successivi 5 caratteri non siano validi */
                fprintf(stderr, "Codice di verifica non valido\n");
                return FALSE;
            }
        }
    }

    /* Caso in cui il codice inserito non sia composto da soli valori numerici */
    fprintf(stderr, "Il codice inserito non è composto da soli numeri\n");
    return FALSE;
}

/**
 * @brief La seguente funzione ha lo scopo di verificare la validità del codice regione del numero di tessera sanitaria passato in input
 * 
 * @param code Numero di tessera sanitaria di cui si vuole verificare la validità del codice regione
 * 
 * @return File_result struttura contenente informazioni relative agli errori di apertura, lettura e scrittura sul file, con l'aggiunta di un valore booleano che ha lo scopo
 *         di interpretare la presenza di una determinata stringa in un file.
 */
File_result check_region_code(char *code)
{
    /* Dichiariamo la stringa che si occuperà di memorizzare i caratteri del codice tessera sanitaria passato in input, corrispondenti al codice regione */
    char sub_code[6];

    /* Copiamo all'interno di @sub_code i 5 caratteri del codice regione */
    strncpy(sub_code, code + 5, 5);
    *(sub_code + 5) = '\0';

    /* Verifichiamo che il codice regione ricavato sia presente all'interno del file @../file/client/tessera_sanitaria_codice_regione */
    return is_code_written_in_file(REGION_FILE_NAME, sub_code, sizeof(sub_code));
}
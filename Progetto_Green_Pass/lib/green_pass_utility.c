/**
 * @file    green_pass_utility.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare alcune funzioni di utilità relative alla documentazione "Green Pass"
 * 
 * @type    Implementazione libreria
 * @version 1.0
 */

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <stdio.h>
#include <string.h>
#include "green_pass_utility.h"
#include "file_utility.h"
#include "date_utility.h"

/* 
 * ================================
 * =   Function Implementation    =
 * ================================
 */

/**
 * @brief La seguente funzione ha lo scopo di reperire le informazioni "Green Pass" relative ad un utente iscritto
 * 
 * @param reviser_package Parametro di output utile a popolare la struttura @Reviser_package relativamente alle informazioni corrispondenti all'utente iscritto desiderato 
 * @param card_code Codice di tessera sanitaria relativo all'utente iscritto di cui si vogliono reperire le informazioni "Green Pass"
 * 
 * @return File_flags* struttura contenente informazioni relative agli errori di apertura, lettura e scrittura sul file, @NULL in caso non vi siano errori
 */
File_flags* generate_reviser_response(Reviser_package* reviser_package, char* card_code)
{
    /* Dichiariamo un puntatore a file per finalizzare l'operazione di apertura del file */
    FILE* vaccinated_file;
    /* Dichiariamo un stringa per la lettura del codice di tessera sanitaria */
    char temp_card_code[21];
    /* 
     * Per controllare la validità del "Green Pass" è necessario confrontare la data di scadenza con data ed ora locale, pertanto tali variabili sono state dichiarate per 
     * memorizzare tale tipo di dato
    */
    struct tm* daytime;
    time_t time_stamp = time(NULL);

    /* Apriamo il file e controlliamo se il file è stato aperto correttamente */
    if((vaccinated_file = fopen(VACCINATED_FILE_NAME, "r")) == NULL)
    {
        /* Caso in cui il file non è stato aperto correttamente */

        /*
         * Popoliamo la struttura @File_flags con i seguenti valori:
         * - @open_file_flag  = 1 - in quanto vi sono stati errori durante l'apertura del file
         * - @write_file_flag = 0 - in quanto non vi sono stati errori durante un'operazione di scrittura del file
         * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
         */
        reviser_package->file_flags.open_file_flag = 1;
        reviser_package->file_flags.write_file_flag = 0;
        reviser_package->file_flags.read_file_flag = 0;

        /* Stampiamo l'errore */
        fprintf(stderr, "Errore nell'apertura del file!\n");

        /* Ritorniamo l'indirizzo della struttura popolata */
        return &(reviser_package->file_flags);
    }

    /* Popoliamo @temp_card_code ed il parametro di output con i valori della riga */
    while(fscanf(vaccinated_file, "%s %d/%d/%d %s %d/%d/%d\n", temp_card_code, &(reviser_package->expiration_date.tm_mday),
                                                                           &(reviser_package->expiration_date.tm_mon), &(reviser_package->expiration_date.tm_year),
                                                                           reviser_package->motivation, &(reviser_package->last_update.tm_mday),
                                                                           &(reviser_package->last_update.tm_mon), &(reviser_package->last_update.tm_year)) != EOF)
    {
        /* Eseguiamo un costrutto di controllo per confrontare @temp_card_code e @card_code */
        if(!strcmp(temp_card_code, card_code))
        {
            /* Caso utente trovato */

            /* Recuperiamo le informazioni relative alla data di scadenza ed ultimo aggiornamento, rendendole conformi alle regole imposte dalla struct @tm */
            reviser_package->expiration_date.tm_mon -= 1;
            reviser_package->expiration_date.tm_year -= 1900;
            reviser_package->last_update.tm_mon -= 1;
            reviser_package->last_update.tm_year -= 1900;

            /* Utente trovato, pertanto possiamo uscire dal ciclo */
            break;
        }

        /* Considerato che ad ogni iterazione popoliamo @reviser_package puliamo il corrispettivo spazio di memoria */
        bzero(reviser_package, sizeof(*reviser_package));
    }

    /*
     * Costruiamo una struttura del tipo struct @tm attraverso la funzione @localtime(), la quale accetterà in input @time_stamp
     * precedentemente inizializzata. La funzione @localtime() potrebbe tornare un puntatore nullo @NULL, pertanto eseguiamo un
     * costrutto di controllo per gestire tale eccezione
     */
    if((daytime = localtime(&time_stamp)) == NULL)
    {
        return NULL;
    }

    /* Inizializziamo il campo di validità del "Green Pass" attraverso la funzione @check_green_pass_validity() */
    reviser_package->is_green_pass_valid = check_green_pass_validity(&reviser_package->expiration_date, daytime, reviser_package->motivation);

    /* Chiusura del file */
    fclose(vaccinated_file);
    /* Ritorniamo Null in quanto non sono stati trovati errori */
    return NULL;
}

/**
 * @brief La seguente funzione ha lo scopo di verificare la validità del "Green Pass" di un determinato utente iscritto alla piattaforma 
 * 
 * @param expiration_date Data di scadenza del "Green Pass"
 * @param today_date  Data ed ora locale
 * @param motivation Motivazione dell'ultimo aggiornamento dell'utente sulla piattaforma (Vaccinazione, covid, guarigione)
 * 
 * @return true Se il "Green Pass" è valido
 * @return false  Se il "Green Pass" non è valido 
 */
bool check_green_pass_validity(struct tm* expiration_date, struct tm* today_date, char* motivation)
{
    /* Se la motivazione passata come argomento corrisponde a covid, allora il "Green Pass" sarà considerato invalidato */
    if(!strcmp(motivation, "Covid"))
    {
        return FALSE;
    }

    /* Nel caso in cui la motivazione passata sia diversa da covid, allora ci sarà bisogno di confrontare le date per verificare che il "Green Pass" sia ancora valido */
    if(compare_date(today_date, expiration_date) < 0)
    {
        strcpy(motivation, "Scaduto");
        return FALSE;
    }

    return TRUE;
}

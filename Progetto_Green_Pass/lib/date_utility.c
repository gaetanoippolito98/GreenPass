/**
 * @file date_utility.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di definire diverse funzionalità e costanti per la gestione delle date utilizzate nel sistema "Green Pass"
 * 
 * @type    Implementazione libreria @date_utility.h
 * @version 1.0
 */

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <stdio.h>
#include <string.h>
#include "date_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */

#define EXPIRATION_DATE_DAYS 31 /* Il nostro protocollo livello applicazione prevede che l'utente non possa inserire una data inferiore ad un mese rispetto quella corrente.
                                 * Pertanto, si è deciso di definire tale costante, in quanto utile al confronto con il numero di giorni passati dopo la data di vaccinazione */

/* 
 * ================================
 * =   Function Implementation    =
 * ================================
 */


/**
 * @brief La seguente funzione è utile a richiedere ad un utente qualsiasi, l'inserimento di una data valida secondo il formato specificato durante l'esecuzione. 
 * 
 * @param daytime Struttura @tm da popolare durante l'inserimento della data specificata dall'utente sullo standard input
 * @param local_time Struttura @tm indicante la data locale. Tale parametro di input può essere @NULL ed in questo caso la funzione non andrà a controllare la differenza di 
 *                   tempo che intercorre tra le due date passate in input come argomento.
 * 
 * @return true Se l'operazione di inserimento è andata a buon fine
 * @return false Se l'operazione di inserimento non è andata a buon fine
 */
bool input_date(struct tm* daytime, struct tm* local_time)
{
    /* 
     * Richiediamo di inserire sullo standard input la data desiderata. Inoltre, con la seguente istruzione andiamo a controllare che l'utente abbia opportunamente inserito
     * una data conforme al formato indicato (gg/mm/yyyy). Quanto inserito dall'utente verrà meorizzato in una struttura @tm definita in @time.h capace di rappresentare un tipo di dato, data
    */
    if(scanf("%d/%d/%d", &(daytime->tm_mday), &(daytime->tm_mon), &(daytime->tm_year)) < 3)
    {
        /* Caso in cui l'utente abbia inserito una data non conforme al formato indicato (gg/mm/yyyy) */

        /* Stampa sullo standard error (file descriptor 2) un messaggio di errore */
        fprintf(stderr, "Formato data invalida\n");

        /* Ritorniamo @FALSE in quanto l'operazione di inserimento data non è andata a buon fine */
        return FALSE;
    }

    /* Forza la scrittura dei dati bufferizzati sullo stream (stdin) tramite la funzione di basso livello write */
    if(fflush(stdin) == EOF)
    {
        /* Caso in cui la funzione fflush abbia riscontrato un errore*/

        /* La seguente funzione produce un messaggio sullo standard error (file descriptor: 2) che descrive la natura dell'errore */
        perror("fflush error");

        /* Ritorniamo @FALSE in quanto l'operazione di inserimento data non è andata a buon fine */
        return FALSE;
    }

    /* Controlliamo che la data inserita dall'utente sia valida, secondo le specifiche fornite dalla documentazione della funzione @date_valid */
    if(!date_valid(daytime->tm_mday, daytime->tm_mon, daytime->tm_year))
    {
        /* Caso in cui l'utente abbia inserito una data non valida */

        /* Stampa sullo standard error (file descriptor 2) un messaggio di errore */
        fprintf(stderr, "Data non valida\n");

        /* Ritorniamo @FALSE in quanto l'operazione di inserimento data non è andata a buon fine */
        return FALSE;
    }

    /* 
     * Effettuiamo un controllo su local time, in quanto il programmatore potrà decidere se inserire un @NULL come secondo parametro. Nel caso in cui il secondo 
     * parametro sia @NULL, la funzione si comporterà dinamicamente non andando a calcolare la differenza di tempo che intercorre tra entrambe le date 
     */
    if(local_time != NULL)
    {
        /* Differenza in termini di giorni tra le date passate in input */
        int diff_time;

        /* La struct @tm definita in @time.h, utile a rappresentare un tipo di dato data, rappresenta i mesi con un intero compreso nel range <0..11> e gli anni con un intero 
         * pari a (anno attuale - 1900)
        */
        daytime->tm_mon -= 1;
        daytime->tm_year -= 1900;

        /* 
         * Andiamo a calcolare la differenza in termini di giorni tra le date passate in input, attraverso la funzione @compare_date. Inoltre in tale istruzione, controlliamo
         * che il numero di giorni calcolati non siano superiore a 31, in quanto il nostro protocollo livello applicazione prevede che l'utente non possa inserire una data 
         * inferiore ad un mese rispetto quella corrente
        */
        if((diff_time = compare_date(daytime, local_time)) > EXPIRATION_DATE_DAYS)
        {
            /* Caso in cui l'utente abbia inserito una data inferiore di un mese rispetto a quella corrente */

            /* Stampa sullo standard error (file descriptor 2) un messaggio di errore*/
            fprintf(stderr, "Ti sei vaccinato da oltre 31 giorni. Impossibile ottenere GreenPass\n");

            /* Ritorniamo @FALSE in quanto l'operazione di inserimento data non è andata a buon fine */
            return FALSE;
        }
        else if(diff_time == -1)
        {
            /* Caso in cui l'utente abbia inserito una data superiore rispetto a quella corrente */
            
            /* Stampa sullo standard error (file descriptor 2) un messaggio di errore */
            fprintf(stderr, "Data di vaccinazione superiore alla data corrente\n");

            /* Ritorniamo @FALSE in quanto l'operazione di inserimento data non è andata a buon fine */
            return FALSE;
        }
    }

    /* Ritorniamo @TRUE in quanto l'operazione di inserimento data è andata a buon fine */
    return TRUE;
}

/**
 * @brief La seguente funzione è utile a verificare la validità di una data, in particolare, il giorno, il mese e l'anno fornito in input saranno controllati secondo le seguenti
 *        condizioni:
 *        
 *        - Anno @year compreso nel range <@MIN_YEAR..@MAX_YEAR>, dove, entrambe le costanti sono definite nella libreria @date_utility.h
 *        - Mese @month compreso nel range <1..12>
 *        - Giorno @day compreso nel range <1..31>
 *        - Anno @year è bisestile
 *        - Mese @month è febbraio
 *        - Mese @month composto da 30 giorni
 * 
 * @param day Giorno della data di cui si vuole verificare la validità
 * @param month Mese della data di cui si vuole verificare la validità
 * @param year Anno della data di cui si vuole verificare la validità
 * 
 * @return true Se la data passata in input è valida
 * @return false Se la data passata in input non è valida
 */
bool date_valid(int day, int month, int year)
{
    /* Nella seguente istruzione andiamo a verificare che l'anno passato in input come argomento sia compreso nel RANGE deciso */
    if(year < MIN_YEAR || year > MAX_YEAR)
    {
        /* Caso in cui l'anno passato in input come argomento non sia compreso nel RANGE deciso */

        /* Ritorniamo @FAlSE in quanto possiamo considerare la data passata in input come argomento non valida */
        return FALSE;
    }

    /* Nella seguente istruzione andiamo a verificare che il mese passato in input come argomento sia compreso nel range <1..12>*/
    if(month < 1 || month > 12)
    {
        /* Caso in cui il mese passato in input come argomento non sia compreso nel RANGE deciso */

        /* Ritorniamo @FAlSE in quanto possiamo considerare la data passata in input come argomento non valida */
        return FALSE;
    }

    /* Nella seguente istruzione andiamo a verificare che il giorno passato in input come argomento sia compreso nel range <1..31>*/
    if(day < 1 || day > 31)
    {
        /* Caso in cui il mese passato in input come argomento non sia compreso nel RANGE deciso */

        /* Ritorniamo @FAlSE in quanto possiamo considerare la data passata in input come argomento non valida */
        return FALSE;
    }

    /* 
     * Se il mese passato in input come argomento corrisponde a febbraio, possono esserci due possibili casi, in quanto, se l'anno è bisestile febbraio sarà composta da 
     * 29 giorni, altrimenti da 28 giorni.
     */
    if(month == 2)
    {
        /* Controlliamo se l'anno passato in input come argomento è bisestile */
        if(is_leap(year))
        {
            return (day <= 29);
        }
        else
        {
            return (day <= 28);
        }
    }

    /* 
     * Alcuni mesi dell'anno sono composti da 30 giorni e non da 31. Pertanto, controlliamo se il mese passato in input come argomento corrisponda ad 
     * Aprile, Giugno, Settembre o Novembre (30 giorni) 
     */
    if(month == 4 || month == 6 || month == 9 || month == 11)
    {
        return (day <= 30);
    }

    /* In qualsiasi altro caso la data passata in input come argomento sarà valida, pertanto ritorneremo @TRUE */
    return TRUE;
}

/**
 * @brief La seguente funzione è utile a verificare se un determinato Anno @year passato in input è bisestile
 * 
 * @param year Anno da verificare se è bisestile
 * 
 * @return true Se l'anno @year è bisestile
 * @return false  Se l'anno @year non è bisestile
 */
bool is_leap(int year)
{
    /**
     * Per verificare se un anno è bisestile si è deciso di progettare ed implementare il seguente algoritmo:
     * 
     * IF year%4 = 0 AND year%100 != 0 OR year%400 = 0
     *      PRINT year is leap
     * ELSE
     *      PRINT year is not leap
     * END IF
    */
    return (((year % 4) == 0 && (year % 100) != 0) || (year % 4) == 0 && (year % 100) == 0 && (year % 400) == 0);
}

/**
 * @brief La seguente funzione permette di confrontare due date passate in input, in modo da interpretare quale delle due date sia maggiore e quanti giorni
 *        intercorrono tra le due date
 * 
 * @param date_1 Struttura @tm utile a rappresentare la prima data che si vuole confrontare
 * @param date_2 Struttura @tm utile a rappresentare la seconda data che si vuole confrontare
 * 
 * @return int La funzione ritornerà in output -1 nel caso in cui @date_1 sia maggiore di @date_2, altrimenti la differenza in termini di giorni
 */
int compare_date(struct tm* date_1, struct tm* date_2)
{
    /* 
     * Per confrontare le due date passate in input come argomento, convertiamo queste ultime in termini di secondi trascorsi dal 1 Gennaio 1970 (@time_t) attraverso
     * la funzione @mktime() definita nella libreria @time.h
     */
    time_t time_1 = mktime(date_1);
    time_t time_2 = mktime(date_2);

    /* Se il primo argomento presenta un valore in termini secondi maggiori del secondo argomento, allora la prima data passata in input sarà superiore alla seconda */
    if(time_1 > time_2)
    {
        /* Caso in cui @date_1 è maggiore di @date_2 */

        /* In questo caso ritorneremo un intero simbolico -1, corrispondente ad un valore di errore */
        return -1;
    }

    /*
     * Calcoliamo la differenza in termini di secondi che intercorre tra la data 2 e la data 1, supponendo che la data 2 sia maggiore della data 1 considerato il precedente
     * costrutto di controllo
     */
    double seconds = difftime(time_2, time_1);

    /* Ritorniamo in output la differenza in termini di giorni andando a moltiplicare il numero di secondi ottenuti dalla funzione @difftime() per 3600 * 24 */
    return (int)(seconds / (60 * 60 * 24));
}

/**
 * @brief La seguente funzione ha lo scopo di aggiungere un tempo prefissato espresso in mesi a una specifica data passata in input 
 * 
 * @param date Struttura @tm utile a rappresentare il tipo di dato data che si vuole aggiornare
 * @param months Numero di mesi da aggiungere alla data specificata in input @date 
 * 
 * @return struct tm Data aggiornata
 */
struct tm add_month_to_date(struct tm date, int months)
{
    /* Aggiungiamo il numero di mesi passato in input come argomento al campo @tm_mon della struttura @tm passata in input */
    date.tm_mon += months;

    /* Il campo @tm_mon della struttura @tm passata in input potrebbe essere fuori dal range dei mesi <0..11>, pertanto, attraverso la funzione @mktime() rinormalizziamo la data
     * secondo il calendario locale 
     */
    mktime(&date);

    return date;
}

/**
 * @brief Tale funzione ha lo scopo di ottenere una stringa contenente la data e l'ora locale
 * 
 * @return char* Stringa contenente data e ora locale
 */
char* get_timestamp(void)
{
    /* Calcoliamo il numero di secondi trascorsi a partire dal 1 Gennaio 1970 attraverso la funzione @time() definita nella libreria @time.h*/
    time_t timestamp = time(NULL);
    /* Otteniamo una stringa formattata contenente data ed ora locale attraverso la funzione @ctime() definita nella libreria @time.h*/
    char *daytime = ctime(&timestamp);

    /* Sostituiamo il carattere newline aggiunto dalla funzione @ctime con un carattere di terminazione stringa */
    daytime[strlen(daytime)-1] = 0;

    /* Ritorniamo la stringa formattata contenente data ed ora locale*/
    return daytime;
}

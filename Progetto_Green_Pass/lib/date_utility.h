/**
 * @file encryption_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di definire diverse funzionalità e costanti per la gestione delle date utilizzate nel sistema "Green Pass"
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @date_utility.h */
#ifndef DATA_UTILITY_H
#define DATA_UTILITY_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <time.h>
#include "bool_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */
#define MAX_YEAR 2022 /* Anno di vaccinazione massimo */
#define MIN_YEAR 2020 /* Anno di vaccinazione minimo */
#define MAX_DATE_LEN 11 /* Lunghezza massima, in termini di caratteri, di una data */

/* 
 * ================================
 * =     Function Definition      =
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
bool input_date(struct tm*, struct tm*);

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
bool date_valid(int, int, int);

/**
 * @brief La seguente funzione è utile a verificare se un determinato Anno @year passato in input è bisestile
 * 
 * @param year Anno da verificare se è bisestile
 * 
 * @return true Se l'anno @year è bisestile
 * @return false  Se l'anno @year non è bisestile
 */
bool is_leap(int);

/**
 * @brief La seguente funzione permette di confrontare due date passate in input, in modo da interpretare quale delle due date sia maggiore e quanti giorni
 *        intercorrono tra le due date
 *
 * @param date_1 Struttura @tm utile a rappresentare la prima data che si vuole confrontare
 * @param date_2 Struttura @tm utile a rappresentare la seconda data che si vuole confrontare
 *
 * @return int La funzione ritornerà in output -1 nel caso in cui @date_1 sia maggiore di @date_2, altrimenti la differenza in termini di giorni
 */
int compare_date(struct tm*, struct tm*);
/**
 * @brief La seguente funzione ha lo scopo di aggiungere un tempo prefissato espresso in mesi a una specifica data passata in input 
 * 
 * @param date Struttura @tm utile a rappresentare il tipo di dato data che si vuole aggiornare
 * @param months Numero di mesi da aggiungere alla data specificata in input @date 
 * 
 * @return struct tm Data aggiornata
 */
struct tm add_month_to_date(struct tm, int);

/**
 * @brief Tale funzione ha lo scopo di ottenere una stringa contenente la data e l'ora locale
 * 
 * @return char* Stringa contenente data e ora locale
 */
char* get_timestamp(void);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @date_utility.h */
#endif // DATA_UTILITY_H

/**
 * @file buffer_utility.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare funzioni legate al funzionamento dei buffer
 * 
 * @type    Implementazione libreria @buffer_utility.c
 * @version 1.0
 */
/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <stdio.h>
#include "buffer_utility.h"

/* 
 * ================================
 * =   Function Implementation    =
 * ================================
 */

/**
 * @brief La seguente funzione ha lo scopo di liberare lo stream dello standard input dai caratteri newline rimasti nello stream. In questo modo evitiamo problemi
 *       legati al funzionamento dello standard input
 * 
 */
void free_input_buffer()
{
    int ch;

    /* Leggiamo caratteri dallo standard input fino a quando non viene letto un carattere di nuova linea oppure un End Of File @EOF */
    while ((ch = getchar()) != '\n' && ch != EOF);
}
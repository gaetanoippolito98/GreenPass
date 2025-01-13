/**
 * @file buffer_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare funzioni legate al funzionamento dei buffer
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @buffer_utility.h */
#ifndef BUFFER_UTILITY_H
#define BUFFER_UTILITY_H

/* 
 * ================================
 * =     Function Definition      =
 * ================================
 */

/**
 * @brief La seguente funzione ha lo scopo di liberare lo stream dello standard input dai caratteri newline rimasti nello stream. In questo modo evitiamo problemi
 *       legati al funzionamento dello standard input
 * 
 */
void free_input_buffer();

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @buffer_utility.h */
#endif //BUFFER_UTILITY_H

/**
 * @file digits_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare funzioni legate al controllo e verifica delle stringhe composte da soli volori numerici
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @digits_utility.h */
#ifndef DIGITS_UTILITY_H
#define DIGITS_UTILITY_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include "bool_utility.h"

/* 
 * ================================
 * =     Function Definition      =
 * ================================
 */

/**
 * @brief La seguente funzione ha lo scopo di verificare che la stringa passata in input @code sia composta da soli caratteri numerici
 * 
 * @param code Stringa di cui si vuole verificare se composta da soli caratteri numerici
 * 
 * @return true Se la stringa è composta da soli caratteri numerici
 * @return false Se la stringa non è composta da soli caratteri numerici
 */
bool is_digits_only(const char *);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @digits_utility.h */
#endif // DIGITS_UTILITY_H

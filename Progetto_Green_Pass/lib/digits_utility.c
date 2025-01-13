/**
 * @file digits_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare funzioni legate al controllo e verifica delle stringhe composte da soli volori numerici
 * 
 * @type    Implementazione libreria @digits_utility.h
 * @version 1.0
 */

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <ctype.h>
#include "digits_utility.h"

/* 
 * ================================
 * =   Function Implementation    =
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
bool is_digits_only(const char* code)
{
    /* Utilizziamo un costrutto iterativo @while, il quale itererà fino a quando il valore puntato da code non è uguale al carattere di terminazione stringa */
    while (*code)
    {

        /* 
         * Per verificare che la stringa sia composta da soli caratteri numerici, viene utilizzata la funzione @is_digit(), la quale ritornerà un valore maggiore di 0 se il
         * carattere è numerico, altrimenti 0. Pertanto, ad ogni iterazione passeremo ad @is_digit() un carattere della stringa passata in input e ne verificheremo la validità.
         * Infine, incrementeremo di 1 il valore del puntatore a stringa per iterare il carattere successivo.
         */
        if (isdigit(*code++) == 0)
        {
            /* Caso in cui il carattere analizzato non sia numerico */
            return FALSE;
        }
    }

    /* Caso in cui la stringa sia composta da soli valori numerici */
    return TRUE;
}


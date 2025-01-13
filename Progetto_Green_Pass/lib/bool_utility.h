/**
 * @file bool_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di definire il tipo boolean, non presente in C come tipo di dato primitivo
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @bool_utility.h */
#ifndef BOOL_UTILITY_H
#define BOOL_UTILITY_H

/* 
 * ================================
 * =      Type Definition         =
 * ================================
 */

/* 
 * Definizione del tipo di dato booleano attraverso il tipo enumerativo, il quale accetterà i casi @FALSE e @TRUE. Per definire il tipo di dato in questione viene utilizzato il
 * tipo enumerativo ,in quanto ad ogni caso associa un valore intero incrementale a partire da 0. In C i valori diversi da 0 vengono considerati veri, mentre zero viene considerato
 * falso, pertanto possiamo notare la forte analogia tra il tipo enumerativo implementato e l'interpretazione dei valori logici in C
 */
typedef enum {FALSE, TRUE} bool;

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @bool_utility.h */
#endif // BOOL_UTILITY_H

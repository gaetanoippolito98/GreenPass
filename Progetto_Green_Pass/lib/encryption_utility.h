/**
 * @file    encryption_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   Il seguente programma ha lo scopo di definire delle funzioni volte alle operazioni di criptazione e decriptazione
 *
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @encryption_utility.h */
#ifndef CODICE_ENCRYPTION_UTILITY_H
#define CODICE_ENCRYPTION_UTILITY_H

/*
 * ================================
 * =     Function Definition      =
 * ================================
 */

/**
 * @brief La seguente funzione ha lo scopo di criptare e decriptare una data stringa con una chiave passata in input
 *
 * @param string_to_encrypt Stringa che si vuole criptare
 * @param key Chiave per l'operazione di criptazione e decriptazione. Se si vuole decriptare una stringa sarà necessaria
 *            la stessa chiave utilizzata durante l'operazione di criptazione
 *
 * @return Stringa criptata o decriptata
 */
char* xor_crypt(char*, int);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @encryption_utility.h */
#endif //CODICE_ENCRYPTION_UTILITY_H

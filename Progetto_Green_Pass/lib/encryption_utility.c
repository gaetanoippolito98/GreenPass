/**
 * @file    vaccinated_client.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   Il seguente programma ha lo scopo di definire delle funzioni volte alle operazioni di criptazione e decriptazione
 *
 * @type    Implementazione libreria
 * @version 1.0
 */

/*
 * ==========================
 * =         Import         =
 * ==========================
 */
#include "encryption_utility.h"
#include <string.h>

/*
 * ================================
 * =   Function Implementation    =
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
char* xor_crypt(char* string_to_encrypt, int key)
{
    int i;
    for(i = 0; i < strlen(string_to_encrypt); i++)
    {
        /* Per l'operazione di criptazione prevediamo uno xor bit a bit di ogni carattere con la chiave passata come argomento */
        string_to_encrypt[i] ^= key;
    }

    /* Ritorniamo la stringa criptata/decriptata */
    return string_to_encrypt;
}

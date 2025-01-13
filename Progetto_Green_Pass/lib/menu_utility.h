/**
 * @file menu_utility.h
  * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare alcune funzioni di utilità generale per gestire l'input ed output dell'utente
 * 
 * @type    Implementazione libreria @manu_utility.h
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @menu_utility.h */
#ifndef MENU_UTILITY_H
#define MENU_UTILITY_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include "date_utility.h"
#include "package_utility.h"

/* 
 * ================================
 * =      Type Definition         =
 * ================================
 */

/**
 * @brief La seguente funzione ha lo scopo di stampare a video il menu utente e di richiedere a quest'ultimo di inserire i dati necessari per l'iscrizione alla 
 *        piattaforma "Green Pass".
 * 
 * @param vaccination_date Parametro di output, passato in input allo scopo di popolare i campi che compongono la struct @tm
 * @param local_daytime struct @tm utile a rappresentare la data locale
 * @param verification_code Parametro di output, passato in input allo scopo di reperire il codice di tessera sanitaria
 * 
 * @return true Se l'esecuzione del menu è terminata con successo
 * @return false Se l'esecuzione del menu è terminata senza successo
 */
bool run_vaccinated_menu(struct tm*, struct tm*, char*);

/**
 * @brief La seguente funzione ha lo scopo di stampare a video il menu utente e di richiedere a quest'ultimo di inserire i dati necessari per ricevere informazioni sul 
 * corrispettivo documento "Green pass"
 * 
 * @param card_code Parametro di output, passato in input allo scopo di reperire il codice di tessera sanitaria
 * 
 * @return true Se l'esecuzione del menu è terminata con successo
 * @return false Se l'esecuzione del menu è terminata senza successo
 */
bool run_reviser_menu(char*);

/**
 * @brief La seguente funzione ha lo scopo di stampare a video il menu utente e di richiedere a quest'ultimo di inserire i dati necessari per modificare lo stato del "Green Pass" 
 *        di un utente 
 * 
 * @param administrator_package Parametro di output, passato in input allo scopo di popolare i campi che compongono la struct @Administrator_package
 * @param code_list Lista degli utenti presenti nella piattaforma "Green Pass"
 * @param code_size_list Numero di utenti presenti nella piattaforma "Green Pass"
 * 
 * @return true Se l'esecuzione del menu è terminata con successo
 * @return false Se l'esecuzione del menu è terminata senza successo
 */
bool run_administrator_menu(Administrator_request_package*, char*, int);

/**
 * @brief La seguente funzione ha lo scopo di stampare a video il logo dei client
 */
void print_logo(void);

/**
 * @brief La seguente funzione è utile a stampare sullo standard output le informazioni inerenti ad un utente iscritto alla piattaforma "Green Pass"
 * 
 * @param reviser_response Struttura di tipo @Reviser_rpackage, contenente le informazioni da stampare, inerenti ad un utente iscritto alla piattaforma "Green Pass"
 * @param divider_length Variabile intera utile a stampare a video un divisore tra le diverse informazioni, a lunghezza variabile
 */
void output_user_information(Reviser_package *reviser_response, int divider_length);

/**
 * @brief La seguente funzione è utile a formattare l'output dei menu utente
 * 
 * @param divider_length Variabile intera utile a stampare a video un divisore tra le diverse informazioni, a lunghezza variabile
 */
void print_menu_divider(int);
/**
 * @brief La seguente funzione è utile a stampare la lista degli utenti iscritti alla piattaforma "Green Pass"
 * 
 * @param code_list Lista degli utenti iscritti alla piattaforma "Green Pass"
 * @param size Dimensione della lista @code_list degli utenti iscritti alla piattaforma "Green Pass"
 */
void print_code_list(char*, int);

/**
 * @brief La seguente funzione è utile a stampare sullo standard input l'accettazione dei termini e condizioni dell'utente vaccinato
 * 
 * @return true Se l'esecuzione è terminata con successo
 * @return false Se l'esecuzione è terminata senza successo
 */
bool accept_terms(void);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @menu_utility.h */
#endif // MENU_UTILITY_H

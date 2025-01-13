/**
 * @file    green_pass_utility.h
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare alcune funzioni di utilità relative alla documentazione "Green Pass"
 * 
 * @type    Libreria
 * @version 1.0
 */

/* Direttive precompilatore volte a definire la libreria @green_pass_utility.h */
#ifndef GREEN_PASS_UTILITY_H
#define GREEN_PASS_UTILITY_H

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include "package_utility.h"

/* 
 * ==========================
 * =       Constants        =
 * ==========================
 */
#define EXPIRATION_DURATION_MONTH 6

/* 
 * ================================
 * =     Function Definition      =
 * ================================
 */

/**
 * @brief La seguente funzione ha lo scopo di reperire le informazioni "Green Pass" relative ad un utente iscritto
 * 
 * @param reviser_package Parametro di output utile a popolare la struttura @Reviser_package relativamente alle informazioni corrispondenti all'utente iscritto desiderato 
 * @param card_code Codice di tessera sanitaria relativo all'utente iscritto di cui si vogliono reperire le informazioni "Green Pass"
 * 
 * @return File_flags* struttura contenente informazioni relative agli errori di apertura, lettura e scrittura sul file, @NULL in caso non vi siano errori
 */
File_flags* generate_reviser_response(Reviser_package *reviser_package, char *card_code);

/**
 * @brief La seguente funzione ha lo scopo di verificare la validità del "Green Pass" di un determinato utente iscritto alla piattaforma 
 * 
 * @param expiration_date Data di scadenza del "Green Pass"
 * @param today_date  Data ed ora locale
 * @param motivation Motivazione dell'ultimo aggiornamento dell'utente sulla piattaforma (Vaccinazione, covid, guarigione)
 * 
 * @return true Se il "Green Pass" è valido
 * @return false  Se il "Green Pass" non è valido 
 */
bool check_green_pass_validity(struct tm*, struct tm*, char*);

/* Direttiva precompilatore utilizzata per indicare la delimitazione di fine libreria @green_pass_utility.h */
#endif // GREEN_PASS_UTILITY_H

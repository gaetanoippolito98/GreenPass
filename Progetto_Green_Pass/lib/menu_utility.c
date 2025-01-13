/**
 * @file    menu_utility.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare alcune funzioni di utilità generale per gestire l'input ed output dell'utente
 * 
 * @type    Implementazione libreria @menu_utility.h
 * @version 1.0
 */
#include <stdio.h>
#include <string.h>
#include "menu_utility.h"
#include "date_utility.h"
#include "code_verification.h"
#include "buffer_utility.h"

/**
 * @brief La seguente funzione ha lo scopo di stampare a video il menu utente e di richiedere a quest'ultimo di inserire i dati necessari per l'iscrizione alla 
 *        piattaforma "Green Pass".
 * 
 * @param vaccination_date Parametro di output, passato in input allo scopo di popolare i campi che compongono la struct @tm
 * @param local_daytime struct @tm utile a rappresentare la data locale del server
 * @param verification_code Parametro di output, passato in input allo scopo di reperire il codice di tessera sanitaria
 * 
 * @return true Se l'esecuzione del menu è terminata con successo
 * @return false Se l'esecuzione del menu è terminata senza successo
 */
bool run_vaccinated_menu(struct tm* vaccination_date, struct tm* local_daytime, char* verification_code)
{
    /* Stampa del logo utilizzato dal client */
    print_logo();

    printf("Benvenuti nel centro vaccinale.\n\n");
    printf("Inserire data di vaccinazione nel formato DD/MM/YYYY: ");

    /* Richiediamo all'untente di inserire la data di vaccinazione, comparandola con la data locale del server */
    if(!input_date(vaccination_date, local_daytime))
    {
        /* Caso in cui la data inserita non sia valida */

        /* Nel caso in cui la data inserita non sia valida, ritorneremo @False*/
        return FALSE;
    }

    printf("Inserire codice tessera sanitaria (controllare il punto '8' sul retro della tessera)\n> ");

    /* Liberiamo lo stream stdin da eventuali caratteri di newline in pendenza */
    free_input_buffer();

    /* Richiediamo all'utente di inserire il codice di tessera sanitaria */
    fscanf(stdin, "%s", verification_code);

    /* 
     * Il menu client vaccinato ritornerà @True solo nel caso in cui l'utente abbia inserito un codice di tessera sanitaria valido e abbia accettato i termini e condizioni, oltre
     * ad inserire una data valida conforme al formato richiesto, precedentemente verificato
     */
    return (verify_card_code(verification_code) && accept_terms());
}

/**
 * @brief La seguente funzione ha lo scopo di stampare a video il menu utente e di richiedere a quest'ultimo di inserire i dati necessari per ricevere informazioni sul 
 * corrispettivo documento "Green pass"
 * 
 * @param card_code Parametro di output, passato in input allo scopo di reperire il codice di tessera sanitaria
 * 
 * @return true Se l'esecuzione del menu è terminata con successo
 * @return false Se l'esecuzione del menu è terminata senza successo
 */
bool run_reviser_menu(char *card_code)
{
    /* Stampa del logo utilizzato dal client */
    print_logo();

    printf("Benvenuti sulla piattaforma Green Pass.\n\n");
    printf("Inserire codice tessera sanitaria (controllare il punto '8' sul retro della tessera)"
                  "di cui si vuole conoscere la scadenza Green Pass: ");
    
    /* Richiediamo all'utente di inserire il codice di tessera sanitaria */
    fscanf(stdin,"%s", card_code);

    /* Liberiamo lo stream stdin da eventuali caratteri di newline in pendenza */
    free_input_buffer();

    /* Il menu client revisore ritornerà @True solo nel caso in cui l'utente abbia inserito un codice di tessera sanitaria valido */
    return (verify_card_code(card_code));
}

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
bool run_administrator_menu(Administrator_request_package* administrator_package, char* code_list, int code_size_list)
{
    /* 
     * L'utente amministratore potrà modificare lo stato di un "Green Pass" presente nel sistema solo in caso di guarigione o covid, pertanto la scelta verrà controllata
     * attraverso una menu, dove sarà possibile selezionare una sola tra le due opzioni
    */
    int index_motivation_choice;

    /* Stampa del logo utilizzato dal client */
    print_logo();

    printf("Benvenuti sulla piattaforma Green Pass.\n\n");

    /* Controlliamo se non sono presenti utenti iscritti alla piattaforma */
    if(!code_size_list)
    {
        /* Caso in cui non siano presenti utenti iscritti alla piattaforma */

        printf("Non ci sono elementi da aggiornare\n");

        /* Nel caso in cui non vi siano utenti iscritti alla piattaforma ritorneremo @FALSE*/
        return FALSE;
    }

    /* Stampiamo la lista degli utenti iscritti alla piattaforma */
    print_code_list(code_list, code_size_list);

    /* Richiediamo all'amministratore di inserire l'indice di uno degli utenti iscritti alla piattaforma */
    printf("Scegliere l'indice di uno dei seguenti codici tessera sanitaria: ");
    fscanf(stdin,"%d", &(administrator_package->index_list));

    /* Controlliamo se l'utente "Amministratore" ha selezionato un indice non presente nella lista*/
    if(administrator_package->index_list < 1 || administrator_package->index_list > code_size_list)
    {
        /* Caso in cui l'indice selezionato non è presente nella lista */

        printf("Elemento selezionato non esistente\n");

        /* Se l'indice selezionato non è nella lista, ritorneremo @FALSE*/
        return FALSE;
    }

    /* L'indice di riga presente nel file ha un offset inferiore di 1, pertanto andremo a decrementare tale variabile */
    administrator_package->index_list -= 1;

    /* Liberiamo lo stream stdin da eventuali caratteri di newline in pendenza */
    free_input_buffer();

    /* Richiediamo all'utente amministratore di selezionare una motivazione tra quelle disponibili */
    printf("Scegliere il codice della motivazione:\n");
    printf("1. Guarigione\n");
    printf("2. Covid\n");
    printf("> ");
    fscanf(stdin, "%d", &index_motivation_choice);

    /* Controlliamo che la motivazione scelta sia tra quelle disponibili */
    if(index_motivation_choice < 1 || index_motivation_choice > 2)
    {
        /* Caso in cui la motivazione scelta non sia tra quelle disponibili */

        fprintf(stderr, "Scelta selezionata errata\n");

        /* Nel caso in cui la motivazione scelta non sia tra quelle disponibili, ritorneremo @FALSE */
        return FALSE;
    }

    /* Popoliamo la motivazione del pacchetto fornito come parametro di output con la scelta selezionata */
    strcpy(administrator_package->motivation, (index_motivation_choice == 1) ? "Guarigione" : "Covid");

    /* Se il menu non presenta errori durante la sua esecuzione, allora ritorneremo @TRUE */
    return TRUE;
}


/**
 * @brief La seguente funzione ha lo scopo di stampare a video il logo dei client
 * 
 */
void print_logo(void)
{
    printf("   ______    ___   ____   ____  _____  ______      __    ______ \n");
    printf(" .' ___  | .'   `.|_  _| |_  _||_   _||_   _ `.   /  | .' ____ '.\n");
    printf("/ .'   \\_|/  .-.  \\ \\ \\   / /    | |    | | `. \\  `| | | (____) |\n");
    printf("| |       | |   | |  \\ \\ / /     | |    | |  | |   | | '_.____. |\n");
    printf("\\ `.___.'\\\\  `-'  /   \\ ' /     _| |_  _| |_.' /  _| |_| \\____| |\n");
    printf(" `.____ .' `.___.'     \\_/     |_____||______.'  |_____|\\______,' \n\n");
}

/**
 * @brief La seguente funzione è utile a stampare sullo standard output le informazioni inerenti ad un utente iscritto alla piattaforma "Green Pass"
 * 
 * @param reviser_response Struttura di tipo @Reviser_rpackage, contenente le informazioni da stampare, inerenti ad un utente iscritto alla piattaforma "Green Pass"
 * @param divider_length Variabile intera utile a stampare a video un divisore tra le diverse informazioni, a lunghezza variabile
 */
void output_user_information(Reviser_package* reviser_response, int divider_length)
{
    /* Stringa contenente la data di scadenza "Green Pass"*/
    char expiration_date[11];

    /* Stampa del campo validità */
    print_menu_divider(divider_length);
    printf("= Validità:             %s %s\n",    "Green Pass", reviser_response->is_green_pass_valid ? "valido" : "non valido");
    print_menu_divider(divider_length);

    /* Costruiamo expiration_date secondo la seguente stringa formattata */
    snprintf(expiration_date, 11, "%d/%d/%d", reviser_response->expiration_date.tm_mday,
                                                               reviser_response->expiration_date.tm_mon + 1,
                                                               reviser_response->expiration_date.tm_year + 1900);
    /* Stampa del campo scadenza del documento "Green Pass" */
    printf("= Scadenza:             %s\n", reviser_response->is_green_pass_valid ?  expiration_date : "-/-/-");
    print_menu_divider(divider_length);

    /* Stampa del campo motivazione del documento "Green Pass" */
    printf("= Motivazione:          %s\n",       reviser_response->motivation);
    print_menu_divider(divider_length);

    /* Stampa del campo ultimo aggiornamento del documento "Green Pass" */
    printf("= Ultimo Aggiornamento: %d/%d/%d\n", reviser_response->last_update.tm_mday,
                                                          reviser_response->last_update.tm_mon + 1,
                                                          reviser_response->last_update.tm_year + 1900);
    print_menu_divider(divider_length);
    printf("\n");
}

/**
 * @brief La seguente funzione è utile a formattare l'output dei menu utente
 * 
 * @param divider_length Variabile intera utile a stampare a video un divisore tra le diverse informazioni, a lunghezza variabile
 */
void print_menu_divider(int divider_length)
{
    int i;

    for(i = 0; i < divider_length; i++)
    {
        printf("=");
    }

    printf("\n");
}

/**
 * @brief La seguente funzione è utile a stampare la lista degli utenti iscritti alla piattaforma "Green Pass"
 * 
 * @param code_list Lista degli utenti iscritti alla piattaforma "Green Pass"
 * @param size Dimensione della lista @code_list degli utenti iscritti alla piattaforma "Green Pass"
 */
void print_code_list(char* code_list, int size)
{
    int i;

    print_menu_divider(45);
    printf("Indice    Codici Tessera Sanitaria\n");
    print_menu_divider(45);
    for(i = 0; i < size; i++)
    {
        printf("%3d.       %-20s\n", i + 1, code_list + i * 21);
    }
    print_menu_divider(45);

    printf("\n");
}

/**
 * @brief La seguente funzione è utile a stampare sullo standard input l'accettazione dei termini e condizioni dell'utente vaccinato
 * 
 * @return true Se l'esecuzione è terminata con successo
 * @return false Se l'esecuzione è terminata senza successo
 */
bool accept_terms()
{
    /* Variabile dichiarata allo scopo di memorizzare la scelta dell'utente riguardo l'accettazione dei termini e condizioni */
    int accepted_terms;

    /* Stampa a video del menu */
    printf("Accettare il trattamento dei dati:\n");
    printf("1. Accetto\n");
    printf("2. Non Accetto\n");
    printf("> ");

    /* Richiediamo all'utente di accettare termini e condizioni*/
    scanf("%d", &accepted_terms);

    /* Controlliamo se la scelta dell'utente è diversa da 1 */
    if(accepted_terms < 1 || accepted_terms >= 2)
    {
        /* Caso in cui la scelta dell'utente sia diversa da 1 */
        fprintf(stderr, "Termini e condizioni necessari per continuare ad utilizzare il software!\n");

        /* Nel caso in cui l'utente abbia scelto un valore divverso da 1 ritorneremo @False*/
        return FALSE;
    }

    return TRUE;
}
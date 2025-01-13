/**
 * @file file_utility.c
 * @author  Roberto Vecchio, Francesco Mabilia & Gaetano Ippolito
 * @brief   La seguente libreria ha lo scopo di incapsulare funzioni legate alla gestione dei file
 * 
 * @type    Implementazione libreria @file_utility.c
 * @version 1.0
 */

/* 
 * ==========================
 * =         Import         =
 * ==========================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_utility.h"
#include "date_utility.h"
#include "green_pass_utility.h"

/* 
 * ================================
 * =   Function Implementation    =
 * ================================
 */

/**
 * @brief Tale funzione ha lo scopo di verificare che una data stringa sia presente in un determinato file
 * 
 * @param file_name File in cui si vuole verificare la presenza della stringa passata in input @code
 * @param code Stringa di cui si vuole verificare la presenza nel file passato in input @file_name
 * @param buffer_size Durante verifica della presenza della stringa nel file, viene effettuata un'operazione di lettura. Pertanto, sarà necessario utilizzare
 *                    un buffer per memorizzare i caratteri letti. Attraverso tale parametro di input, sarà possibile decide la dimensione del buffer di lettura
 * 
 * @return File_result struttura contenente informazioni relative agli errori di apertura, lettura e scrittura sul file, con l'aggiunta di un valore booleano che ha lo scopo
 *         di interpretare la presenza di una determinata stringa in un file.
 */
File_result is_code_written_in_file(char *file_name, char *code, int buffer_size)
{
    /* Dichiariamo una stringa, la quale fungerà da buffer per la lettura del file. Tale stringa verrà allocata dinamicamente in base alla size del buffer passato in input */
    char* line = (char *)malloc(buffer_size * sizeof(char));
    /* Dichiariamo un puntatore a char che ci permetterà di memorizzare le stringhe usuddivide con una strtok ad ogni occorrenza di uno spazio */
    char* tokens;

    /* Eseguiamo il controllo sulla mallo, in quanto potrebbe ritornare un puntatore nullo */
    if(line == NULL)
    {
        /* Caso in cui la malloc ritorna un puntatore nullo */

        /* Recuperiamo la data ed ora locale per stampare un log del server */
        char* time_stamp = get_timestamp();

        /* 
         * Popoliamo la struttura @File_result con i seguenti valori:
         * - @result_flag     =  0 -  in quanto non abbiamo trovato l'occorenza della stringa nel file 
         * - @open_file_flag  =  0 -  in quanto non vi sono stati errori durante l'apertura del file
         * - @write_file_flag =  0 -  in quanto non vi sono stati errori durante un'operazione di scrittura del file
         * - @read_file_flag  =  1 -  in quanto essendo la verifica della presenza della stringa in un file un'operazione di lettura, è stato assegnato il valore simbolico 1
         *                           anche in caso di errore sulla funzione @malloc
         */
        File_result file_errors = {.result_flag = 0, .file_flags.open_file_flag = 0, .file_flags.write_file_flag = 0, .file_flags.read_file_flag = 1};

        /* Stampiamo l'errore */
        fprintf(stderr, "%s - Errore nell'apertura del File\n", (time_stamp != NULL) ? time_stamp : "/");
        /* Ritorniamo la struttura creata */
        return file_errors;
    }

    /* Dichiariamo un puntatore a file per finalizzare l'operazione di apertura del file specificato come parametro di input*/
    FILE* file_codes;
    
    /* Apriamo il file contenente tutti i sottocodici delle regioni e controlliamo se il file è stato aperto correttamente*/
    if ((file_codes = fopen(file_name, "r")) == NULL)
    {
        /* Caso in cui il file non è stato aperto correttamente */

        /* Recuperiamo la data ed ora locale per stampare un log del server */
        char* time_stamp = get_timestamp();

        /* 
         * Popoliamo la struttura @File_result con i seguenti valori:
         * - @result_flag     = 0 - in quanto non abbiamo trovato l'occorenza della stringa nel file 
         * - @open_file_flag  = 1 - in quanto vi sono stati errori durante l'apertura del file
         * - @write_file_flag = 0 - in quanto non vi sono stati errori durante un'operazione di scrittura del file
         * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
         */
        File_result file_errors = {.result_flag = 0, .file_flags.open_file_flag = 1, .file_flags.write_file_flag = 0, .file_flags.read_file_flag = 0};

        /* Stampiamo l'errore */
        fprintf(stderr, "%s - Errore nell'apertura del File\n", (time_stamp != NULL) ? time_stamp : "/");
        /* Liberiamo la memoria allocata dinamicamente */
        free(line);
        /* Ritorniamo la struttura creata */
        return file_errors;
    }

    /* Eseguiamo una lettura da file finchè il cursore non raggiunge l'END OF FILE @EOF */
    while (!feof(file_codes))
    {
        /* Leggiamo da file l'intera riga e la memorizziamo nella variabile @line precedentemente dichiarata */
        fgets(line, buffer_size, file_codes);
        /* Suddividiamo la stringa contenuta nel buffer @line in parole ogni volta che troviamo un'occorrenza del carattere spazio */
        tokens = strtok(line, " ");

        /* La strtok può ritornare un puntatore a @NULL, pertanto andiamo a gestire l'eccezione */
        if(tokens == NULL)
        {
            /* Caso in cui tokens sia nullo */

            /* 
             * Popoliamo la struttura @File_result con i seguenti valori:
             * - @result_flag     = 0 - in quanto non abbiamo trovato l'occorenza della stringa nel file 
             * - @open_file_flag  = 0 - in quanto non vi sono stati errori durante l'apertura del file
             * - @write_file_flag = 0 - in quanto non vi sono stati errori durante un'operazione di scrittura del file
             * - @read_file_flag  = 1 - in quanto essendo la verifica della presenza della stringa in un file un'operazione di lettura, è stato assegnato il valore simbolico 1
             *                          anche in caso di errore sulla funzione @strtok
             */
            File_result file_errors = {.result_flag = 0, .file_flags.open_file_flag = 0, .file_flags.write_file_flag = 0, .file_flags.read_file_flag = 1};

            /* Liberiamo la memoria allocata dinamicamente */
            free(line);
            /* Chiudiamo il file precedentemente aperto */
            fclose(file_codes);
            /* Ritorniamo la struttura creata */
            return file_errors;
        }
        /* 
         * Durante l'operazione di lettura da file, vengono memorizzati anche i caratteri di newline e risulta necessario considerare che alla fine di ogni riga viene 
         * utilizzato un carattere di nuova linea. Andiamo, quindi, a sostituire il carattere \n posto alla fine dell'ultima stringa ottenuta con la @strtok, con un carattere
         * di terminazione di stringa. L'indice di tale carattere viene reperito utilizzando la funzione @strcspn(), la quale permette di restituire l'indice della prima 
         * occorrenza
         */
        tokens[strcspn(tokens, "\n")] = '\0';

        /*
         * I file utilizzati dal sistema Green Pass vengono memorizzati in modo tale che ci sia sempre un codice come prima parola, sia nel caso della lista di utenti
         * iscritti, sia nel caso del file contenente la lista dei codici regione. Pertanto andremo a confrontare il codice passato in input come argomento con la prima 
         * parola ottenuta con la @strtok()
         */
        if (!strcmp(tokens, code))
        {
            /* Caso in cui il codice passato in input venga trovato all'interno del file */

            /* 
             * Popoliamo la struttura @File_result con i seguenti valori:
             * - @result_flag     = 1 - in quanto abbiamo trovato l'occorenza della stringa nel file 
             * - @open_file_flag  = 0 - in quanto non vi sono stati errori durante l'apertura del file
             * - @write_file_flag = 0 - in quanto non vi sono stati errori durante un'operazione di scrittura del file
             * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
             */
            File_result file_errors = {.result_flag = 1, .file_flags.open_file_flag = 0, .file_flags.write_file_flag = 0, .file_flags.read_file_flag = 0};


            /* Liberiamo la memoria allocata dinamicamente */
            free(line);
            /* Chiudiamo il file precedentemente aperto */
            fclose(file_codes);
            /* Ritorniamo la struttura creata */
            return file_errors;
        }
    }

    /* 
     * Popoliamo la struttura @File_result con i seguenti valori:
     * - @result_flag     = 0 - in quanto non abbiamo trovato l'occorenza della stringa nel file 
     * - @open_file_flag  = 0 - in quanto non vi sono stati errori durante l'apertura del file
     * - @write_file_flag = 0 - in quanto non vi sono stati errori durante un'operazione di scrittura del file
     * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
     */
    File_result file_errors = {.result_flag = 0, .file_flags.open_file_flag = 0, .file_flags.write_file_flag = 0, .file_flags.read_file_flag = 0};

    /* Deallocazione della memoria */
    free(line);
    /* Chiusura del file */
    fclose(file_codes);
    /* Ritorniamo la struttura creata */
    return file_errors;
}

/**
 * @brief Funzione utile a memorizzare un utente vaccinato nel file @../file/server/vaccinated
 * 
 * @param vaccinated_client_info Stringa corrispondente alle informazioni dell'utente vaccinato
 * 
 * @return struttura contenente informazioni relative agli errori di apertura, lettura e scrittura sul file, con l'aggiunta di un valore booleano che ha lo scopo
 *         di interpretare la presenza di una determinata stringa in un file.  
 */
File_result subscribe_vaccinated_client(char* vaccinated_client_info)
{
    /* Dichiariamo un puntatore a file per finalizzare l'operazione di apertura del file @../file/server/vaccinated */
    FILE *vaccinated_file;

    /* Apriamo il file contenente tla lista degli utenti iscritti e controlliamo se il file è stato aperto correttamente */
    if((vaccinated_file = fopen(VACCINATED_FILE_NAME, "a")) == NULL)
    {
        /* Caso in cui il file non è stato aperto correttamente */

        /* Recuperiamo la data ed ora locale per stampare un log del server */
        char* time_stamp = get_timestamp();

        /* 
         * Popoliamo la struttura @File_result con i seguenti valori:
         * - @result_flag     = 0 - in quanto non abbiamo trovato l'occorenza della stringa nel file 
         * - @open_file_flag  = 1 - in quanto vi sono stati errori durante l'apertura del file
         * - @write_file_flag = 0 - in quanto non vi sono stati errori durante un'operazione di scrittura del file
         * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
         */
        File_result file_errors = {.result_flag = 0, .file_flags.open_file_flag = 1, .file_flags.write_file_flag = 0, .file_flags.read_file_flag = 0};

        /* Stampiamo l'errore */
        fprintf(stderr, "%s - Errore nell'apertura del File\n", (time_stamp != NULL) ? time_stamp : "/");
        /* Ritorniamo la struttura creata */
        return file_errors;
    }

    /* 
     * Andiamo a stampare su file la stringa formattata passata in input con l'aggiunta di un carattere di newline. Inoltre effettuiamo un controllo sull'esecuzione 
     * dell'operazione di scrittura, in quanto la fprintf ritornerà un valore negativo nel caso in cui vi siano errori durante la scrittura su file
     */
    if(fprintf(vaccinated_file, "%s\n", vaccinated_client_info) < 0)
    {
        /* Caso l'operazione di scrittura su file non sia andata a buon fine */

        /* Recuperiamo la data ed ora locale per stampare un log del server */
        char* time_stamp = get_timestamp();

        /* 
         * Popoliamo la struttura @File_result con i seguenti valori:
         * - @result_flag     = 0 - in quanto non abbiamo trovato l'occorenza della stringa nel file 
         * - @open_file_flag  = 0 - in quanto non vi sono stati errori durante l'apertura del file
         * - @write_file_flag = 1 - in quanto vi sono stati errori durante un'operazione di scrittura del file
         * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
         */
        File_result file_errors = {.result_flag = 0, .file_flags.open_file_flag = 0, .file_flags.write_file_flag = 1, .file_flags.read_file_flag = 0};

        /* Stampiamo l'errore */
        fprintf(stderr, "%s - Errore nella scrittura del File\n", (time_stamp != NULL) ? time_stamp : "/");
        /* Chiusura del file */
        fclose(vaccinated_file);
        /* Ritorniamo la struttura creata */
        return file_errors;
    }

    /* 
     * Popoliamo la struttura @File_result con i seguenti valori:
     * - @result_flag     = 1 - in quanto abbiamo trovato l'occorenza della stringa nel file 
     * - @open_file_flag  = 0 - in quanto non vi sono stati errori durante l'apertura del file
     * - @write_file_flag = 0 - in quanto non vi sono stati errori durante un'operazione di scrittura del file
     * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
     */
    File_result file_errors = {.result_flag = 1, .file_flags.open_file_flag = 0, .file_flags.write_file_flag = 0, .file_flags.read_file_flag = 0};

    /* Chiusura del file */
    fclose(vaccinated_file);
    /* Ritorniamo la struttura creata */
    return file_errors;
}

/**
 * @brief La seguente funzione ha lo scopo di reperire la lista degli utenti iscritti alla piattaforma "Green Pass"
 * 
 * @param file_name Nome del file dal quale si vuole reperire la lista di utenti
 * @param list_codes Parametro di output utile a memorizzare la lista di utenti
 * 
 * @return int -1 in caso di errore, altrimenti il numero di utenti iscritti alla piattaforma
 */
int get_file_list(char* file_name, char** list_codes)
{
    /* Dichiariamo un puntatore a file per finalizzare l'operazione di apertura del file specificato come parametro di input */
    FILE* file_point;
    /* Dichiariamo una variabile intera utile a rappresentare il numero di utenti presenti nel file */
    int line_counter = 0;
    /* Dichiariamo una variabile intera utile a rappresentare il numero di caratteri letti */
    int read_character;
    /* Dichiariamo una stringa, utile a memorizzare il codice di tessera sanitaria di ogni utente */
    char buffer[21];
    /* Dichiariamo una variabile intera utile ad interpretare in quale dei 4 campi (parole) dell'utente iscritto, presenti su di una riga, ci troviamo */
    int counter = 0;

    /* Apriamo il file e controlliamo se il file è stato aperto correttamente */
    if((file_point = fopen(file_name, "r")) == NULL)
    {
        /* Caso in cui il file non è stato aperto correttamente */

        /* Recuperiamo la data ed ora locale per stampare un log del server */
        char* time_stamp = get_timestamp();
        /* Stampiamo l'errore */
        fprintf(stderr, "%s - Errore nell'apertura del File\n", (time_stamp != NULL) ? time_stamp : "/");
        /* Ritorniamo -1 in quanto abbiamo riscontrato un errore*/
        return -1;
    }

    /* Eseguiamo un costrutto iterativo fino a quando il carattere letto attraverso la funzione @fgetc() è diverso da @EOF*/
    while((read_character = fgetc(file_point)) != EOF)
    {
        /* 
         * Se viene letto un carattere newline, ciò starà a significare che un intero rigo è stato letto. Pertanto, possiamo incrementare il contatore di utenti iscritti
         * @line_counter
         */
        if(read_character == '\n')
        {
            line_counter++;
        }
    }

    /* controlliamo che il numero di iscritti alla piattaforma "Green Pass" nel file indicato, sia diverso da 0, altrimenti, chiuderemo il file e ritorneremo 0 */
    if(line_counter != 0)
    {
        /* Essendo @list_codes un array di stringhe, andiamo ad allocare spazio per memorizzare al suo interno la lista di codici di tessera sanitaria */
        *list_codes = (char*)malloc(line_counter * 21);
        /* Riassegnamo il valore 0 a @line_counter per*/
        line_counter = 0;

        /* La funzione @malloc() può ritornare un puntatore nullo @NULL, pertanto effettueremo un controllo sull'esito di tale operazione */
        if(list_codes == NULL)
        {
            /* Stampiamo l'errore */
            fprintf(stderr, "Errore durante l'allocazione\n");
            /* Ritorniamo -1 in quanto abbiamo riscontrato un errore*/
            return -1;
        }

        /* Usiamo la funzione @fseek per spostare il cursore all'inizio del file */
        fseek(file_point, 0, SEEK_SET);

        /* 
         * Eseguiamo un costrutto iterativo che itererà fino a quando la stringa letta sarà diversa da EOF. Ad ogni iterazione verrà effettuata una lettura da file e memorizzata
         * nel @buffer del codice di tessera sanitaria 
         */
        while(fscanf(file_point, "%s", buffer) != EOF)
        {
            /* 
             * Eseguiamo un costrutto di controllo per eseguire un set di istruzioni solo nel caso in cui la parola letta sia la prima tra quattro, contenente il codice di 
             * tessera sanitaria 
             */
            if(counter % 4 == 0)
            {
                /* Effettuiamo una copia del buffer nell'array di stringhe dichiarato nella signature della funzione come parametro di output*/
                strcpy((*list_codes + (line_counter * 21)), buffer);
                /* Incrementiamo il contatore di utenti in quanto quest'ultimo dovrà fungere da offset per la memorizzazione attraverso la funzione @strcpy() */
                line_counter++;
            }

            /* Incrementiamo il counter */
            counter++;
        }
    }

    /* Chiusura del file */
    fclose(file_point);
    /* Ritorniamo il numero di utenti iscritti alla piattaforma "Green Pass" */
    return line_counter;
}

/**
 * @brief La seguente funzione ha lo scopo di sostituire le vecchie informazioni relative ad un utente con le nuove informazioni passate in input
 * 
 * @param update_package struct @Administrator_request_package utile a rappresentare le informazioni che si vogliono aggiornare nel @file_name indicato, alla riga corrispondente
 *                       all'utente
 * @param administrator_response Parametro di output, utile a rappresentare le modifiche effettuate nel @file_name indicato, alla riga corrispondente a quella indicata 
 *                               nell' @update_package nel file
 *    
 * @param file_name stringa corrispondente al nome del file che si vuole aggiornare con le nuove informazioni
 * 
 * @return true Se l'operazione di aggiornamento è andata a buon fine
 * @return false Se l'operazione di aggiornamento non è andata a buon fine
 */
bool change_information_in_file(Administrator_request_package* update_package, Administrator_response_package* administrator_response, char* file_name)
{
    /* Dichiariamo un puntatore a file per finalizzare l'operazione di apertura del file specificato come parametro di input */
    FILE* file_point;
    /* Dichiariamo una variabile intera utile a rappresentare il numero di utenti presenti nel file */
    int count_lines = 0;
    /* Dichiariamo una variabile intera utile a rappresentare la posizione del cursore */
    long position_cursor;
    /* Dichiariamo una stringa utile a rappresentare l'aggiornamento da sostituire con le vecchie informazioni*/
    char overwrite_string[MAX_FILE_LINE_SIZE];
    /* Variabili utili a rappresentare la data di ultimo aggiornamento da sostituire */
    time_t daytime;
    struct tm* local_time;

    /* Apriamo il file e controlliamo se il file è stato aperto correttamente */
    if((file_point = fopen(file_name, "r+")) == NULL)
    {
        /* Caso in cui il file non è stato aperto correttamente */

        /* Recuperiamo la data ed ora locale per stampare un log del server */
        char* time_stamp = get_timestamp();

        /*
         * Popoliamo la struttura @File_flags con i seguenti valori:
         * - @open_file_flag  = 1 - in quanto vi sono stati errori durante l'apertura del file
         * - @write_file_flag = 0 - in quanto non vi sono stati errori durante un'operazione di scrittura del file
         * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
         */
        File_flags file_errors = {.open_file_flag = 1, .write_file_flag = 0, .read_file_flag = 0};

        /*
         * Inizializziamo il campo @reviser_package.file_flags della struttura @administrator_response con il valore di @file_errors
         * precedentemente popolata
         */
        administrator_response->reviser_package.file_flags = file_errors;

        /* Stampiamo l'errore */
        fprintf(stderr, "%s - Errore nell'apertura del File\n", (time_stamp != NULL) ? time_stamp : "/");
        return FALSE;
    }

    /* Calcoliamo il numero di secondi passati dal 01/01/1970 */
    daytime = time(NULL);

    /*
     * Costruiamo una struttura del tipo struct @tm attraverso la funzione @localtime(), la quale accetterà in input @daytime
     * precedentemente inizializzata. La funzione @localtime() potrebbe tornare un puntatore nullo @NULL, pertanto eseguiamo un
     * costrutto di controllo per gestire tale eccezione
     */
    if((local_time = localtime(&daytime)) == NULL)
    {
        /* Caso in cui @localtime() restituisca un puntatore a @NULL*/

        /*
         * Popoliamo la struttura @File_flags con i seguenti valori:
         * - @open_file_flag  = 0 - in quanto non vi sono stati errori durante l'apertura del file
         * - @write_file_flag = 1 - in quanto essendo la sostituzione di una stringa in un file un'operazione di scrittura,
                                    è stato assegnato il valore simbolico 1 anche in caso di errore sulla funzione @localtime()
         * - @read_file_flag  = 0 - in quanto non vi sono stati errori durante un'operazione di lettura del file
         */
        File_flags file_errors = {.open_file_flag = 0, .write_file_flag = 1, .read_file_flag = 0};

        /*
         * Inizializziamo il campo @reviser_package.file_flags della struttura @administrator_response con il valore di @file_errors
         * precedentemente popolata
         */
        administrator_response->reviser_package.file_flags = file_errors;

        /* Stampiamo l'errore */
        fprintf(stderr, "Errore durante l'utilizzo della localtime\n");
        /* Chiusura del file */
        fclose(file_point);
        return FALSE;
    }

    /*
     * La stringa da sostituire potrebbe avere lunghezza minore rispetto a quella originale. Per non corrompere il file, inizializzeremo la stringa
     * da sovrascrivere con caratteri di spazio per una lunghezza fissata @MAX_FILE_LINE_SIZE - 1 ed aggiungiamo un carattere di terminazione di stringa 
     */
    memset(overwrite_string, 32, MAX_FILE_LINE_SIZE - 1);
    overwrite_string[MAX_FILE_LINE_SIZE-1] = 0;

    /* Attraverso il seguente costrutto iterativo ricerchiamo la stringa da aggiornare */
    while(count_lines != update_package->index_list)
    {
        if((fgetc(file_point)) == '\n')
        {
            count_lines++;
        }
    }

    /* Memorizziamo la posizione del cursore */
    position_cursor = ftell(file_point);

    /* Leggiamo dal file le informazioni da sostituire, memorizzandole temporaneamente nella struttura @administrator_response */
    fscanf(file_point, "%s %d/%d/%d %s", administrator_response->code, &(administrator_response->reviser_package.expiration_date.tm_mday),
                                                       &(administrator_response->reviser_package.expiration_date.tm_mon),
                                                       &(administrator_response->reviser_package.expiration_date.tm_year),
                                                       administrator_response->reviser_package.motivation);

    /* 
     * Eseguiamo un costrutto di controllo, in quanto se la motivazione dell'aggiornamento è guarigione e l'utente non ha contratto il covid, oppure, la motivazione 
     * dell'aggiornamento è uguale all'informazione già presente (covid, covid...guarigione, guarigione), allora le vecchie informazioni non saranno aggiornabili 
     */
    if((strcmp(administrator_response->reviser_package.motivation, "Covid") && !strcmp(update_package->motivation, "Guarigione")) || !strcmp(administrator_response->reviser_package.motivation, update_package->motivation))
    {
        administrator_response->not_updatable = 1;

        /* Chiusura del file */
        fclose(file_point);
        return TRUE;
    }

    /* Nel caso in cui la motivazione sia guarigione andiamo a rinnovare la data di scadenza del "Green pass" attraverso la funzione @add_month_to_date()*/
    if(!strcmp(update_package->motivation, "Guarigione"))
    {
        /* Caso di guarigione */
        administrator_response->reviser_package.expiration_date = add_month_to_date(*local_time, EXPIRATION_DURATION_MONTH);
    }
    else
    {
        /* Caso covid */

        /* 
         * In questo caso la data di scadenza "Green Pass" non verrà aggiornata, in quanto il documento è invalidato. Inoltre, vengono eseguite alcune operazioni 
         * per rendere la data conforme alle regole della struct @tm
         */
        administrator_response->reviser_package.expiration_date.tm_mon -= 1;
        administrator_response->reviser_package.expiration_date.tm_year -= 1900;
    }

    /* 
     * Riassegnamo al parametro di output, corrispondentemente al campo @reviser_package.motivation la nuova motivazione reperita dalla struttura @update_package, attraverso una
     * @strcpy()
     */
    bzero(administrator_response->reviser_package.motivation, sizeof(administrator_response->reviser_package.motivation));
    strcpy(administrator_response->reviser_package.motivation,update_package->motivation);

    /* Modifichiamo la data dell'ultimo aggiornamento con la data locale del server*/
    administrator_response->reviser_package.last_update.tm_mday = local_time->tm_mday;
    administrator_response->reviser_package.last_update.tm_mon = local_time->tm_mon;
    administrator_response->reviser_package.last_update.tm_year = local_time->tm_year;

    /* Aggiorniamo la vvalidità del "Green Pass", andandolo a calcolare attraverso le nuove informazioni ottenute */
    administrator_response->reviser_package.is_green_pass_valid = check_green_pass_validity(&(administrator_response->reviser_package.expiration_date),
                                                                                            &(administrator_response->reviser_package.last_update),
                                                                                            administrator_response->reviser_package.motivation);


    /* Memorizziamo una stringa formattata nel buffer @overwrite_string, popolandola con le nuove informazioni ottenute */
    snprintf(overwrite_string, MAX_FILE_LINE_SIZE - 1, "%s %d/%d/%d %s %d/%d/%d", administrator_response->code,
                                                                                                   administrator_response->reviser_package.expiration_date.tm_mday,
                                                                                                   administrator_response->reviser_package.expiration_date.tm_mon + 1,
                                                                                                   administrator_response->reviser_package.expiration_date.tm_year + 1900,
                                                                                                   administrator_response->reviser_package.motivation,
                                                                                                   administrator_response->reviser_package.last_update.tm_mday,
                                                                                                   administrator_response->reviser_package.last_update.tm_mon + 1,
                                                                                                   administrator_response->reviser_package.last_update.tm_year + 1900);
    
    /* 
     * Eliminiamo il carattere di newline alla fine della stringa in quanto la nuova stringa costruita sarà sicuramente minore della lunghezza massima raggiungibile. Pertanto,
     * sarà necessario avvere degli spazi fino al raggiungimento della dimensione massima.
     */
    overwrite_string[strlen(overwrite_string)] = 32;

    /* Riportiamo il cursore alla riga corrispondente all'aggiornamento dell'utente */
    fseek(file_point,position_cursor,SEEK_SET);

    /* Stampiamo su file la nuova stringa formattata, aggiungendo un \n*/
    fprintf(file_point, "%s\n", overwrite_string);

    /* Avendo aggiornato il file non sarà necessario notificare il client che la riga corrispondente all'utente non è aggiornabile*/
    administrator_response->not_updatable = 0;

    /* Chiusura del file */
    fclose(file_point);
    return TRUE;
}
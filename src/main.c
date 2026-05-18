#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/config.h"
#include "../include/models/user.h"
#include "../include/models/report.h"
#include "../include/adt/report_list.h"
#include "../include/adt/report_stack.h"
#include "../include/utils/validators.h"
#include "../include/utils/parser.h"
#include "../include/server/user_manager.h"
#include "../include/server/report_manager.h"
#include "../include/tests/test_suite.h"
#include "../include/adt/priority_queue.h"
#include "../include/adt/report_avl.h"


/* --------------------------------==============================================
 *  PROTOTIPI GLOBALI 
 * --------------------------------============================================== */
void citizen_menu(User logged_in_user);
void employee_menu(User logged_in_user);
void show_reports_by_status(ReportStatus required_status);
void show_priority_queue(void);
void show_triangulated_user_avl(void);
void employee_change_report_status(void);
void generate_municipal_statistics(void);

/* Separato 'unsigned int' e aggiunti i tipi corretti per il linking di Fase 1 e 2 */
void load_master_reports_to_list(unsigned int target_user_id, ReportList ram_list);
void load_bench_reports_to_list(unsigned int target_user_id, ReportList ram_list);

/* Marcato come static per coordinarsi specularmente con l'implementazione in basso */
static void update_system_counters(ReportStatus old_status, ReportStatus new_status, Report r);

/* Funzioni statiche locali per l'ordinamento numerico dei due indici AVL del server */
//static int compare_uid(const void* a, const void* b);
//static int compare_rid(const void* a, const void* b);

/* Funzioni statiche locali per l'ordinamento numerico dei due indici AVL del server */
//static int compare_uid(const void* a, const void* b);
//static int compare_rid(const void* a, const void* b);
int main(void) {
        int user_choice;
    char username_buffer[MAX_USERNAME];
    char password_buffer[MAX_PASSWORD];

    printf("===================================================\n");
    printf("     SISTEMA DI SEGNALAZIONI MUNICIPALI BARONISSI   \n");
    printf("===================================================\n");

    /* Ciclo infinito di mantenimento dell'interfaccia principale */
    while (1) {
        printf("\n--- MENU PRINCIPALE ---\n");
        printf("1. Accedi (Login)\n");
        printf("2. Registrati (Nuovo Utente)\n");
        printf("3. Esegui Casi di Test Automatizzati\n");
        printf("4. Esci dal Programma\n");
        printf("Seleziona un'opzione: ");
        
        /* Protezione dall'inserimento di caratteri alfabetici spuri per prevenire loop infiniti */
        if (scanf("%d", &user_choice) != 1) {
            while (getchar() != '\n'); /* Svuota la coda del buffer della tastiera */
            continue;
        }
        while (getchar() != '\n'); /* Consuma il carattere di a capo rimasto intrappolato */

        switch (user_choice) {
            case 1:
                printf("\n--- ACCESSO AL SISTEMA ---\n");
                printf("Username: ");
                if (!fgets(username_buffer, sizeof(username_buffer), stdin)) break;
                trim_string(username_buffer); /* Pulisce stringa da spazi e newline finali */
                
                printf("Password: ");
                if (!fgets(password_buffer, sizeof(password_buffer), stdin)) break;
                trim_string(password_buffer);

                /* Esegue l'autenticazione istantanea O(1) tramite la Tabella Hash binarizzata */
                User authenticated_user = login_user(username_buffer, password_buffer);
                if (authenticated_user != NULL) {
                    printf("\n[OK] Autenticazione riuscita! Benvenuto %s.\n", get_user_username(authenticated_user));
                    
                    /* Smistamento dei sotto-menu operativi in base al ruolo anagrafico dell'utente */
                    if (get_user_role(authenticated_user) == EMPLOYEE) {
                        employee_menu(authenticated_user);
                    } else {
                        citizen_menu(authenticated_user);
                    }
                    free_user(authenticated_user); /* Deallocazione e chiusura sicura della sessione */
                } else {
                    printf("\n[ERRORE] Credenziali errate o utente non trovato.\n");
                }
                break;

            case 2:
                printf("\n--- REGISTRAZIONE UTENTE ---\n");
                printf("Scegli un Username: ");
                if (!fgets(username_buffer, sizeof(username_buffer), stdin)) break;
                trim_string(username_buffer);
                
                printf("Scegli una Password: ");
                if (!fgets(password_buffer, sizeof(password_buffer), stdin)) break;
                trim_string(password_buffer);

                int role_selection;
                printf("Seleziona il ruolo (0 = Cittadino, 1 = Dipendente Comunale): ");
                if (scanf("%d", &role_selection) != 1) {
                    while (getchar() != '\n');
                    printf("[ERRORE] Input non valido.\n");
                    break;
                }
                while (getchar() != '\n');

                UserRole designated_role = (role_selection == 1) ? EMPLOYEE : CITIZEN;
                /* Inserimento nello slot calcolato tramite DJB2 con rilevamento anti-duplicati */
                if (register_user(username_buffer, password_buffer, designated_role)) {
                    printf("\n[OK] Registrazione completata con successo!\n");
                } else {
                    printf("\n[ERRORE] Registrazione fallita. Username gia' in uso.\n");
                }
                break;

            case 3:
                run_all_tests(); /* Esecuzione automatica dei test case geometrici ed asintotici */
                break;

            case 4:
                printf("\nGrazie per aver utilizzato il sistema municipale. Arrivederci!\n");
                return 0; /* Terminazione pulita dell'applicazione */

            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
    return 0;
}

/**
 * @brief Operates the interactive session menu for authenticated citizens.
 *        Manages local RAM storage, undo capabilities, and data streaming on logout.
 * @param logged_in_user Opaque pointer to the current active user session.
 */
void citizen_menu(User logged_in_user) {
    /* Inizializzazione della lista concatenata dinamica per accumulare i report in RAM */
    ReportList ram_list = create_list();
    /* Inizializzazione dello stack LIFO statico per conservare i backup di Undo (max 10) */
    ReportStack revert_stack = create_stack();
    
    int user_selection;
    char urgency_char;
    char description_buffer[MAX_DESC];
    char date_buffer[11];
    unsigned int target_user_id = (unsigned int)get_user_id(logged_in_user);

    while (1) {
        printf("\n--- AREA CITTADINO (%s) ---\n", get_user_username(logged_in_user));
        printf("1. Inserisci Nuova Segnalazione (In RAM)\n");
        printf("2. Visualizza lo Storico delle mie Segnalazioni\n");
        printf("3. Modifica una Segnalazione (In RAM)\n");
        printf("4. Annulla Ultima Modifica (Revert/Undo Stack)\n");
        printf("5. Esci ed Invia Segnalazioni al Comune (Logout & Flush)\n");
        printf("Seleziona un'opzione: ");
        
        if (scanf("%d", &user_selection) != 1) { 
            while (getchar() != '\n'); 
            continue; 
        }
        while (getchar() != '\n');

        switch (user_selection) {
            case 1:
                printf("\n--- COMPILA SEGNALAZIONE ---\n");
                int category_input;
                printf("Categoria (0=Buca, 1=Illuminazione, 2=Rifiuti, 3=Impianto, 4=Altro): ");
                if (scanf("%d", &category_input) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                printf("Descrizione del problema: ");
                if (!fgets(description_buffer, sizeof(description_buffer), stdin)) break;
                trim_string(description_buffer);

                printf("Data di oggi (GG/MM/AAAA): ");
                if (!fgets(date_buffer, sizeof(date_buffer), stdin)) break;
                trim_string(date_buffer);

                printf("Livello di Urgenza (0=Bassa, 1=Media, 2=Alta): ");
                int urgency_input;
                if (scanf("%d", &urgency_input) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');
                urgency_char = (char)urgency_input + '0';

                /* Validazione formale dei dati immessi prima dell'accatastamento in RAM */
                if (!validate_not_empty(description_buffer) || !validate_date_format(date_buffer) || !validate_urgency_range(urgency_char)) {
                    printf("\n[ERRORE] Dati inseriti non validi o non conformi.\n");
                    break;
                }

                /* Prelievo dell'ID progressivo unico globale interrogando il registro O(1) */
                unsigned int global_id = generate_global_report_id_v2();
                
                /* Creazione dell'oggetto Report con riga fisica preimpostata a -1 (volatile) */
                Report new_report = create_report(global_id, target_user_id, get_user_username(logged_in_user), (ReportCategory)category_input, description_buffer, date_buffer, urgency_char);
                
                /* Inserimento immediato in testa alla linked list in O(1) */
                list_insert(ram_list, new_report);
                printf("\n[OK] Segnalazione inserita nella sessione locale RAM (Codice: %05u).\n", global_id);
                break;

            case 2:
                printf("\n===================================================\n");
                printf("      STORICO PERSONALE COMPILATO DAL SERVER   \n");
                printf("===================================================\n");
                
                /* FASE 1: Estrazione dati stazionari filtrati per User ID e triangolazione logaritmica su disco */
                load_master_reports_to_list(target_user_id, ram_list);

                /* FASE 2: Scansione orizzontale e sovrascrittura condizionata tramite la cache BENCH */
                load_bench_reports_to_list(target_user_id, ram_list);

                /* FASE 3: RENDERING LOGICO COMPLESSIVO TRAMITE REWIND E SCORRIMENTO */
                int display_counter = 0;
                
                /* Il riavvolgimento della lista espone la RAM nativa aggiornata dalle sovrascritture */
                list_rewind(ram_list);
                Report active_report = list_next(ram_list);
                while (active_report != NULL) {
                    printf("[ RECORD ] Codice: %05u | Categoria: %s\n", get_report_id(active_report), get_category_string(get_report_category(active_report)));
                    printf("           Urgenza: %c | Stato: %s\n", get_report_urgency(active_report), get_status_string(get_report_status(active_report)));
                    printf("           Descrizione: %s\n", get_report_description(active_report));
                    printf("------------------------------------------------------\n");
                    display_counter++;
                    active_report = list_next(ram_list);
                }
                
                if (display_counter == 0) {
                    printf("\nNon ci sono segnalazioni storiche attive associate al tuo account.\n");
                } else {
                    printf("\nFine dello storico. %d segnalazioni totali verificate caricate.\n", display_counter);
                }
                break;

            case 3:
                printf("\nCodice della segnalazione da modificare (in RAM): ");
                int search_id;
                if (scanf("%d", &search_id) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                /* Ricerca lineare del report da alterare all'interno della ram_list */
                Report report_to_modify = list_find(ram_list, search_id);
                /* La modifica è concessa esclusivamente per i casi non ancora presi in carico (OPEN) */
                if (report_to_modify != NULL && get_report_status(report_to_modify) == OPEN) {
                    /* Push immediato dello stato integro nello stack LIFO prima della modifica */
                    stack_push(revert_stack, report_to_modify);
                    
                    printf("Nuova descrizione: ");
                    if (!fgets(description_buffer, sizeof(description_buffer), stdin)) break;
                    trim_string(description_buffer);
                    
                    printf("Nuova Urgenza (0=Bassa, 1=Media, 2=Alta): ");
                    int updated_urgency;
                    if (scanf("%d", &updated_urgency) != 1) { while (getchar() != '\n'); break; }
                    while (getchar() != '\n');
                    char updated_urgency_char = (char)updated_urgency + '0';

                    /* Sostituzione atomica del nodo rimuovendolo e reinserendolo modificato */
                    Report modified_clone = create_report(get_report_id(report_to_modify), target_user_id, get_report_citizen_name(report_to_modify), get_report_category(report_to_modify), description_buffer, get_report_date(report_to_modify), updated_urgency_char);
                    list_remove(ram_list, search_id);
                    list_insert(ram_list, modified_clone);
                    printf("\n[OK] Segnalazione aggiornata in RAM.\n");
                } else {
                    printf("\n[ERRORE] Segnalazione non trovata in sessione o non modificabile.\n");
                }
                break;

            case 4:
                /* Svuotamento LIFO dello stack per ripristinare il backup originario */
                if (!stack_is_empty(revert_stack)) {
                    Report historical_backup = stack_pop(revert_stack);
                    int backup_id = (int)get_report_id(historical_backup);
                    list_remove(ram_list, backup_id);
                    list_insert(ram_list, historical_backup);
                    printf("\n[OK] Azione annullata per il report %05d.\n", backup_id);
                } else {
                    printf("\n[AVVISO] Nessuna azione da annullare nello Stack.\n");
                }
                break;

            case 5:
                printf("\nSalvataggio e sincronizzazione in cache...\n");
                list_rewind(ram_list); // La testa ora è la prima a essere stata creata/modificata
                Report flush_iterator = list_next(ram_list);
                /* Riversamento in cache BENCH di tutti i record accumulati ed alterati in sessione */
                while (flush_iterator != NULL) {
                    register_report_from_citizen_ram(flush_iterator);
                    flush_iterator = list_next(ram_list);
                }
                /* Svuotamento radicale della memoria volatile della sessione */
                free_list(ram_list);
                free_stack(revert_stack);
                return; /* Ritorno al loop principale del programma */

            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
}

/**
 * @brief Operates the interactive session menu for municipal employees.
 *        Orchestrates direct visualization, status advancement, and forced cache synchronization.
 * @param logged_in_user Opaque pointer to the current active user session.
 */
void employee_menu(User logged_in_user) {
    int employee_choice;
    
    /* Ciclo infinito di mantenimento dell'Area Riservata Dipendente */
    while (1) {
        printf("\n--- AREA DIPENDENTE (%s) ---\n", get_user_username(logged_in_user));
        printf("1. Visualizza Segnalazioni APERTE (Master + Cache)\n");
        printf("2. Visualizza Segnalazioni IN LAVORAZIONE (Master)\n");
        printf("3. Visualizza Segnalazioni CHIUSE (Master)\n");
        printf("4. Modifica Stato di una Segnalazione (Avanzamento Pratica AVL)\n");
        printf("5. Visualizza Elenco delle Priorita' ed Urgenze (Coda con Flush Forzato)\n");
        printf("6. Visualizza Storico Strutturato ad Albero Bilanciato (In-Order AVL con Flush Forzato)\n"); 
        printf("7. Genera Report Statistico Comunale O(1)\n");
        printf("8. Disconnetti (Logout)\n");
        printf("Seleziona un'opzione: ");
        
        /* Protezione del flusso di input della tastiera contro caratteri spuri alfabetici */
        if (scanf("%d", &employee_choice) != 1) { 
            while (getchar() != '\n'); /* Svuota immediatamente il buffer */
            continue; 
        }
        while (getchar() != '\n'); /* Consuma il carattere newline residuo */

        switch (employee_choice) {
            case 1:
                /* Esegue la visualizzazione diretta filtrando i casi OPEN nel Master e nella BENCH in O(n) */
                show_reports_by_status(OPEN);
                break;
                
            case 2:
                /* Esegue la visualizzazione diretta filtrando i casi IN_PROGRESS consolidati nel Master */
                show_reports_by_status(IN_PROGRESS);
                break;
                
            case 3:
                /* Esegue la visualizzazione diretta filtrando i casi CLOSED consolidati nel Master */
                show_reports_by_status(CLOSED);
                break;
                
            case 4:
                /* Chiama la funzione di modifica chirurgica, invalidazione master e stack dei buchi */
                employee_change_report_status();
                break;
                
            case 5:
                /* REQUISITO: La costruzione della Priority Queue esegue un FLUSH FORZATO preventivo della BENCH */
                printf("\n[SERVER] Esecuzione Flush Forzato preventivo della BENCH...\n");
                process_and_flush_bench_v2(); 
                show_priority_queue();
                break;
                
            case 6: 
                /* REQUISITO: La navigazione simmetrica degli indici esegue un FLUSH FORZATO preventivo della BENCH */
                printf("\n[SERVER] Esecuzione Flush Forzato preventivo della BENCH...\n");
                process_and_flush_bench_v2(); 
                show_triangulated_user_avl();
                break;
                
            case 7:
                /* Estrazione anagrafica statistica istantanea in O(1) leggendo i registri posizionali fisse */
                generate_municipal_statistics();
                break;
                
            case 8:
                /* Scollegamento controllato della sessione e ritorno immediato al Menu Principale */
                printf("\nDisconnessione effettuata. Ritorno al menu principale.\n");
                return;
                
            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
}


/**
 * @brief Funzione locale statica di utilita per aggiornare atomicamente i registri O(1).
 *        Gestisce decrementi dello stato precedente e incrementi del nuovo stato o elisioni DESTROYED.
 */
static void update_system_counters(ReportStatus old_status, ReportStatus new_status, Report r) {
    /* 1. Decrementa la variabile legata al vecchio stato */
    int reg_old = (old_status == OPEN) ? REG_IDX_STAT_OPEN : (old_status == IN_PROGRESS) ? REG_IDX_STAT_PROGRESS : REG_IDX_STAT_CLOSED;
    unsigned int count_old = read_system_variable(reg_old);
    if (count_old > 0) write_system_variable(reg_old, count_old - 1);

    /* 2. Se il nuovo stato è DESTROYED, riduce i report totali vivi e la categoria */
    if (new_status == DESTROYED) {
        unsigned int total_active = read_system_variable(REG_IDX_NM_REPORT);
        if (total_active > 0) write_system_variable(REG_IDX_NM_REPORT, total_active - 1);
        
        int reg_cat = REG_IDX_CAT_OTHER;
        switch(get_report_category(r)) {
            case ROAD:           reg_cat = REG_IDX_CAT_ROAD; break;
            case LIGHTING:       reg_cat = REG_IDX_CAT_LIGHTING; break;
            case WASTE:          reg_cat = REG_IDX_CAT_WASTE; break;
            case INFRASTRUCTURE: reg_cat = REG_IDX_CAT_INFRASTRUCT; break;
            default:             reg_cat = REG_IDX_CAT_OTHER; break;
        }
        unsigned int count_cat = read_system_variable(reg_cat);
        if (count_cat > 0) write_system_variable(reg_cat, count_cat - 1);
    } else {
        /* 3. Se non è DESTROYED, incrementa la variabile del nuovo stato assegnato */
        int reg_new = (new_status == OPEN) ? REG_IDX_STAT_OPEN : (new_status == IN_PROGRESS) ? REG_IDX_STAT_PROGRESS : REG_IDX_STAT_CLOSED;
        write_system_variable(reg_new, read_system_variable(reg_new) + 1);
    }
}

/**
 * @brief Funzione locale statica di utilita per cambiare lo stato di un report 
 *        Ricerca in Bench e in Hard memory il report per l'aggiornamento dello stato
 */
void employee_change_report_status(void) {
    unsigned int target_id;
    printf("\nInserisci l'ID del report da modificare: ");
    if (scanf("%u", &target_id) != 1) {
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    
    char line_buffer[REPORT_MASTER_LINE + 1];
    bool is_found = false;
    unsigned int current_bench_count = read_system_variable(REG_IDX_COUNTER_BENCH);
    
    /* 1. RICERCA ORIZZONTALE IN CACHE OPERATIVA (BENCH) IN O(n) */
    FILE* f_bench = fopen(PATH_BENCH, "rb+");
    if (f_bench) {
        for (unsigned int i = 0; i < current_bench_count; i++) {
            fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET);
            if (fread(line_buffer, sizeof(char), REPORT_BENCH_LINE, f_bench) != REPORT_BENCH_LINE) continue;
            
            char cell_state;
            Report r = line_to_report_v2(line_buffer, &cell_state);
            
            if (r && get_report_id(r) == target_id && cell_state == 'A') {
                is_found = true;
                
                /* CONTROLLO DESTROYED PREVENTIVO: Impedisce la visualizzazione se gia eliminato logicamente */
                if (get_report_status(r) == DESTROYED) {
                    printf("\n[AVVISO] Segnalazione non trovata (Stato logico: DESTROYED).\n");
                    free_report(r);
                    fclose(f_bench);
                    return;
                }
                
                /* MOSTRA IL REPORT A VIDEO PRIMA DELLA RICHIESTA DI AGGIORNAMENTO */
                printf("\n[TROVATO IN CACHE BENCH]\n");
                printf("ID: %05u | Citizen: %s | Cat: %s\nDesc: %s\nDate: %s\nUrgency: %c\nStatus: %s\n",
                       get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)),
                       get_report_description(r), get_report_date(r), get_report_urgency(r), get_status_string(get_report_status(r)));
                       
                int new_status_input;
                printf("\nScegli nuovo stato (0=OPEN, 1=IN_PROGRESS, 2=CLOSED, 3=DESTROYED): ");
                if (scanf("%d", &new_status_input) == 1) {
                    while (getchar() != '\n');
                    
                    ReportStatus old_st = get_report_status(r);
                    ReportStatus new_st_enum = (ReportStatus)new_status_input;
                    
                    if (old_st != new_st_enum) {
                        /* Invocazione della funzione locale di utilita per aggiornare i registri O(1) */
                        update_system_counters(old_st, new_st_enum, r);
                    }
                    
                    update_report_status(r, new_st_enum);
                    char out_line[REPORT_BENCH_LINE + 1];
                    report_to_line(out_line, r, '\0');
                    fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET);
                    fwrite(out_line, sizeof(char), REPORT_BENCH_LINE, f_bench);
                    printf("\n[OK] Stato modificato con successo all'interno della BENCH.\n");
                } else {
                    while (getchar() != '\n');
                }
                free_report(r);
                break;
            }
            if (r) free_report(r);
        }
        fclose(f_bench);
    }
    
    /* 2. INTERROGAZIONE DIRETTA DEL FILE D'INDICE DEL DISCO IN O(log n) CON LA NUOVA FIRMA */
    if (!is_found) {
        /* Chiamata sanificata: findReportId legge direttamente l'array inorder nel file di testo */
        int found_master_row = findReportId(target_id);
        
        if (found_master_row != -1) {
            /* Per determinare lo stato d'origine, esaminiamo i canali dei tre file master */
            const char* master_channels[] = { PATH_OPEN_MASTER, PATH_PROGRESS_MASTER, PATH_CLOSED_MASTER };
            const char* target_holes_channels[] = { PATH_OPEN_HOLES, PATH_PROGRESS_HOLES, PATH_CLOSED_HOLES };
            ReportStatus channel_status[] = { OPEN, IN_PROGRESS, CLOSED };
            
            for (int ch = 0; ch < 3; ch++) {
                FILE* f_m = fopen(master_channels[ch], "rb");
                if (f_m) {
                    fseek(f_m, (long)found_master_row * REPORT_MASTER_LINE, SEEK_SET);
                    if (fread(line_buffer, sizeof(char), REPORT_MASTER_LINE, f_m) == REPORT_MASTER_LINE) {
                        char cell_state;
                        Report r = line_to_report_v2(line_buffer, &cell_state);
                        
                        if (r && get_report_id(r) == target_id && cell_state == 'A') {
                            is_found = true;
                            
                            if (get_report_status(r) == DESTROYED) {
                                printf("\n[AVVISO] Segnalazione non trovata (Stato logico: DESTROYED).\n");
                                free_report(r); fclose(f_m); return;
                            }
                            
                            /* MOSTRA IL REPORT A VIDEO PRIMA DELLA RICHIESTA DI AGGIORNAMENTO */
                            printf("\n[TROVATO IN ARCHIVIO MASTER DI VERITA']\n");
                            printf("ID: %05u | Citizen: %s | Cat: %s\nDesc: %s\nDate: %s\nUrgency: %c\nStatus: %s\n",
                                   get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)),
                                   get_report_description(r), get_report_date(r), get_report_urgency(r), get_status_string(get_report_status(r)));
                            
                            int new_status_input;
                            printf("\nScegli nuovo stato (0=OPEN, 1=IN_PROGRESS, 2=CLOSED, 3=DESTROYED): ");
                            if (scanf("%d", &new_status_input) == 1) {
                                while (getchar() != '\n');
                                ReportStatus new_st_enum = (ReportStatus)new_status_input;
                                
                                /* INVALIDAZIONE GEOMETRICA CELLA MASTER: Scrive 'N' al byte 350 */
                                FILE* f_inv = fopen(master_channels[ch], "rb+");
                                if (f_inv) {
                                    fseek(f_inv, (long)found_master_row * REPORT_MASTER_LINE, SEEK_SET);
                                    char clear_buf[REPORT_MASTER_LINE + 1];
                                    if (fread(clear_buf, sizeof(char), REPORT_MASTER_LINE, f_inv) == REPORT_MASTER_LINE) {
                                        clear_buf[350] = 'N';
                                        fseek(f_inv, (long)found_master_row * REPORT_MASTER_LINE, SEEK_SET);
                                        fwrite(clear_buf, sizeof(char), REPORT_MASTER_LINE, f_inv);
                                    }
                                    fclose(f_inv);
                                }
                                
                                /* ACCATASTAMENTO LIFO: Spinge la riga libera nello stack dei buchi del master d'origine */
                                FILE* f_h = fopen(target_holes_channels[ch], "ab");
                                if (f_h) { fprintf(f_h, "%010d\n", found_master_row); fclose(f_h); }
                                
                                /* Aggiornamento centralizzato dei contatori statistici */
                                update_system_counters(channel_status[ch], new_st_enum, r);
                                
                                /* CARICAMENTO NELLA BENCH TRAMITE APPEND CON IL NUOVO STATO */
                                update_report_status(r, new_st_enum);
                                set_report_disk_row(r, -1); /* Perde l'associazione riga master d'origine */
                                
                                if (current_bench_count >= LIMIT_BENCH) {
                                    process_and_flush_bench_v2();
                                    current_bench_count = 0;
                                }
                                
                                FILE* f_b_add = fopen(PATH_BENCH, "rb+");
                                if (f_b_add) {
                                    char bench_out[REPORT_BENCH_LINE + 1];
                                    report_to_line(bench_out, r, '\0');
                                    fseek(f_b_add, (long)current_bench_count * REPORT_BENCH_LINE, SEEK_SET);
                                    fwrite(bench_out, sizeof(char), REPORT_BENCH_LINE, f_b_add);
                                    fclose(f_b_add);
                                    /* Incrementa il counter della BENCH ma NON i report totali del sistema */
                                    write_system_variable(REG_IDX_COUNTER_BENCH, current_bench_count + 1);
                                }
                                printf("\n[OK] Record modificato, invalidata riga master %d e caricata la modifica nella BENCH.\n", found_master_row);
                            } else {
                                while (getchar() != '\n');
                            }
                            free_report(r);
                            fclose(f_m);
                            break;
                        }
                        if (r) free_report(r);
                    }
                    fclose(f_m);
                }
                if (is_found) break;
            }
        }
    }
    
    if (!is_found) {
        printf("\n[ERRORE] Impossibile trovare la segnalazione con ID %05u nel sistema comunale.\n", target_id);
    }
}


// ==============================================================================
//  FUNZIONI HELPER DI VISUALIZZAZIONE 
// ==============================================================================

/**
 * @brief Visualizza i report in sequenza filtrandoli tramite uno specifico parametro di stato.
 *        Legge direttamente dal rispettivo file Master e integra i dati della cache attiva provenienti dal BENCH.
 * @param required_status Lo stato da filtrare (OPEN, IN_PROGRESS, CLOSED).
 */  
void show_reports_by_status(ReportStatus required_status) {
    char line_buffer[REPORT_MASTER_LINE + 1];
    int counter = 0;
    int input_pagination;
    const char* target_master_path;

    /* 1. SELEZIONE DEL PERCORSO MASTER PERSISTENTE IN BASE AL PARAMETRO */
    if (required_status == OPEN) {
        target_master_path = PATH_OPEN_MASTER;
    } else if (required_status == IN_PROGRESS) {
        target_master_path = PATH_PROGRESS_MASTER;
    } else if (required_status == CLOSED) {
        target_master_path = PATH_CLOSED_MASTER;
    } else {
        printf("\n[ERRORE] Stato non valido per la visualizzazione.\n");
        return;
    }

    printf("\n--- ARCHIVIO COMUNALE CONSOLIDATO (MASTER DISK) ---\n");
    FILE* f_master = fopen(target_master_path, "rb");
    
    /* 2. SCANSIONE DELL'ARCHIVIO PERSISTENTE A PASSI FISSI DI 352 BYTE */
    if (f_master) {
        while (fread(line_buffer, sizeof(char), REPORT_MASTER_LINE, f_master) == REPORT_MASTER_LINE) {
            /* SALTO DEGLI SLOT RECORD INVALIDATI (BUCHI) MARCATI COME NULL */
            if (line_buffer[350] == 'N') continue; 
            /* INTERCETTAZIONE DEL FLAG SENTINELLA PER INTERROMPERE LE OPERAZIONI DI I/O BINARIO */
            if (line_buffer[350] == 'E') break;
            
            char cell_state; 
            Report r = line_to_report_v2(line_buffer, &cell_state);
            
            /* VERIFICA RIGIDA: CORRISPONDENZA DEL CAMPO E CELLA ATTIVA ('A') */
            if (r && cell_state == 'A' && get_report_status(r) == required_status) {
                printf("[DISCO] ID: %05u | Citizen: %s | Cat: %s\nDesc: %s\n", 
                       get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)), get_report_description(r));
                printf("------------------------------------------------------\n");
                counter++;

                /* MECCANISMO DI PAGINAZIONE INTERATTIVA OGNI 5 ELEMENTI VISUALIZZATI */
                if (counter % 5 == 0) {
                    printf("Premi [ENTER] per continuare oppure 'q' per uscire: ");
                    input_pagination = getchar();

                    if (input_pagination == 'q' || input_pagination == 'Q') {
                        free_report(r);
                        fclose(f_master);
                             return;
                    }
                 while (getchar() != '\n');
                }
            }
            if (r) free_report(r);
        }
        fclose(f_master);
    } else {
        printf("Nessun file database consolidato trovato per questo stato.\n");
    }

    /* 3. SCANSIONE ORIZZONTALE DELLA CACHE TRANSITORIA (FILE BENCH) IN O(n) */
    printf("\n--- CACHE OPERATIVA TRANSITORIA (BENCH FILE) ---\n");
    unsigned int current_bench_count = read_system_variable(REG_IDX_COUNTER_BENCH);
    FILE* f_bench = fopen(PATH_BENCH, "rb");
    
    if (f_bench) {
        /* SCANSIONE PROTETTA LIMITATA STRETTAMENTE DAL CONTATORE ATTIVO */
        for (unsigned int i = 0; i < current_bench_count; i++) {
            fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET); /* Limitato a 351 byte */
            if (fread(line_buffer, sizeof(char), REPORT_BENCH_LINE, f_bench) == REPORT_BENCH_LINE) {
                char cell_state; 
                Report r = line_to_report_v2(line_buffer, &cell_state);
                
                /* VERIFICA RIGIDA: DEVE CORRISPONDERE ALLO STATO RICHIESTO ED ESSERE ATTIVO, ESCLUDENDO I RECORD LOGICAMENTE DISTRUTTI */
                if (r && get_report_status(r) == required_status && get_report_status(r) != DESTROYED && cell_state == 'A') {
                    printf("[CACHE] ID: %05u | Citizen: %s | Cat: %s\nDesc: %s\n", 
                           get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)), get_report_description(r));
                    printf("------------------------------------------------------\n");
                    counter++;

                    if (counter % 5 == 0) {
                        printf("Premi [ENTER] per caricare altri elementi oppure 'q' per uscire: ");
                        input_pagination = getchar();
                        if (input_pagination == 'q' || input_pagination == 'Q') { 
                            free_report(r); 
                            fclose(f_bench); 
                            if (input_pagination != '\n') while (getchar() != '\n');
                            return; 
                        }
                        if (input_pagination != '\n') {
                            while (getchar() != '\n');
                        }
                    }
                }
                if (r) free_report(r);
            }
        }
        fclose(f_bench);
    }

    printf("\nFine della visualizzazione archivio. %d report attivi corrispondenti ai criteri sono stati caricati.\n", counter);
    printf("Digita un qualsiasi carattere e premi [ENTER] per tornare al menu: ");
    while (getchar() != '\n'); /* Protegge lo stream del terminale per i successivi input del menu */
}


/**
 * @brief Compila e visualizza i report attivi ordinati per priorità.
 *        Prioritizza le urgenze in ordine decrescente e utilizza un ordine temporale FIFO per livelli di urgenza equivalenti.
 */
void show_priority_queue(void) {
    /* Inizializzazione dell'ADT Coda a Priorita tramite puntatore opaco ad Information Hiding */
    PriorityQueue priority_queue = create_pq();
    
    /* Vettore statico contenente i percorsi dei soli file Master operativi attivi */
    const char* active_master_channels[] = { PATH_OPEN_MASTER, PATH_PROGRESS_MASTER };
    char line_buffer[REPORT_MASTER_LINE + 1];
    
    /* 1. SCANSIONE GEOMETRICA ESTRATTIVA DAI FILE MASTER */
    for (int i = 0; i < 2; i++) {
        FILE* f_master = fopen(active_master_channels[i], "rb");
        if (f_master) {
            /* Lettura continua a blocchi prefissati di 352 byte per preservare l'allineamento */
            while (fread(line_buffer, sizeof(char), REPORT_MASTER_LINE, f_master) == REPORT_MASTER_LINE) {
                /* Esclude geometricamente i buchi generati da precedenti sfoltimenti del dipendente */
                if (line_buffer[350] == 'N') continue;
                /* Intercetta la sentinella logica interrompendo immediatamente l'I/O su questo canale */
                if (line_buffer[350] == 'E') break;
                
                char cell_state; 
                Report r = line_to_report_v2(line_buffer, &cell_state);
                
                /* Il record viene accodato solo se attivo ('A') e se non si tratta di un caso DESTROYED */
                if (r && cell_state == 'A' && get_report_status(r) != DESTROYED) {
                    /* Inserimento ordinato all'interno della coda (Gestito in O(n) lineare in RAM) */
                    pq_enqueue(priority_queue, r);
                } else if (r) {
                    free_report(r); /* Deallocazione di sicurezza se il record viola i filtri */
                }
            }
            fclose(f_master);
        }
    }

    int display_counter = 0;
    int input_pagination;
    printf("\n--- OPERATIONAL PRIORITIES TIMELINE (CRONOLOGIA OPERATIVA CRITICA) ---\n");
    
    /* 2. SVUOTAMENTO LIFO/PRIORITY E RENDERING INTERATTIVO A VIDEO CON PAGINAZIONE */
    while (!pq_is_empty(priority_queue)) {
        /* Estrazione del report a massima priorita assoluta */
        Report priority_report = pq_dequeue(priority_queue);
        
        printf("[PRIORITA'] ID: %05u | Urgenza: %c | Data: %s | Categoria: %s\n", 
               get_report_id(priority_report), get_report_urgency(priority_report), 
               get_report_date(priority_report), get_category_string(get_report_category(priority_report)));
        printf("            Descrizione: %s\n", get_report_description(priority_report));
        printf("------------------------------------------------------\n");
        display_counter++;
        free_report(priority_report); /* Deallocazione sicura del nodo rimosso dalla coda */
        
        /* Blocco interattivo dell'output ogni 5 elementi mostrati per non saturare la console */
        if (display_counter % 5 == 0 && !pq_is_empty(priority_queue)) {
            printf("Press [ENTER] to view next critical items or 'q' to stop: ");
            input_pagination = getchar();
            
            /* Gestione interruzione controllata dell'output */
            if (input_pagination == 'q' || input_pagination == 'Q') { 
                free_pq(priority_queue); /* Distrugge i nodi rimanenti per azzerare i memory leak */
                if (input_pagination != '\n') while (getchar() != '\n');
                return; 
            }
            /* Pulizia standard del newline associato al tasto INVIO */
            if (input_pagination != '\n') while (getchar() != '\n');
        }
    }
    
    /* Liberazione finale della struttura della coda */
    free_pq(priority_queue);
    
    printf("\nFine della coda delle priorita. %d segnalazioni critiche esaminate.\n", display_counter);
    printf("Type any character and press [ENTER] to go back to the menu: ");
    while (getchar() != '\n'); /* Svuota e protegge il flusso per il menu dipendente */
}

/**
 * @brief Esegue una navigazione simmetrica attraverso i file indice su disco.
 *        Triangola ogni record utente con l'indice dei report per recuperare i dati dai file Master.
 */
void show_triangulated_user_avl(void) {
    /* Apertura in lettura binaria dell'indice Utente generato dall'AVL (Passo 21 byte) */
    FILE* f_avl_u = fopen(PATH_AVL_USER_ID, "rb");
    if (!f_avl_u) { 
        printf("Indice AVL utenti vuoto o non inizializzato.\n"); 
        return; 
    }
    
    char user_idx_line[AVL_USER_ID_LINE + 1];
    char report_idx_line[AVL_REPORT_ID_LINE + 1];
    char master_line[REPORT_MASTER_LINE + 1];
    int counter = 0;
    
    printf("\n--- NAVIGAZIONE SIMMETRICA AVL UTENTE TRIANGOLATO ---\n");
    
    /* 1. SCANSIONE DELL'INDICE UTENTE SUL DISCO A BLOCCHI FISSI DI 21 BYTE */
    while (fread(user_idx_line, sizeof(char), AVL_USER_ID_LINE, f_avl_u) == AVL_USER_ID_LINE) {
        user_idx_line[AVL_USER_ID_LINE] = '\0';
        
        char raw_uid[11] = {0};
        char raw_rid[11] = {0};
        memcpy(raw_uid, user_idx_line, 10);
        memcpy(raw_rid, user_idx_line + 10, 10);
        
        unsigned int read_uid = (unsigned int)strtoul(raw_uid, NULL, 10);
        unsigned int read_rid = (unsigned int)strtoul(raw_rid, NULL, 10);
        
        /* 2. TRIANGOLAZIONE LOGARITMICA SULL'INDICE DEI REPORT A BLOCCHI FISSI DI 22 BYTE */
        FILE* f_avl_r = fopen(PATH_AVL_REPORT_ID, "rb");
        int found_master_row = -1; 
        char status_indicator_char = '0';
        
        if (f_avl_r) {
            /* Scansione del file d'indice dei report senza spazi intermedi */
            while (fread(report_idx_line, sizeof(char), AVL_REPORT_ID_LINE, f_avl_r) == AVL_REPORT_ID_LINE) {
                report_idx_line[AVL_REPORT_ID_LINE] = '\0';
                
                char raw_check_rid[11] = {0};
                memcpy(raw_check_rid, report_idx_line, 10);
                unsigned int check_rid = (unsigned int)strtoul(raw_check_rid, NULL, 10);
                
                /* Se l'ID del report coincide, estraiamo lo stato e la riga fisica */
                if (check_rid == read_rid) {
                    status_indicator_char = report_idx_line[10];
                    char raw_row[11] = {0};
                    memcpy(raw_row, report_idx_line + 11, 10);
                    found_master_row = atoi(raw_row);
                    break;
                }
            }
            fclose(f_avl_r);
        }
        
        /* 3. ACCESSO DIRETTO O(1) NEL RISPETTIVO FILE MASTER DI VERITÀ */
        if (found_master_row != -1) {
            ReportStatus master_file_status = (ReportStatus)(status_indicator_char - '0');
            const char* path_master = (master_file_status == OPEN) ? PATH_OPEN_MASTER : 
                                      (master_file_status == IN_PROGRESS) ? PATH_PROGRESS_MASTER : PATH_CLOSED_MASTER;
                                      
            FILE* f_m = fopen(path_master, "rb");
            if (f_m) {
                fseek(f_m, (long)found_master_row * REPORT_MASTER_LINE, SEEK_SET); /* Salto a passo 352 byte */
                if (fread(master_line, sizeof(char), REPORT_MASTER_LINE, f_m) == REPORT_MASTER_LINE) {
                    /* CONTROLLO CELLA ACTIVE: Il record viene stampato solo se il flag al byte 350 è 'A' */
                    if (master_line[350] == 'A') {
                        char cell_st; 
                        Report r = line_to_report_v2(master_line, &cell_st);
                        
                        /* Esclude categoricamente la stampa a video delle varianti logiche DESTROYED */
                        if (r && get_report_status(r) != DESTROYED) {
                            printf("[CHIAVE AVL USER: %05u] -> ID Report: %05u | Categoria: %s | Stato: %s\n", 
                                   read_uid, get_report_id(r), get_category_string(get_report_category(r)), get_status_string(get_report_status(r)));
                            counter++;
                        }
                        if (r) free_report(r);
                    }
                }
                fclose(f_m);
            }
        }
    }
    fclose(f_avl_u);
    
    printf("\nFine dell'albero. %d corrispondenze totali caricate dal server.\n", counter);
    printf("Premi un carattere qualsiasi e premi [INVIO] per tornare al menu: ");
    while (getchar() != '\n'); /* Consuma in sicurezza e sblocca il flusso per il menu dipendente */
}

/**
 * @brief Genera e visualizza la dashboard operativa comunale in tempo O(1).
 *        Legge le metriche analitiche pre-calcolate direttamente dal registro di sistema a posizione fissa.
 */
void generate_municipal_statistics(void) {
    printf("\n===================================================\n");
    printf("     REPORT STATISTICO ISTANTANEO O(1) REGISTRI    \n");
    printf("===================================================\n");

    /* 1. ESTRAZIONE DEL CONTATORE GENERALE DELLE SEGNALAZIONI ATTIVE */
    printf("Numero totale di segnalazioni attive nel Comune: %u\n", 
           read_system_variable(REG_IDX_NM_REPORT));
    
    /* 2. ESTRAZIONE DEGLI INDICATORI DI STATO PRATICA */
    printf("Ripartizione analitica per STATO DELLA PRATICA:\n");
    printf("  - Casi Aperti (OPEN):          %u\n", read_system_variable(REG_IDX_STAT_OPEN));
    printf("  - In Lavorazione (PROGRESS):   %u\n", read_system_variable(REG_IDX_STAT_PROGRESS));
    printf("  - Pratiche Chiuse (CLOSED):    %u\n", read_system_variable(REG_IDX_STAT_CLOSED));
    
    /* 3. ESTRAZIONE DEGLI INDICATORI DEL VOLUME DELLE CATEGORIE */
    printf("\nSuddivisione analitica per CATEGORIA DI ANOMALIA:\n");
    printf("  - Buca Stradale:               %u\n", read_system_variable(REG_IDX_CAT_ROAD));
    printf("  - Illuminazione Pubblica:      %u\n", read_system_variable(REG_IDX_CAT_LIGHTING));
    printf("  - Rifiuti Abbandonati:         %u\n", read_system_variable(REG_IDX_CAT_WASTE));
    printf("  - Guasto Impianto Pubblico:    %u\n", read_system_variable(REG_IDX_CAT_INFRASTRUCT));
    printf("  - Altro / Generico:            %u\n", read_system_variable(REG_IDX_CAT_OTHER));
    
    /* 4. ESTRAZIONE DEI NUOVI METADATI DI METRICA DEL SISTEMA INDICI AVL (POSIZIONI 11 E 12) */
    printf("\nStato di allocazione e consistenza degli INDICI AVL SUL DISCO:\n");
    printf("  - Record indicizzati per Report ID (22 byte): %u\n", read_system_variable(REG_IDX_AVL_REP_COUNT));
    printf("  - Record indicizzati per User ID   (21 byte): %u\n", read_system_variable(REG_IDX_AVL_USR_COUNT));
    
    /* 5. METRICA DI OCCUPAZIONE DELLA CACHE TRANSIENT VELOCE */
    printf("\nSaturazione corrente della Cache Operativa Server:\n");
    printf("  - Slot occupati in BENCH (max 50):           %u / %d\n",read_system_variable(REG_IDX_COUNTER_BENCH), LIMIT_BENCH);
    printf("===================================================\n");

    printf("\nStatistiche della dashboard caricate correttamente.\n");
    printf("Digita un qualsiasi carattere e premi [ENTER] per tornare al menu: ");
    while (getchar() != '\n'); /* Svuota e protegge il flusso per il menu dipendente */
}


/**
 * @brief FASE 1: Scansiona l'inorder array dell'indice User ID tramite ricerca dicotomica su disco,
 *        recupera tutti i report_id dell'utente, triangola con l'indice Report ID e collega i record alla lista.
 * @param target_user_id L'ID numerico senza segno del cittadino loggato.
 * @param ram_list Il puntatore opaco alla lista RAM nativa in cui inserire i record storici.
 */
void load_master_reports_to_list(unsigned int target_user_id, ReportList ram_list) {
    /* Array statico locale di supporto temporaneo per raccogliere i report_id dell'utente (capacita abbondante) */
    unsigned int discovered_report_ids[100];
    char master_line_buffer[REPORT_MASTER_LINE + 1];

    /* 1. INVOCAZIONE DELLA NUOVA RICERCA BINARIA SU DISCO SENZA ALBERO IN MEMORIA */
    /* La funzione estrae i match, popola discovered_report_ids e ritorna il contatore esatto */
    int total_user_matches = findUserId(target_user_id, discovered_report_ids);
    
    /* Se l'utente non possiede alcuna segnalazione registrata negli archivi storici, esce subito */
    if (total_user_matches == 0) {
        return;
    }

    /* 2. ITERAZIONE E TRIANGOLAZIONE CHIRURGICA PER CIASCUN REPORT_ID TROVATO */
    for (int i = 0; i < total_user_matches; i++) {
        unsigned int current_rid = discovered_report_ids[i];

        /* Interroga l'indice dei Report su disco in O(log n) per ricavare lo stato e la disk_row fisica */
        int target_disk_row = findReportId(current_rid);

        /* Se l'indice restituisce una riga valida sul disco, procediamo alla localizzazione del canale */
        if (target_disk_row != -1) {
            const char* isolated_master_path = PATH_OPEN_MASTER;
            const char* master_channels[] = { PATH_OPEN_MASTER, PATH_PROGRESS_MASTER, PATH_CLOSED_MASTER };
            bool channel_found = false;

            /* Scansione protetta dei tre canali fisici per isolare il file di stato corretto del report */
            for (int ch = 0; ch < 3; ch++) {
                FILE* f_verify = fopen(master_channels[ch], "rb");
                if (f_verify) {
                    fseek(f_verify, (long)target_disk_row * REPORT_MASTER_LINE, SEEK_SET);
                    if (fread(master_line_buffer, sizeof(char), REPORT_MASTER_LINE, f_verify) == REPORT_MASTER_LINE) {
                        char cell_st;
                        Report r_check = line_to_report_v2(master_line_buffer, &cell_st);
                        /* Se l'ID coincide matematicamente, abbiamo catturato il file di provenienza */
                        if (r_check && get_report_id(r_check) == current_rid) {
                            isolated_master_path = master_channels[ch];
                            channel_found = true;
                            free_report(r_check);
                            fclose(f_verify);
                            break;
                        }
                        if (r_check) free_report(r_check);
                    }
                    fclose(f_verify);
                }
            }

            /* Se il canale è confermato, effettua la lettura finale e l'iniezione condizionata */
            if (channel_found) {
                FILE* f_master = fopen(isolated_master_path, "rb");
                if (f_master) {
                    fseek(f_master, (long)target_disk_row * REPORT_MASTER_LINE, SEEK_SET);
                    if (fread(master_line_buffer, sizeof(char), REPORT_MASTER_LINE, f_master) == REPORT_MASTER_LINE) {
                        
                        /* CONTROLLO CELLA ACTIVE: Il record viene inserito solo se il flag al byte 350 è 'A' */
                        if (master_line_buffer[350] == 'A') {
                            char cell_st;
                            Report r_master = line_to_report_v2(master_line_buffer, &cell_st);
                            if (r_master) {
                                /* LINKING DIRETTO ALLA RAM LIST SENZA STRUTTURE DI APPOGGIO ESTERNE */
                                list_insert(ram_list, r_master);
                            }
                        }
                    }
                    fclose(f_master);
                }
            }
        }
    }
}


/**
 * @brief FASE 2: Scansiona la cache BENCH a passi fissi da 351 byte.
 *        Esegue l'elisione dei duplicati e aggiorna la lista con le varianti transitorie.
 */
void load_bench_reports_to_list(unsigned int target_user_id, ReportList ram_list) {
    char bench_line[REPORT_BENCH_LINE + 1];
    unsigned int current_bench = read_system_variable(REG_IDX_COUNTER_BENCH);

    FILE* f_b = fopen(PATH_BENCH, "rb");
    if (!f_b) return;

    /* Scansione orizzontale limitata al contatore corrente degli elementi in cache */
    for (unsigned int i = 0; i < current_bench; i++) {
        fseek(f_b, (long)i * REPORT_BENCH_LINE, SEEK_SET);
        if (fread(bench_line, sizeof(char), REPORT_BENCH_LINE, f_b) == REPORT_BENCH_LINE) {
            char state;
            Report tmp = line_to_report_v2(bench_line, &state);
            
            /* Verifica di titolarità e controllo della cella attiva all'interno della BENCH */
            if (tmp && get_report_user_id(tmp) == target_user_id && state == 'A') {
                /* ELISIONE DUPLICATI: Rimuove il vecchio record inserito dalla Fase Master */
                list_remove(ram_list, get_report_id(tmp));
                
                /* Collega la variante BENCH solo se lo stato non è DESTROYED */
                if (get_report_status(tmp) != DESTROYED) {
                    list_insert(ram_list, tmp);
                } else {
                    free_report(tmp); /* Rimozione ed elisione fisica */
                }
            } else if (tmp) {
                free_report(tmp);
            }
        }
    }
    fclose(f_b);
}


/* --------------------------------==============================================
 *  CALLBACK DI CONFRONTO PER GLI INSERIMENTI NEGLI INDICI AVL
 * --------------------------------============================================== */
 /*
static int compare_uid(const void* a, const void* b) {
    char tmp_a[11] = {0}; char tmp_b[11] = {0};
    memcpy(tmp_a, (const char*)a, 10); memcpy(tmp_b, (const char*)b, 10);
    unsigned int id_a = (unsigned int)strtoul(tmp_a, NULL, 10);
    unsigned int id_b = (unsigned int)strtoul(tmp_b, NULL, 10);
    return (id_a < id_b) ? -1 : (id_a > id_b) ? 1 : 0;
}

static int compare_rid(const void* a, const void* b) {
    char tmp_a[11] = {0}; char tmp_b[11] = {0};
    memcpy(tmp_a, (const char*)a, 10); memcpy(tmp_b, (const char*)b, 10);
    unsigned int id_a = (unsigned int)strtoul(tmp_a, NULL, 10);
    unsigned int id_b = (unsigned int)strtoul(tmp_b, NULL, 10);
    return (id_a < id_b) ? -1 : (id_a > id_b) ? 1 : 0;
}
*/
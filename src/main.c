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

void menu_cittadino(User logged_in_user);
void menu_dipendente(User logged_in_user);
//void mostra_segnalazioni_paginate(const char* file_path);
void esegui_casi_test();
void genera_statistiche_comunali();
void mostra_segnalazioni_paginate_filtrate(const char* file_path, ReportStatus stato_richiesto);
void mostra_priority_queue_binaria(const char* file_path);
void mostra_bst_utente_triangolato(void);
int main(void) {
    int scelta;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    printf("===================================================\n");
    printf("     SISTEMA DI SEGNALAZIONI MUNICIPALI BARONISSI   \n");
    printf("===================================================\n");

    while (1) {
        printf("\n--- MENU PRINCIPALE ---\n");
        printf("1. Accedi (Login)\n");
        printf("2. Registrati (Nuovo Utente)\n");
        printf("3. Esegui Casi di Test Automatizzati\n");
        printf("4. Esci dal Programma\n");
        printf("Seleziona un'opzione: ");
        if (scanf("%d", &scelta) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); 

        switch (scelta) {
            case 1:
                printf("\n--- ACCESSO AL SISTEMA ---\n");
                printf("Username: ");
                if (!fgets(username, sizeof(username), stdin)) break;
                trim_string(username);
                printf("Password: ");
                if (!fgets(password, sizeof(password), stdin)) break;
                trim_string(password);

                User u = login_user(username, password);
                if (u != NULL) {
                    printf("\n[OK] Autenticazione riuscita! Benvenuto %s.\n", get_user_username(u));
                    if (get_user_role(u) == EMPLOYEE) menu_dipendente(u);
                    else menu_cittadino(u);
                    free_user(u); 
                } else {
                    printf("\n[ERRORE] Credenziali errate o utente non trovato.\n");
                }
                break;

            case 2:
                printf("\n--- REGISTRAZIONE UTENTE ---\n");
                printf("Scegli un Username: ");
                if (!fgets(username, sizeof(username), stdin)) break;
                trim_string(username);
                printf("Scegli una Password: ");
                if (!fgets(password, sizeof(password), stdin)) break;
                trim_string(password);

                int ruolo_scelta;
                printf("Seleziona il ruolo (0 = Cittadino, 1 = Dipendente Comunale): ");
                if (scanf("%d", &ruolo_scelta) != 1) {
                    while (getchar() != '\n');
                    printf("[ERRORE] Input non valido.\n");
                    break;
                }
                while (getchar() != '\n');

                UserRole r = (ruolo_scelta == 1) ? EMPLOYEE : CITIZEN;
                if (register_user(username, password, r)) {
                    printf("\n[OK] Registrazione completata con successo!\n");
                } else {
                    printf("\n[ERRORE] Registrazione fallita. Username gia' in uso.\n");
                }
                break;

            case 3:
                esegui_casi_test();
                break;

            case 4:
                printf("\nGrazie per aver utilizzato il sistema municipale. Arrivecerci!\n");
                return 0;

            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
    return 0;
}

void menu_cittadino(User logged_in_user) {
    ReportList ram_list = create_list();
    ReportStack revert_stack = create_stack();
    int scelta, urgenza;
    char desc_str[MAX_DESC];
    char data_str[11];

    while (1) {
        printf("\n--- AREA CITTADINO (%s) ---\n", get_user_username(logged_in_user));
        printf("1. Inserisci Nuova Segnalazione (In RAM)\n");
        printf("2. Visualizza lo Storico delle mie Segnalazioni\n");
        printf("3. Modifica una Segnalazione (In RAM)\n");
        printf("4. Annulla Ultima Modifica (Revert/Undo Stack)\n");
        printf("5. Esci ed Invia Segnalazioni al Comune (Logout & Flush)\n");
        printf("Seleziona un'opzione: ");
        if (scanf("%d", &scelta) != 1) { while (getchar() != '\n'); continue; }
        while (getchar() != '\n');

        switch (scelta) {
            case 1:
                printf("\n--- COMPILA SEGNALAZIONE ---\n");
                int cat_scelta;
                printf("Categoria (0=Buca, 1=Illuminazione, 2=Rifiuti, 3=Impianto, 4=Altro): ");
                if (scanf("%d", &cat_scelta) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                printf("Descrizione del problema: ");
                if (!fgets(desc_str, sizeof(desc_str), stdin)) break;
                trim_string(desc_str);

                // Rimozione esplicita del newline di fgets se rimasto intrappolato nella stringa
                int len_d = strlen(desc_str);
                if (len_d > 0 && desc_str[len_d - 1] == '\n') {
                desc_str[len_d - 1] = '\0';
                }

                printf("Data di oggi (GG/MM/AAAA): ");
                if (!fgets(data_str, sizeof(data_str), stdin)) break;
                trim_string(data_str);

                printf("Livello di Urgenza (1=Bassa, 2=Media, 3=Alta): ");
                if (scanf("%d", &urgenza) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                if (!validate_not_empty(desc_str) || !validate_date_format(data_str) || !validate_urgency_range(urgenza)) {
                    printf("\n[ERRORE] Dati inseriti non validi o non conformi.\n");
                    break;
                }

                int global_id = generate_global_report_id();
                Report new_r = create_report(global_id, get_user_username(logged_in_user), (ReportCategory)cat_scelta, desc_str, data_str, urgenza);
                list_insert(ram_list, new_r);
                printf("\n[OK] Segnalazione inserita nella sessione locale RAM (Codice: %05d).\n", global_id);
                break;

            case 2:
                printf("\n===================================================\n");
                printf("      STORICO PERSONALE DEL CITTADINO   \n");
                printf("===================================================\n");
                int cittadino_counter = 1;
                char bst_u_line[14]; // Buffer per la riga ridotta da 12 byte

                // 1. Stampa i record volatili presenti nella sessione RAM corrente
                list_rewind(ram_list);
                Report r_ram = list_next(ram_list);
                while (r_ram != NULL) {
                    printf("[ RAM LOCAL ] Codice: %05d | Categoria: %s\n", get_report_id(r_ram), get_category_string(get_report_category(r_ram)));
                    printf("              Urgenza: %d | Stato: %s | Desc: %s\n", get_report_urgency(r_ram), get_status_string(get_report_status(r_ram)), get_report_description(r_ram));
                    printf("------------------------------------------------------\n");
                    cittadino_counter++;
                    r_ram = list_next(ram_list);
                }

                // 2. Calcolo dell'ID hash numerico del cittadino loggato per la ricerca ad albero
                unsigned long hash_cittadino = 5381;
                const char* username_ptr = get_user_username(logged_in_user);
                int ch;
                while ((ch = (unsigned char)*username_ptr++)) hash_cittadino = ((hash_cittadino << 5) + hash_cittadino) + ch;
                int target_user_id = (int)(hash_cittadino % 100000);

                // 3. Apertura binarizzata dell'indice ridotto da 12 byte
                FILE* f_bst_u = fopen(PATH_BST_USER_ID, "rb");
                if (f_bst_u) {
                    /* Legge i blocchi da 12 byte sequenziali per estrarre le corrispondenze */
                    while (fread(bst_u_line, sizeof(char), 12, f_bst_u) == 12) {
                        bst_u_line[12] = '\0';
                        int read_user_id, read_report_id;
                        // Funzione esterna del parser caricata in precedenza
                        void line_to_user_index(const char*, int*, int*);
                        line_to_user_index(bst_u_line, &read_user_id, &read_report_id);

                        // Se l'ID utente corrisponde, abbiamo trovato un codice report associato al cittadino
                        if (read_user_id == target_user_id) {
                            
                            // TRIANGOLAZIONE CHIRURGICA: Risolviamo l'ID sul Punto di Verità (bst_by_report_id)
                            FILE* f_verify_rep = fopen(PATH_BST_REPORT_ID, "rb");
                            if (f_verify_rep) {
                                char main_rep_line[REPORT_LINE_TOTAL + 1];
                                while (fread(main_rep_line, sizeof(char), REPORT_LINE_TOTAL, f_verify_rep) == REPORT_LINE_TOTAL) {
                                    main_rep_line[REPORT_LINE_TOTAL] = '\0';
                                    char s_flag; int r_row;
                                    Report r_real = line_to_report(main_rep_line, &s_flag, &r_row);
                                    
                                    // Stampiamo il record con lo stato aggiornato in tempo reale dal dipendente
                                    if (r_real && get_report_id(r_real) == read_report_id && s_flag == 'A') {
                                        printf("[ ARCHIVIO ] Codice: %05d | Categoria: %s\n", get_report_id(r_real), get_category_string(get_report_category(r_real)));
                                        printf("             Urgenza: %d | Stato: %s\n", get_report_urgency(r_real), get_status_string(get_report_status(r_real)));
                                        printf("             Descrizione: %s\n", get_report_description(r_real));
                                        printf("------------------------------------------------------\n");
                                        cittadino_counter++;
                                        free_report(r_real);
                                        break;
                                    }
                                    if (r_real) free_report(r_real);
                                }
                                fclose(f_verify_rep);
                            }
                        }
                    }
                    fclose(f_bst_u);
                }
                if (cittadino_counter == 1) printf("\nNon ci sono segnalazioni storiche associate al tuo account.\n");
                break;

            case 3:
                printf("\nCodice della segnalazione da modificare (in RAM): ");
                int target_id;
                if (scanf("%d", &target_id) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                Report r_mod = list_find(ram_list, target_id);
                if (r_mod != NULL && get_report_status(r_mod) == OPEN) {
                    stack_push(revert_stack, r_mod);
                    printf("Nuova descrizione: ");
                    if (!fgets(desc_str, sizeof(desc_str), stdin)) break;
                    trim_string(desc_str);
                    
                    Report clonizzato = create_report(get_report_id(r_mod), get_report_citizen_name(r_mod), get_report_category(r_mod), desc_str, get_report_date(r_mod), get_report_urgency(r_mod));
                    list_remove(ram_list, target_id);
                    list_insert(ram_list, clonizzato);
                    printf("\n[OK] Segnalazione aggiornata in RAM.\n");
                } else {
                    printf("\n[ERRORE] Segnalazione non trovata o non modificabile.\n");
                }
                break;

            case 4:
                if (!stack_is_empty(revert_stack)) {
                    Report vecchio_stato = stack_pop(revert_stack);
                    int old_id = get_report_id(vecchio_stato);
                    list_remove(ram_list, old_id);
                    list_insert(ram_list, vecchio_stato);
                    printf("\n[OK] Azione annullata per il report %05d.\n", old_id);
                } else {
                    printf("\n[AVVISO] Nessuna azione da annullare nello Stack.\n");
                }
                break;

            case 5:
                printf("\nSalvataggio e sincronizzazione in corso...\n");
                flush_session_to_bench(ram_list);
                free_list(ram_list);
                free_stack(revert_stack);
                return;

            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
}

void menu_dipendente(User logged_in_user) {
    int scelta, rep_id, new_st;
    while (1) {
        printf("\n--- AREA DIPENDENTE (%s) ---\n", get_user_username(logged_in_user));
        printf("1. Visualizza Segnalazioni APERTE\n");
        printf("2. Visualizza Segnalazioni IN LAVORAZIONE\n");
        printf("3. Visualizza Segnalazioni CHIUSE\n");
        printf("4. Modifica Stato di una Segnalazione (Avanzamento Pratica)\n");
        printf("5. Visualizza Elenco delle Priorita' ed Urgenze (Server Queue)\n");
        printf("6. Visualizza Storico Strutturato ad Albero (Report BST per Utente)\n"); 
        printf("7. Genera Report Statistico Comunale\n");
        printf("8. Disconnetti (Logout)\n");
        printf("Seleziona un'opzione: ");
        if (scanf("%d", &scelta) != 1) { while (getchar() != '\n'); continue; }
        while (getchar() != '\n');
/* ... Continuazione nativa dal termine della parte 2 di main.c ... */
        switch (scelta) {
            case 1:
                printf("\n--- ELENCO SEGNALAZIONI APERTE (Filtro Combinato) ---\n");
                mostra_segnalazioni_paginate_filtrate(PATH_OPEN_MASTER, OPEN);
                break;
            case 2:
                printf("\n--- ELENCO SEGNALAZIONI IN LAVORAZIONE (Filtro Combinato) ---\n");
                mostra_segnalazioni_paginate_filtrate(PATH_PROGRESS_MASTER, IN_PROGRESS);
                break;
            case 3:
                printf("\n--- ELENCO SEGNALAZIONI CHIUSE (Filtro Combinato) ---\n");
                mostra_segnalazioni_paginate_filtrate(PATH_CLOSED_MASTER, CLOSED);
                break;
            case 4:
                printf("\n--- CAMBIO STATO ---\n");
                printf("Codice numerico del Report: ");
                if (scanf("%d", &rep_id) != 1) { while (getchar() != '\n'); break; }
                
                printf("Nuovo stato (1=IN_PROGRESS, 2=CLOSED): ");
                if (scanf("%d", &new_st) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                if (update_report_state_server(rep_id, (ReportStatus)new_st)) {
                    printf("\n[OK] Stato modificato temporaneamente in cache nella BENCH.\n");
                } else {
                    printf("\n[ERRORE] Impossibile trovare il report specificato nel sistema.\n");
                }
                break;
            case 5:
                printf("\n--- CRONOLOGIA DELLE PRIORITA' OPERATIVE (Server Queue) ---\n");
                rebuild_priority_file(); // Esegue il flush pesante e compila la coda
                mostra_priority_queue_binaria(PATH_PRIORITY_FILE); // Stampa l'indice pre-ordinato
                break;
            case 6: 
                printf("\n--- STORICO STRUTTURATO AD ALBERO (Report BST) ---\n");
                process_and_flush_bench(); // Svuota la cache
                rebuild_report_bst_file(); // Rigenera sia il BST Report ID che il BST User ID
                mostra_bst_utente_triangolato(); // Esegue la ricerca logaritmica accoppiata
                break;
            case 7:
			    process_and_flush_bench();
                genera_statistiche_comunali();
                break;
            case 8:
                return;
            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
}

void mostra_segnalazioni_paginate_filtrate(const char* file_path, ReportStatus stato_richiesto) {
    char line[REPORT_LINE_TOTAL + 1];
    int counter = 0;
    int input_pag;

    printf("--- DATI CORRENTI IN CACHE OPERATIVA (BENCH) ---\n");
    
    FILE* f_bench = fopen(PATH_BENCH, "rb");
    if (f_bench) {
        /* CORREZIONE: Il ciclo legge fino al reale contatore condiviso extern */
        for (int i = 0; i < contatore_bench_aggiunte; i++) {
            fseek(f_bench, i * REPORT_LINE_TOTAL, SEEK_SET);
            if (fread(line, sizeof(char), REPORT_LINE_TOTAL, f_bench) == REPORT_LINE_TOTAL) {
                line[REPORT_LINE_TOTAL] = '\0';
                
                char rec_state = line[330];
                int row;
                Report r = line_to_report(line, &rec_state, &row);
                
                if (r != NULL && rec_state == 'A' && get_report_status(r) == stato_richiesto) {
                    printf("[IN CACHE] Codice: %05d | Cittadino: %-15s | Cat: %s\n", 
                           get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)));
                    printf("           Data: %s   | Urgenza: %d                 | Stato: %s\n", 
                           get_report_date(r), get_report_urgency(r), get_status_string(get_report_status(r)));
                    printf("           Descrizione: %s\n", get_report_description(r));
                    printf("------------------------------------------------------\n");
                    counter++;
                }
                if (r != NULL) free_report(r);
            }
        }
        fclose(f_bench);
    }

    printf("\n--- DATI CONSOLIDATI ARCHIVIO COMUNALE (MASTER DISCO) ---\n");

    FILE* f_master = fopen(file_path, "rb");
    if (!f_master) {
        printf("Nessun dato consolidato presente su questo canale.\n");
        printf("Fine dell'elenco. %d segnalazioni attive mostrate.\n", counter);
        return;
    }

    while (fread(line, sizeof(char), REPORT_LINE_TOTAL, f_master) == REPORT_LINE_TOTAL) {
        line[REPORT_LINE_TOTAL] = '\0';

        if (line[330] == 'E') break; 
        if (line[330] == 'V') continue; 
        
        if (line[0] != ' ' && line[0] != '\n') {
            char rec_state = line[330]; int cit_id;
            Report r = line_to_report(line, &rec_state, &cit_id);
            
            if (r != NULL && rec_state == 'A') {
                bool presente_in_bench = false;
                FILE* f_bench_check = fopen(PATH_BENCH, "rb");
                if (f_bench_check) {
                    char check_line[REPORT_LINE_TOTAL + 1];
                    /* CORREZIONE: Controllo anti-duplicazione tarato sul contatore reale shared */
                    for (int k = 0; k < contatore_bench_aggiunte; k++) {
                        fseek(f_bench_check, k * REPORT_LINE_TOTAL, SEEK_SET);
                        if (fread(check_line, sizeof(char), REPORT_LINE_TOTAL, f_bench_check) == REPORT_LINE_TOTAL) {
                            check_line[REPORT_LINE_TOTAL] = '\0';
                            char c_state = check_line[330]; int c_row;
                            Report r_check = line_to_report(check_line, &c_state, &c_row);
                            if (r_check && get_report_id(r_check) == get_report_id(r) && c_state == 'A') {
                                presente_in_bench = true;
                                free_report(r_check);
                                break;
                            }
                            if (r_check) free_report(r_check);
                        }
                    }
                    fclose(f_bench_check);
                }

                if (!presente_in_bench) {
                    printf("--- [DISCO] Codice: %05d | Cittadino: %-15s | Cat: %s\n", 
                           get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)));
                    printf("            Data: %s   | Urgenza: %d                 | Stato: %s\n", 
                           get_report_date(r), get_report_urgency(r), get_status_string(get_report_status(r)));
                    printf("            Descrizione: %s\n", get_report_description(r));
                    printf("------------------------------------------------------\n");
                    counter++;

                    if (counter % 5 == 0) {
                        printf("------------------------------------------------------\n");
                        printf("Premi [INVIO] per caricare altri elementi o 'q' per fermarti: ");
                        input_pag = getchar();
                        if (input_pag == 'q' || input_pag == 'Q') {
                            free_report(r); fclose(f_master); return;
                        }
                        if (input_pag != '\n') while (getchar() != '\n');
                    }
                }
            }
            if (r != NULL) free_report(r);
        }
    }
    printf("------------------------------------------------------\n");
    printf("Fine dell'elenco storico. %d segnalazioni attive caricate.\n", counter);
    fclose(f_master);
}


void genera_statistiche_comunali(void) {
    int totali = 0, aperte = 0, lavorazione = 0, chiuse = 0;
    int cat_counts[5] = {0};
    char line[REPORT_LINE_TOTAL + 1];
    const char* paths[] = { PATH_OPEN_MASTER, PATH_PROGRESS_MASTER, PATH_CLOSED_MASTER };

    for (int i = 0; i < 3; i++) {
        FILE* f = fopen(paths[i], "rb");
        if (f) {
            /* Sincronizzazione totale tramite fread a blocchi stabili */
            while (fread(line, sizeof(char), REPORT_LINE_TOTAL, f) == REPORT_LINE_TOTAL) {
                line[REPORT_LINE_TOTAL] = '\0';
                if (line[330] == 'E') break;
                if (line[330] == 'V') continue;
                
                if (line[0] != ' ' && line[0] != '\n') {
                    char state; int c_id;
                    Report r = line_to_report(line, &state, &c_id);
                    if (r != NULL && state == 'A') {
                        totali++;
                        cat_counts[(int)get_report_category(r)]++;
                        if (get_report_status(r) == OPEN) aperte++;
                        else if (get_report_status(r) == IN_PROGRESS) lavorazione++;
                        else if (get_report_status(r) == CLOSED) chiuse++;
                    }
                    if (r != NULL) free_report(r);
                }
            }
            fclose(f);
        }
    }

    printf("\n===================================================\n");
    printf("         REPORT STATISTICO COMUNALE GENERATO        \n");
    printf("===================================================\n");
    printf("Numero totale di segnalazioni registrate: %d\n", totali);
    printf("  - Pratiche Aperte (OPEN): %d\n", aperte);
    printf("  - Pratiche In Lavorazione (IN_PROGRESS): %d\n", lavorazione);
    printf("  - Pratiche Risolte/Chiuse (CLOSED): %d\n", chiuse);
    
    printf("\nSuddivisione analitica per Categoria:\n");
    for (int i = 0; i < 5; i++) printf("  - %-25s: %d\n", get_category_string((ReportCategory)i), cat_counts[i]);
    
    int max_idx = 0;
    for (int i = 1; i < 5; i++) if (cat_counts[i] > cat_counts[max_idx]) max_idx = i;
    printf("\nTipologia di problema piu' frequente: %s\n===================================================\n", get_category_string((ReportCategory)max_idx));
}

void esegui_casi_test(void) { run_all_tests(); }

void mostra_bst_utente_triangolato(void) {
    // Apertura esplicita in modalità lettura binaria per preservare l'allineamento
    FILE* f_bst_u = fopen(PATH_BST_USER_ID, "rb");
    if (!f_bst_u) {
        printf("Albero BST degli utenti vuoto o non ancora inizializzato dal server.\n");
        return;
    }
    
    // Allocazione del buffer di stringa a 32 byte per accogliere in sicurezza la riga con \r\n
    char bst_line[32]; 
    int counter = 0;

    printf("\n--- NAVIGAZIONE SIMMETRICA BST (ORDINATO PER UTENTE) ---\n");
    
    /* 
       SOSTITUZIONE DI SICUREZZA: Usiamo fgets per scorrere l'indice ridotto dell'utente.
       Questo neutralizza le variazioni geometriche dei newline (\r\n vs \n), sbloccando il ciclo.
    */
    while (fgets(bst_line, sizeof(bst_line), f_bst_u)) {
        trim_string(bst_line); // Pulisce i caratteri invisibili di fine riga (\r, \n)
        
        // Esegue il parsing solo se la riga contiene i caratteri numerici attesi
        if (strlen(bst_line) >= 10) {
            int read_user_id = 0;
            int read_report_id = 0;
            
            // Scompone in modo sicuro i due interi numerici puri (es. 30769 e 1)
            line_to_user_index(bst_line, &read_user_id, &read_report_id);
            
            /* 
               2. RICERCA BINARIA O(log n): Risolviamo l'ID report estratto 
                  all'interno dell'albero dei report generali (Punto di Verità)
            */
            FILE* f_bst_r = fopen(PATH_BST_REPORT_ID, "rb");
            if (f_bst_r) {
                char main_line[REPORT_LINE_TOTAL + 1];
                bool trovato = false;
                
                // Forza il riavvolgimento del puntatore all'inizio del file ad ogni iterazione
                fseek(f_bst_r, 0, SEEK_SET);
                
                // L'indice dei report completi da 332 byte viene scansionato rigorosamente con fread
                while (!trovato && fread(main_line, sizeof(char), REPORT_LINE_TOTAL, f_bst_r) == REPORT_LINE_TOTAL) {
                    main_line[REPORT_LINE_TOTAL] = '\0';
                    
                    char flag = main_line[330]; 
                    int row;
                    Report r_real = line_to_report(main_line, &flag, &row);
					if (r_real != NULL) {
    printf("[DEBUG MASTER] Cercato: %d | Estratto dal file: %d\n", read_report_id, get_report_id(r_real));
} else {
    printf("[DEBUG MASTER] Errore: Il parser line_to_report ha restituito NULL!\n");
}
                    
                    // STAMPA DI DIAGNOSTICA INTERNA ATTIVATA
                    if (r_real != NULL) {
                        if (get_report_id(r_real) == read_report_id) {
                            printf("------------------------------------------------------\n");
                            printf("[CHIAVE BST USER ID: %05d] -> Corrispondenza Trovata nel BST Report ID!\n", read_user_id);
                            printf("Codice Report: %05d | Cittadino: %-15s | Stato: %s\n", 
                                   get_report_id(r_real), get_report_citizen_name(r_real), get_status_string(get_report_status(r_real)));
                            printf("Categoria: %s | Descrizione: %s\n", 
                                   get_category_string(get_report_category(r_real)), get_report_description(r_real));
                            counter++;
                            trovato = true; // Interrompe il ciclo interno passando alla riga utente successiva
                        }
                        free_report(r_real);
                    }
                }
                fclose(f_bst_r);
            }
        }
    }
    printf("------------------------------------------------------\n");
    printf("Fine dell'albero. %d corrispondenze storiche caricate.\n", counter);
    fclose(f_bst_u);
}




void mostra_priority_queue_binaria(const char* file_path) {
    FILE* f = fopen(file_path, "rb");
    if (!f) {
        printf("Coda delle priorita' vuota o non ancora inizializzata.\n");
        return;
    }
    char line[REPORT_LINE_TOTAL + 1];
    int counter = 0;
    int input_pag;

    // Carica blocchi binari granitici da 332 byte per preservare l'ordine della coda
    while (fread(line, sizeof(char), REPORT_LINE_TOTAL, f) == REPORT_LINE_TOTAL) {
        line[REPORT_LINE_TOTAL] = '\0';
        
        char rec_state; int row;
        Report r = line_to_report(line, &rec_state, &row);
        
        if (r != NULL) {
            printf("------------------------------------------------------\n");
            printf("[PRIORITÀ] Codice: %05d | Cittadino: %-15s | Cat: %s\n", 
                   get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)));
            printf("           Data: %s   | Urgenza: %d                 | Stato: %s\n", 
                   get_report_date(r), get_report_urgency(r), get_status_string(get_report_status(r)));
            printf("           Descrizione: %s\n", get_report_description(r));
            counter++;

            if (counter % 5 == 0) {
                printf("------------------------------------------------------\n");
                printf("Premi [INVIO] per continuare o 'q' per fermarti: ");
                input_pag = getchar();
                if (input_pag == 'q' || input_pag == 'Q') {
                    free_report(r); fclose(f); return;
                }
                if (input_pag != '\n') while (getchar() != '\n');
            }
            free_report(r);
        }
    }
    printf("------------------------------------------------------\nFine della coda. %d segnalazioni urgenti caricate.\n", counter);
    fclose(f);
}



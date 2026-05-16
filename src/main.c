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

void menu_cittadino(User logged_in_user);
void menu_dipendente(User logged_in_user);
void mostra_segnalazioni_paginate_filtrate(const char* file_path, ReportStatus stato_richiesto);
void mostra_priority_queue_binaria(const char* file_path);
void mostra_avl_utente_triangolato(void);
void area_dipendente_cambio_stato(void);
void genera_statistiche_comunali_v2(void);

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
                run_all_tests();
                break;

            case 4:
                printf("\nGrazie per aver utilizzato il sistema municipale. Arrivederci!\n");
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
    int scelta;
    char urgenza_char;
    char desc_str[MAX_DESC];
    char data_str[11];
    unsigned int target_user_id = (unsigned int)get_user_id(logged_in_user);

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

                printf("Data di oggi (GG/MM/AAAA): ");
                if (!fgets(data_str, sizeof(data_str), stdin)) break;
                trim_string(data_str);

                printf("Livello di Urgenza (0=Bassa, 1=Media, 2=Alta): ");
                int urg_in;
                if (scanf("%d", &urg_in) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');
                urgenza_char = (char)urg_in + '0';

                if (!validate_not_empty(desc_str) || !validate_date_format(data_str) || !validate_urgency_range(urgenza_char)) {
                    printf("\n[ERRORE] Dati inseriti non validi o non conformi.\n");
                    break;
                }

                unsigned int global_id = generate_global_report_id_v2();
                Report new_r = create_report(global_id, target_user_id, get_user_username(logged_in_user), (ReportCategory)cat_scelta, desc_str, data_str, urgenza_char);
                list_insert(ram_list, new_r);
                printf("\n[OK] Segnalazione inserita nella sessione locale RAM (Codice: %05u).\n", global_id);
                break;

            case 2:
                printf("\n===================================================\n");
                printf("      STORICO PERSONALE DEL CITTADINO   \n");
                printf("===================================================\n");
                int cittadino_counter = 1;
                char line[REPORT_MASTER_LINE + 1];

                list_rewind(ram_list);
                Report r_ram = list_next(ram_list);
                while (r_ram != NULL) {
                    if (get_report_status(r_ram) != DESTROYED) {
                        printf("[ RAM LOCAL ] Codice: %05u | Categoria: %s\n", get_report_id(r_ram), get_category_string(get_report_category(r_ram)));
                        printf("              Urgenza: %c | Stato: %s | Desc: %s\n", get_report_urgency(r_ram), get_status_string(get_report_status(r_ram)), get_report_description(r_ram));
                        printf("------------------------------------------------------\n");
                        cittadino_counter++;
                    }
                    r_ram = list_next(ram_list);
                }

                unsigned int current_bench = read_system_variable(REG_IDX_COUNTER_BENCH);
                unsigned int internal_bench_ids[50];
                int internal_bench_count = 0;
                
                FILE* f_b = fopen(PATH_BENCH, "rb");
                if (f_b) {
                    for (unsigned int i = 0; i < current_bench; i++) {
                        fseek(f_b, (long)i * REPORT_MASTER_LINE, SEEK_SET);
                        if (fread(line, sizeof(char), REPORT_MASTER_LINE, f_b) == REPORT_MASTER_LINE) {
                            char state;
                            Report tmp = line_to_report_v2(line, &state);
                            if (tmp && get_report_user_id(tmp) == target_user_id && state == 'A') {
                                internal_bench_ids[internal_bench_count++] = get_report_id(tmp);
                                if (get_report_status(tmp) != DESTROYED) {
                                    printf("[ CACHE BENCH ] Codice: %05u | Categoria: %s\n", get_report_id(tmp), get_category_string(get_report_category(tmp)));
                                    printf("                Urgenza: %c | Stato: %s | Desc: %s\n", get_report_urgency(tmp), get_status_string(get_report_status(tmp)), get_report_description(tmp));
                                    printf("------------------------------------------------------\n");
                                    cittadino_counter++;
                                }
                            }
                            if (tmp) free_report(tmp);
                        }
                    }
                    fclose(f_b);
                }

                FILE* f_avl_u = fopen(PATH_AVL_USER_ID, "rb");
                if (f_avl_u) {
                    unsigned int read_uid, read_rid;
                    while (fscanf(f_avl_u, "%u%u\n", &read_uid, &read_rid) == 2) {
                        if (read_uid == target_user_id) {
                            bool in_bench = false;
                            for (int k = 0; k < internal_bench_count; k++) {
                                if (internal_bench_ids[k] == read_rid) { in_bench = true; break; }
                            }

                            if (!in_bench) {
                                FILE* f_avl_r = fopen(PATH_AVL_REPORT_ID, "rb");
                                int found_row = -1; char st_ch;
                                if (f_avl_r) {
                                    unsigned int r_id; char st; int r_row;
                                    while (fscanf(f_avl_r, "%u %c %d\n", &r_id, &st, &r_row) == 3) {
                                        if (r_id == read_rid) { found_row = r_row; st_ch = st; break; }
                                    }
                                    fclose(f_avl_r);
                                }

                                if (found_row != -1) {
                                    ReportStatus vec_st = (ReportStatus)(st_ch - '0');
                                    const char* path_master = (vec_st == OPEN) ? PATH_OPEN_MASTER : (vec_st == IN_PROGRESS) ? PATH_PROGRESS_MASTER : PATH_CLOSED_MASTER;
                                    FILE* f_m = fopen(path_master, "rb");
                                    if (f_m) {
                                        fseek(f_m, (long)found_row * REPORT_MASTER_LINE, SEEK_SET);
                                        if (fread(line, sizeof(char), REPORT_MASTER_LINE, f_m) == REPORT_MASTER_LINE && line[350] == 'A') {
                                            char st_cell; Report r_real = line_to_report_v2(line, &st_cell);
                                            if (r_real && get_report_status(r_real) != DESTROYED) {
                                                printf("[ MASTER DISK ] Codice: %05u | Categoria: %s\n", get_report_id(r_real), get_category_string(get_report_category(r_real)));
                                                printf("                Urgenza: %c | Stato: %s | Desc: %s\n", get_report_urgency(r_real), get_status_string(get_report_status(r_real)), get_report_description(r_real));
                                                printf("------------------------------------------------------\n");
                                                cittadino_counter++;
                                            }
                                            if (r_real) free_report(r_real);
                                        }
                                        fclose(f_m);
                                    }
                                }
                            }
                        }
                    }
                    fclose(f_avl_u);
                }
                if (cittadino_counter == 1) printf("\nNon ci sono segnalazioni attive associate al tuo account.\n");
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
                    
                    printf("Nuova Urgenza (0=Bassa, 1=Media, 2=Alta): ");
                    int n_urg;
                    if (scanf("%d", &n_urg) != 1) { while (getchar() != '\n'); break; }
                    while (getchar() != '\n');
                    char n_urg_c = (char)n_urg + '0';

                    Report clonizzato = create_report(get_report_id(r_mod), target_user_id, get_report_citizen_name(r_mod), get_report_category(r_mod), desc_str, get_report_date(r_mod), n_urg_c);
                    list_remove(ram_list, target_id);
                    list_insert(ram_list, clonizzato);
                    printf("\n[OK] Segnalazione aggiornata in RAM.\n");
                } else {
                    printf("\n[ERRORE] Segnalazione non trovata in sessione o non modificabile.\n");
                }
                break;

            case 4:
                if (!stack_is_empty(revert_stack)) {
                    Report vecchio_stato = stack_pop(revert_stack);
                    int old_id = (int)get_report_id(vecchio_stato);
                    list_remove(ram_list, old_id);
                    list_insert(ram_list, vecchio_stato);
                    printf("\n[OK] Azione annullata per il report %05d.\n", old_id);
                } else {
                    printf("\n[AVVISO] Nessuna azione da annullare nello Stack.\n");
                }
                break;

            case 5:
                printf("\nSalvataggio e sincronizzazione in cache...\n");
                list_rewind(ram_list);
                Report r_flush = list_next(ram_list);
                while (r_flush != NULL) {
                    register_report_from_citizen_ram(r_flush);
                    r_flush = list_next(ram_list);
                }
                free_list(ram_list);
                free_stack(revert_stack);
                return;

            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
}

void menu_dipendente(User logged_in_user) {
    int scelta;
    while (1) {
        printf("\n--- AREA DIPENDENTE (%s) ---\n", get_user_username(logged_in_user));
        printf("1. Visualizza Segnalazioni APERTE\n");
        printf("2. Visualizza Segnalazioni IN LAVORAZIONE\n");
        printf("3. Visualizza Segnalazioni CHIUSE\n");
        printf("4. Modifica Stato di una Segnalazione (Avanzamento Pratica AVL)\n");
        printf("5. Visualizza Elenco delle Priorita' ed Urgenze (Flush Forzato)\n");
        printf("6. Visualizza Storico Strutturato ad Albero Bilanciato (In-Order AVL)\n"); 
        printf("7. Genera Report Statistico Comunale O(1)\n");
        printf("8. Disconnetti (Logout)\n");
        printf("Seleziona un'opzione: ");
        if (scanf("%d", &scelta) != 1) { while (getchar() != '\n'); continue; }
        while (getchar() != '\n');

        switch (scelta) {
            case 1:
                mostra_segnalazioni_paginate_filtrate(PATH_OPEN_MASTER, OPEN);
                break;
            case 2:
                mostra_segnalazioni_paginate_filtrate(PATH_PROGRESS_MASTER, IN_PROGRESS);
                break;
            case 3:
                mostra_segnalazioni_paginate_filtrate(PATH_CLOSED_MASTER, CLOSED);
                break;
            case 4:
                area_dipendente_cambio_stato();
                break;
            case 5:
                process_and_flush_bench_v2(); 
                mostra_priority_queue_binaria(PATH_PRIORITY_FILE);
                break;
            case 6: 
                process_and_flush_bench_v2(); 
                mostra_avl_utente_triangolato();
                break;
            case 7:
                genera_statistiche_comunali_v2();
                break;
            case 8:
                return;
            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
}

void area_dipendente_cambio_stato(void) {
    unsigned int target_id;
    printf("\nInserisci l'ID del report da modificare: ");
    if (scanf("%u", &target_id) != 1) {
        while (getchar() != '\n');
        return;
    }
    while (getchar() != '\n');
    
    char line[REPORT_MASTER_LINE + 1];
    bool trovato = false;
    unsigned int counter_bench = read_system_variable(REG_IDX_COUNTER_BENCH);
    
    // 1. RICERCA ORIZZONTALE IN CACHE OPERATIVA (BENCH) IN O(n)
    FILE* f_bench = fopen(PATH_BENCH, "rb+");
    if (f_bench) {
        for (unsigned int i = 0; i < counter_bench; i++) {
            fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET);
            if (fread(line, sizeof(char), REPORT_BENCH_LINE, f_bench) != REPORT_BENCH_LINE) continue;
            char state;
            Report r = line_to_report_v2(line, &state);
            if (r && get_report_id(r) == target_id && state == 'A') {
                trovato = true;
                if (get_report_status(r) == DESTROYED) {
                    printf("\n[AVVISO] Segnalazione non trovata (Stato logico: DESTROYED).\n");
                    free_report(r);
                    fclose(f_bench);
                    return;
                }
                
                int nuovo_st;
                printf("\n[TROVATO IN CACHE BENCH]\n");
                printf("ID Segnalazione: %05u\nCittadino: %s\nCategoria: %s\nDescrizione: %s\nData: %s\nUrgenza: %c\nStato Corrente: %s\n",
                       get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)),
                       get_report_description(r), get_report_date(r), get_report_urgency(r), get_status_string(get_report_status(r)));
                       
                printf("\nScegli nuovo stato (0=OPEN, 1=IN_PROGRESS, 2=CLOSED, 3=DESTROYED): ");
                if (scanf("%d", &nuovo_st) == 1) {
                    while (getchar() != '\n');
                    
                    ReportStatus vecchio_st = get_report_status(r);
                    ReportStatus nuovo_st_enum = (ReportStatus)nuovo_st;
                    
                    if (vecchio_st != nuovo_st_enum) {
                        int reg_diminuisci = (vecchio_st == OPEN) ? REG_IDX_STAT_OPEN : (vecchio_st == IN_PROGRESS) ? REG_IDX_STAT_PROGRESS : REG_IDX_STAT_CLOSED;
                        unsigned int v_count = read_system_variable(reg_diminuisci);
                        if (v_count > 0) write_system_variable(reg_diminuisci, v_count - 1);
                        
                        if (nuovo_st_enum != DESTROYED) {
                            int reg_aumenta = (nuovo_st_enum == OPEN) ? REG_IDX_STAT_OPEN : (nuovo_st_enum == IN_PROGRESS) ? REG_IDX_STAT_PROGRESS : REG_IDX_STAT_CLOSED;
                            write_system_variable(reg_aumenta, read_system_variable(reg_aumenta) + 1);
                        } else {
                            unsigned int active_reps = read_system_variable(REG_IDX_NM_REPORT);
                            if (active_reps > 0) write_system_variable(REG_IDX_NM_REPORT, active_reps - 1);
                            
                            int cat_reg = REG_IDX_CAT_OTHER;
                            switch(get_report_category(r)) {
                                case ROAD:           cat_reg = REG_IDX_CAT_ROAD; break;
                                case LIGHTING:       cat_reg = REG_IDX_CAT_LIGHTING; break;
                                case WASTE:          cat_reg = REG_IDX_CAT_WASTE; break;
                                case INFRASTRUCTURE: cat_reg = REG_IDX_CAT_INFRASTRUCT; break;
                                default:             cat_reg = REG_IDX_CAT_OTHER; break;
                            }
                            unsigned int cat_count = read_system_variable(cat_reg);
                            if (cat_count > 0) write_system_variable(cat_reg, cat_count - 1);
                        }
                    }
                    
                    update_report_status(r, nuovo_st_enum);
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
    
    // 2. CADUTA LOGARITMICA NELL'INDICE AVL BY REPORT ID IN O(log n)
    if (!trovato) {
        FILE* f_avl = fopen(PATH_AVL_REPORT_ID, "rb");
        int found_row = -1; 
        char status_char = '0';
        if (f_avl) {
            unsigned int r_id; char st; int r_row;
            while (fscanf(f_avl, "%u %c %d\n", &r_id, &st, &r_row) == 3) {
                if (r_id == target_id) { found_row = r_row; status_char = st; break; }
            }
            fclose(f_avl);
        }
        
        if (found_row != -1) {
            ReportStatus vecchio_stato = (ReportStatus)(status_char - '0');
            if (vecchio_stato == DESTROYED) {
                printf("\n[AVVISO] Segnalazione non trovata (Stato logico: DESTROYED).\n");
                return;
            }
            
            const char* path_old = (vecchio_stato == OPEN) ? PATH_OPEN_MASTER : 
                                  (vecchio_stato == IN_PROGRESS) ? PATH_PROGRESS_MASTER : PATH_CLOSED_MASTER;
            const char* path_holes = (vecchio_stato == OPEN) ? PATH_OPEN_HOLES : 
                                   (vecchio_stato == IN_PROGRESS) ? PATH_PROGRESS_HOLES : PATH_CLOSED_HOLES;
            
            FILE* f_master = fopen(path_old, "rb");
            if (f_master) {
                fseek(f_master, (long)found_row * REPORT_MASTER_LINE, SEEK_SET);
                if (fread(line, sizeof(char), REPORT_MASTER_LINE, f_master) == REPORT_MASTER_LINE) {
                    char st_cell;
                    Report r = line_to_report_v2(line, &st_cell);
                    if (r && st_cell == 'A') {
                        trovato = true;
                        
                        printf("\n[TROVATO IN ARCHIVIO MASTER DI VERITA']\n");
                        printf("ID Segnalazione: %05u\nCittadino: %s\nCategoria: %s\nDescrizione: %s\nData: %s\nUrgenza: %c\nStato Corrente: %s\n",
                               get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)),
                               get_report_description(r), get_report_date(r), get_report_urgency(r), get_status_string(get_report_status(r)));
                        
                        int nuovo_st;
                        printf("\nScegli nuovo stato (0=OPEN, 1=IN_PROGRESS, 2=CLOSED, 3=DESTROYED): ");
                        if (scanf("%d", &nuovo_st) == 1) {
                            while (getchar() != '\n');
                            ReportStatus nuovo_st_enum = (ReportStatus)nuovo_st;
                            
                            // Invalida geometricamente la vecchia cella impostandola a 'N'
                            FILE* f_inv = fopen(path_old, "rb+");
                            if (f_inv) {
                                fseek(f_inv, (long)found_row * REPORT_MASTER_LINE, SEEK_SET);
                                char clear_buf[REPORT_MASTER_LINE + 1];
                                if (fread(clear_buf, sizeof(char), REPORT_MASTER_LINE, f_inv) == REPORT_MASTER_LINE) {
                                    clear_buf[350] = 'N'; 
                                    fseek(f_inv, (long)found_row * REPORT_MASTER_LINE, SEEK_SET);
                                    fwrite(clear_buf, sizeof(char), REPORT_MASTER_LINE, f_inv);
                                }
                                fclose(f_inv);
                            }
                            
                            FILE* f_h = fopen(path_holes, "ab");
                            if (f_h) { fprintf(f_h, "%010d\n", found_row); fclose(f_h); }
                            
                            int old_reg = (vecchio_stato == OPEN) ? REG_IDX_STAT_OPEN : (vecchio_stato == IN_PROGRESS) ? REG_IDX_STAT_PROGRESS : REG_IDX_STAT_CLOSED;
                            unsigned int prev_c = read_system_variable(old_reg);
                            if (prev_c > 0) write_system_variable(old_reg, prev_c - 1);
                            
                            if (nuovo_st_enum != DESTROYED) {
                                int new_reg = (nuovo_st_enum == OPEN) ? REG_IDX_STAT_OPEN : (nuovo_st_enum == IN_PROGRESS) ? REG_IDX_STAT_PROGRESS : REG_IDX_STAT_CLOSED;
                                write_system_variable(new_reg, read_system_variable(new_reg) + 1);
                            } else {
                                unsigned int active_reps = read_system_variable(REG_IDX_NM_REPORT);
                                if (active_reps > 0) write_system_variable(REG_IDX_NM_REPORT, active_reps - 1);
                                
                                int cat_reg = REG_IDX_CAT_OTHER;
                                switch(get_report_category(r)) {
                                    case ROAD:           cat_reg = REG_IDX_CAT_ROAD; break;
                                    case LIGHTING:       cat_reg = REG_IDX_CAT_LIGHTING; break;
                                    case WASTE:          cat_reg = REG_IDX_CAT_WASTE; break;
                                    case INFRASTRUCTURE: cat_reg = REG_IDX_CAT_INFRASTRUCT; break;
                                    default:             cat_reg = REG_IDX_CAT_OTHER; break;
                                }
                                unsigned int cat_count = read_system_variable(cat_reg);
                                if (cat_count > 0) write_system_variable(cat_reg, cat_count - 1);
                            }
                            
                            update_report_status(r, nuovo_st_enum);
                            set_report_disk_row(r, -1); 
                            
                            if (counter_bench >= LIMIT_BENCH) {
                                process_and_flush_bench_v2();
                                counter_bench = 0;
                            }
                            
                            FILE* f_b_add = fopen(PATH_BENCH, "rb+");
                            if (f_b_add) {
                                char out_l[REPORT_BENCH_LINE + 1];
                                report_to_line(out_l, r, '\0');
                                fseek(f_b_add, (long)counter_bench * REPORT_BENCH_LINE, SEEK_SET);
                                fwrite(out_l, sizeof(char), REPORT_BENCH_LINE, f_b_add);
                                fclose(f_b_add);
                                write_system_variable(REG_IDX_COUNTER_BENCH, counter_bench + 1);
                            }
                            printf("\n[OK] Record estratto dall'archivio, invalidata riga %d e caricata la modifica nella BENCH.\n", found_row);
                        } else {
                            while (getchar() != '\n');
                        }
                    }
                    if (r) free_report(r);
                }
                fclose(f_master);
            }
        }
    }
    
    if (!trovato) {
        printf("\n[ERRORE] Impossibile trovare la segnalazione con ID %05u nel sistema comunale.\n", target_id);
    }
}

void mostra_segnalazioni_paginate_filtrate(const char* file_path, ReportStatus stato_richiesto) {
    char line[REPORT_MASTER_LINE + 1];
    int counter = 0;
    int input_pag;
    unsigned int current_bench = read_system_variable(REG_IDX_COUNTER_BENCH);

    if (stato_richiesto == OPEN) {
        printf("--- DATI VELOCI IN CACHE OPERATIVA (BENCH) ---\n");
        FILE* f_bench = fopen(PATH_BENCH, "rb");
        if (f_bench) {
            for (unsigned int i = 0; i < current_bench; i++) {
                fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET);
                if (fread(line, sizeof(char), REPORT_BENCH_LINE, f_bench) == REPORT_BENCH_LINE) {
                    char st; Report r = line_to_report_v2(line, &st);
                    if (r && get_report_status(r) == OPEN) {
                        printf("[IN CACHE] ID: %05u | Cittadino: %s | Cat: %s\nDesc: %s\n", 
                               get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)), get_report_description(r));
                        printf("------------------------------------------------------\n");
                        counter++;
                    }
                    if (r) free_report(r);
                }
            }
            fclose(f_bench);
        }
    }

    printf("\n--- DATI STABILI ARCHIVIO COMUNALE (MASTER) ---\n");
    FILE* f_master = fopen(file_path, "rb");
    if (!f_master) { printf("Fine dell'elenco. %d segnalazioni attive mostrate.\n", counter); return; }

    while (fread(line, sizeof(char), REPORT_MASTER_LINE, f_master) == REPORT_MASTER_LINE) {
        if (line[350] == 'N') continue; 
        if (line[350] == 'E') break;
        
        char cell_st; Report r = line_to_report_v2(line, &cell_st);
        if (r && cell_st == 'A') {
            printf("[DISCO] ID: %05u | Cittadino: %s | Cat: %s\nDesc: %s\n", 
                   get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)), get_report_description(r));
            printf("------------------------------------------------------\n");
            counter++;

            if (counter % 5 == 0) {
                printf("Premi [INVIO] per caricare altri elementi o 'q' per fermarti: ");
                input_pag = getchar();
                if (input_pag == 'q' || input_pag == 'Q') { free_report(r); fclose(f_master); return; }
                if (input_pag != '\n') while (getchar() != '\n');
            }
        }
        if (r) free_report(r);
    }
    fclose(f_master);
}


void mostra_priority_queue_binaria(const char* file_path) {
    (void)file_path;
    PriorityQueue pq = create_pq();
    const char* paths[] = { PATH_OPEN_MASTER, PATH_PROGRESS_MASTER };
    char line[REPORT_MASTER_LINE + 1];
    
    for (int i = 0; i < 2; i++) {
        FILE* f = fopen(paths[i], "rb");
        if (f) {
            while (fread(line, sizeof(char), REPORT_MASTER_LINE, f) == REPORT_MASTER_LINE) {
                if (line[350] == 'N') continue;
                char st; Report r = line_to_report_v2(line, &st);
                if (r && st == 'A' && get_report_status(r) != DESTROYED) pq_enqueue(pq, r);
                else if (r) free_report(r);
            }
            fclose(f);
        }
    }

    int counter = 0;
    printf("\n--- CRONOLOGIA DELLE PRIORITA' OPERATIVE ---\n");
    while (!pq_is_empty(pq)) {
        Report r = pq_dequeue(pq);
        printf("[PRIORITA'] ID: %05u | Urgenza: %c | Data: %s | Cat: %s\nDesc: %s\n", 
               get_report_id(r), get_report_urgency(r), get_report_date(r), get_category_string(get_report_category(r)), get_report_description(r));
        printf("------------------------------------------------------\n");
        counter++;
        free_report(r);
        
        if (counter % 5 == 0 && !pq_is_empty(pq)) {
            printf("Premi [INVIO] per caricare altri elementi o 'q' per fermarti: ");
            int ch = getchar();
            if (ch == 'q' || ch == 'Q') { free_pq(pq); return; }
            if (ch != '\n') while (getchar() != '\n');
        }
    }
    free_pq(pq);
}


void mostra_avl_utente_triangolato(void) {
    FILE* f_avl_u = fopen(PATH_AVL_USER_ID, "rb");
    if (!f_avl_u) { printf("Indice AVL utenti vuoto o non inizializzato.\n"); return; }
    
    unsigned int read_uid, read_rid;
    char line[REPORT_MASTER_LINE + 1];
    int counter = 0;
    
    printf("\n--- NAVIGAZIONE SIMMETRICA AVL UTENTE TRIANGOLATO ---\n");
    while (fscanf(f_avl_u, "%u%u\n", &read_uid, &read_rid) == 2) {
        FILE* f_avl_r = fopen(PATH_AVL_REPORT_ID, "rb");
        int found_row = -1; char st_ch = '0';
        if (f_avl_r) {
            unsigned int r_id; char st; int r_row;
            while (fscanf(f_avl_r, "%u %c %d\n", &r_id, &st, &r_row) == 3) {
                if (r_id == read_rid) { found_row = r_row; st_ch = st; break; }
            }
            fclose(f_avl_r);
        }
        
        if (found_row != -1) {
            ReportStatus vec_st = (ReportStatus)(st_ch - '0');
            const char* path_master = (vec_st == OPEN) ? PATH_OPEN_MASTER : (vec_st == IN_PROGRESS) ? PATH_PROGRESS_MASTER : PATH_CLOSED_MASTER;
            FILE* f_m = fopen(path_master, "rb");
            if (f_m) {
                fseek(f_m, (long)found_row * REPORT_MASTER_LINE, SEEK_SET);
                if (fread(line, sizeof(char), REPORT_MASTER_LINE, f_m) == REPORT_MASTER_LINE && line[350] == 'A') {
                    char cell_st; Report r = line_to_report_v2(line, &cell_st);
                    if (r && get_report_status(r) != DESTROYED) {
                        printf("[CHIAVE AVL USER: %05u] -> ID Report: %05u | Cat: %s | Stato: %s\n", 
                               read_uid, get_report_id(r), get_category_string(get_report_category(r)), get_status_string(get_report_status(r)));
                        counter++;
                    }
                    if (r) free_report(r);
                }
                fclose(f_m);
            }
        }
    }
    fclose(f_avl_u);
    printf("Fine dell'albero. %d corrispondenze totali caricate.\n", counter);
}

void genera_statistiche_comunali_v2(void) {
    printf("\n===================================================\n");
    printf("     REPORT STATISTICO ISTANTANEO O(1) REGISTRI    \n");
    printf("===================================================\n");
    printf("Numero totale di segnalazioni attive nel Comune: %u\n", read_system_variable(REG_IDX_NM_REPORT));
    printf("  - Casi Aperti (OPEN):          %u\n", read_system_variable(REG_IDX_STAT_OPEN));
    printf("  - In Lavorazione (PROGRESS):   %u\n", read_system_variable(REG_IDX_STAT_PROGRESS));
    printf("  - Pratiche Chiuse (CLOSED):    %u\n", read_system_variable(REG_IDX_STAT_CLOSED));
    
    printf("\nSuddivisione analitica per Categoria:\n");
    printf("  - Buca Stradale:               %u\n", read_system_variable(REG_IDX_CAT_ROAD));
    printf("  - Illuminazione Pubblica:      %u\n", read_system_variable(REG_IDX_CAT_LIGHTING));
    printf("  - Rifiuti Abbandonati:         %u\n", read_system_variable(REG_IDX_CAT_WASTE));
    printf("  - Guasto Impianto:             %u\n", read_system_variable(REG_IDX_CAT_INFRASTRUCT));
    printf("  - Altro:                       %u\n", read_system_variable(REG_IDX_CAT_OTHER));
    printf("===================================================\n");
}
#include "../../include/server/report_manager.h"
#include "../../include/utils/parser.h"
#include "../../include/adt/report_bst.h"
#include "../../include/adt/priority_queue.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definizione globale pulita condivisa con main.c */
int contatore_bench_aggiunte = 0;

/**
 * @brief Trova l'indice del primo slot contrassegnato come vuoto ('V') all'interno di un file master stazionario.
 * @pre path punta a un file master di stato valido.
 * @post Ritorna il numero di riga logica del primo buco libero, oppure la fine del file se è saturo.
 * @note Complessità temporale: O(n) nel caso peggiore di scansione delle sole celle, mitigata dalla sentinella 'E'.
 */
static int trova_primo_buco_master(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    
    char line[REPORT_LINE_TOTAL + 3];
    int current_row = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (line[330] == 'V') { // Trovata cella vuota disponibile per il riciclo geometrico
            fclose(f);
            return current_row;
        }
        if (line[330] == 'E') { // Raggiunta la sentinella logica di fine dati stabili
            fclose(f);
            return current_row;
        }
        current_row++;
    }
    fclose(f);
    return current_row;
}

/**
 * @brief Scrive il record speciale sentinella 'E' per sigillare l'indice logico del file di stato.
 * @pre f descrittore file master aperto in modalità scrittura binarizzata coerente. row riga in cui inserire EOF.
 * @post Il file conterrà una riga standard da 332 byte con flag 'E' per bloccare le scansioni successive.
 */
static void assegna_sentinella_master(FILE* f, int row) {
    char sentinella[REPORT_LINE_TOTAL + 3];
    memset(sentinella, ' ', REPORT_LINE_TOTAL - 2);
    sentinella[330] = 'E'; // Flag Sentinella EOF
    sentinella[331] = '\n';
    sentinella[332] = '\0';
    
    fseek(f, row * REPORT_LINE_TOTAL, SEEK_SET);
    fputs(sentinella, f);
}

int generate_global_report_id(void) {
    int ultimo_id_emesso = 0;
    
    // Apertura in modalità lettura binaria del file di testo sequenziale
    FILE* f_seq = fopen(PATH_SEQUENCE, "rb");
    if (f_seq) {
        char buffer[16] = {0};
        // Carica la stringa numerica in tempo costante O(1)
        if (fgets(buffer, sizeof(buffer), f_seq)) {
            ultimo_id_emesso = atoi(buffer);
        }
        fclose(f_seq);
    } else {
        // Primo avvio assoluto del sistema comunale: il contatore parte da zero
        ultimo_id_emesso = 0;
    }
    
    // Incremento atomico rigoroso del contatore per la nuova segnalazione
    ultimo_id_emesso++;
    
    // Riscrittura immediata del nuovo valore sul disco in modalità sovrascrittura binarizzata
    f_seq = fopen(PATH_SEQUENCE, "wb");
    if (f_seq) {
        fprintf(f_seq, "%d\n", ultimo_id_emesso);
        fclose(f_seq);
    }
    
    return ultimo_id_emesso;
}

bool flush_session_to_bench(ReportList local_list) {
    if (!local_list) return false;
    
    FILE* f_bench = fopen(PATH_BENCH, "rb+");
    if (!f_bench) return false;
    
    char formatted_line[REPORT_LINE_TOTAL + 3];
    list_rewind(local_list);
    Report r = list_next(local_list);
    
    // Scrittura sequenziale simulando un vettore ad avanzamento rigido
    while (r && contatore_bench_aggiunte < LIMIT_BENCH) {
        fseek(f_bench, contatore_bench_aggiunte * REPORT_LINE_TOTAL, SEEK_SET);
        report_to_line(formatted_line, r, 'A');
        fputs(formatted_line, f_bench);
        
        contatore_bench_aggiunte++; // Avanzamento del cursore di prima cella disponibile
        r = list_next(local_list);
    }
    fclose(f_bench);
    
    if (contatore_bench_aggiunte >= SOGLIA_FLUSH) {
        process_and_flush_bench();
    }
    return true;
}

bool process_and_flush_bench() {
    if (contatore_bench_aggiunte == 0) return true; 
    
    FILE* f_bench = fopen(PATH_BENCH, "rb");
    if (!f_bench) return false;
    
    char line[REPORT_LINE_TOTAL + 3];
    char bst_line[REPORT_LINE_TOTAL + 3];
    
    // Scansione dei record accumulati nella cache
    for (int i = 0; i < contatore_bench_aggiunte; i++) {
        fseek(f_bench, i * REPORT_LINE_TOTAL, SEEK_SET);
        if (!fgets(line, sizeof(line), f_bench)) continue;
        
        char rec_state; int old_row;
        Report r = line_to_report(line, &rec_state, &old_row);
        
        if (r != NULL && rec_state == 'A') {
            const char* path_target;
            if (get_report_status(r) == OPEN) path_target = PATH_OPEN_MASTER;
            else if (get_report_status(r) == IN_PROGRESS) path_target = PATH_PROGRESS_MASTER;
            else path_target = PATH_CLOSED_MASTER;
            
            // CANCELLAZIONE CHIRURGICA O(1) COERENTE
            if (old_row != -1) {
                /* 
                   Risoluzione del bug di duplicazione: interroghiamo il BST stazionario su disco 
                   per estrarre lo STATO ORIGINALE della segnalazione prima della modifica in cache.
                */
                ReportStatus vecchio_stato = OPEN; // Default di sicurezza
                FILE* f_bst_check = fopen(PATH_BST_REPORT_ID, "rb");
                if (f_bst_check) {
                    while (fgets(bst_line, sizeof(bst_line), f_bst_check)) {
                        char check_state; int check_row;
                        Report tmp_check = line_to_report(bst_line, &check_state, &check_row);
                        if (tmp_check && get_report_id(tmp_check) == get_report_id(r)) {
                            vecchio_stato = get_report_status(tmp_check); // Trovato lo stato reale sul disco
                            free_report(tmp_check);
                            break;
                        }
                        if (tmp_check) free_report(tmp_check);
                    }
                    fclose(f_bst_check);
                }

                // Determina il file master di provenienza corretto al 100%
                const char* path_old = (vecchio_stato == OPEN) ? PATH_OPEN_MASTER : 
                                      (vecchio_stato == IN_PROGRESS) ? PATH_PROGRESS_MASTER : PATH_CLOSED_MASTER;
                
                FILE* f_old = fopen(path_old, "rb+");
                if (f_old) {
                    fseek(f_old, old_row * REPORT_LINE_TOTAL, SEEK_SET);
                    char raw_clear[REPORT_LINE_TOTAL + 3];
                    if (fgets(raw_clear, sizeof(raw_clear), f_old)) {
                        raw_clear[330] = 'V'; // Generazione fisica del buco nel file originale
                        fseek(f_old, old_row * REPORT_LINE_TOTAL, SEEK_SET);
                        fputs(raw_clear, f_old);
                    }
                    fclose(f_old);
                }
            }
            
            // INSERIMENTO INTELLIGENTE NEL NUOVO FILE DI STATO
            int dest_row = trova_primo_buco_master(path_target);
            FILE* f_master = fopen(path_target, "rb+");
            if (!f_master) f_master = fopen(path_target, "wb+");
            
            if (f_master) {
                set_report_disk_row(r, dest_row); // Aggiorna l'indirizzo fisso
                char out_line[REPORT_LINE_TOTAL + 3];
                report_to_line(out_line, r, 'A');
                
                fseek(f_master, dest_row * REPORT_LINE_TOTAL, SEEK_SET);
                fputs(out_line, f_master);
                
                assegna_sentinella_master(f_master, dest_row + 1);
                fclose(f_master);
            }
        }
        if (r != NULL) free_report(r);
    }
    fclose(f_bench);
    
    // Reset del cursore circolare della cache
    contatore_bench_aggiunte = 0; 
    
    // Rigenerazione finale ordinata In-Order dei due indici storici
    rebuild_report_bst_file();
    return true;
}


static void load_master_into_report_id_bst(ReportBST bst, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return;
    
    char line[REPORT_LINE_TOTAL + 1];
    
    // Scorrimento a blocchi binari fissi da 332 byte continui
    while (fread(line, sizeof(char), REPORT_LINE_TOTAL, f) == REPORT_LINE_TOTAL) {
        line[REPORT_LINE_TOTAL] = '\0';
        
        // CORREZIONE CRITICA: Ispezione dell'indice array geometrico 330 per bloccare l'I/O
        if (line[330] == 'E') break;     // Raggiunta la sentinella logica di fine archivio
        if (line[330] == 'V') continue;  // Salto immediato della cella vuota (Buco riciclato)
        
        if (line[0] != ' ' && line[0] != '\n') {
            char rec_state; int row;
            Report r = line_to_report(line, &rec_state, &row);
            if (r != NULL && rec_state == 'A') {
                bst_insert(bst, get_report_id(r), r); // Caricamento nel Punto di Verità
            } else if (r != NULL) {
                free_report(r);
            }
        }
    }
    fclose(f);
}

void rebuild_report_bst_file() {
    // 1. Rigenerazione dell'indice unico di ricerca dei report per ID (332 byte per riga)
    ReportBST bst_rep = create_bst();
    load_master_into_report_id_bst(bst_rep, PATH_OPEN_MASTER);
    load_master_into_report_id_bst(bst_rep, PATH_PROGRESS_MASTER);
    load_master_into_report_id_bst(bst_rep, PATH_CLOSED_MASTER);
    
    FILE* f_bst_rep = fopen(PATH_BST_REPORT_ID, "wb");
    if (f_bst_rep) {
        bst_write_inorder(bst_rep, f_bst_rep, write_report_callback);
        fclose(f_bst_rep);
    }
    free_bst(bst_rep);

    // 2. Rigenerazione dell'indice ridotto delle corrispondenze utente (12 byte per riga)
    ReportBST bst_usr = create_bst();
    const char* paths[] = { PATH_OPEN_MASTER, PATH_PROGRESS_MASTER, PATH_CLOSED_MASTER };
    char line[REPORT_LINE_TOTAL + 1];
    
    for (int i = 0; i < 3; i++) {
        FILE* f = fopen(paths[i], "rb");
        if (f) {
            while (fread(line, sizeof(char), REPORT_LINE_TOTAL, f) == REPORT_LINE_TOTAL) {
                line[REPORT_LINE_TOTAL] = '\0';
                
                // CORREZIONE CRITICA: Ispezione dell'indice array geometrico 330
                if (line[330] == 'E') break;
                if (line[330] == 'V') continue;
                
                if (line[0] != ' ' && line[0] != '\n') {
                    char state; int row;
                    Report r = line_to_report(line, &state, &row);
                    
                    if (r != NULL && state == 'A') {
                        unsigned long hash_user = 5381;
                        const char* u_ptr = get_report_citizen_name(r);
                        int c;
                        while ((c = (unsigned char)*u_ptr++)) {
                            hash_user = ((hash_user << 5) + hash_user) + c;
                        }
                        
                        // Inserimento ed aggregazione logaritmica nel BST utente
                        int final_key = (int)(hash_user % 100000);
                        bst_insert(bst_usr, final_key, r);
                    } else if (r != NULL) {
                        free_report(r);
                    }
                }
            }
            fclose(f);
        }
    }
    FILE* f_bst_usr = fopen(PATH_BST_USER_ID, "wb");
    if (f_bst_usr) {
        bst_write_inorder(bst_usr, f_bst_usr, write_user_bst_callback);
        fclose(f_bst_usr);
    }
    free_bst(bst_usr);
}

/**
 * @brief Carica un file master stazionario all'interno della coda a priorità operativa del server.
 *        Risolve il bug di congelamento della coda ispezionando l'indice 330 del buffer.
 */
static void load_master_into_pq(PriorityQueue pq, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return;
    
    char line[REPORT_LINE_TOTAL + 1];
    
    while (fread(line, sizeof(char), REPORT_LINE_TOTAL, f) == REPORT_LINE_TOTAL) {
        line[REPORT_LINE_TOTAL] = '\0';
        
        // CORREZIONE CRITICA: Ispezione dell'indice array geometrico 330
        if (line[330] == 'E') break;
        if (line[330] == 'V') continue;
        
        if (line[0] != ' ' && line[0] != '\n') {
            char state; int row;
            Report r = line_to_report(line, &state, &row);
            if (r != NULL && state == 'A') {
                pq_enqueue(pq, r); // Accumulo condizionato nella coda
            } else if (r != NULL) {
                free_report(r);
            }
        }
    }
    fclose(f);
}

void rebuild_priority_file() {
    // Svuotamento forzato preventivo della BENCH imposto per garantire la consistenza totale dei master
    process_and_flush_bench();
    
    PriorityQueue pq = create_pq();
    load_master_into_pq(pq, PATH_OPEN_MASTER);
    load_master_into_pq(pq, PATH_PROGRESS_MASTER);
    
    FILE* f_pq = fopen(PATH_PRIORITY_FILE, "wb");
    if (f_pq) {
        while (!pq_is_empty(pq)) {
            Report extracted = pq_dequeue(pq);
            char out_line[REPORT_LINE_TOTAL + 1];
            report_to_line(out_line, extracted, 'A');
            fputs(out_line, f_pq);
            free_report(extracted);
        }
        fclose(f_pq);
    }
    free_pq(pq);
}


bool update_report_state_server(int report_id, ReportStatus new_status) {
    FILE* f_bench = fopen(PATH_BENCH, "rb+");
    if (!f_bench) return false;
    
    char line[REPORT_LINE_TOTAL + 1];
    bool trovato_in_bench = false;
    int current_slot = 0;
    
    /* Scansione a blocchi binari su tutti i 50 slot potenziali della cache */
    while (current_slot < LIMIT_BENCH && fread(line, sizeof(char), REPORT_LINE_TOTAL, f_bench) == REPORT_LINE_TOTAL) {
        line[REPORT_LINE_TOTAL] = '\0';
        
        char state = line[330];
        int row;
        Report tmp = line_to_report(line, &state, &row);
        
        if (tmp && get_report_id(tmp) == report_id && state == 'A') {
            // Aggiornamento dello stato in cache
            update_report_status(tmp, new_status);
            
            char update_line[REPORT_LINE_TOTAL + 1];
            report_to_line(update_line, tmp, 'A');
            
            fseek(f_bench, current_slot * REPORT_LINE_TOTAL, SEEK_SET);
            fputs(update_line, f_bench);
            
            free_report(tmp);
            trovato_in_bench = true;
            break;
        }
        if (tmp) free_report(tmp);
        current_slot++;
    }
    fclose(f_bench);
    
    // Se assente in BENCH, estrae dall'indice unico bst_by_report_id e lo carica in cache
    if (!trovato_in_bench) {
        if (contatore_bench_aggiunte >= LIMIT_BENCH) {
            process_and_flush_bench();
        }

        FILE* f_bst = fopen(PATH_BST_REPORT_ID, "rb");
        if (!f_bst) return false;
        
        Report target_rep = NULL;
        while (fread(line, sizeof(char), REPORT_LINE_TOTAL, f_bst) == REPORT_LINE_TOTAL) {
            line[REPORT_LINE_TOTAL] = '\0';
            char state = line[330];
            int row;
            Report tmp = line_to_report(line, &state, &row);
            
            if (tmp && get_report_id(tmp) == report_id) {
                target_rep = tmp;
                break;
            }
            if (tmp) free_report(tmp);
        }
        fclose(f_bst);
        
        if (!target_rep) return false;
        
        update_report_status(target_rep, new_status);
        
        f_bench = fopen(PATH_BENCH, "rb+");
        if (f_bench) {
            fseek(f_bench, contatore_bench_aggiunte * REPORT_LINE_TOTAL, SEEK_SET);
            char out_line[REPORT_LINE_TOTAL + 1];
            report_to_line(out_line, target_rep, 'A');
            fputs(out_line, f_bench);
            fclose(f_bench);
            
            contatore_bench_aggiunte++; // Avanzamento visibile a tutto l'ecosistema
        }
        free_report(target_rep);
    }
    
    return true;
}
/*
bool update_report_state_server(int report_id, ReportStatus new_status) {
    // Modifica immediata e diretta gestita in cache nella BENCH
    FILE* f_bench = fopen(PATH_BENCH, "rb+");
    if (!f_bench) return false;
    
    char line[REPORT_LINE_TOTAL + 3];
    bool trovato_in_bench = false;
    
    for (int i = 0; i < contatore_bench_aggiunte; i++) {
        fseek(f_bench, i * REPORT_LINE_TOTAL, SEEK_SET);
        if (!fgets(line, sizeof(line), f_bench)) continue;
        
        char state; int row;
        Report tmp = line_to_report(line, &state, &row);
        if (tmp && get_report_id(tmp) == report_id && state == 'A') {
            update_report_status(tmp, new_status);
            char update_line[REPORT_LINE_TOTAL + 3];
            report_to_line(update_line, tmp, 'A');
            fseek(f_bench, i * REPORT_LINE_TOTAL, SEEK_SET);
            fputs(update_line, f_bench);
            free_report(tmp);
            trovato_in_bench = true;
            break;
        }
        if (tmp) free_report(tmp);
    }
    fclose(f_bench);
    
    // Se il record non era in cache, viene estratto dal BST principale e posizionato in BENCH per la modifica
    if (!trovato_in_bench) {
        FILE* f_bst = fopen(PATH_BST_REPORT_ID, "rb");
        if (!f_bst) return false;
        
        Report target_rep = NULL;
        while (fgets(line, sizeof(line), f_bst)) {
            char state; int row;
            Report tmp = line_to_report(line, &state, &row);
            if (tmp && get_report_id(tmp) == report_id) {
                target_rep = tmp;
                break;
            }
            if (tmp) free_report(tmp);
        }
        fclose(f_bst);
        
        if (!target_rep) return false;
        
        // Cambio di stato applicato sulla copia caricata in BENCH
        update_report_status(target_rep, new_status);
        f_bench = fopen(PATH_BENCH, "rb+");
        if (f_bench) {
            fseek(f_bench, contatore_bench_aggiunte * REPORT_LINE_TOTAL, SEEK_SET);
            char out_line[REPORT_LINE_TOTAL + 3];
            report_to_line(out_line, target_rep, 'A');
            fputs(out_line, f_bench);
            fclose(f_bench);
            
            // NOTA DI TRACCIA: Caricare un vecchio report storico NON fa incrementare il contatore di aggiunta
        }
        free_report(target_rep);
    }
    return true;
}
*/
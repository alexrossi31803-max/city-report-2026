#include "../../include/server/report_manager.h"
#include "../../include/utils/parser.h"
#include "../../include/adt/report_bst.h"
#include "../../include/adt/priority_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int contatore_modifiche_sincro = 0;

int generate_global_report_id() {
    // Il calcolo legge quante righe totali sono state accumulate sommandole tra i vari canali master
    int total_id = 1;
    char line[335];
    const char* file_paths[] = { PATH_OPEN_LATEST, PATH_PROGRESS_LATEST, PATH_CLOSED_LATEST };
    
    for (int i = 0; i < 3; i++) {
        FILE* f = fopen(file_paths[i], "r");
        if (f) {
            while (fgets(line, sizeof(line), f)) total_id++;
            fclose(f);
        }
    }
    
    // Conta anche quanti slot occupati ci sono nel bench transitorio attuale
    FILE* f_bench = fopen(PATH_BENCH, "r");
    if (f_bench) {
        while (fgets(line, sizeof(line), f_bench)) {
            if (line[0] != ' ' && line[0] != '\n') {
                // Il flag di stato si trova all'indice 330 (ultimo carattere prima di \n)
                char state = line[330];
                if (state == 'A') total_id++;
            }
        }
        fclose(f_bench);
    }
    
    return total_id;
}
bool flush_session_to_bench(ReportList local_list) {
    if (!local_list) return false;
    
    FILE* f_bench = fopen(PATH_BENCH, "r+");
    if (!f_bench) return false;
    
    // Allargato a 335 per ospitare comodamente la riga da 331 caratteri senza troncamenti
    char line[335]; 
    list_rewind(local_list);
    Report r = list_next(local_list);
    
    int current_slot = 0;
    // Scansiona i 50 slot statici della cache alla ricerca di spazi vuoti
    while (r && current_slot < LIMIT_BENCH) {
        fseek(f_bench, current_slot * 331, SEEK_SET);
        if (!fgets(line, sizeof(line), f_bench)) {
            // Se il file è finito precocemente, lo slot è considerabile libero
            line[0] = '\0';
        }
        
        // Un blocco è vuoto se inizia con spazio, null, newline o ritorno a capo (\r di Windows)
        if (line[0] == ' ' || line[0] == '\0' || line[0] == '\n' || line[0] == '\r') {
            
            // Si posiziona all'inizio dello slot libero per sovrascriverlo
            fseek(f_bench, current_slot * 331, SEEK_SET);
            
            // Allocazione di 340 byte per impedire lo stack overflow durante la scrittura a riga fissa
            char formatted_line[335]; 
            
            // Forza la formattazione a riga fissa attiva 'A'
            report_to_line(formatted_line, r, 'A'); 
            fputs(formatted_line, f_bench);
            
            // Avanza alla prossima segnalazione accumulata nella RAM dinamica
            r = list_next(local_list); 
        }
        current_slot++;
    }
    
    fclose(f_bench);
    
    // Controlla il livello di riempimento complessivo per attivare l'autoflush
    int occupati = 0;
    f_bench = fopen(PATH_BENCH, "r");
    if (f_bench) {
        while (fgets(line, sizeof(line), f_bench)) {
            // Conta la riga come occupata solo se contiene dati grafici effettivi
            if (line[0] != ' ' && line[0] != '\n' && line[0] != '\r' && line[0] != '\0') {
                occupati++;
            }
        }
        fclose(f_bench);
    }
    
    // Se la cache è satura al 90%, invoca il flushing pesante nei database master
    if (occupati >= LIMIT_BENCH - 5) {
        process_and_flush_bench(); 
    }
    
    return true;
}

bool process_and_flush_bench() {
    FILE* f_bench = fopen(PATH_BENCH, "r+");
    if (!f_bench) return false;
    
    // Buffer espansi a 335 per accogliere in sicurezza le righe da 331 caratteri
    char line[335];
    char clear_line[335];
    
    // Prepara la riga vuota di reset da 330 spazi + \n + \0 (331 caratteri totali)
    memset(clear_line, ' ', 330);
    clear_line[330] = '\n';
    clear_line[331] = '\0';
    
    FILE* f_open = fopen(PATH_OPEN_LATEST, "a");
    FILE* f_prog = fopen(PATH_PROGRESS_LATEST, "a");
    FILE* f_clsd = fopen(PATH_CLOSED_LATEST, "a");
    
    if (!f_open || !f_prog || !f_clsd) {
        if (f_open) fclose(f_open);
        if (f_prog) fclose(f_prog);
        if (f_clsd) fclose(f_clsd);
        fclose(f_bench);
        return false;
    }
    
    for (int i = 0; i < LIMIT_BENCH; i++) {
        fseek(f_bench, i * 331, SEEK_SET);
        if (!fgets(line, sizeof(line), f_bench)) continue;
        
        // Verifica se lo slot contiene una segnalazione attiva
        if (line[0] != ' ' && line[0] != '\n' && line[0] != '\r' && line[0] != '\0') {
            char rec_state; int cit_id;
            Report r = line_to_report(line, &rec_state, &cit_id);
            
            if (r != NULL && rec_state == 'A') {
                // Buffer out_line espanso a 335 per supportare la riscrittura fissa
                char out_line[335];
                report_to_line(out_line, r, 'A');
                
                // Smista la riga nel file master corretto
                if (get_report_status(r) == OPEN) fputs(out_line, f_open);
                else if (get_report_status(r) == IN_PROGRESS) fputs(out_line, f_prog);
                else if (get_report_status(r) == CLOSED) fputs(out_line, f_clsd);
                
                contatore_modifiche_sincro++;
            }
            if (r != NULL) free_report(r); 
            
            // Ripristina e pulisce lo slot del bench inserendo la riga di spazi
            fseek(f_bench, i * 331, SEEK_SET);
            fputs(clear_line, f_bench);
        }
    }
    
    fclose(f_open); fclose(f_prog); fclose(f_clsd);
    fclose(f_bench);
    
    // Controllo della soglia critica per la rigenerazione degli indici pesanti
    if (contatore_modifiche_sincro >= SOGLIA_SINCRO) {
        rebuild_report_bst_file();
        rebuild_priority_file();
        contatore_modifiche_sincro = 0;
    }
    
    return true;
}


// Funzione interna per caricare un intero file all'interno del BST organizzandolo per ID_USER
static void load_file_into_bst(ReportBST bst, const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return;
    char line[335];
    while (fgets(line, sizeof(line), f)) {
        char rec_state; int cit_id;
        Report r = line_to_report(line, &rec_state, &cit_id);
        if (rec_state == 'A') {
            bst_insert(bst, cit_id, r); // Carica il report aggregandolo per ID Cittadino
        } else {
            free_report(r);
        }
    }
    fclose(f);
}

void rebuild_report_bst_file() {
    ReportBST bst = create_bst();
    
    // Carica tutti i segmenti storici correnti
    load_file_into_bst(bst, PATH_OPEN_LATEST);
    load_file_into_bst(bst, PATH_PROGRESS_LATEST);
    load_file_into_bst(bst, PATH_CLOSED_LATEST);
    
    FILE* f_bst = fopen(PATH_BST_FILE, "w");
    if (f_bst) {
        // Scrive la sequenza lineare ordinata applicando il callback in-order dell'ADT
        bst_write_inorder(bst, f_bst, write_report_callback);
        fclose(f_bst);
    }
    free_bst(bst); // Dealloca l'albero temporaneo in RAM
}

// Funzione interna per inserire le righe attive di un file dentro la coda a priorità
static void load_file_into_pq(PriorityQueue pq, const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return;
    char line[335];
    while (fgets(line, sizeof(line), f)) {
        char rec_state; int cit_id;
        Report r = line_to_report(line, &rec_state, &cit_id);
        if (rec_state == 'A' && get_report_status(r) != CLOSED) {
            pq_enqueue(pq, r); // Inserisce applicando l'algoritmo FIFO incrociato con l'urgenza
        } else {
            free_report(r);
        }
    }
    fclose(f);
}

void rebuild_priority_file() {
    PriorityQueue pq = create_pq();
    
    // Considera solo le segnalazioni attive escludendo categoricamente i casi risolti 'CLOSED'
    load_file_into_pq(pq, PATH_OPEN_LATEST);
    load_file_into_pq(pq, PATH_PROGRESS_LATEST);
    
    FILE* f_pq = fopen(PATH_PRIORITY_FILE, "w");
    if (f_pq) {
        while (!pq_is_empty(pq)) {
            Report extracted = pq_dequeue(pq);
            char out_line[335];
            report_to_line(out_line, extracted, 'A');
            fputs(out_line, f_pq);
            free_report(extracted);
        }
        fclose(f_pq);
    }
    free_pq(pq);
}

/*bool update_report_state_server(int report_id, ReportStatus current_status, ReportStatus new_status) {
    const char* path_source = (current_status == OPEN) ? PATH_OPEN_LATEST : PATH_PROGRESS_LATEST;
    const char* path_dest = (new_status == IN_PROGRESS) ? PATH_PROGRESS_LATEST : PATH_CLOSED_LATEST;
    
    FILE* f_src = fopen(path_source, "r+");
    if (!f_src) return false;
    
    char line[335]; 
    bool target_trovato = false;
    Report r_aggiornato = NULL;
    int target_citizen_id = 0;
    int current_row = 0;
    
    // Scansione lineare O(n) per riga geometrica fissa (331 caratteri)
    while (fgets(line, sizeof(line), f_src)) {
        char rec_state; int cit_id;
        Report tmp = line_to_report(line, &rec_state, &cit_id);
        
        if (tmp != NULL) {
            if (get_report_id(tmp) == report_id && rec_state == 'A') {
                target_trovato = true;
                target_citizen_id = cit_id;
                
                // Clona l'oggetto per il file di destinazione prima di effettuare la cancellazione logica
                r_aggiornato = create_report(get_report_id(tmp), get_report_citizen_name(tmp),
                                             get_report_category(tmp), get_report_description(tmp),
                                             get_report_date(tmp), get_report_urgency(tmp));
                update_report_status(r_aggiornato, new_status);
                
                // Sovrascrive il record originale nel file sorgente impostando la cancellazione logica 'X'
                // Lascia inalterato lo stato logico di partenza (current_status)
                fseek(f_src, current_row * 331, SEEK_SET);
                char line_buffer[335];
                report_to_line(line_buffer, tmp, 'X'); 
                fputs(line_buffer, f_src);
                
                free_report(tmp);
                break;
            }
            free_report(tmp); // Deallocazione sistematica per prevenire memory leak
        }
        current_row++;
    }
    fclose(f_src);
    
    if (!target_trovato) return false;
    
    // Scrittura (Append) del nuovo record contrassegnato come attivo 'A' nel file di destinazione
    FILE* f_dst = fopen(path_dest, "a");
    if (!f_dst) {
        free_report(r_aggiornato);
        return false;
    }
    
    char dest_line[335]; 
    char fixed_date[11]; 
    char padded_name[MAX_NAME + 1]; 
    char padded_desc[MAX_DESC + 1]; 
    
    pad_string(padded_name, get_report_citizen_name(r_aggiornato), MAX_NAME);
    pad_string(padded_desc, get_report_description(r_aggiornato), MAX_DESC);
    
    strncpy(fixed_date, get_report_date(r_aggiornato), 10); 
    fixed_date[10] = '\0';
    
    // Geometria di scrittura a 331 caratteri totali sincronizzata con il server
    snprintf(dest_line, sizeof(dest_line), "%05d%05d%02d%d%d%s%s%s%c\n",
            get_report_id(r_aggiornato), 
            target_citizen_id, 
            (int)get_report_category(r_aggiornato),
            get_report_urgency(r_aggiornato), 
            (int)get_report_status(r_aggiornato), 
            fixed_date, 
            padded_name, 
            padded_desc, 
            'A');
            
    fputs(dest_line, f_dst);
    fclose(f_dst);
    free_report(r_aggiornato);
    
    // Avanzamento e controllo della soglia di sincronizzazione indici
    contatore_modifiche_sincro++;
    if (contatore_modifiche_sincro >= SOGLIA_SINCRO) {
        rebuild_report_bst_file();
        rebuild_priority_file();
        contatore_modifiche_sincro = 0;
    }
    
    return true;
}*/

bool update_report_state_server(int report_id, ReportStatus current_status, ReportStatus new_status) {
    const char* path_source = (current_status == OPEN) ? PATH_OPEN_LATEST : PATH_PROGRESS_LATEST;
    const char* path_dest = (new_status == IN_PROGRESS) ? PATH_PROGRESS_LATEST : PATH_CLOSED_LATEST;
    
    FILE* f_src = fopen(path_source, "r+");
    if (!f_src) return false;
    
    char line[335]; 
    bool target_trovato = false;
    Report r_aggiornato = NULL;
    int target_citizen_id = 0;
    int current_row = 0;
    
    // Scansione lineare O(n) per riga geometrica fissa
    while (fgets(line, sizeof(line), f_src)) {
        char rec_state; int cit_id;
        Report tmp = line_to_report(line, &rec_state, &cit_id);
        
        if (tmp != NULL) {
            if (get_report_id(tmp) == report_id && rec_state == 'A') {
                target_trovato = true;
                target_citizen_id = cit_id;
                
                // Clona l'oggetto per il file di destinazione prima di effettuare la cancellazione logica
                r_aggiornato = create_report(get_report_id(tmp), get_report_citizen_name(tmp),
                                             get_report_category(tmp), get_report_description(tmp),
                                             get_report_date(tmp), get_report_urgency(tmp));
                update_report_status(r_aggiornato, new_status);
                
                // CORREZIONE WINDOWS COMPATIBLE:
                // Moltiplichiamo per 332 invece di 331. Questo compensa il carattere invisibile '\r'
                // aggiunto automaticamente da Windows (\r\n), posizionando il cursore al millimetro.
                fseek(f_src, current_row * 332, SEEK_SET);
                
                char line_buffer[335];
                // Rigenera la riga originale impostando la cancellazione logica 'X' (Spostato)
                report_to_line(line_buffer, tmp, 'X'); 
                fputs(line_buffer, f_src);
                
                free_report(tmp);
                break;
            }
            free_report(tmp); // Deallocazione sistematica per prevenire memory leak
        }
        current_row++;
    }
    fclose(f_src);
    
    if (!target_trovato) return false;
    
    // Scrittura (Append) del nuovo record contrassegnato come attivo 'A' nel file di destinazione
    FILE* f_dst = fopen(path_dest, "a");
    if (!f_dst) {
        free_report(r_aggiornato);
        return false;
    }
    
    char dest_line[335]; 
    // Usiamo direttamente la nostra report_to_line per scrivere nel file di destinazione,
    // garantendo che la riga venga formattata con la stessa identica geometria del parser.
    report_to_line(dest_line, r_aggiornato, 'A');
            
    fputs(dest_line, f_dst);
    fclose(f_dst);
    free_report(r_aggiornato);
    
    // Avanzamento e controllo della soglia di sincronizzazione indici
    contatore_modifiche_sincro++;
    if (contatore_modifiche_sincro >= SOGLIA_SINCRO) {
        rebuild_report_bst_file();
        rebuild_priority_file();
        contatore_modifiche_sincro = 0;
    }
    
    return true;
}

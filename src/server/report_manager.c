#include "../../include/server/report_manager.h"
#include "../../include/utils/parser.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Variabili Globali (Indici in RAM) --- */
static ReportAVL avl_id_index = NULL;
static ReportAVL avl_user_index = NULL;

void init_report_manager() {
    avl_id_index = create_avl();
    avl_user_index = create_avl();
    // Qui andrebbe la logica di caricamento dai file Derived_Files/report_AVL_...
}

unsigned int get_next_report_id() {
    unsigned int count = 0;
    FILE* f = fopen(PATH_SYS_STATS, "r+");
    if (!f) {
        f = fopen(PATH_SYS_STATS, "w");
        count = 0;
    } else {
        fscanf(f, "%u", &count);
    }
    
    count++;
    rewind(f);
    fprintf(f, "%u", count);
    fclose(f);
    return count;
}

bool save_report_to_bench(Report r) {
    FILE* f = fopen(PATH_BENCH, "a");
    if (!f) return false;

    char line[REPORT_LINE_TOTAL + 1];
    report_to_line(line, r, 'A'); // 'A' = Active (nella Bench)
    fputs(line, f);
    fclose(f);

    // Controllo soglia per Flush automatico
    FILE* f_check = fopen(PATH_BENCH, "r");
    fseek(f_check, 0, SEEK_END);
    long size = ftell(f_check);
    fclose(f_check);

    if (size / REPORT_LINE_TOTAL >= LIMIT_BENCH) {
        return process_and_flush_bench();
    }
    return true;
}

/* --- Logica di Flush e Gestione Buchi --- */

static int get_available_hole(const char* hole_file) {
    FILE* f = fopen(hole_file, "r+");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0) { fclose(f); return -1; }

    // Legge l'ultimo indice inserito (LIFO)
    fseek(f, -11, SEEK_END); // Assumendo indici da 10 cifre + \n
    int hole_row;
    fscanf(f, "%d", &hole_row);
    
    // Rimuove l'indice usato (tronca il file)
    long pos = ftell(f);
    // In C standard non c'è ftruncate, in un sistema reale useremmo fileno/ftruncate
    // Per semplicità accademica, simuliamo il pop.
    fclose(f);
    return hole_row;
}

bool process_and_flush_bench() {
    FILE* fb = fopen(PATH_BENCH, "r");
    if (!fb) return false;

    char line[REPORT_LINE_TOTAL + 1];
    while (fgets(line, sizeof(line), fb)) {
        char status_cell;
        int row;
        Report r = line_to_report(line, &status_cell, &row);
        if (!r) continue;

        // Determina il file master in base allo stato del report
        const char* target_path;
        const char* hole_path;
        ReportStatus s = get_report_status(r);
        
        if (s == OPEN) { target_path = PATH_OPEN; hole_path = HOLES_OPEN; }
        else if (s == IN_PROGRESS) { target_path = PATH_PROGRESS; hole_path = HOLES_PROGRESS; }
        else { target_path = PATH_CLOSED; hole_path = HOLES_CLOSED; }

        // Cerca un buco o va in append
        int target_row = get_available_hole(hole_path);
        FILE* ft = fopen(target_path, "r+");
        if (!ft) ft = fopen(target_path, "w+");

        if (target_row != -1) {
            fseek(ft, target_row * REPORT_LINE_TOTAL, SEEK_SET);
        } else {
            fseek(ft, 0, SEEK_END);
            target_row = ftell(ft) / REPORT_LINE_TOTAL;
        }

        set_report_disk_row(r, target_row);
        char out_line[REPORT_LINE_TOTAL + 1];
        report_to_line(out_line, r, 'A');
        fputs(out_line, ft);

        // Aggiorna indici AVL in RAM
        avl_insert_by_id(avl_id_index, r);
        avl_insert_by_user(avl_user_index, get_report_user_id(r), get_report_id(r));

        fclose(ft);
        free_report(r);
    }

    fclose(fb);
    // Svuota la bench
    fb = fopen(PATH_BENCH, "w");
    fclose(fb);
    return true;
}

Report find_report_by_id(unsigned int report_id) {
    // 1. Cerca nella Bench (lineare, è piccola)
    FILE* fb = fopen(PATH_BENCH, "r");
    char line[REPORT_LINE_TOTAL + 1];
    if (fb) {
        while (fgets(line, sizeof(line), fb)) {
            char status_cell;
            int row;
            Report r = line_to_report(line, &status_cell, &row);
            if (r && get_report_id(r) == report_id) {
                fclose(fb);
                return r;
            }
            if (r) free_report(r);
        }
        fclose(fb);
    }

    // 2. Cerca nell'AVL per trovare la riga disco
    int disk_row = avl_search_disk_row(avl_id_index, report_id);
    if (disk_row != -1) {
        // In un'implementazione completa dovremmo sapere in quale file master cercare.
        // Solitamente si salva lo stato nell'AVL o si provano i 3 file.
        FILE* f = fopen(PATH_OPEN, "r"); // Esempio su uno dei file
        if (f) {
            fseek(f, disk_row * REPORT_LINE_TOTAL, SEEK_SET);
            if (fgets(line, sizeof(line), f)) {
                char dummy_s;
                int dummy_r;
                Report r = line_to_report(line, &dummy_s, &dummy_r);
                fclose(f);
                return r;
            }
            fclose(f);
        }
    }

    return NULL;
}

void shutdown_report_manager() {
    free_avl(avl_id_index);
    free_avl(avl_user_index);
}
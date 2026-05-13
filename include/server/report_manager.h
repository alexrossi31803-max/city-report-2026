#ifndef REPORT_MANAGER_H
#define REPORT_MANAGER_H

#include "../models/report.h"
#include "../adt/report_list.h"
#include <stdbool.h>

#define PATH_BENCH "database/Derived_Files/reports_bench.txt"
#define PATH_OPEN_LATEST "database/Master_Files/open_latest.txt"
#define PATH_PROGRESS_LATEST "database/Master_Files/in_progress_latest.txt"
#define PATH_CLOSED_LATEST "database/Master_Files/closed_latest.txt"
#define PATH_BST_FILE "database/Derived_Files/report_BST_ID_USER.txt"
#define PATH_PRIORITY_FILE "database/Derived_Files/reports_by_priority.txt"

#define LIMIT_BENCH 50
#define SOGLIA_SINCRO 50

// Riversa la Linked List di sessione della RAM del cittadino dentro il reports_bench.txt globale
bool flush_session_to_bench(ReportList local_list);

// Esegue lo svuotamento forzato o automatico del Bench File smistando i dati nei file master e rigenerando il BST
bool process_and_flush_bench();

// Genera il file strutturato ad albero binario serializzato in-order basato sull'aggregazione per ID Utente
void rebuild_report_bst_file();

// Compila la coda a priorità unendo i dati attivi e scrive l'array lineare ordinato in reports_by_priority.txt
void rebuild_priority_file();

// Aggiorna lo stato di una segnalazione modificando il record d'origine e accodando lo stato nei file _latest
bool update_report_state_server(int report_id, ReportStatus current_status, ReportStatus new_status);

// Restituisce l'ID progressivo per inserire un nuovo report unico a livello globale
int generate_global_report_id();

#endif
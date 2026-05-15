#ifndef REPORT_MANAGER_H
#define REPORT_MANAGER_H

#include "../models/report.h"
#include "../adt/report_list.h"
#include "../adt/report_avl.h"
#include <stdbool.h>

/**
 * @brief Inizializza il sistema dei report caricando gli indici AVL in RAM.
 * Da chiamare all'avvio del server.
 */
void init_report_manager();

/**
 * @brief Chiude il sistema, salvando gli indici AVL su disco e liberando la RAM.
 */
void shutdown_report_manager();

/**
 * @brief Ottiene l'ID univoco per un nuovo report e incrementa il contatore globale.
 * Legge da database/Master_Files/system_total_report.txt
 */
unsigned int get_next_report_id();

/**
 * @brief Salva un report nella Bench (database/Derived_Files/reports_bench).
 * Se la Bench raggiunge LIMIT_BENCH, scatta automaticamente il process_and_flush_bench().
 */
bool save_report_to_bench(Report r);

/**
 * @brief Esegue il commit dei dati dalla Bench ai file Master.
 * Fase 1: Spostamento record e gestione buchi (LIFO tramite null_pointer.txt).
 * Fase 2: Ricostruzione AVL ID (Derived_Files/report_AVL_BY_REPORT_ID.txt).
 * Fase 3: Ricostruzione AVL User (Derived_Files/report_AVL_BY_USER_ID.txt).
 */
bool process_and_flush_bench();

/**
 * @brief Ricerca un report per ID.
 * Gerarchia: 1. Bench -> 2. AVL Index -> 3. File Master corrispondente.
 */
Report find_report_by_id(unsigned int report_id);

/**
 * @brief Recupera tutti i report di un utente tramite l'AVL dedicato.
 */
ReportList get_reports_by_user(unsigned int user_id);

/**
 * @brief Aggiorna lo stato di un report esistente (es. da OPEN a PROGRESS).
 * Gestisce lo spostamento fisico tra i file master se lo stato cambia.
 */
bool update_report_status(unsigned int report_id, ReportStatus new_status);

/**
 * @brief Genera le statistiche di sistema leggendo i contatori e gli indici.
 */
void generate_system_statistics();

#endif
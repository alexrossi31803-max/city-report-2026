#ifndef REPORT_LIST_H
#define REPORT_LIST_H

#include "../models/report.h"
#include <stdbool.h>

/**
 * @brief Puntatore opaco alla struttura della lista.
 * Implementa l'Information Hiding.
 */
typedef struct ReportList* ReportList;

/**
 * @brief Crea e inizializza una nuova lista di report in RAM.
 * @return Puntatore alla lista creata.
 */
ReportList create_list();

/**
 * @brief Dealloca la lista e tutti i report contenuti.
 * Da invocare a fine sessione o dopo il flush su disco.
 */
void free_list(ReportList l);

/**
 * @brief Inserisce un report in testa alla lista.
 * Complessità: O(1).
 */
void list_insert(ReportList l, Report r);

/**
 * @brief Rimuove un report dalla lista in base all'ID.
 * Utile se l'utente decide di eliminare una segnalazione prima del logout.
 * @return true se rimosso, false se non trovato.
 */
bool list_remove(ReportList l, unsigned int report_id);

/**
 * @brief Cerca un report nella lista tramite ID.
 */
Report list_find(ReportList l, unsigned int report_id);

/**
 * @brief Restituisce il numero di elementi presenti.
 */
int list_size(ReportList l);

/* --- Funzioni Iteratore (per visualizzazione UI) --- */

/**
 * @brief Riporta il puntatore di navigazione all'inizio della lista.
 */
void list_rewind(ReportList l);

/**
 * @brief Restituisce il report corrente e sposta il puntatore al prossimo.
 * @return Il report corrente o NULL se la lista è terminata.
 */
Report list_next(ReportList l);

#endif

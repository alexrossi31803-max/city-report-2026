#ifndef REPORT_MANAGER_H
#define REPORT_MANAGER_H

#include "../models/report.h"
#include "../adt/report_list.h"

/**
 * @brief Genera e incrementa in O(1) l'ID unico globale leggendo il registro.
 * @return L'identificatore numerico unsigned int generato.
 */
unsigned int generate_global_report_id_v2(void);

/**
 * @brief Esegue l'operazione di Flush Pesante della cache BENCH verso i tre file Master.
 *        Applica il riciclo LIFO dei buchi in O(1) e rigenera i due indici ordinati su disco.
 * @return true se l'operazione transazionale va a buon fine, false altrimenti.
 */
bool process_and_flush_bench_v2(void);

/**
 * @brief Rigenera da zero i file d'indice binarizzati costruendo i due alberi AVL ad Information Hiding.
 *        Esegue il dump In-Order a larghezza fissa (22 byte e 21 byte) e aggiorna i registri finali.
 */
void rebuild_avl_indices_server(void);

/**
 * @brief Gestisce l'immissione iniziale controllata dei report OPEN inviati dal cittadino in cache.
 *        Applica i filtri anti-duplicazione in BENCH e AVL stornando atomicamente le categorie in O(1).
 * @param r Oggetto Report opaco da validare e inserire.
 * @return true ad inserimento o sovrascrittura completata, false in caso di errore I/O.
 */
bool register_report_from_citizen_ram(Report r);

#endif

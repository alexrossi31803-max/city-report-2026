#ifndef REPORT_MANAGER_H
#define REPORT_MANAGER_H

#include "../models/report.h"
#include "../adt/report_list.h"

/**
 * @brief Genera e incrementa in O(1) l'ID unico globale per un nuovo report tramite file di registro.
 */
unsigned int generate_global_report_id_v2(void);

/**
 * @brief Svuota la cache BENCH riversando i record attivi nei 3 master ed eseguendo il riciclo LIFO dei buchi.
 */
bool process_and_flush_bench_v2(void);

/**
 * @brief Rigenera da zero i file d'indice binarizzati costruendo i due alberi AVL auto-bilancianti.
 */
void rebuild_avl_indices_server(void);

/**
 * @brief Inserisce un report inviato dal cittadino nella BENCH gestendo sovrascritture anti-duplicazione e contatori.
 */
bool register_report_from_citizen_ram(Report r);

#endif

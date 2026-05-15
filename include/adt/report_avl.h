#ifndef REPORT_AVL_H
#define REPORT_AVL_H

#include "../models/report.h"

typedef struct ReportAVL* ReportAVL;

ReportAVL create_avl();
void free_avl(ReportAVL t);

/* Inserimento per ID (Fase 2 del Flush) */
void avl_insert_by_id(ReportAVL t, Report r);

/* Inserimento per User (Fase 3 del Flush) */
void avl_insert_by_user(ReportAVL t, unsigned int user_id, unsigned int report_id);

/* Ricerca riga disco per ID */
int avl_search_disk_row(ReportAVL t, unsigned int report_id);

#endif


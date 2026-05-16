#ifndef REPORT_AVL_H
#define REPORT_AVL_H

#include <stdio.h>
#include "../models/report.h"

typedef struct ReportAVL* ReportAVL;

ReportAVL create_avl();
void free_avl(ReportAVL t);

/**
 * @brief Inserimento bilanciato AVL strutturato sulla chiave univoca report_id.
 * @pre t inizializzato, r istanza di report valida e configurata sul disco.
 */
void avl_insert_by_report_id(ReportAVL t, unsigned int report_id, Report r);

/**
 * @brief Inserimento e aggregazione logaritmica AVL strutturata sulla chiave user_id.
 * @pre t inizializzato, supporta l'accorpamento dinamico di più report_id per cittadino.
 */
void avl_insert_by_user_id(ReportAVL t, unsigned int user_id, unsigned int report_id);

/**
 * @brief Attraversamento simmetrico In-Order dell'albero con riversamento sul file d'indice binarizzato.
 */
void avl_write_inorder(ReportAVL t, FILE* f_out, void (*write_func)(FILE*, unsigned int, unsigned int, int, char));

#endif



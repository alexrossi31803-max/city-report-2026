#ifndef REPORT_AVL_H
#define REPORT_AVL_H

#include <stdio.h>
#include <stdbool.h>
#include "report_list.h"

/* Definizione del tipo enumerativo per le modalita operative */
typedef enum {
   TYPE_AVL_UID, /* [user_id(10)][report_id(10)]\n -> 21 byte */
   TYPE_AVL_RID  /* [report_id(10)][status(1)][row(10)]\n -> 22 byte */
} Type_Avl;

/* Struttura opaca esposta all'esterno */
typedef struct nodeAVL *ReportAvl;

/* Funzioni di base richieste dalla specifica */
ReportAvl createNode(void *elem, Type_Avl type);
int height(ReportAvl t);
int size(ReportAvl t);
int balanceFactor(ReportAvl t);
void updateHeight(ReportAvl t);

/* Rotazioni dell'albero AVL */
ReportAvl rotateLeft(ReportAvl t);
ReportAvl rotateRight(ReportAvl t);
ReportAvl rotateLeftRight(ReportAvl t);
ReportAvl rotateRightLeft(ReportAvl t);
ReportAvl rebalance(ReportAvl t);

/* Inserimento bilanciato nell'AVL */
ReportAvl insert(
   ReportAvl t,
   void *elem,
   Type_Avl type,
   int (*compare)(const void *, const void *)
);

/* Visita simmetrica per la persistenza e generazione del file ordinato */
void inorder(ReportAvl t, FILE *file);

/* Ricerca univoca per REPORT_ID (Restituisce la disk_row memorizzata o -1) */
int findReportId(unsigned int report_id);

/* Interroga direttamente l'indice User ID su disco (21 byte) tramite Ricerca Binaria */
int findUserId(
   unsigned int uid,
   unsigned int *results
);
/* Deallocazione ricorsiva sicura della memoria */
void free_avl_tree(ReportAvl t);

#endif





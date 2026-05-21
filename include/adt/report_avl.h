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

/* -Interroga direttamente l'indice Report_id su disco (22 byte) tramite Ricerca Binaria. 
   -Ricerca univoca per REPORT_ID 
   -Usa report id come parametro di confronto 
   -Restituisce la disk_row memorizzata o -1
   */
   
/*
La funzione findReportId interroga l'inorder array persistito sul disco in tempo logaritmico certo pari a O(logn). Poiché l'albero AVL effettua lo 
scaricamento ordinato dei blocchi a passi fissi da 22 byte (AVL_REPORT_ID_LINE), il server legge dal registro comunale il numero complessivo di record 
censiti ed esegue salti logaritmici mediante combinazioni di fseek posizionali sul punto medio (mid). Questo azzera la necessità di effettuare scansioni 
sequenziali lineari in O(n), garantendo che la ricerca della riga fisica Master rimanga efficiente ed esente dal volume complessivo dei dati stoccati."
*/
int findReportId(unsigned int report_id);

/* -Interroga direttamente l'indice User ID su disco (21 byte) tramite Ricerca Binaria.  
   -Usa uid ovvero user_id come parametro di confronto
   -Restituisce il numero esatto di segnalazioni storiche isolate di un user_id
   -Alloca ed espande dinamicamente l'array dei risultati results
   */
/*
   "La ricerca delle segnalazioni storiche di un singolo cittadino è governata dalla funzione findUserId, la quale implementa una Ricerca Binaria 
   Logaritmica ad espansione bilaterale contigua. Sfruttando l'ordinamento nativo dell'Inorder Array generato dall'albero AVL, il server localizza un 
   punto di contatto casuale all'interno del file a passi da 21 byte in tempo O(logn).
   Trovato l'intervallo, un puntatore scansiona a ritroso il blocco per isolare l'indice di partenza esatto, e un ciclo sequenziale estrae i Report ID
   associati riallocando dinamicamente la memoria RAM tramite raddoppio geometrico predittivo. Questo approccio garantisce la totale scalabilità del sistema,
   isolando la memoria tramite una doppia referenza ed evitando il degradamento prestazionale dell'I/O sul disco."
   */
int findUserId(
   unsigned int uid,
   unsigned int **results
);


/* Deallocazione ricorsiva sicura della memoria */
void free_avl_tree(ReportAvl t);

#endif





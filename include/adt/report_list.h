#ifndef REPORT_LIST_H
#define REPORT_LIST_H

#include "../models/report.h"
#include <stdbool.h>

/**
 * @brief Struttura opaca (Opaque Pointer) per l'incapsulamento della lista concatenata.
 *        Garantisce il principio dell'Information Hiding oscurando l'implementazione fisica.
 */
typedef struct ReportList* ReportList;

/**
 * @brief Costruttore dell'ADT ReportList. Alloca lo spazio per la struttura di controllo.
 * @return Un'istanza valida di tipo ReportList inizializzata, o NULL in caso di fallimento RAM.
 */
ReportList create_list(void);

/**
 * @brief Distruttore dell'ADT ReportList. Libera ricorsivamente tutta la memoria occupata.
 * @param l La lista da deallocare. Svuota sia i nodi di giunzione che le istanze opache Report.
 */
void free_list(ReportList l);

/**
 * @brief Inserimento immediato in tempo costante O(1) di un nuovo elemento.
 * @param l La lista in cui inserire il record.
 * @param r L'istanza dell'oggetto Report da memorizzare.
 * @note L'inserimento avviene nativamente in testa per massimizzare l'efficienza temporale.
 */
void list_insert(ReportList l, Report r);

/**
 * @brief Rimuove in modo selettivo un report dalla lista in base al suo identificativo univoco.
 * @param l La lista da cui sfoltire l'elemento.
 * @param report_id Il codice identificativo numerico senza segno del report da eliminare.
 * @return true ad avvenuta estrazione e deallocazione del nodo, false se l'ID non compare nella lista.
 */
bool list_remove(ReportList l, int report_id);

/**
 * @brief Ricerca lineare di un report all'interno della lista RAM di sessione locale.
 * @param l La lista in cui cercare.
 * @param report_id Il codice identificativo numerico del report desiderato.
 * @return Il puntatore opaco all'oggetto Report se presente, NULL se non viene trovato alcun match.
 */
Report list_find(ReportList l, int report_id);

/**
 * @brief Resetta l'iteratore interno posizionandolo nuovamente in testa alla lista.
 *        Indispensabile prima di innescare un ciclo di visualizzazione o scansione orizzontale.
 * @param l La lista su cui resettare i puntatori di scorrimento.
 */
void list_rewind(ReportList l);

/**
 * @brief Avanza l'iteratore interno e restituisce l'elemento corrente.
 * @param l La lista da scorrere sequenzialmente.
 * @return Il puntatore opaco all'oggetto Report corrente, o NULL se la navigazione raggiunge la coda.
 */
Report list_next(ReportList l);

/**
 * @brief Restituisce il numero corrente di elementi attivi memorizzati nella lista.
 * @param l La lista da interrogare.
 * @return Il contatore interno degli elementi (size) in tempo costante O(1).
 */
int list_size(ReportList l);

#endif



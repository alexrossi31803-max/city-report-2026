#ifndef REPORT_STACK_H
#define REPORT_STACK_H

#include "../models/report.h"
#include <stdbool.h>

/**
 * @brief Puntatore opaco allo Stack (Information Hiding).
 */
typedef struct ReportStack* ReportStack;

/**
 * @brief Crea uno stack vuoto per la gestione dell'Undo.
 * @return Puntatore allo stack.
 */
ReportStack create_stack();

/**
 * @brief Dealloca lo stack. 
 * Nota: non dealloca i report contenuti, poiché sono gestiti dalla ReportList.
 */
void free_stack(ReportStack s);

/**
 * @brief Inserisce un riferimento al report nello stack (Azione eseguita).
 * @param s Lo stack.
 * @param r Il report nello stato attuale.
 */
void stack_push(ReportStack s, Report r);

/**
 * @brief Estrae l'ultimo report inserito (Undo).
 * @return Il puntatore al report o NULL se lo stack è vuoto.
 */
Report stack_pop(ReportStack s);

/**
 * @brief Verifica se ci sono azioni da annullare.
 */
bool stack_is_empty(ReportStack s);

/**
 * @brief Restituisce il numero di azioni memorizzate nello stack.
 */
int stack_size(ReportStack s);

#endif

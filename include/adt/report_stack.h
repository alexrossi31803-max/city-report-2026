#ifndef REPORT_STACK_H
#define REPORT_STACK_H

#include "../models/report.h"
#include <stdbool.h>

/**
 * @brief Struttura opaca (Opaque Pointer) per l'incapsulamento dello stack.
 *        Nasconde i dettagli della preallocazione statica garantendo l'Information Hiding.
 */
typedef struct ReportStack* ReportStack;

/**
 * @brief Costruttore dell'ADT ReportStack. Alloca la struttura di controllo in RAM.
 * @return Un'istanza valida di tipo ReportStack inizializzata, o NULL in caso di fallimento della malloc.
 */
ReportStack create_stack(void);

/**
 * @brief Distruttore dell'ADT ReportStack. Libera tutta la memoria associata.
 * @param s Lo stack da deallocare. Svuota in modo controllato i report clonati rimasti in cima.
 */
void free_stack(ReportStack s);

/**
 * @brief Inserisce un nuovo elemento in cima allo stack (operazione Push) in tempo costante O(1).
 *        Esegue una clonazione profonda (deep copy) dell'oggetto per isolarlo da modifiche RAM successive.
 * @param s Lo stack in cui inserire il record.
 * @param r L'istanza dell'oggetto Report da salvare come punto di ripristino.
 * @return true se l'operazione riesce, false se lo stack è saturo (raggiunti i 10 elementi massimi).
 */
bool stack_push(ReportStack s, Report r);

/**
 * @brief Estrae e rimuove l'ultimo elemento inserito in cima allo stack (operazione Pop) in O(1).
 * @param s Lo stack da cui estrarre il record.
 * @return Il puntatore opaco all'oggetto Report clonato in precedenza, o NULL se lo stack è scarico.
 */
Report stack_pop(ReportStack s);

/**
 * @brief Ispeziona l'elemento in cima allo stack senza rimuoverlo (operazione Top/Peek) in O(1).
 * @param s Lo stack da ispezionare.
 * @return Il puntatore opaco all'oggetto Report in cima, o NULL se lo stack è vuoto.
 */
Report stack_top(ReportStack s);

/**
 * @brief Verifica se lo stack è privo di elementi.
 * @param s Lo stack da controllare.
 * @return true se lo stack è scarico (top == -1), false altrimenti.
 */
bool stack_is_empty(ReportStack s);

/**
 * @brief Restituisce il numero di elementi attualmente contenuti nello stack.
 * @param s Lo stack da interrogare.
 * @return Il conteggio numerico intero degli elementi residenti in RAM.
 */
int stack_size(ReportStack s);

#endif


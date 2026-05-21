#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "../models/report.h"
#include <stdbool.h>

/**
 * @brief Struttura opaca (Opaque Pointer) per l'incapsulamento della Coda a Priorità.
 *        Garantisce il principio dell'Information Hiding, nascondendo all'esterno 
 *        i nodi e l'algoritmo di inserimento ordinato in memoria RAM.
 */
typedef struct PriorityQueue* PriorityQueue;

/**
 * @brief Costruttore dell'ADT PriorityQueue. Alloca lo spazio per la struttura di controllo.
 * @return Un'istanza valida di tipo PriorityQueue inizializzata, o NULL in caso di fallimento RAM.
 */
PriorityQueue create_pq(void);

/**
 * @brief Distruttore dell'ADT PriorityQueue. Libera ricorsivamente tutta la memoria occupata.
 * @param pq La coda a priorità da deallocare. Svuota sia i nodi di giunzione che le istanze opache Report.
 */
void free_pq(PriorityQueue pq);

/**
 * @brief Inserimento condizionato e ordinato di un report nella coda (operazione Enqueue).
 *        L'algoritmo effettua un inserimento ordinato in RAM basato su criteri incrociati:
 *        priorità all'urgenza scalare decrescente ('2' -> '1' -> '0') e, a parità, data FIFO remota.
 * @param pq La coda a priorità in cui inserire il record.
 * @param r L'istanza dell'oggetto Report da memorizzare.
 */
void pq_enqueue(PriorityQueue pq, Report r);

/**
 * @brief Estrae e rimuove l'elemento a massima priorità assoluta dalla testa della coda (operazione Dequeue) in O(1).
 * @param pq La coda a priorità da cui estrarre il record.
 * @return Il puntatore opaco all'oggetto Report estratto, o NULL se la coda è scarica.
 */
Report pq_dequeue(PriorityQueue pq);

/**
 * @brief Ispeziona l'elemento a massima priorità in testa alla coda senza rimuoverlo (operazione Peek/Top) in O(1).
 * @param pq La coda a priorità da ispezionare.
 * @return Il puntatore opaco all'oggetto Report in testa, o NULL se la coda è vuota.
 */
Report pq_peek(PriorityQueue pq);

/**
 * @brief Verifica se la coda a priorità è priva di elementi.
 * @param pq La coda da controllare.
 * @return true se la coda è scarica (size == 0), false altrimenti.
 */
bool pq_is_empty(PriorityQueue pq);

/**
 * @brief Restituisce il numero corrente di elementi attivi memorizzati nella coda a priorità.
 * @param pq La coda da interrogare.
 * @return Il contatore interno degli elementi (size) in tempo costante O(1).
 */
int pq_size(PriorityQueue pq);

#endif


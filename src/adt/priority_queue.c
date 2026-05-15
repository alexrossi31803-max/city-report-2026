#include "../../include/adt/priority_queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Nodo interno della Coda a Priorità */
typedef struct PQNode {
    Report data;
    struct PQNode* next;
} PQNode;

/* Descrittore della Coda (Opaque Pointer) */
struct PriorityQueue {
    PQNode* head;
    int size;
};

/**
 * @brief Confronta due report per determinare la precedenza.
 * @return > 0 se r1 ha priorità maggiore di r2, < 0 altrimenti.
 */
static int compare_priority(Report r1, Report r2) {
    // 1. Confronto per Urgenza (3: Alta, 2: Media, 1: Bassa)
    if (get_report_urgency(r1) != get_report_urgency(r2)) {
        return get_report_urgency(r1) - get_report_urgency(r2); 
    }

    // 2. A parità di urgenza, criterio FIFO (Data più vecchia ha la precedenza)
    // strcmp su "AAAA/MM/GG" funzionerebbe direttamente, 
    // ma con "GG/MM/AAAA" confrontiamo le date in modo che la minore (più vecchia) vinca.
    // Invertiamo il risultato di strcmp perché vogliamo che la data "minore" sia in testa.
    return strcmp(get_report_date(r2), get_report_date(r1));
}

PriorityQueue create_pq() {
    PriorityQueue pq = (PriorityQueue)malloc(sizeof(struct PriorityQueue));
    if (pq) {
        pq->head = NULL;
        pq->size = 0;
    }
    return pq;
}

void free_pq(PriorityQueue pq) {
    if (!pq) return;
    PQNode* curr = pq->head;
    while (curr) {
        PQNode* next = curr->next;
        // Non liberiamo il Report qui perché è gestito dal sistema master/cache
        free(curr);
        curr = next;
    }
    free(pq);
}

void pq_enqueue(PriorityQueue pq, Report r) {
    if (!pq || !r) return;

    PQNode* new_node = (PQNode*)malloc(sizeof(PQNode));
    if (!new_node) return;
    new_node->data = r;

    // Caso 1: Coda vuota o nuovo report ha priorità maggiore della testa
    if (!pq->head || compare_priority(r, pq->head->data) > 0) {
        new_node->next = pq->head;
        pq->head = new_node;
    } 
    else {
        // Caso 2: Ricerca della posizione corretta (Inserimento ordinato)
        PQNode* curr = pq->head;
        while (curr->next && compare_priority(r, curr->next->data) <= 0) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
    
    pq->size++;
}

Report pq_dequeue(PriorityQueue pq) {
    if (!pq || !pq->head) return NULL;

    PQNode* temp = pq->head;
    Report r = temp->data;
    
    pq->head = pq->head->next;
    free(temp);
    pq->size--;

    return r;
}

bool pq_is_empty(PriorityQueue pq) {
    return (pq == NULL || pq->head == NULL);
}

int pq_size(PriorityQueue pq) {
    return pq ? pq->size : 0;
}
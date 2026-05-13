#include "../../include/adt/priority_queue.h"
#include <stdlib.h>
#include <string.h>

typedef struct PQNode {
    Report data;
    struct PQNode* next;
} PQNode;

struct PriorityQueue {
    PQNode* head;
};

PriorityQueue create_pq() {
    PriorityQueue pq = (PriorityQueue)malloc(sizeof(struct PriorityQueue));
    if (pq) pq->head = NULL;
    return pq;
}

void free_pq(PriorityQueue pq) {
    if (!pq) return;
    PQNode* curr = pq->head;
    while (curr) {
        PQNode* next = curr->next;
        free_report(curr->data);
        free(curr);
        curr = next;
    }
    free(pq);
}

// Funzione di confronto priorità (Criterio 1: Urgenza descrescente, Criterio 2: Data più vecchia)
static int confronta_priorita(Report r1, Report r2) {
    if (get_report_urgency(r1) != get_report_urgency(r2)) {
        return get_report_urgency(r1) - get_report_urgency(r2); // Positivo se r1 ha urgenza maggiore
    }
    // A parità di urgenza, si confronta la stringa data "GG/MM/AAAA" in ordine invertito (o timestamp)
    // Per semplicità accademica assumiamo un confronto di stringhe temporali standard
    return strcmp(get_report_date(r2), get_report_date(r1)); 
}

void pq_enqueue(PriorityQueue pq, Report r) {
    if (!pq || !r) return;
    PQNode* new_node = (PQNode*)malloc(sizeof(PQNode));
    if (!new_node) return;
    new_node->data = r;

    // Inserimento ordinato nella lista della coda
    if (!pq->head || confronta_priorita(r, pq->head->data) > 0) {
        new_node->next = pq->head;
        pq->head = new_node;
    } else {
        PQNode* curr = pq->head;
        while (curr->next && confronta_priorita(r, curr->next->data) <= 0) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
}

Report pq_dequeue(PriorityQueue pq) {
    if (!pq || pq_is_empty(pq)) return NULL;
    PQNode* temp = pq->head;
    Report r = temp->data;
    pq->head = pq->head->next;
    free(temp);
    return r;
}

bool pq_is_empty(PriorityQueue pq) {
    return (!pq || pq->head == NULL);
}

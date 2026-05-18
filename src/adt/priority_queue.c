#include "../../include/adt/priority_queue.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Struttura interna del singolo nodo di giunzione della coda a priorità.
 */
typedef struct PQNode {
    Report data;          /* Puntatore opaco all'istanza dell'oggetto reale Report */
    struct PQNode* next;  /* Collegamento logico al nodo successivo della catena */
} PQNode;

/**
 * @brief Struttura di controllo principale dell'ADT PriorityQueue (Oggetto istanziato in RAM).
 */
struct PriorityQueue {
    PQNode* head;         /* Puntatore al primo nodo (testa della coda, elemento a massima priorità) */
    int size;             /* Contatore per restituire la dimensione della struttura in O(1) */
};

/**
 * @brief Converte e analizza le stringhe temporali delle date (formato GG/MM/AAAA)
 *        restituendo un valore comparabile per stabilire quale sia la più remota (ordinamento FIFO).
 * @return Un intero negativo se date1 è più remota di date2, positivo se più recente, 0 se identiche.
 */
static int compare_dates_fifo(const char* date1, const char* date2) {
    if (!date1 || !date2 || strlen(date1) != 10 || strlen(date2) != 10) return 0;

    int day1 = (date1[0] - '0') * 10 + (date1[1] - '0');
    int month1 = (date1[3] - '0') * 10 + (date1[4] - '0');
    int year1 = (date1[6] - '0') * 1000 + (date1[7] - '0') * 100 + (date1[8] - '0') * 10 + (date1[9] - '0');

    int day2 = (date2[0] - '0') * 10 + (date2[1] - '0');
    int month2 = (date2[3] - '0') * 10 + (date2[4] - '0');
    int year2 = (date2[6] - '0') * 1000 + (date2[7] - '0') * 100 + (date2[8] - '0') * 10 + (date2[9] - '0');

    if (year1 != year2) return year1 - year2;
    if (month1 != month2) return month1 - month2;
    return day1 - day2;
}

PriorityQueue create_pq(void) {
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
    /* Svuotamento ricorsivo per azzerare i memory leak eliminando nodi e report interni */
    while (curr) {
        PQNode* next_node = curr->next;
        free_report(curr->data);
        free(curr);
        curr = next_node;
    }
    free(pq);
}

void pq_enqueue(PriorityQueue pq, Report r) {
    if (!pq || !r) return;

    PQNode* new_node = (PQNode*)malloc(sizeof(PQNode));
    if (!new_node) return;
    new_node->data = r;
    new_node->next = NULL;

    /* SEZIONE DI CONFRONTO STRUTTURATA SULLA CRITICITÀ INCROCIATA (ALGEBRA DELLE PRIORITÀ) */
    PQNode* curr = pq->head;
    PQNode* prev = NULL;

    char new_urgency = get_report_urgency(r);
    const char* new_date = get_report_date(r);

    /* Scorre la coda per individuare il punto esatto di inserimento ordinato */
    while (curr) {
        char curr_urgency = get_report_urgency(curr->data);
        
        /* RECOLA 1: Se l'urgenza del nuovo report è maggiore di quella del nodo corrente, ha priorità alta */
        if (new_urgency > curr_urgency) {
            break;
        }
        
        /* REGOLA 2: A parità di urgenza scalare, scatta il controllo FIFO temporale sulla data più remota */
        if (new_urgency == curr_urgency) {
            const char* curr_date = get_report_date(curr->data);
            /* Se la nuova data è più remota (valore negativo), deve precedere il record corrente */
            if (compare_dates_fifo(new_date, curr_date) < 0) {
                break;
            }
        }
        
        prev = curr;
        curr = curr->next;
    }

    /* Inserimento geometrico del nodo all'interno della catena RAM */
    if (!prev) {
        /* Il nuovo elemento si posiziona in testa alla coda (massima priorità corrente) */
        new_node->next = pq->head;
        pq->head = new_node;
    } else {
        /* Il nuovo elemento si posiziona in mezzo o in coda alla struttura */
        new_node->next = curr;
        prev->next = new_node;
    }
    pq->size++; /* Incremento atomico della dimensione */
}

Report pq_dequeue(PriorityQueue pq) {
    if (!pq || !pq->head) return NULL;

    /* Estrazione immediata della testa in O(1), che per costruzione è l'elemento a priorità massima */
    PQNode* temp_node = pq->head;
    Report r = temp_node->data;
    
    pq->head = pq->head->next; /* Avanza la testa al nodo successivo */
    free(temp_node);           /* Dealloca il nodo di giunzione */
    pq->size--;                /* Decremento immediato per consistenza della size */
    
    return r;
}

Report pq_peek(PriorityQueue pq) {
    /* Ispezione pura della testa senza alterazione dei puntatori di catena */
    return (pq && pq->head) ? pq->head->data : NULL;
}

bool pq_is_empty(PriorityQueue pq) {
    return (pq == NULL || pq->size == 0);
}

int pq_size(PriorityQueue pq) {
    return pq ? pq->size : 0;
}


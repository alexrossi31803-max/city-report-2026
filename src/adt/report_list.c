#include "../../include/adt/report_list.h"
#include <stdlib.h>

/**
 * @brief Struttura interna del singolo nodo di giunzione della lista concatenata.
 */
typedef struct Node {
    Report data;        /* Puntatore opaco all'istanza dell'oggetto reale Report */
    struct Node* next;  /* Collegamento logico al nodo successivo della catena */
} Node;

/**
 * @brief Struttura di controllo principale dell'ADT ReportList (Oggetto istanziato in RAM).
 */
struct ReportList {
    Node* head;         /* Puntatore al primo nodo (testa della lista) */
    Node* current;      /* Puntatore di supporto per implementare l'iteratore esterno nei cicli */
    int size;           /* Contatore degli elementi per restituire la dimensione in tempo costante O(1) */
};

ReportList create_list(void) {
    ReportList l = (ReportList)malloc(sizeof(struct ReportList));
    if (l) {
        l->head = NULL;
        l->current = NULL;
        l->size = 0; /* Una lista appena nata contiene zero elementi */
    }
    return l;
}

void free_list(ReportList l) {
    if (!l) return;
    Node* curr = l->head;
    /* Scorrimento sequenziale distruttivo per azzerare i memory leak */
    while (curr) {
        Node* next_node = curr->next;
        /* Richiamo obbligatorio del distruttore specifico del modello Report per liberare i campi interni */
        free_report(curr->data);
        free(curr); /* Libera la struttura del nodo di giunzione in RAM */
        curr = next_node;
    }
    free(l); /* Libera la struttura di controllo principale dell'ADT */
}

void list_insert(ReportList l, Report r) {
    if (!l || !r) return;
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return;
    
    /* Configurazione dei puntatori per l'inserimento in testa in tempo costante O(1) */
    new_node->data = r;
    new_node->next = l->head;
    l->head = new_node;
    l->size++; /* Incremento atomico del contatore di dimensione */
}

bool list_remove(ReportList l, int report_id) {
    if (!l || !l->head) return false;
    Node* curr = l->head;
    Node* prev = NULL;

    /* Scansione lineare alla ricerca del codice identificativo specificato */
    while (curr) {
        /* Cast esplicito coerente con le chiavi numeriche senza segno get_report_id */
        if ((int)get_report_id(curr->data) == report_id) {
            /* Sgancio geometrico dei puntatori per escludere il nodo corrente dalla catena */
            if (!prev) {
                l->head = curr->next; /* Il nodo da eliminare era la testa della lista */
            } else {
                prev->next = curr->next; /* Il nodo si trovava in posizione intermedia o in coda */
            }
            
            /* Cancellazione fisica della memoria sia dell'anagrafica che del contenitore */
            free_report(curr->data);
            free(curr);
            l->size--; /* Decremento immediato per mantenere la consistenza in O(1) */
            return true; /* Rimozione completata con successo */
        }
        prev = curr;
        curr = curr->next;
    }
    return false; /* Codice non presente all'interno della lista */
}

Report list_find(ReportList l, int report_id) {
    if (!l) return NULL;
    Node* curr = l->head;
    /* Ispezione sequenziale alla ricerca del match numerico */
    while (curr) {
        if ((int)get_report_id(curr->data) == report_id) {
            return curr->data; /* Corrispondenza localizzata: restituisce il puntatore opaco */
        }
        curr = curr->next;
    }
    return NULL; /* L'elemento non risiede nella memoria volatile di sessione */
}

void list_rewind(ReportList l) {
    /* Riposiziona il cursore mobile dell'iteratore all'inizio esatto della lista (testa) */
    if (l) l->current = l->head;
}

Report list_next(ReportList l) {
    if (!l || !l->current) return NULL;
    /* Estrae il dato puntato correntemente prima di avanzare la testina di lettura */
    Report r = l->current->data;
    l->current = l->current->next; /* Avanzamento logico al nodo successivo */
    return r;
}

int list_size(ReportList l) {
    /* Restituisce la dimensione in O(1) leggendo la variabile di controllo preallocata */
    return l ? l->size : 0;
}

#include "../../include/adt/report_list.h"
#include <stdlib.h>

/* Nodo della lista concatenata */
typedef struct ListNode {
    Report data;
    struct ListNode* next;
} ListNode;

/* Struttura della lista con iteratore interno */
struct ReportList {
    ListNode* head;
    ListNode* current; // Puntatore per l'iteratore
    int size;
};

ReportList create_list() {
    ReportList l = (ReportList)malloc(sizeof(struct ReportList));
    if (l) {
        l->head = NULL;
        l->current = NULL;
        l->size = 0;
    }
    return l;
}

void free_list(ReportList l) {
    if (!l) return;
    ListNode* curr = l->head;
    while (curr) {
        ListNode* next = curr->next;
        // Liberiamo il report contenuto nel nodo
        free_report(curr->data);
        free(curr);
        curr = next;
    }
    free(l);
}

void list_insert(ReportList l, Report r) {
    if (!l || !r) return;
    
    ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
    if (!new_node) return;

    new_node->data = r;
    // Inserimento in testa: O(1)
    new_node->next = l->head;
    l->head = new_node;
    l->size++;
}

bool list_remove(ReportList l, unsigned int report_id) {
    if (!l || !l->head) return false;

    ListNode* curr = l->head;
    ListNode* prev = NULL;

    while (curr) {
        if (get_report_id(curr->data) == report_id) {
            if (prev == NULL) {
                l->head = curr->next;
            } else {
                prev->next = curr->next;
            }
            free_report(curr->data);
            free(curr);
            l->size--;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

Report list_find(ReportList l, unsigned int report_id) {
    if (!l) return NULL;
    ListNode* curr = l->head;
    while (curr) {
        if (get_report_id(curr->data) == report_id) {
            return curr->data;
        }
        curr = curr->next;
    }
    return NULL;
}

int list_size(ReportList l) {
    return l ? l->size : 0;
}

/* --- Implementazione Iteratore --- */

void list_rewind(ReportList l) {
    if (l) l->current = l->head;
}

Report list_next(ReportList l) {
    if (!l || !l->current) return NULL;
    
    Report r = l->current->data;
    l->current = l->current->next;
    return r;
}

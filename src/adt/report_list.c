#include "../../include/adt/report_list.h"
#include <stdlib.h>

typedef struct Node {
    Report data;
    struct Node* next;
} Node;

struct ReportList {
    Node* head;
    Node* current; // Puntatore per l'iteratore
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
    Node* curr = l->head;
    while (curr) {
        Node* next = curr->next;
        free_report(curr->data);
        free(curr);
        curr = next;
    }
    free(l);
}

void list_insert(ReportList l, Report r) {
    if (!l || !r) return;
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return;
    new_node->data = r;
    new_node->next = l->head;
    l->head = new_node;
    l->size++;
}

bool list_remove(ReportList l, int report_id) {
    if (!l || !l->head) return false;
    Node* curr = l->head;
    Node* prev = NULL;

    while (curr) {
        if (get_report_id(curr->data) == report_id) {
            if (!prev) l->head = curr->next;
            else prev->next = curr->next;
            
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

Report list_find(ReportList l, int report_id) {
    if (!l) return NULL;
    Node* curr = l->head;
    while (curr) {
        if (get_report_id(curr->data) == report_id) return curr->data;
        curr = curr->next;
    }
    return NULL;
}

void list_rewind(ReportList l) {
    if (l) l->current = l->head;
}

Report list_next(ReportList l) {
    if (!l || !l->current) return NULL;
    Report r = l->current->data;
    l->current = l->current->next;
    return r;
}

int list_size(ReportList l) {
    return l ? l->size : 0;
}

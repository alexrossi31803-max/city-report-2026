#include "../../include/adt/report_stack.h"
#include <stdlib.h>

/* Nodo dello Stack */
typedef struct StackNode {
    Report data;
    struct StackNode* next;
} StackNode;

/* Descrittore dello Stack (Opaque Pointer) */
struct ReportStack {
    StackNode* top;
    int size;
};

ReportStack create_stack() {
    ReportStack s = (ReportStack)malloc(sizeof(struct ReportStack));
    if (s) {
        s->top = NULL;
        s->size = 0;
    }
    return s;
}

void free_stack(ReportStack s) {
    if (!s) return;
    StackNode* curr = s->top;
    while (curr) {
        StackNode* next = curr->next;
        // Non liberiamo 'data' (il Report) perché è gestito altrove
        free(curr);
        curr = next;
    }
    free(s);
}

void stack_push(ReportStack s, Report r) {
    if (!s || !r) return;

    StackNode* new_node = (StackNode*)malloc(sizeof(StackNode));
    if (!new_node) return;

    new_node->data = r;
    new_node->next = s->top;
    s->top = new_node;
    s->size++;
}

Report stack_pop(ReportStack s) {
    if (!s || !s->top) return NULL;

    StackNode* temp = s->top;
    Report r = temp->data;

    s->top = temp->next;
    free(temp);
    s->size--;

    return r;
}

bool stack_is_empty(ReportStack s) {
    return (s == NULL || s->top == NULL);
}

int stack_size(ReportStack s) {
    return s ? s->size : 0;
}

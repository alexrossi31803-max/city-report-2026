#include "../../include/adt/report_stack.h"
#include <stdlib.h>

struct ReportStack {
    Report data[MAX_STACK];
    int top;
};

ReportStack create_stack() {
    ReportStack s = (ReportStack)malloc(sizeof(struct ReportStack));
    if (s) s->top = -1;
    return s;
}

void free_stack(ReportStack s) {
    if (!s) return;
    for (int i = 0; i <= s->top; i++) {
        free_report(s->data[i]);
    }
    free(s);
}

bool stack_push(ReportStack s, Report r) {
    if (!s || s->top >= MAX_STACK - 1) return false;
    
    /* Clonazione profonda (Deep Copy) coerente con le nuove specifiche del costruttore */
    Report backup = create_report(
        get_report_id(r), 
        get_report_user_id(r),
        get_report_citizen_name(r), 
        get_report_category(r),
        get_report_description(r), 
        get_report_date(r), 
        get_report_urgency(r)
    );
    if (!backup) return false;
    update_report_status(backup, get_report_status(r));
    
    s->top++;
    s->data[s->top] = backup;
    return true;
}

Report stack_pop(ReportStack s) {
    if (!s || stack_is_empty(s)) return NULL;
    Report r = s->data[s->top];
    s->top--;
    return r;
}

bool stack_is_empty(ReportStack s) {
    return (!s || s->top == -1);
}



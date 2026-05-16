#ifndef REPORT_STACK_H
#define REPORT_STACK_H

#include "../models/report.h"

#define MAX_STACK 10

typedef struct ReportStack* ReportStack;

ReportStack create_stack();
void free_stack(ReportStack s);

bool stack_push(ReportStack s, Report r);
Report stack_pop(ReportStack s);
bool stack_is_empty(ReportStack s);

#endif


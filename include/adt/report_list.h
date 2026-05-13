#ifndef REPORT_LIST_H
#define REPORT_LIST_H

#include "../models/report.h"

typedef struct ReportList* ReportList;

ReportList create_list();
void free_list(ReportList l);

void list_insert(ReportList l, Report r);
bool list_remove(ReportList l, int report_id);
Report list_find(ReportList l, int report_id);

// Funzioni per scorrere la lista (Iteratore esterno per l'UI)
void list_rewind(ReportList l);
Report list_next(ReportList l);
int list_size(ReportList l);

#endif

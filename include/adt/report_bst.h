#ifndef REPORT_BST_H
#define REPORT_BST_H

#include <stdio.h>
#include "../models/report.h"

//opaque pointer
typedef struct ReportBST* ReportBST;

ReportBST create_bst();
void free_bst(ReportBST t);

// Inserimento con tipo Report 
void bst_insert(ReportBST t, int id_user, Report r);

// Scrittura In-Order passandoli come puntatori espliciti
void bst_write_inorder(ReportBST t, FILE* f_out, void (*write_func)(FILE*, Report));

#endif


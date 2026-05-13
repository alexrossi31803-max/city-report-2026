#include "../../include/adt/report_bst.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct BSTNode {
    int id_user;
    Report data;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

struct ReportBST {
    BSTNode* root;
};

ReportBST create_bst() {
    ReportBST t = (ReportBST)malloc(sizeof(struct ReportBST));
    if (t) t->root = NULL;
    return t;
}

static void free_bst_node(BSTNode* n) {
    if (!n) return;
    free_bst_node(n->left);
    free_bst_node(n->right);
    free_report(n->data);
    free(n);
}

void free_bst(ReportBST t) {
    if (t) {
        free_bst_node(t->root);
        free(t);
    }
}

static BSTNode* insert_node(BSTNode* n, int id_user, Report r) {
    if (!n) {
        BSTNode* new_node = (BSTNode*)malloc(sizeof(BSTNode));
        if (new_node) {
            new_node->id_user = id_user;
            new_node->data = r;
            new_node->left = new_node->right = NULL;
        }
        return new_node;
    }
    if (id_user < n->id_user) n->left = insert_node(n->left, id_user, r);
    else n->right = insert_node(n->right, id_user, r);
    
    return n;
}

void bst_insert(ReportBST t, int id_user, Report r) {
    if (t) t->root = insert_node(t->root, id_user, r);
}

static void inorder_write(BSTNode* n, FILE* f, void (*write_func)(FILE*, Report)) {
    if (!n) return;
    inorder_write(n->left, f, write_func);
    write_func(f, n->data); 
    inorder_write(n->right, f, write_func);
}

void bst_write_inorder(ReportBST t, FILE* f_out, void (*write_func)(FILE*, Report)) {
    if (t && f_out && write_func) inorder_write(t->root, f_out, write_func);
}

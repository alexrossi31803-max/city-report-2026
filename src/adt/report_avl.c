#include "../../include/adt/report_avl.h"
#include "../../include/config.h"
#include <stdlib.h>
#include <stdio.h>

/* Struttura del Nodo AVL */
typedef struct AVLNode {
    unsigned int key;           // Può essere ReportID o UserID
    int disk_row;               // Posizione fisica nel file master (usato per AVL ID)
    
    // Per AVL User: gestiamo un array dinamico di ID Report associati
    unsigned int* report_ids;   
    int count_reports;
    
    struct AVLNode *left, *right;
    int height;
} AVLNode;

struct ReportAVL {
    AVLNode* root;
};

/* --- Funzioni Utility Private --- */

static int height(AVLNode* n) {
    return n ? n->height : 0;
}

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static int get_balance(AVLNode* n) {
    return n ? height(n->left) - height(n->right) : 0;
}

static AVLNode* create_node(unsigned int key, int row) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) return NULL;
    node->key = key;
    node->disk_row = row;
    node->left = node->right = NULL;
    node->height = 1;
    
    // Inizializzazione per User AVL
    node->report_ids = NULL;
    node->count_reports = 0;
    
    return node;
}

/* --- Rotazioni per il bilanciamento --- */

static AVLNode* right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

static AVLNode* left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

/* --- Logica di Inserimento --- */

static AVLNode* insert_id(AVLNode* node, unsigned int id, int row) {
    if (!node) return create_node(id, row);

    if (id < node->key)
        node->left = insert_id(node->left, id, row);
    else if (id > node->key)
        node->right = insert_id(node->right, id, row);
    else
        return node; // ID duplicato non ammesso

    node->height = 1 + max(height(node->left), height(node->right));
    int balance = get_balance(node);

    // Caso Left Left
    if (balance > 1 && id < node->left->key) return right_rotate(node);
    // Caso Right Right
    if (balance < -1 && id > node->right->key) return left_rotate(node);
    // Caso Left Right
    if (balance > 1 && id > node->left->key) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }
    // Caso Right Left
    if (balance < -1 && id < node->right->key) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

static AVLNode* insert_user(AVLNode* node, unsigned int user_id, unsigned int report_id) {
    if (!node) {
        AVLNode* new_n = create_node(user_id, -1);
        new_n->report_ids = malloc(sizeof(unsigned int));
        new_n->report_ids[0] = report_id;
        new_n->count_reports = 1;
        return new_n;
    }

    if (user_id < node->key)
        node->left = insert_user(node->left, user_id, report_id);
    else if (user_id > node->key)
        node->right = insert_user(node->right, user_id, report_id);
    else {
        // Utente già presente: aggiungiamo il report_id alla sua lista
        node->count_reports++;
        node->report_ids = realloc(node->report_ids, node->count_reports * sizeof(unsigned int));
        node->report_ids[node->count_reports - 1] = report_id;
        return node;
    }

    node->height = 1 + max(height(node->left), height(node->right));
    int balance = get_balance(node);

    if (balance > 1 && user_id < node->left->key) return right_rotate(node);
    if (balance < -1 && user_id > node->right->key) return left_rotate(node);
    if (balance > 1 && user_id > node->left->key) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }
    if (balance < -1 && user_id < node->right->key) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

/* --- API Pubbliche --- */

ReportAVL create_avl() {
    ReportAVL t = malloc(sizeof(struct ReportAVL));
    if (t) t->root = NULL;
    return t;
}

void avl_insert_by_id(ReportAVL t, Report r) {
    if (t && r) {
        t->root = insert_id(t->root, get_report_id(r), get_report_disk_row(r));
    }
}

void avl_insert_by_user(ReportAVL t, unsigned int user_id, unsigned int report_id) {
    if (t) {
        t->root = insert_user(t->root, user_id, report_id);
    }
}

int avl_search_disk_row(ReportAVL t, unsigned int report_id) {
    AVLNode* curr = t->root;
    while (curr) {
        if (report_id == curr->key) return curr->disk_row;
        if (report_id < curr->key) curr = curr->left;
        else curr = curr->right;
    }
    return -1;
}

static void free_nodes(AVLNode* n) {
    if (!n) return;
    free_nodes(n->left);
    free_nodes(n->right);
    if (n->report_ids) free(n->report_ids);
    free(n);
}

void free_avl(ReportAVL t) {
    if (t) {
        free_nodes(t->root);
        free(t);
    }
}
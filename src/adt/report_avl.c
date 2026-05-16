#include "../../include/adt/report_avl.h"
#include <stdlib.h>

typedef struct AVLNode {
    unsigned int key;              // Contiene report_id o user_id
    unsigned int* report_ids;      // Vettore dinamico per supportare più segnalazioni per User ID
    int num_reports;
    int disk_row;                  // Mappatura fisica se usato come indice report
    char status_val;               // Stato salvato compresso
    int height;
    struct AVLNode* left;
    struct AVLNode* right;
} AVLNode;

struct ReportAVL {
    AVLNode* root;
};

ReportAVL create_avl() {
    ReportAVL t = (ReportAVL)malloc(sizeof(struct ReportAVL));
    if (t) t->root = NULL;
    return t;
}

static int get_height(AVLNode* n) {
    return n ? n->height : 0;
}

static int get_balance(AVLNode* n) {
    return n ? get_height(n->left) - get_height(n->right) : 0;
}

static int max_val(int a, int b) {
    return (a > b) ? a : b;
}

static AVLNode* create_node(unsigned int key) {
    AVLNode* n = (AVLNode*)malloc(sizeof(AVLNode));
    if (!n) return NULL;
    n->key = key;
    n->report_ids = NULL;
    n->num_reports = 0;
    n->disk_row = -1;
    n->status_val = '0';
    n->height = 1;
    n->left = n->right = NULL;
    return n;
}

static void free_avl_nodes(AVLNode* n) {
    if (!n) return;
    free_avl_nodes(n->left);
    free_avl_nodes(n->right);
    if (n->report_ids) free(n->report_ids);
    free(n);
}

void free_avl(ReportAVL t) {
    if (t) {
        free_avl_nodes(t->root);
        free(t);
    }
}

/* Rotazione Destra (Caso Sinistra-Sinistra LL) */
static AVLNode* rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max_val(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_val(get_height(x->left), get_height(x->right)) + 1;

    return x;
}

/* Rotazione Sinistra (Caso Destra-Destra RR) */
static AVLNode* rotate_left(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max_val(get_height(x->left), get_height(x->right)) + 1;
    y->height = max_val(get_height(y->left), get_height(y->right)) + 1;

    return y;
}

/* Funzione ricorsiva di inserimento e bilanciamento AVL per Report ID */
static AVLNode* insert_rep_node(AVLNode* node, unsigned int report_id, Report r, bool* inserted) {
    if (!node) {
        AVLNode* n = create_node(report_id);
        if (n) {
            n->disk_row = get_report_disk_row(r);
            n->status_val = (char)get_report_status(r) + '0';
        }
        *inserted = true;
        return n;
    }

    if (report_id < node->key) {
        node->left = insert_rep_node(node->left, report_id, r, inserted);
    } else if (report_id > node->key) {
        node->right = insert_rep_node(node->right, report_id, r, inserted);
    } else {
        node->disk_row = get_report_disk_row(r);
        node->status_val = (char)get_report_status(r) + '0';
        return node;
    }

    node->height = 1 + max_val(get_height(node->left), get_height(node->right));
    int balance = get_balance(node);

    if (balance > 1 && report_id < node->left->key)
        return rotate_right(node);

    if (balance < -1 && report_id > node->right->key)
        return rotate_left(node);

    if (balance > 1 && report_id > node->left->key) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    if (balance < -1 && report_id < node->right->key) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}

void avl_insert_by_report_id(ReportAVL t, unsigned int report_id, Report r) {
    if (!t) return;
    bool inserted = false;
    t->root = insert_rep_node(t->root, report_id, r, &inserted);
}

/* Funzione ricorsiva di inserimento e bilanciamento AVL per User ID con aggregazione locale multipla */
static AVLNode* insert_user_node(AVLNode* node, unsigned int user_id, unsigned int report_id) {
    if (!node) {
        AVLNode* n = create_node(user_id);
        if (n) {
            n->num_reports = 1;
            n->report_ids = (unsigned int*)malloc(sizeof(unsigned int));
            if (n->report_ids) n->report_ids[0] = report_id;
        }
        return n;
    }

    if (user_id < node->key) {
        node->left = insert_user_node(node->left, user_id, report_id);
    } else if (user_id > node->key) {
        node->right = insert_user_node(node->right, user_id, report_id);
    } else {
        int exists = 0;
        for (int i = 0; i < node->num_reports; i++) {
            if (node->report_ids[i] == report_id) { exists = 1; break; }
        }
        if (!exists) {
            node->num_reports++;
            unsigned int* temp = (unsigned int*)realloc(node->report_ids, node->num_reports * sizeof(unsigned int));
            if (temp) {
                node->report_ids = temp;
                node->report_ids[node->num_reports - 1] = report_id;
            }
        }
        return node;
    }

    node->height = 1 + max_val(get_height(node->left), get_height(node->right));
    int balance = get_balance(node);

    if (balance > 1 && user_id < node->left->key)
        return rotate_right(node);

    if (balance < -1 && user_id > node->right->key)
        return rotate_left(node);

    if (balance > 1 && user_id > node->left->key) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    if (balance < -1 && user_id < node->right->key) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}

void avl_insert_by_user_id(ReportAVL t, unsigned int user_id, unsigned int report_id) {
    if (t) t->root = insert_user_node(t->root, user_id, report_id);
}

static void inorder_traversal(AVLNode* n, FILE* f, void (*write_func)(FILE*, unsigned int, unsigned int, int, char)) {
    if (!n) return;
    inorder_traversal(n->left, f, write_func);
    
    if (n->report_ids != NULL) {
        for (int i = 0; i < n->num_reports; i++) {
            write_func(f, n->key, n->report_ids[i], -1, '0');
        }
    } else {
        write_func(f, n->key, 0, n->disk_row, n->status_val);
    }
    
    inorder_traversal(n->right, f, write_func);
}

void avl_write_inorder(ReportAVL t, FILE* f_out, void (*write_func)(FILE*, unsigned int, unsigned int, int, char)) {
    if (t && f_out && write_func) inorder_traversal(t->root, f_out, write_func);
}


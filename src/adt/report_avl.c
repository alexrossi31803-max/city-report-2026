#include "../../include/adt/report_avl.h"
#include "../../include/config.h"
#include "../../include/utils/parser.h"
#include <stdlib.h>
#include <string.h>

/* ==============================================================================
 *  STRUTTURA INTERNA DEL NODO AVL (CONFORME AL PROMPT DEFINITIVO)
 * ============================================================================== */
typedef struct nodeAVL {
   void *elem;          /* Puntatore generico alla stringa record allocata in RAM */
   Type_Avl type;       /* Discriminante per stabilire se si tratta di UID o RID */

   int height;          /* Altezza del nodo: fondamentale per calcolare il Balance Factor */
   int size;            /* Numero totale di nodi nel sottoalbero radicato qui (per il file system) */

   struct nodeAVL *left;  /* Puntatore al figlio sinistro (chiavi minori) */
   struct nodeAVL *right; /* Puntatore al figlio destro (chiavi maggiori o uguali) */
} NodeAVL;

/* ==============================================================================
 *  FUNZIONI HELPER STATICHE PER ESTRAZIONE CHIAVI (GEOMETRIA STRINGHE SENZA SPAZI)
 * ============================================================================== */

/**
 * @brief Estrae l'User ID convertendo i primi 10 caratteri della stringa UID.
 * Geometria: [USER_ID(10)][REPORT_ID(10)]\n
 */
static unsigned int get_user_id_from_uid_line(const char *elem) {
    char tmp[11] = {0};
    memcpy(tmp, elem, 10);
    return (unsigned int)strtoul(tmp, NULL, 10);
}


/**
 * @brief Estrae il Report ID convertendo i secondi 10 caratteri della stringa UID.
 * Geometria: [USER_ID(10)][REPORT_ID(10)]\n
 */
static unsigned int get_report_id_from_uid(const char *elem) {
    char tmp[11] = {0};
    memcpy(tmp, elem + 10, 10); /* Salto di 10 byte per isolare il Report ID */
    return (unsigned int)strtoul(tmp, NULL, 10);
}


/* ==============================================================================
 *  FUNZIONI DI GESTIONE METRICHE E MEMORIA DELLA STRUTTURA AVL
 * ============================================================================== */

ReportAvl createNode(void *elem, Type_Avl type) {
    ReportAvl n = (ReportAvl)malloc(sizeof(NodeAVL));
    if (!n) return NULL;

    /* Determina la lunghezza fissa esatta in base al tipo (21 o 22 byte) */
    int len = (type == TYPE_AVL_UID) ? 21 : 22;
    
    /* Allocazione dello spazio per la stringa più il terminatore di sicurezza '\0' */
    n->elem = malloc(len + 1);
    if (!n->elem) {
        free(n);
        return NULL;
    }
    
    /* Hard copy bloccata della stringa grezza passata come void* */
    memcpy(n->elem, (const char *)elem, len);
    ((char *)n->elem)[len] = '\0'; /* Sigillo di chiusura stringa in RAM */

    n->type = type;
    n->height = 1; /* Un nuovo nodo nasce come foglia, quindi altezza iniziale = 1 */
    n->size = 1;   /* Inizialmente il sottoalbero contiene solo se stesso */
    n->left = n->right = NULL;
    return n;
}

int height(ReportAvl t) {
    return t ? t->height : 0; /* Un sottoalbero vuoto (NULL) ha altezza 0 */
}

int size(ReportAvl t) {
    return t ? t->size : 0; /* Un sottoalbero vuoto (NULL) contiene 0 nodi */
}

int balanceFactor(ReportAvl t) {
    /* Balance Factor = Altezza Sottoalbero Sinistro - Altezza Sottoalbero Destro */
    return t ? height(t->left) - height(t->right) : 0;
}

static int max_val(int a, int b) {
    return (a > b) ? a : b;
}

void updateHeight(ReportAvl t) {
    if (t) {
        /* L'altezza è 1 + il massimo tra le altezze dei due figli intermedi */
        t->height = 1 + max_val(height(t->left), height(t->right));
        /* La size del sottoalbero è 1 + la somma dei nodi a sinistra e a destra */
        t->size = 1 + size(t->left) + size(t->right);
    }
}

/* ==============================================================================
 *  ROTAZIONI DI BILANCIAMENTO MATEMATICO AVL
 * ============================================================================== */

ReportAvl rotateLeft(ReportAvl t) {
    /* Il figlio destro diventa la nuova radice del sottoalbero */
    ReportAvl y = t->right;
    ReportAvl T2 = y->left;

    /* Sostituzione geometrica dei puntatori */
    y->left = t;
    t->right = T2;

    /* Ricalcolo rigoroso delle altezze partendo dal nodo sceso in basso */
    updateHeight(t);
    updateHeight(y);

    return y; /* Ritorna la nuova radice bilanciata */
}

ReportAvl rotateRight(ReportAvl t) {
    /* Il figlio sinistro diventa la nuova radice del sottoalbero */
    ReportAvl x = t->left;
    ReportAvl T2 = x->right;

    /* Sostituzione geometrica dei puntatori */
    x->right = t;
    t->left = T2;

    /* Ricalcolo rigoroso delle altezze partendo dal nodo sceso in basso */
    updateHeight(t);
    updateHeight(x);

    return x; /* Ritorna la nuova radice bilanciata */
}

ReportAvl rotateLeftRight(ReportAvl t) {
    /* Caso LR: Rotazione sinistra sul figlio sinistro, seguita da rotazione destra sul padre */
    t->left = rotateLeft(t->left);
    return rotateRight(t);
}

ReportAvl rotateRightLeft(ReportAvl t) {
    /* Caso RL: Rotazione destra sul figlio destro, seguita da rotazione sinistra sul padre */
    t->right = rotateRight(t->right);
    return rotateLeft(t);
}

ReportAvl rebalance(ReportAvl t) {
    if (!t) return NULL;
    
    /* Aggiorna le metriche del nodo corrente prima di ispezionare il fattore di forma */
    updateHeight(t);
    int balance = balanceFactor(t);

    /* CASO 1: Sbilanciamento a Sinistra (BF > 1) */
    if (balance > 1) {
        if (balanceFactor(t->left) >= 0) {
            return rotateRight(t); /* Sotto-caso Sinistra-Sinistra: Rotazione Singola (LL) */
        } else {
            return rotateLeftRight(t); /* Sotto-caso Sinistra-Destra: Rotazione Doppia (LR) */
        }
    }
    
    /* CASO 2: Sbilanciamento a Destra (BF < -1) */
    if (balance < -1) {
        if (balanceFactor(t->right) <= 0) {
            return rotateLeft(t); /* Sotto-caso Destra-Destra: Rotazione Singola (RR) */
        } else {
            return rotateRightLeft(t); /* Sotto-caso Destra-Sinistra: Rotazione Doppia (RL) */
        }
    }
    return t; /* Il nodo è già bilanciato, lo restituisce immutato */
}

/* ==============================================================================
 *  MOTORE DI INSERIMENTO COMPATIBILE CON DUPLICATI LOGICI (UID)
 * ============================================================================== */

ReportAvl insert(
   ReportAvl t,
   void *elem,
   Type_Avl type,
   int (*compare)(const void *, const void *)
) {
    /* Se raggiungiamo una foglia vuota, allochiamo dinamicamente il nodo ed eseguiamo l'hard copy */
    if (!t) return createNode(elem, type);

    /* Invocazione del puntatore a funzione per confrontare polimorficamente i record */
    int comp_res = compare(elem, t->elem);

    if (comp_res < 0) {
        t->left = insert(t->left, elem, type, compare);
    } else {
        /* REGOLA SPERIMENTALE: I duplicati logici (stesso user_id) scivolano stagna a destra */
        t->right = insert(t->right, elem, type, compare);
    }

    /* Risalita ricorsiva con esecuzione del rebalance automatico su tutti i nodi antenati */
    return rebalance(t);
}

/* ==============================================================================
 *  VISITA INORDER PER LA GENERAZIONE DEL FILE SEQUENZIALE PERSISTENTE
 * ============================================================================== */

void inorder(ReportAvl t, FILE *file) {
    if (!t || !file) return;
    
    /* Algoritmo Inorder: Sottoalbero Sinistro -> Nodo Corrente -> Sottoalbero Destro */
    inorder(t->left, file);
    
    int len = (t->type == TYPE_AVL_UID) ? 21 : 22;
    /* Scrittura binaria massiva sul file flat senza l'aggiunta di spazi spuri */
    fwrite(t->elem, sizeof(char), len, file);
    
    inorder(t->right, file);
}

/* ==============================================================================
 *  MOTORI DI RICERCA INTERNA (FIND COERENTE CON IL TIPO SPECIFICO)
 * ============================================================================== */

int findReportId(unsigned int report_id) {
    FILE* f = fopen(PATH_AVL_REPORT_ID, "rb");
    if (!f) return -1;

    char line_buf[AVL_REPORT_ID_LINE];
    /* Scansione diretta del file d'indice inorder sequenziale a blocchi fissi */
    while (fread(line_buf, sizeof(char), AVL_REPORT_ID_LINE, f) == AVL_REPORT_ID_LINE) {
        line_buf[AVL_REPORT_ID_LINE] = '\n';
        
        char raw_rid[11] = {0};
        char raw_row[11] = {0};
        
        memcpy(raw_rid, line_buf, 10);
        memcpy(raw_row, line_buf + 11, 10);
        
        unsigned int curr_id = (unsigned int)strtoul(raw_rid, NULL, 10);
        
        /* Se l'ID corrisponde, estrae immediatamente l'indice fisico disk_row */
        if (curr_id == report_id) {
            int disk_row = atoi(raw_row);
            fclose(f);
            return disk_row;
        }
    }
    fclose(f);
    return -1; /* Chiave non trovata nell'indice del disco */
}


int findUserId(unsigned int uid, unsigned int *results) {
    /* LETTURA IN O(1) DEL NUMERO DI RECORD DAL REGISTRO CENTRALIZZATO */
    int total_records = (int)read_system_variable(REG_IDX_AVL_USR_COUNT);
    if (total_records == 0) return 0;

    FILE* f = fopen(PATH_AVL_USER_ID, "rb");
    if (!f) return 0;

    int low = 0;
    int high = total_records - 1;
    int match_index = -1;
    char line_buffer[AVL_USER_ID_LINE + 1];

    /* 1. RICERCA BINARIA LOGARITMICA O(log n) SULL'ARRAY INORDER SU DISCO SU FILE DA 21 BYTE */
    while (low <= high) {
        int mid = low + (high - low) / 2;
        fseek(f, (long)mid * AVL_USER_ID_LINE, SEEK_SET);
        if (fread(line_buffer, sizeof(char), AVL_USER_ID_LINE, f) != AVL_USER_ID_LINE) break;
        line_buffer[AVL_USER_ID_LINE] = '\0';

        /* ADATTAMENTO: Richiamo del nome univoco get_user_id_from_uid_line per azzerare i conflitti con user.h */
        unsigned int curr_uid = get_user_id_from_uid_line(line_buffer);

        if (curr_uid == uid) {
            match_index = mid; /* Primo punto di contatto localizzato */
            break; 
        } else if (curr_uid < uid) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    /* Se l'User ID cercato non compare in nessuna cella dell'indice */
    if (match_index == -1) {
        fclose(f);
        return 0;
    }

    int match_counter = 0;

    /* 2. ESPANSIONE A SINISTRA: Individua l'inizio del blocco di duplicati contigui */
    int scan_left = match_index;
    while (scan_left >= 0) {
        fseek(f, (long)scan_left * AVL_USER_ID_LINE, SEEK_SET);
        if (fread(line_buffer, sizeof(char), AVL_USER_ID_LINE, f) != AVL_USER_ID_LINE) break;
        line_buffer[AVL_USER_ID_LINE] = '\0';

        /* ADATTAMENTO: Sincronizzazione con il nome della funzione helper interna univoca */
        if (get_user_id_from_uid_line(line_buffer) != uid) {
            break; /* Si ferma non appena l'User ID cambia nel file ordinato */
        }
        scan_left--;
    }
    
    /* Imposta il punto di partenza della raccolta subito dopo l'ultimo estraneo trovato */
    int start_scan = scan_left + 1;

    /* 3. RACCOLTA SEQUENZIALE DA SINISTRA VERSO DESTRA DI TUTTI I REPORT ID ACCOPPIATI */
    while (start_scan < total_records) {
        fseek(f, (long)start_scan * AVL_USER_ID_LINE, SEEK_SET);
        if (fread(line_buffer, sizeof(char), AVL_USER_ID_LINE, f) != AVL_USER_ID_LINE) break;
        line_buffer[AVL_USER_ID_LINE] = '\0';

        /* ADATTAMENTO: Sincronizzazione con il nome della funzione helper interna univoca */
        if (get_user_id_from_uid_line(line_buffer) != uid) {
            break; /* Termina la raccolta non appena usciamo dall'intervallo dell'utente */
        }

        /* Estrae il report_id associato e lo deposita direttamente nel vettore di output results */
        results[match_counter] = get_report_id_from_uid(line_buffer);
        match_counter++;
        start_scan++;
    }

    fclose(f);
    return match_counter; /* Restituisce il numero totale di segnalazioni storiche isolate */
}



void free_avl_tree(ReportAvl t) {
    if (!t) return;
    free_avl_tree(t->left);
    free_avl_tree(t->right);
    if (t->elem) free(t->elem); /* Deallocazione hard copy stringa interna */
    free(t);                    /* Deallocazione del nodo strutturale */
}

#include "../../include/server/report_manager.h"
#include "../../include/utils/parser.h"
#include "../../include/adt/report_avl.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Estrae l'ultimo indice di riga master disponibile dallo stack LIFO dei buchi ausiliario.
 *        Applica la semantica Pop troncando fisicamente la riga consumata in tempo costante O(1).
 */
static int pop_hole_index(const char* hole_path) {
    FILE* f = fopen(hole_path, "rb+");
    if (!f) return -1;
    
    char buffer[SYSTEM_REG_LINE + 1] = {0};
    int line_count = 0;
    
    /* Conta le righe dello stack dei buchi (ciascuna riga occupa stabilmente 11 byte) */
    while (fgets(buffer, sizeof(buffer), f)) {
        line_count++;
    }
    
    /* Se lo stack dei buchi risulta scarico, restituisce -1 per indicare inserimento in Append */
    if (line_count == 0) {
        fclose(f);
        return -1;
    }
    
    /* Si posiziona geometricamente sull'ultima riga scritta (cima dello stack LIFO) */
    fseek(f, (long)(line_count - 1) * SYSTEM_REG_LINE, SEEK_SET);
    if (!fgets(buffer, sizeof(buffer), f)) { 
        fclose(f); 
        return -1; 
    }
    
    int target_row = atoi(buffer);
    fclose(f);

    /* Esegue il troncamento sul posto (Pop fisica) per eliminare l'indice consumato dallo stack */
    long new_size = (long)(line_count - 1) * SYSTEM_REG_LINE;
#ifdef _WIN32
    #include <io.h>
    int fd = _open(hole_path, 0x0002); /* Apertura in lettura/scrittura a basso livello Windows */
    if (fd != -1) { 
        _chsize(fd, new_size); 
        _close(fd); 
    }
#else
    #include <unistd.h>
    truncate(hole_path, new_size); /* Comando di troncamento POSIX standard */
#endif

    return target_row;
}

/**
 * @brief Inserisce un indice riga libero in coda allo stack LIFO dei buchi specifico (Push).
 */
static void push_hole_index(const char* hole_path, int disk_row) {
    FILE* f = fopen(hole_path, "ab");
    if (f) {
        /* Scrive l'indice a riga fissa da 11 byte comprensiva di padding a zero e newline */
        fprintf(f, "%010d\n", disk_row);
        fclose(f);
    }
}

unsigned int generate_global_report_id_v2(void) {
    unsigned int current_id = read_system_variable(REG_IDX_GLOBAL_ID);
    /* Incrementa il contatore unico globale sul registro di controllo in O(1) */
    write_system_variable(REG_IDX_GLOBAL_ID, current_id + 1);
    return current_id;
}

/**
 * @brief Modifica chirurgica ad accesso immediato O(1) del flag di cella Master al byte 350.
 */
static void set_master_cell_state(const char* path, int disk_row, char state) {
    if (disk_row < 0) return;
    FILE* f = fopen(path, "rb+");
    if (!f) return;
    
    /* Salto millimetrico calibrato sul passo REPORT_MASTER_LINE (352 byte) */
    fseek(f, (long)disk_row * REPORT_MASTER_LINE, SEEK_SET);
    char line[REPORT_MASTER_LINE + 1] = {0};
    
    if (fread(line, sizeof(char), REPORT_MASTER_LINE, f) == REPORT_MASTER_LINE) {
        line[350] = state; /* Sovrascrive il flag impostandolo a 'N' per indicare un buco vacuo */
        fseek(f, (long)disk_row * REPORT_MASTER_LINE, SEEK_SET);
        fwrite(line, sizeof(char), REPORT_MASTER_LINE, f);
    }
    fclose(f);
}

/**
 * @brief Callback interna di utilita per confrontare i record stringa durante la ricostruzione degli indici.
 */
static int internal_compare_nodes(const void* a, const void* b) {
    char tmp_a[11] = {0}; char tmp_b[11] = {0};
    memcpy(tmp_a, (const char*)a, 10); memcpy(tmp_b, (const char*)b, 10);
    unsigned int id_a = (unsigned int)strtoul(tmp_a, NULL, 10);
    unsigned int id_b = (unsigned int)strtoul(tmp_b, NULL, 10);
    return (id_a < id_b) ? -1 : (id_a > id_b) ? 1 : 0;
}

bool process_and_flush_bench_v2(void) {
    unsigned int counter_bench = read_system_variable(REG_IDX_COUNTER_BENCH);
    /* Se la cache della BENCH è vuota, l'operazione non è necessaria e si arresta subito */
    if (counter_bench == 0) return true;
    
    FILE* f_bench = fopen(PATH_BENCH, "rb");
    if (!f_bench) return false;
    
    char line[REPORT_MASTER_LINE + 1] = {0};
    
    /* ----------------------------------------------------------------------
       FASE 1: SCARICAMENTO ED INSERIMENTO INTELLIGENTE NEI FILE MASTER IN O(1)
       ---------------------------------------------------------------------- */
    for (unsigned int i = 0; i < counter_bench; i++) {
        fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET); /* Salto a passo 351 byte */
        if (fread(line, sizeof(char), REPORT_BENCH_LINE, f_bench) != REPORT_BENCH_LINE) continue;
        
        char cell_state;
        Report r = line_to_report_v2(line, &cell_state);
        if (!r) continue;
        
        /* FILTRO REQUISITO: Se lo stato è DESTROYED, il record viene perso fisicamente senza travaso */
        if (get_report_status(r) == DESTROYED) { 
            free_report(r); 
            continue; 
        }
        
        const char* path_target;
        const char* path_holes;
        int reg_idx_status;
        
        /* Smistamento dei canali fisici basato sullo stato del report valido */
        if (get_report_status(r) == OPEN) { 
            path_target = PATH_OPEN_MASTER; path_holes = PATH_OPEN_HOLES; reg_idx_status = REG_IDX_STAT_OPEN; 
        } else if (get_report_status(r) == IN_PROGRESS) { 
            path_target = PATH_PROGRESS_MASTER; path_holes = PATH_PROGRESS_HOLES; reg_idx_status = REG_IDX_STAT_PROGRESS; 
        } else { 
            path_target = PATH_CLOSED_MASTER; path_holes = PATH_CLOSED_HOLES; reg_idx_status = REG_IDX_STAT_CLOSED; 
        }
        
        /* Interrogazione dello Stack dei buchi LIFO in O(1) per trovare celle 'N' riciclabili */
        int assigned_row = pop_hole_index(path_holes);
        FILE* f_master = fopen(path_target, "rb+");
        if (!f_master) f_master = fopen(path_target, "wb+");
        
        if (f_master) {
            /* Se lo stack dei buchi era vuoto (ritorna -1), si inserisce in Append in coda al file */
            if (assigned_row == -1) {
                fseek(f_master, 0, SEEK_END);
                assigned_row = (int)(ftell(f_master) / REPORT_MASTER_LINE);
            }
            
            /* Iniezione millimetrica della disk_row definitiva assegnata al report */
            set_report_disk_row(r, assigned_row);
            
            char out_line[REPORT_MASTER_LINE + 1] = {0};
            /* Il parser genera la riga marchiandola al byte 350 con il flag di cella Active 'A' */
            report_to_line(out_line, r, 'A');
            
            /* Sovrascrive o appende la riga a 352 byte completi nel rispettivo file Master */
            fseek(f_master, (long)assigned_row * REPORT_MASTER_LINE, SEEK_SET);
            fwrite(out_line, sizeof(char), REPORT_MASTER_LINE, f_master);
            fclose(f_master);
            
            /* Incrementa in O(1) sul registro la variabile legata allo stato della pratica */
            write_system_variable(reg_idx_status, read_system_variable(reg_idx_status) + 1);
        }
        free_report(r);
    }
    fclose(f_bench);
    
    /* Reset atomico del contatore elementi cache BENCH sul registro di controllo */
    write_system_variable(REG_IDX_COUNTER_BENCH, 0); 
    
    /* ----------------------------------------------------------------------
       FASE 2 E FASE 3: RIGENERAZIONE DA ZERO ED ESPORTAZIONE INORDER DEGLI INDICI AVL
       ---------------------------------------------------------------------- */
    rebuild_avl_indices_server();
    return true;
}

/**
 * @brief Scansiona un canale master, estrae i record vivi ed esegue il rebalance AVL in RAM.
 */
static void parse_and_load_avl_from_file(ReportAvl avl_rep, ReportAvl avl_usr, const char* path, int* rep_count, int* usr_count) {
    FILE* f = fopen(path, "rb");
    if (!f) return;
    
    char line[REPORT_MASTER_LINE + 1] = {0};
    /* Scansione a passi rigidi di 352 byte escludendo buchi ed interrompendo su sentinelle */
    while (fread(line, sizeof(char), REPORT_MASTER_LINE, f) == REPORT_MASTER_LINE) {
        if (line[350] == 'N') continue; /* Salta i buchi logici vacuati */
        if (line[350] == 'E') break;    /* Si arresta davanti alla sentinella di fine archivio */
        
        char state;
        Report r = line_to_report_v2(line, &state);
        if (r && state == 'A' && get_report_status(r) != DESTROYED) {
            char item_buffer[32] = {0};
            
            /* Formatta il tracciato record compresso da 22 byte per l'indice Report ID */
            sprintf(item_buffer, "%010u%c%010d\n", get_report_id(r), (char)(get_report_status(r) + '0'), get_report_disk_row(r));
            insert(avl_rep, item_buffer, TYPE_AVL_RID, internal_compare_nodes);
            (*rep_count)++;

            /* Formatta il tracciato record contratto da 21 byte per l'indice User ID */
            sprintf(item_buffer, "%010u%010u\n", get_report_user_id(r), get_report_id(r));
            insert(avl_usr, item_buffer, TYPE_AVL_UID, internal_compare_nodes);
            (*usr_count)++;
        }
        if (r) free_report(r);
    }
    fclose(f);
}

void rebuild_avl_indices_server(void) {
    /* Allocazione in RAM delle due istanze ad albero auto-bilanciante AVL opaco */
    ReportAvl avl_report_id_tree = NULL;
    ReportAvl avl_user_id_tree = NULL;
    
    int total_avl_rep_nodes = 0;
    int total_avl_usr_nodes = 0;
    
    /* Caricamento incrociato dei soli record attivi estratti dai tre database Master */
    parse_and_load_avl_from_file(avl_report_id_tree, avl_user_id_tree, PATH_OPEN_MASTER, &total_avl_rep_nodes, &total_avl_usr_nodes);
    parse_and_load_avl_from_file(avl_report_id_tree, avl_user_id_tree, PATH_PROGRESS_MASTER, &total_avl_rep_nodes, &total_avl_usr_nodes);
    parse_and_load_avl_from_file(avl_report_id_tree, avl_user_id_tree, PATH_CLOSED_MASTER, &total_avl_rep_nodes, &total_avl_usr_nodes);
    
    /* Generazione persistente dell'indice ordinato Report ID tramite scrittura Inorder (22 byte) */
    FILE* f_rep = fopen(PATH_AVL_REPORT_ID, "wb");
    if (f_rep) { 
        inorder(avl_report_id_tree, f_rep); 
        fclose(f_rep); 
    }
    
    /* Generazione persistente dell'indice ordinato User ID tramite scrittura Inorder (21 byte) */
    FILE* f_usr = fopen(PATH_AVL_USER_ID, "wb");
    if (f_usr) { 
        inorder(avl_user_id_tree, f_usr); 
        fclose(f_usr); 
    }
    
    /* Scrittura atomica in O(1) sul registro centrale dei contatori esatti delle righe degli indici */
    write_system_variable(REG_IDX_AVL_REP_COUNT, (unsigned int)total_avl_rep_nodes);
    write_system_variable(REG_IDX_AVL_USR_COUNT, (unsigned int)total_avl_usr_nodes);
    
    /* Liberazione radicale della memoria transitoria occupata dai nodi dell'AVL in RAM */
    free_avl_tree(avl_report_id_tree);
    free_avl_tree(avl_user_id_tree);
}

bool register_report_from_citizen_ram(Report r) {
    unsigned int counter_bench = read_system_variable(REG_IDX_COUNTER_BENCH);
    unsigned int report_id = get_report_id(r);
    char line_buffer[REPORT_MASTER_LINE + 1] = {0};
    
    bool is_found_in_bench = false;
    unsigned int bench_slot_index = 0;
    ReportCategory old_bench_category = OTHER;
    
    /* ----------------------------------------------------------------------
       SCENARIO 1: IL REPORT RISIEDE GIA NELLA CACHE BENCH (MATCH IN CACHE O(n))
       ---------------------------------------------------------------------- */
    FILE* f_bench = fopen(PATH_BENCH, "rb+");
    if (f_bench) {
        for (unsigned int i = 0; i < counter_bench; i++) {
            fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET);
            if (fread(line_buffer, sizeof(char), REPORT_BENCH_LINE, f_bench) != REPORT_BENCH_LINE) continue;
            char st; 
            Report tmp = line_to_report_v2(line_buffer, &st);
            
            if (tmp && get_report_id(tmp) == report_id && get_report_status(tmp) == OPEN && st == 'A') {
                is_found_in_bench = true;
                bench_slot_index = i;
                old_bench_category = get_report_category(tmp);
                free_report(tmp);
                break;
            }
            if (tmp) free_report(tmp);
        }
        
        if (is_found_in_bench) {
            /* Aggiorna i contatori analitici delle categorie in O(1) se modificata in RAM */
            if (old_bench_category != get_report_category(r)) {
                int reg_old = (old_bench_category == ROAD) ? REG_IDX_CAT_ROAD : (old_bench_category == LIGHTING) ? REG_IDX_CAT_LIGHTING : (old_bench_category == WASTE) ? REG_IDX_CAT_WASTE : (old_bench_category == INFRASTRUCTURE) ? REG_IDX_CAT_INFRASTRUCT : REG_IDX_CAT_OTHER;
                int reg_new = (get_report_category(r) == ROAD) ? REG_IDX_CAT_ROAD : (get_report_category(r) == LIGHTING) ? REG_IDX_CAT_LIGHTING : (get_report_category(r) == WASTE) ? REG_IDX_CAT_WASTE : (get_report_category(r) == INFRASTRUCTURE) ? REG_IDX_CAT_INFRASTRUCT : REG_IDX_CAT_OTHER;
                
                unsigned int c_old = read_system_variable(reg_old);
                if (c_old > 0) write_system_variable(reg_old, c_old - 1);
                write_system_variable(reg_new, read_system_variable(reg_new) + 1);
            }
            
            char out_line[REPORT_BENCH_LINE + 1] = {0};
            report_to_line(out_line, r, '\0'); /* Formattazione a 351 byte */
            fseek(f_bench, (long)bench_slot_index * REPORT_BENCH_LINE, SEEK_SET);
            fwrite(out_line, sizeof(char), REPORT_BENCH_LINE, f_bench);
            fclose(f_bench);
            return true; /* SOVRASCRITTURA EFFETTUATA: COUNTER BENCH E REPORT TOTALI NON CRESCONO */
        }
        fclose(f_bench);
    }
    
    /* ----------------------------------------------------------------------
       SCENARIO 2: IL REPORT E PRESENTE NEI FILE MASTER (MATCH IN INDICE DISCO O(log n))
       ---------------------------------------------------------------------- */
    int found_old_row = findReportId(report_id);
    
    if (found_old_row != -1) {
        /* Accede ed estrae il vecchio record stazionario dal Master delle segnalazioni aperte */
        FILE* f_master_open = fopen(PATH_OPEN_MASTER, "rb");
        ReportCategory old_master_category = OTHER;
        bool master_cell_is_active = false;
        
        if (f_master_open) {
            fseek(f_master_open, (long)found_old_row * REPORT_MASTER_LINE, SEEK_SET);
            if (fread(line_buffer, sizeof(char), REPORT_MASTER_LINE, f_master_open) == REPORT_MASTER_LINE) {
                if (line_buffer[350] == 'A') {
                    char cell_st;
                    Report r_old_master = line_to_report_v2(line_buffer, &cell_st);
                    if (r_old_master) {
                        master_cell_is_active = true;
                        old_master_category = get_report_category(r_old_master);
                        free_report(r_old_master);
                    }
                }
            }
            fclose(f_master_open);
        }
        
        if (master_cell_is_active) {
            /* Invalida geometricamente la vecchia cella Master Open marchiandola a 'N' */
            set_master_cell_state(PATH_OPEN_MASTER, found_old_row, 'N');
            /* Accatasta la riga libera nello stack dei buchi LIFO di competenza */
            push_hole_index(PATH_OPEN_HOLES, found_old_row);
            
            /* Storna e riallinea istantaneamente le statistiche delle categorie anomalie */
            if (old_master_category != get_report_category(r)) {
                int reg_old = (old_master_category == ROAD) ? REG_IDX_CAT_ROAD : (old_master_category == LIGHTING) ? REG_IDX_CAT_LIGHTING : (old_master_category == WASTE) ? REG_IDX_CAT_WASTE : (old_master_category == INFRASTRUCTURE) ? REG_IDX_CAT_INFRASTRUCT : REG_IDX_CAT_OTHER;
                int reg_new = (get_report_category(r) == ROAD) ? REG_IDX_CAT_ROAD : (get_report_category(r) == LIGHTING) ? REG_IDX_CAT_LIGHTING : (get_report_category(r) == WASTE) ? REG_IDX_CAT_WASTE : (get_report_category(r) == INFRASTRUCTURE) ? REG_IDX_CAT_INFRASTRUCT : REG_IDX_CAT_OTHER;
                
                unsigned int c_old = read_system_variable(reg_old);
                if (c_old > 0) write_system_variable(reg_old, c_old - 1);
                write_system_variable(reg_new, read_system_variable(reg_new) + 1);
            }
            
            /* Decrementa il contatore globale dei casi OPEN per bilanciare l'invalidazione della cella */
            unsigned int current_open_count = read_system_variable(REG_IDX_STAT_OPEN);
            if (current_open_count > 0) write_system_variable(REG_IDX_STAT_OPEN, current_open_count - 1);
        }
    } else {
        /* ----------------------------------------------------------------------
           SCENARIO 3: NUOVO INSERIMENTO ASSOLUTO (NESSUN MATCH RILEVATO)
           ---------------------------------------------------------------------- */
        /* Incrementa il numero totale delle segnalazioni attive memorizzate nel Comune */
        write_system_variable(REG_IDX_NM_REPORT, read_system_variable(REG_IDX_NM_REPORT) + 1);
        
        /* Incrementa il contatore della specifica categoria assegnata al report inedito */
        int reg_new_cat = (get_report_category(r) == ROAD) ? REG_IDX_CAT_ROAD : (get_report_category(r) == LIGHTING) ? REG_IDX_CAT_LIGHTING : (get_report_category(r) == WASTE) ? REG_IDX_CAT_WASTE : (get_report_category(r) == INFRASTRUCTURE) ? REG_IDX_CAT_INFRASTRUCT : REG_IDX_CAT_OTHER;
        write_system_variable(reg_new_cat, read_system_variable(reg_new_cat) + 1);
    }
    
    /* Controllo di saturazione della cache operativa server: esegue il flush se colma */
    if (counter_bench >= LIMIT_BENCH) {
        process_and_flush_bench_v2();
        counter_bench = 0;
    }
    
    /* Scrittura finale del record in Append nella cache BENCH a passo 351 byte */
    f_bench = fopen(PATH_BENCH, "rb+");
    if (!f_bench) f_bench = fopen(PATH_BENCH, "wb+");
    if (f_bench) {
        char out_line[REPORT_BENCH_LINE + 1] = {0};
        report_to_line(out_line, r, '\0');
        fseek(f_bench, (long)counter_bench * REPORT_BENCH_LINE, SEEK_SET);
        fwrite(out_line, sizeof(char), REPORT_BENCH_LINE, f_bench);
        fclose(f_bench);
        /* Incrementa il counter corrente della cache BENCH sul file di controllo */
        write_system_variable(REG_IDX_COUNTER_BENCH, counter_bench + 1);
    }
    return true;
}




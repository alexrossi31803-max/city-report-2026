#include "../../include/server/report_manager.h"
#include "../../include/utils/parser.h"
#include "../../include/adt/report_avl.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int pop_hole_index(const char* hole_path) {
    FILE* f = fopen(hole_path, "rb+");
    if (!f) return -1;
    char buffer[32] = {0};
    int line_count = 0;
    while (fgets(buffer, sizeof(buffer), f)) line_count++;
    if (line_count == 0) { fclose(f); return -1; }
    fseek(f, (line_count - 1) * SYSTEM_REG_LINE, SEEK_SET);
    if (!fgets(buffer, sizeof(buffer), f)) { fclose(f); return -1; }
    int target_row = atoi(buffer);
    fclose(f);
    long new_size = (long)(line_count - 1) * SYSTEM_REG_LINE;
#ifdef _WIN32
    #include <io.h>
    int fd = _open(hole_path, 0x0002);
    if (fd != -1) { _chsize(fd, new_size); _close(fd); }
#else
    #include <unistd.h>
    truncate(hole_path, new_size);
#endif
    return target_row;
}

static void push_hole_index(const char* hole_path, int disk_row) {
    FILE* f = fopen(hole_path, "ab");
    if (f) { fprintf(f, "%010d\n", disk_row); fclose(f); }
}

unsigned int generate_global_report_id_v2(void) {
    unsigned int current_id = read_system_variable(REG_IDX_GLOBAL_ID);
    write_system_variable(REG_IDX_GLOBAL_ID, current_id + 1);
    return current_id;
}

static void set_master_cell_state(const char* path, int disk_row, char state) {
    if (disk_row < 0) return;
    FILE* f = fopen(path, "rb+");
    if (!f) return;
    fseek(f, (long)disk_row * REPORT_MASTER_LINE, SEEK_SET);
    char line[REPORT_MASTER_LINE + 1] = {0};
    if (fread(line, sizeof(char), REPORT_MASTER_LINE, f) == REPORT_MASTER_LINE) {
        line[350] = state;
        fseek(f, (long)disk_row * REPORT_MASTER_LINE, SEEK_SET);
        fwrite(line, sizeof(char), REPORT_MASTER_LINE, f);
    }
    fclose(f);
}

bool process_and_flush_bench_v2(void) {
    unsigned int counter_bench = read_system_variable(REG_IDX_COUNTER_BENCH);
    if (counter_bench == 0) return true;
    
    FILE* f_bench = fopen(PATH_BENCH, "rb");
    if (!f_bench) return false;
    
    char line[REPORT_MASTER_LINE + 1] = {0};
    for (unsigned int i = 0; i < counter_bench; i++) {
        fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET);
        if (fread(line, sizeof(char), REPORT_BENCH_LINE, f_bench) != REPORT_BENCH_LINE) continue;
        char cell_state;
        Report r = line_to_report_v2(line, &cell_state);
        if (!r) continue;
        
        if (get_report_status(r) == DESTROYED) { free_report(r); continue; }
        
        const char* path_target;
        const char* path_holes;
        int reg_idx_status;
        
        if (get_report_status(r) == OPEN) { path_target = PATH_OPEN_MASTER; path_holes = PATH_OPEN_HOLES; reg_idx_status = REG_IDX_STAT_OPEN; }
        else if (get_report_status(r) == IN_PROGRESS) { path_target = PATH_PROGRESS_MASTER; path_holes = PATH_PROGRESS_HOLES; reg_idx_status = REG_IDX_STAT_PROGRESS; }
        else { path_target = PATH_CLOSED_MASTER; path_holes = PATH_CLOSED_HOLES; reg_idx_status = REG_IDX_STAT_CLOSED; }
        
        int assigned_row = pop_hole_index(path_holes);
        FILE* f_master = fopen(path_target, "rb+");
        if (!f_master) f_master = fopen(path_target, "wb+");
        
        if (f_master) {
            if (assigned_row == -1) {
                fseek(f_master, 0, SEEK_END);
                assigned_row = (int)(ftell(f_master) / REPORT_MASTER_LINE);
            }
            set_report_disk_row(r, assigned_row);
            char out_line[REPORT_MASTER_LINE + 1] = {0};
            report_to_line(out_line, r, 'A');
            fseek(f_master, (long)assigned_row * REPORT_MASTER_LINE, SEEK_SET);
            fwrite(out_line, sizeof(char), REPORT_MASTER_LINE, f_master);
            fclose(f_master);
            
            write_system_variable(reg_idx_status, read_system_variable(reg_idx_status) + 1);
            int cat_idx = REG_IDX_CAT_OTHER;
            switch(get_report_category(r)) {
                case ROAD: cat_idx = REG_IDX_CAT_ROAD; break;
                case LIGHTING: cat_idx = REG_IDX_CAT_LIGHTING; break;
                case WASTE: cat_idx = REG_IDX_CAT_WASTE; break;
                case INFRASTRUCTURE: cat_idx = REG_IDX_CAT_INFRASTRUCT; break;
                default: cat_idx = REG_IDX_CAT_OTHER; break;
            }
            write_system_variable(cat_idx, read_system_variable(cat_idx) + 1);
        }
        free_report(r);
    }
    fclose(f_bench);
    write_system_variable(REG_IDX_COUNTER_BENCH, 0); 
    rebuild_avl_indices_server();
    return true;
}

static void parse_and_load_avl(ReportAVL avl_rep, ReportAVL avl_usr, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return;
    char line[REPORT_MASTER_LINE + 1] = {0};
    while (fread(line, sizeof(char), REPORT_MASTER_LINE, f) == REPORT_MASTER_LINE) {
        if (line[350] == 'N') continue;
        char state;
        Report r = line_to_report_v2(line, &state);
        if (r && state == 'A') {
            avl_insert_by_report_id(avl_rep, get_report_id(r), r);
            avl_insert_by_user_id(avl_usr, get_report_user_id(r), get_report_id(r));
        }
        if (r) free_report(r);
    }
    fclose(f);
}

void rebuild_avl_indices_server(void) {
    ReportAVL avl_rep = create_avl();
    ReportAVL avl_usr = create_avl();
    parse_and_load_avl(avl_rep, avl_usr, PATH_OPEN_MASTER);
    parse_and_load_avl(avl_rep, avl_usr, PATH_PROGRESS_MASTER);
    parse_and_load_avl(avl_rep, avl_usr, PATH_CLOSED_MASTER);
    FILE* f_rep = fopen(PATH_AVL_REPORT_ID, "wb");
    if (f_rep) { avl_write_inorder(avl_rep, f_rep, write_avl_report_callback); fclose(f_rep); }
    FILE* f_usr = fopen(PATH_AVL_USER_ID, "wb");
    if (f_usr) { avl_write_inorder(avl_usr, f_usr, write_avl_user_callback); fclose(f_usr); }
    free_avl(avl_rep); free_avl(avl_usr);
}

bool register_report_from_citizen_ram(Report r) {
    unsigned int counter_bench = read_system_variable(REG_IDX_COUNTER_BENCH);
    unsigned int report_id = get_report_id(r);
    char line[REPORT_MASTER_LINE + 1] = {0};
    bool trovato_in_bench = false;
    unsigned int bench_slot_index = 0;
    
    FILE* f_bench = fopen(PATH_BENCH, "rb+");
    if (f_bench) {
        for (unsigned int i = 0; i < counter_bench; i++) {
            fseek(f_bench, (long)i * REPORT_BENCH_LINE, SEEK_SET);
            if (fread(line, sizeof(char), REPORT_BENCH_LINE, f_bench) != REPORT_BENCH_LINE) continue;
            char st; Report tmp = line_to_report_v2(line, &st);
            if (tmp && get_report_id(tmp) == report_id && get_report_status(tmp) == OPEN) {
                trovato_in_bench = true; bench_slot_index = i; free_report(tmp); break;
            }
            if (tmp) free_report(tmp);
        }
        if (trovato_in_bench) {
            char out_line[REPORT_MASTER_LINE + 1] = {0};
            report_to_line(out_line, r, '\0'); 
            fseek(f_bench, (long)bench_slot_index * REPORT_BENCH_LINE, SEEK_SET);
            fwrite(out_line, sizeof(char), REPORT_BENCH_LINE, f_bench);
            fclose(f_bench);
            return true;
        }
        fclose(f_bench);
    }
    
    FILE* f_avl = fopen(PATH_AVL_REPORT_ID, "rb");
    int found_old_row = -1; char found_status_char = '0';
    if (f_avl) {
        unsigned int r_id; char st; int r_row;
        while (fscanf(f_avl, "%u %c %d\n", &r_id, &st, &r_row) == 3) {
            if (r_id == report_id) { found_old_row = r_row; found_status_char = st; break; }
        }
        fclose(f_avl);
    }
    
    if (found_old_row != -1) {
        ReportStatus vecchio_stato = (ReportStatus)(found_status_char - '0');
        const char* path_old = (vecchio_stato == OPEN) ? PATH_OPEN_MASTER : (vecchio_stato == IN_PROGRESS) ? PATH_PROGRESS_MASTER : PATH_CLOSED_MASTER;
        const char* path_holes = (vecchio_stato == OPEN) ? PATH_OPEN_HOLES : (vecchio_stato == IN_PROGRESS) ? PATH_PROGRESS_HOLES : PATH_CLOSED_HOLES;
        set_master_cell_state(path_old, found_old_row, 'N');
        push_hole_index(path_holes, found_old_row);
        int old_reg = (vecchio_stato == OPEN) ? REG_IDX_STAT_OPEN : (vecchio_stato == IN_PROGRESS) ? REG_IDX_STAT_PROGRESS : REG_IDX_STAT_CLOSED;
        unsigned int prev_st_count = read_system_variable(old_reg);
        if (prev_st_count > 0) write_system_variable(old_reg, prev_st_count - 1);
    } else {
        write_system_variable(REG_IDX_NM_REPORT, read_system_variable(REG_IDX_NM_REPORT) + 1);
    }
    
    if (counter_bench >= LIMIT_BENCH) { process_and_flush_bench_v2(); counter_bench = 0; }
    f_bench = fopen(PATH_BENCH, "rb+");
    if (!f_bench) f_bench = fopen(PATH_BENCH, "wb+");
    if (f_bench) {
        char out_line[REPORT_MASTER_LINE + 1] = {0};
        report_to_line(out_line, r, '\0');
        fseek(f_bench, (long)counter_bench * REPORT_BENCH_LINE, SEEK_SET);
        fwrite(out_line, sizeof(char), REPORT_BENCH_LINE, f_bench);
        fclose(f_bench);
        write_system_variable(REG_IDX_COUNTER_BENCH, counter_bench + 1);
    }
    return true;
}



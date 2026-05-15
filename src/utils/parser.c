#include "../../include/utils/parser.h"
#include "../../include/config.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* --- Utility di formattazione --- */

void pad_string(char* dest, const char* src, int fixed_length) {
    int src_len = src ? (int)strlen(src) : 0;
    int copy_len = (src_len > fixed_length) ? fixed_length : src_len;
    
    if (copy_len > 0) {
        memcpy(dest, src, copy_len);
    }
    // Riempimento con spazi per mantenere la geometria fissa
    for (int i = copy_len; i < fixed_length; i++) {
        dest[i] = ' ';
    }
    dest[fixed_length] = '\0';
}

void trim_string(char* str) {
    int len = (int)strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

/* --- Logica per l'Utente (107 byte) --- */

void user_to_line(char* line_buffer, unsigned int id, const char* user, const char* pass, UserRole role) {
    char p_user[MAX_USERNAME + 1];
    char p_pass[MAX_PASSWORD + 1];
    
    pad_string(p_user, user, MAX_USERNAME);
    pad_string(p_pass, pass, MAX_PASSWORD);

    // Formato: ID(10) + USER(50) + PASS(50) + ROLE(1) + \n = 112... 
    // Rettifica per aderire ai 107 byte definiti: ID(5) + USER(50) + PASS(50) + ROLE(1) + \n
    sprintf(line_buffer, "%05u%-50s%-50s%c\n", id, p_user, p_pass, (char)role);
}

void line_to_user_data(const char* line_buffer, unsigned int* id, char* user, char* pass, UserRole* role) {
    char temp_id[6] = {0};
    
    memcpy(temp_id, line_buffer, 5);
    *id = (unsigned int)atoi(temp_id);

    memcpy(user, line_buffer + 5, MAX_USERNAME);
    user[MAX_USERNAME] = '\0';
    trim_string(user);

    memcpy(pass, line_buffer + 55, MAX_PASSWORD);
    pass[MAX_PASSWORD] = '\0';
    trim_string(pass);

    *role = (UserRole)line_buffer[105];
}

/* --- Logica per il Report (351 byte) --- */

void report_to_line(char* line_buffer, Report r, char cell_status) {
    char p_name[MAX_NAME];
    char p_desc[MAX_DESC];
    
    pad_string(p_name, get_report_citizen_name(r), MAX_NAME - 1);
    pad_string(p_desc, get_report_description(r), MAX_DESC - 1);

    /* Geometria 351 byte:
     * ID(10)+UID(10)+NAME(50)+CAT(1)+DESC(256)+DATE(11)+URG(1)+STAT(1)+ROW(10)+CELL(1)+\n
     */
    sprintf(line_buffer, "%010u%010u%-50s%c%-256s%-11s%c%c%010d%c\n",
            get_report_id(r),
            get_report_user_id(r),
            p_name,
            (char)get_report_category(r),
            p_desc,
            get_report_date(r),
            get_report_urgency(r),
            (char)get_report_status(r),
            get_report_disk_row(r),
            cell_status);
}

Report line_to_report(const char* line_buffer, char* cell_status_out, int* row_out) {
    char id_s[11] = {0}, uid_s[11] = {0}, name[51] = {0}, cat_c, desc[257] = {0}, 
         date[12] = {0}, urg_c, stat_c, row_s[11] = {0};

    // Estrazione millimetrica tramite offset fissi
    memcpy(id_s,   line_buffer, 10);
    memcpy(uid_s,  line_buffer + 10, 10);
    memcpy(name,   line_buffer + 20, 50);
    cat_c =        line_buffer[70];
    memcpy(desc,   line_buffer + 71, 256);
    memcpy(date,   line_buffer + 327, 11);
    urg_c =        line_buffer[338];
    stat_c =       line_buffer[339];
    memcpy(row_s,  line_buffer + 340, 10);
    *cell_status_out = line_buffer[350];

    if (*cell_status_out == 'V' || *cell_status_out == 'E') return NULL;

    trim_string(name);
    trim_string(desc);
    trim_string(date);
    *row_out = atoi(row_s);

    Report r = create_report(atoi(id_s), atoi(uid_s), name, (ReportCategory)cat_c, 
                             desc, date, urg_c, (ReportStatus)stat_c);
    set_report_disk_row(r, *row_out);
    
    return r;
}

/* --- Logica per Indici AVL --- */

void avl_to_line(char* line_buffer, unsigned int key, int value) {
    // Formato fisso 22 byte: [10 cifre][10 cifre]\n\0
    sprintf(line_buffer, "%010u%010d\n", key, value);
}
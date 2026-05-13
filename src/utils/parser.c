#include "../../include/utils/parser.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void pad_string(char* dest, const char* src, int fixed_length) {
    int src_len = src ? (int)strlen(src) : 0;
    int copy_len = (src_len > fixed_length) ? fixed_length : src_len;
    
    if (copy_len > 0) {
        memcpy(dest, src, copy_len);
    }
    
    // Riempie il resto dello spazio disponibile con caratteri di spazio ' '
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

void user_to_line(char* line_buffer, int id, const char* user, const char* pass, UserRole role) {
    char padded_user[MAX_USERNAME + 1];
    char padded_pass[MAX_PASSWORD + 1];
    
    pad_string(padded_user, user, MAX_USERNAME);
    pad_string(padded_pass, pass, MAX_PASSWORD);
    
    char role_char = (role == EMPLOYEE) ? 'D' : 'C';
    
    // Formatta la riga assicurandosi che occupi esattamente 107 caratteri (106 + \n)
    sprintf(line_buffer, "%05d%s%s%c\n", id, padded_user, padded_pass, role_char);
}

void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role) {
    char raw_id[6] = {0};
    char raw_role;
    
    // 1. Estrazione ID
    memcpy(raw_id, line_buffer, 5);
    *id = atoi(raw_id);
    
    // 2. Estrazione Username (Prende i 50 caratteri fissi)
    memcpy(user, line_buffer + 5, MAX_USERNAME);
    user[MAX_USERNAME] = '\0';
    
    // Pulisce la stringa partendo dal fondo eliminando spazi, \r, \n e spazzatura binaria
    int len_u = MAX_USERNAME;
    while (len_u > 0 && (user[len_u - 1] <= 32 || user[len_u - 1] == ' ')) {
        user[len_u - 1] = '\0';
        len_u--;
    }
    
    // 3. Estrazione Password (Prende i 50 caratteri fissi dall'indice 55)
    memcpy(pass, line_buffer + 5 + MAX_USERNAME, MAX_PASSWORD);
    pass[MAX_PASSWORD] = '\0';
    
    int len_p = MAX_PASSWORD;
    while (len_p > 0 && (pass[len_p - 1] <= 32 || pass[len_p - 1] == ' ')) {
        pass[len_p - 1] = '\0';
        len_p--;
    }
    
    // 4. Estrazione Ruolo
    raw_role = line_buffer[5 + MAX_USERNAME + MAX_PASSWORD];
    *role = (raw_role == 'D') ? EMPLOYEE : CITIZEN;
}

void report_to_line(char* line_buffer, Report r, char record_state) {
    // Scrive i dati fino all'indice 329. La stringa di formato contiene i campi 
    // fino alla descrizione (5+5+2+1+1+10+50+256 = 330 caratteri iniziali esclusi i controlli)
    sprintf(line_buffer, "%05d%05d%02d%d%d%-10.10s%-50.50s%-256.256s",
            get_report_id(r),
            0, 
            (int)get_report_category(r),
            get_report_urgency(r),
            (int)get_report_status(r),
            get_report_date(r),
            get_report_citizen_name(r),
            get_report_description(r));

    // Sanificazione di sicurezza contro i newline distruttivi all'interno dei dati
    for (int i = 0; i < 330; i++) {
        if (line_buffer[i] == '\n' || line_buffer[i] == '\r' || line_buffer[i] == '\0') {
            line_buffer[i] = ' '; 
        }
    }
    
    // Assegnazione millimetrica basata sulla mappa dei caratteri:
    line_buffer[330] = record_state; // Lo stato del record ('A', 'X') va all'indice 330
    line_buffer[331] = '\n';         // Il newline va all'indice 331 (332esimo carattere totale)
    line_buffer[332] = '\0';         // Il terminatore chiude la stringa in RAM
}




Report line_to_report(const char* line_buffer, char* record_state, int* citizen_id_out) {
    char raw_rep_id[6] = {0};
    char raw_cit_id[6] = {0};
    char raw_cat[3] = {0};
    char raw_urg;
    char raw_status;
    char raw_date[11] = {0};
    char raw_name[MAX_NAME + 1] = {0};
    char raw_desc[MAX_DESC + 1] = {0};
    
    memcpy(raw_rep_id, line_buffer, 5);
    int rep_id = atoi(raw_rep_id);
    
    memcpy(raw_cit_id, line_buffer + 5, 5);
    if (citizen_id_out) *citizen_id_out = atoi(raw_cit_id);
    
    memcpy(raw_cat, line_buffer + 10, 2);
    ReportCategory cat = (ReportCategory)atoi(raw_cat);
    
    raw_urg = line_buffer[12];
    int urgency = raw_urg - '0';
    
    raw_status = line_buffer[13];
    ReportStatus status_pratica = (ReportStatus)(raw_status - '0');
    
    memcpy(raw_date, line_buffer + 14, 10);
    raw_date[10] = '\0';
    trim_string(raw_date);
    
    memcpy(raw_name, line_buffer + 24, MAX_NAME);
    raw_name[MAX_NAME] = '\0';
    trim_string(raw_name);
    
    // Estrae i 256 caratteri della descrizione (Indici da 74 a 329)
    memcpy(raw_desc, line_buffer + 24 + MAX_NAME, MAX_DESC);
    raw_desc[MAX_DESC] = '\0';
    trim_string(raw_desc);
    
    // Legge lo stato del record esattamente all'indice 330
    *record_state = line_buffer[330];
    
    Report r = create_report(rep_id, raw_name, cat, raw_desc, raw_date, urgency);
    if (r != NULL) {
        update_report_status(r, status_pratica);
    }
    
    return r;
}


/*
void report_to_line(char* line_buffer, Report r, char record_state) {
    char padded_name[MAX_NAME + 1] = {0};
    char padded_desc[MAX_DESC + 1] = {0};
    char fixed_date[11] = {0};
    
    // Genera le stringhe pulite riempite di spazi
    pad_string(padded_name, get_report_citizen_name(r), MAX_NAME);
    pad_string(padded_desc, get_report_description(r), MAX_DESC);
    
    strncpy(fixed_date, get_report_date(r), 10);
    fixed_date[10] = '\0';
    
    int id_cittadino = 0; 
    
    // Blinda la scrittura forzando l'output a occupare esattamente 331 caratteri (330 + \n)
    // %05d(5) + %05d(5) + %02d(2) + %d(1) + %d(1) + %10s(10) + %50s(50) + %256s(256) + %c(1) = 331
    snprintf(line_buffer, 332, "%05d%05d%02d%d%d%s%s%s%c\n",
            get_report_id(r), 
            id_cittadino, 
            (int)get_report_category(r),
            get_report_urgency(r),
            (int)get_report_status(r),
            fixed_date, 
            padded_name, 
            padded_desc, 
            record_state);
}



Report line_to_report(const char* line_buffer, char* record_state, int* citizen_id_out) {
    char raw_rep_id[6] = {0};
    char raw_cit_id[6] = {0};
    char raw_cat[3] = {0};
    char raw_urg;
    char raw_status;
    char raw_date[11] = {0};
    char raw_name[MAX_NAME + 1] = {0};
    char raw_desc[MAX_DESC + 1] = {0};
    
    // 1. ID Report (0-4)
    memcpy(raw_rep_id, line_buffer, 5);
    int rep_id = atoi(raw_rep_id);
    
    // 2. ID Cittadino (5-9)
    memcpy(raw_cit_id, line_buffer + 5, 5);
    if (citizen_id_out) *citizen_id_out = atoi(raw_cit_id);
    
    // 3. Categoria (10-11)
    memcpy(raw_cat, line_buffer + 10, 2);
    ReportCategory cat = (ReportCategory)atoi(raw_cat);
    
    // 4. Urgenza (12)
    raw_urg = line_buffer[12];
    int urgency = raw_urg - '0';
    
    // 5. Stato Pratica (13)
    raw_status = line_buffer[13];
    ReportStatus status_pratica = (ReportStatus)(raw_status - '0');
    
    // 6. Data (14-23)
    memcpy(raw_date, line_buffer + 14, 10);
    raw_date[10] = '\0';
    
    // 7. Nome Cittadino (24-73)
    memcpy(raw_name, line_buffer + 24, MAX_NAME);
    raw_name[MAX_NAME] = '\0';
    trim_string(raw_name);
    
    // 8. Descrizione (74-329)
    memcpy(raw_desc, line_buffer + 24 + MAX_NAME, MAX_DESC);
    raw_desc[MAX_DESC] = '\0';
    trim_string(raw_desc);
    
    // 9. Stato Record (330)
    *record_state = line_buffer[24 + MAX_NAME + MAX_DESC];
    
    Report r = create_report(rep_id, raw_name, cat, raw_desc, raw_date, urgency);
    if (r != NULL) {
        update_report_status(r, status_pratica);
    }
    
    return r;
}

*/
void write_report_callback(FILE* f_out, Report r) {
    char line[285];
    report_to_line(line, r, 'A'); // Di default scrive come record attivo 'A'
    fputs(line, f_out);
}

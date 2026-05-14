#include "../../include/utils/parser.h"
#include "../../include/config.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void pad_string(char* dest, const char* src, int fixed_length) {
    int src_len = src ? (int)strlen(src) : 0;
    int copy_len = (src_len > fixed_length) ? fixed_length : src_len;
    
    if (copy_len > 0) {
        memcpy(dest, src, copy_len);
    }
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
    sprintf(line_buffer, "%05d%s%s%c\n", id, padded_user, padded_pass, role_char);
}

void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role) {
    char raw_id[6] = {0};
    memcpy(raw_id, line_buffer, 5);
    *id = atoi(raw_id);
    
    memcpy(user, line_buffer + 5, MAX_USERNAME);
    user[MAX_USERNAME] = '\0';
    trim_string(user);
    
    memcpy(pass, line_buffer + 5 + MAX_USERNAME, MAX_PASSWORD);
    pass[MAX_PASSWORD] = '\0';
    trim_string(pass);
    
    char raw_role = line_buffer[5 + MAX_USERNAME + MAX_PASSWORD];
    *role = (raw_role == 'D') ? EMPLOYEE : CITIZEN;
}
void report_to_line(char* line_buffer, Report r, char record_state) {
    // 1. Inizializzazione totale del buffer a caratteri di spazio per garantire un padding pulito
    memset(line_buffer, ' ', REPORT_LINE_TOTAL - 2);
    
    // 2. Formattazione temporanea dei campi dati
    char temp_data[REPORT_LINE_TOTAL + 5];
    sprintf(temp_data, "%05d%05d%02d%d%d%-10.10s%-50.50s%-256.256s",
            get_report_id(r),
            get_report_disk_row(r),
            (int)get_report_category(r),
            get_report_urgency(r),
            (int)get_report_status(r),
            get_report_date(r),
            get_report_citizen_name(r),
            get_report_description(r));

    // 3. Sanificazione totale di ogni possibile carattere di a capo inserito dall'input utente
    for (int i = 0; i < 330; i++) {
        if (temp_data[i] == '\n' || temp_data[i] == '\r' || temp_data[i] == '\0') {
            temp_data[i] = ' '; 
        }
    }   
    
    // 4. Copia dei 330 byte di testo sanificato nel buffer finale di riga
    memcpy(line_buffer, temp_data, 330);
    
    // 5. Posizionamento geometrico millimetrico dei caratteri di controllo di fine riga
    line_buffer[330] = record_state; // Il flag va esattamente al byte 330 (es. 'A')
    line_buffer[331] = '\n';         // Il newline va esattamente al byte 331
    line_buffer[332] = '\0';         // Chiusura stringa RAM
}


Report line_to_report(const char* line_buffer, char* record_state, int* disk_row_out) {
    char raw_rep_id[6] = {0};
    char raw_disk_row[6] = {0};
    char raw_cat[3] = {0};
    
    memcpy(raw_rep_id, line_buffer, 5);
    int rep_id = atoi(raw_rep_id);
    
    memcpy(raw_disk_row, line_buffer + 5, 5);
    int disk_row = atoi(raw_disk_row);
    if (disk_row_out) *disk_row_out = disk_row;
    
    memcpy(raw_cat, line_buffer + 10, 2);
    ReportCategory cat = (ReportCategory)atoi(raw_cat);
    
    int urgency = line_buffer[12] - '0';
    ReportStatus status_pratica = (ReportStatus)(line_buffer[13] - '0');
    
    char raw_date[11] = {0};
    memcpy(raw_date, line_buffer + 14, 10);
    trim_string(raw_date);
    
    char raw_name[MAX_NAME + 1] = {0};
    memcpy(raw_name, line_buffer + 24, MAX_NAME);
    trim_string(raw_name);   

    char raw_desc[MAX_DESC + 1] = {0};
    memcpy(raw_desc, line_buffer + 24 + MAX_NAME, MAX_DESC);
    trim_string(raw_desc);    
    
    *record_state = line_buffer[330]; // Estrazione del flag di cella logico    
    
    Report r = create_report(rep_id, raw_name, cat, raw_desc, raw_date, urgency);
    if (r != NULL) {
        update_report_status(r, status_pratica);
        set_report_disk_row(r, disk_row);
    }
    
    return r;
}

void write_report_callback(FILE* f_out, Report r) {
    char line[REPORT_LINE_TOTAL + 3];
    report_to_line(line, r, 'A');
    fputs(line, f_out);
}

// Scrive una riga d'indice ridotta da 12 byte: 5 cifre ID Utente + 5 cifre ID Report + \n + \0
void user_index_to_line(char* line_buffer, int id_user, int id_report) {
    sprintf(line_buffer, "%05d%05d\n", id_user, id_report);
}

// Legge la riga d'indice ridotta estraendo le chiavi numeriche
void line_to_user_index(const char* line_buffer, int* id_user_out, int* id_report_out) {
    char raw_user[6] = {0};
    char raw_report[6] = {0};
    
    // Estrazione millimetrica dei soli caratteri numerici
    memcpy(raw_user, line_buffer, 5);
    memcpy(raw_report, line_buffer + 5, 5);
    
    if (id_user_out) *id_user_out = atoi(raw_user);
    if (id_report_out) *id_report_out = atoi(raw_report);
}


// Callback speciale per il BST Utente che scrive solo le coppie di ID a 12 byte
void write_user_bst_callback(FILE* f_out, Report r) {
    char line[14];
    // Recuperiamo l'ID del report
    int rep_id = get_report_id(r);
    // Calcoliamo l'ID numerico dell'utente convertendo l'username stocastico
    unsigned long hash_user = 5381;
    const char* u_ptr = get_report_citizen_name(r);
    int c;
    while ((c = (unsigned char)*u_ptr++)) hash_user = ((hash_user << 5) + hash_user) + c;
    int user_id = (int)(hash_user % 100000); // Confinato in 5 cifre geometriche
    
    user_index_to_line(line, user_id, rep_id);
    fputs(line, f_out);
}

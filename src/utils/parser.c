#include "../../include/utils/parser.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

unsigned int read_system_variable(int reg_index) {
    FILE* f = fopen(PATH_SEQUENCE, "rb");
    if (!f) return 0;
    
    char buffer[16] = {0};
    fseek(f, reg_index * SYSTEM_REG_LINE, SEEK_SET);
    if (!fgets(buffer, sizeof(buffer), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return (unsigned int)strtoul(buffer, NULL, 10);
}

void write_system_variable(int reg_index, unsigned int value) {
    FILE* f = fopen(PATH_SEQUENCE, "rb+");
    if (!f) {
        f = fopen(PATH_SEQUENCE, "wb+");
        if (!f) return;
        /* Prealloca le 11 righe necessarie riempiendole a zero a geometria fissa */
        for (int i = 0; i < 11; i++) {
            fprintf(f, "%010u\n", 0);
        }
        fclose(f);
        f = fopen(PATH_SEQUENCE, "rb+");
        if (!f) return;
    }
    fseek(f, reg_index * SYSTEM_REG_LINE, SEEK_SET);
    fprintf(f, "%010u", value); // <-- SANIFICATO: rimosso \n. Sovrascrive solo i 10 byte numerici puri lasciando intatto il newline del file
    fclose(f);
}


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
    memset(line_buffer, ' ', REPORT_LINE_TOTAL - 2);
    
    char temp_data[REPORT_LINE_TOTAL + 10];
    sprintf(temp_data, "%010u%010u%-50.50s%d%-256.256s%-11.11s%c%d%010d",
            get_report_id(r),
            get_report_user_id(r),
            get_report_citizen_name(r),
            (int)get_report_category(r),
            get_report_description(r),
            get_report_date(r),
            get_report_urgency(r),
            (int)get_report_status(r),
            get_report_disk_row(r));

    for (int i = 0; i < 350; i++) {
        if (temp_data[i] == '\n' || temp_data[i] == '\r' || temp_data[i] == '\0') {
            temp_data[i] = ' '; 
        }
    }   
    
    memcpy(line_buffer, temp_data, 350);
    line_buffer[350] = record_state; 
    line_buffer[351] = '\n';         
}

Report line_to_report_v2(const char* line_buffer, char* record_state) {
    char raw_id[11] = {0};
    char raw_user_id[11] = {0};
    char raw_name[MAX_NAME + 1] = {0};
    char raw_desc[MAX_DESC + 1] = {0};
    char raw_date[12] = {0};
    char raw_row[11] = {0};
    
    memcpy(raw_id, line_buffer, 10);
    unsigned int id = (unsigned int)strtoul(raw_id, NULL, 10);
    
    memcpy(raw_user_id, line_buffer + 10, 10);
    unsigned int user_id = (unsigned int)strtoul(raw_user_id, NULL, 10);
    
    memcpy(raw_name, line_buffer + 20, 50);
    trim_string(raw_name);
    
    ReportCategory cat = (ReportCategory)(line_buffer[70] - '0');
    
    memcpy(raw_desc, line_buffer + 71, 256);
    trim_string(raw_desc);
    
    memcpy(raw_date, line_buffer + 327, 11);
    trim_string(raw_date);
    
    char urgency = line_buffer[338];
    ReportStatus status = (ReportStatus)(line_buffer[339] - '0');
    
    memcpy(raw_row, line_buffer + 340, 10);
    int disk_row = atoi(raw_row);
    
    *record_state = line_buffer[350];
    
    Report r = create_report(id, user_id, raw_name, cat, raw_desc, raw_date, urgency);
    if (r != NULL) {
        update_report_status(r, status);
        set_report_disk_row(r, disk_row);
    }
    return r;
}

void write_avl_report_callback(FILE* f, unsigned int key_id, unsigned int opt_id, int row, char status) {
    (void)opt_id;
    fprintf(f, "%010u %c %010d\n", key_id, status, row);
}

void write_avl_user_callback(FILE* f, unsigned int user_id, unsigned int report_id, int row, char status) {
    (void)row; (void)status;
    fprintf(f, "%010u%010u\n", user_id, report_id);
}

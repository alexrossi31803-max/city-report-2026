#include "../../include/utils/parser.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

unsigned int read_system_variable(int reg_index) {
    FILE* f = fopen(PATH_SEQUENCE, "rb");
    if (!f) return 0;
    
    char buffer[SYSTEM_REG_LINE + 3] = {0};
    /* Salto millimetrico O(1) calibrato sulla riga da 11 byte (SYSTEM_REG_LINE) */
    fseek(f, (long)reg_index * SYSTEM_REG_LINE, SEEK_SET);
    
    if (!fgets(buffer, sizeof(buffer), f)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    /* Converte la stringa numerica a 10 cifre in un intero senza segno */
    return (unsigned int)strtoul(buffer, NULL, 10);
}

void write_system_variable(int reg_index, unsigned int value) {
    /* CONTROLLO CREAZIONE PREVENTIVO: Se il file non esiste, prealloca rigidamente le 13 righe a zero */
    FILE* f_check = fopen(PATH_SEQUENCE, "rb");
    if (!f_check) {
        FILE* f_init = fopen(PATH_SEQUENCE, "wb");
        if (f_init) {
            /* Scrive le 13 righe riempiendole con 10 zeri ed un carattere di a capo (143 byte totali) */
            for (int i = 0; i < 13; i++) {
                fprintf(f_init, "%010u\n", 0);
            }
            fclose(f_init);
        }
    } else {
        fclose(f_check);
    }

    FILE* f = fopen(PATH_SEQUENCE, "rb+");
    if (!f) return;
    
    /* Salto posizionale atomico alla cella numerica bersaglio */
    fseek(f, (long)reg_index * SYSTEM_REG_LINE, SEEK_SET);
    
    /* SOVRASCRITTURA PROTETTA A 10 BYTE: Omette il \n finale. 
       In questo modo sovrascrive esclusivamente le cifre lasciando intatto il newline strutturale del file. */
    fprintf(f, "%010u", value);
    fclose(f);
}

void pad_string(char* dest, const char* src, int fixed_length) {
    int src_len = src ? (int)strlen(src) : 0;
    /* Evita buffer overflow troncando il testo se eccede la capacita fissa prescritta */
    int copy_len = (src_len > fixed_length) ? fixed_length : src_len;
    
    if (copy_len > 0) {
        memcpy(dest, src, copy_len);
    }
    /* Riempie i restanti byte con caratteri spazio per bloccare la larghezza geometrica hardware */
    for (int i = copy_len; i < fixed_length; i++) {
        dest[i] = ' ';
    }
    dest[fixed_length] = '\0'; /* Chiusura della stringa per la manipolazione in RAM */
}

void trim_string(char* str) {
    int len = (int)strlen(str);
    /* Scorre all'indietro eliminando spazi vuoti di padding, tabulazioni e newline finali */
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

void user_to_line(char* line_buffer, int id, const char* user, const char* pass, UserRole role) {
    char padded_user[MAX_USERNAME + 1];
    char padded_pass[MAX_PASSWORD + 1];
    
    /* Configura i campi stringa a larghezza rigidamente prefissata (50 byte ciascuno) */
    pad_string(padded_user, user, MAX_USERNAME);
    pad_string(padded_pass, pass, MAX_PASSWORD);
    
    char role_char = (role == EMPLOYEE) ? 'D' : 'C';
    /* Genera la stringa anagrafica finale: 5 + 50 + 50 + 1 + 1 = 107 byte complessivi */
    sprintf(line_buffer, "%05d%s%s%c\n", id, padded_user, padded_pass, role_char);
}

void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role) {
    char raw_id[6] = {0};
    /* Estrazione tramite offset statici rigidi derivati dalla compressione anagrafica */
    memcpy(raw_id, line_buffer, 5);
    *id = atoi(raw_id);
    
    memcpy(user, line_buffer + 5, MAX_USERNAME);
    user[MAX_USERNAME] = '\0';
    trim_string(user); /* Rimuove gli spazi inseriti dal padding in fase di scrittura */
    
    memcpy(pass, line_buffer + 5 + MAX_USERNAME, MAX_PASSWORD);
    pass[MAX_PASSWORD] = '\0';
    trim_string(pass);
    
    /* Il carattere del ruolo risiede esattamente all'indice offset 105 della riga */
    *role = (line_buffer[5 + MAX_USERNAME + MAX_PASSWORD] == 'D') ? EMPLOYEE : CITIZEN;
}

void report_to_line(char* line_buffer, Report r, char record_state) {
    char padded_name[MAX_NAME + 1];
    char padded_desc[MAX_DESC + 1];
    char padded_date[12];
    
    pad_string(padded_name, get_report_citizen_name(r), MAX_NAME);
    pad_string(padded_desc, get_report_description(r), MAX_DESC);
    pad_string(padded_date, get_report_date(r), 11);
    
    char temp_body[351] = {0};
    /* COMPRESSIONE AD ALTA DENSITÀ INFORMATIVA SENZA SPAZIATORI O TRATTINI AUSILIARI:
       - ID: 10 byte (%010u)         - USER_ID: 10 byte (%010u)     - NAME: 50 byte (fisso)
       - CATEGORY: 1 byte (%d)       - DESCRIPTION: 256 byte (fisso)- DATE: 11 byte (fisso)
       - URGENCY: 1 byte (%c)        - STATUS: 1 byte (%d)          - DISK_ROW: 10 byte (%010d)
       TOTALE CORPO RECORD = 10 + 10 + 50 + 1 + 256 + 11 + 1 + 1 + 10 = 350 byte puri. */
    sprintf(temp_body, "%010u%010u%s%d%-256.256s%s%c%d%010d",
            get_report_id(r),
            get_report_user_id(r),
            padded_name,
            (int)get_report_category(r),
            padded_desc,
            padded_date,
            get_report_urgency(r),
            (int)get_report_status(r),
            get_report_disk_row(r));
            
    if (record_state != '\0') {
        /* CONFIGURAZIONE MASTER ASIMMETRICA (REPORT_MASTER_LINE = 352 byte) */
        memset(line_buffer, ' ', REPORT_MASTER_LINE - 1);
        memcpy(line_buffer, temp_body, 350);
        line_buffer[350] = record_state; /* Flag di cella logica Active/Null/End inserito al byte 350 */
        line_buffer[351] = '\n';         /* Newline strutturale inserito al byte 351 */
        line_buffer[352] = '\0';
    } else {
        /* CONFIGURAZIONE CACHE BENCH ASIMMETRICA (REPORT_BENCH_LINE = 351 byte) */
        memset(line_buffer, ' ', REPORT_BENCH_LINE - 1);
        memcpy(line_buffer, temp_body, 350);
        line_buffer[350] = '\n';         /* Sulla BENCH il newline viene posizionato direttamente al byte 350 */
        line_buffer[351] = '\0';
    }
}

Report line_to_report_v2(const char* line_buffer, char* record_state) {
    char raw_id[11] = {0};
    char raw_user_id[11] = {0};
    char raw_name[MAX_NAME + 1] = {0};
    char raw_desc[MAX_DESC + 1] = {0};
    char raw_date[12] = {0};
    char raw_row[11] = {0};
    
    /* ESTRAZIONE FILTRATA TRAMITE OFFSET FISSI HARDWARE (SIMULAZIONE INTERFACCIA DB RELAZIONALE) */
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
    
    /* Il flag dello stato della cella risiede immancabilmente all'indice offset 350 del blocco riga */
    *record_state = line_buffer[350];
    
    /* Istanziazione controllata dell'oggetto opaco di ritorno con riallineamento dei mutatori */
    Report r = create_report(id, user_id, raw_name, cat, raw_desc, raw_date, urgency);
    if (r != NULL) {
        update_report_status(r, status);
        set_report_disk_row(r, disk_row);
    }
    return r;
}

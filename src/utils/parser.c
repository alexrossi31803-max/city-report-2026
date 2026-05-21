#include "../../include/utils/parser.h"
#include "../../include/config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
unsigned int read_system_variable(int reg_index) {
	/* controllo limiti (13 righe nel file) */
    if (reg_index < 0 || reg_index >= SYSTEM_REG_COUNT)return 0;    
    FILE* f = fopen(PATH_SEQUENCE, "rb");
    if (!f) return 0;
    char buffer[SYSTEM_REG_LINE + 1] = {0}; /* buffer per 11 byte + '\0' per sicurezza C-string */
    /* Salto millimetrico O(1) calibrato sulla riga da 11 byte (SYSTEM_REG_LINE) */
    fseek(f, (long)reg_index * SYSTEM_REG_LINE, SEEK_SET);
    
    size_t read = fread(buffer, 1, SYSTEM_REG_LINE, f); /* leggo ESATTAMENTE 11 byte, fread legge SYSTEM_REG_LINE elementi da 1 byte da f */
    fclose(f);

    if (read != SYSTEM_REG_LINE)return 0;
    /* Converte la stringa numerica a 10 cifre in un intero senza segno */
    return (unsigned int)strtoul(buffer, NULL, 10);
}

void write_system_variable(int reg_index, unsigned int value){
       // Controllo validità indice: valori validi da 0 a 12 
    if (reg_index < 0|| reg_index >= SYSTEM_REG_COUNT) return;
       // Se il file non esiste lo creiamo allocando 13 righe.
       // Ogni riga deve essere lunga ESATTAMENTE 11 byte: "0000000000\n" 10 cifre + '\n'
    FILE* f_check = fopen(PATH_SEQUENCE, "rb");
    if (!f_check)
    {
        FILE* f_init = fopen(PATH_SEQUENCE, "wb");

        if (f_init)
        {
            const char line[SYSTEM_REG_LINE + 1] = "0000000000\n"; // riga da inserire 13 volte nel file system_total_report.txt
            for (int i = 0; i < SYSTEM_REG_COUNT; i++)
            {
                fwrite(line, 1, SYSTEM_REG_LINE, f_init); // Scrive la riga ogni volta 
            }
            fclose(f_init);
        }
    }
    else
    {
        fclose(f_check);
    }
    //Apertura file in lettura/scrittura binaria
    FILE* f = fopen(PATH_SEQUENCE, "rb+");
    if (!f)
        return;
    long offset = (long)reg_index * SYSTEM_REG_LINE; // Calcolo offset riga

    fseek(f, offset, SEEK_SET); // Sposta il cursore all'inizio della riga 

    char line[SYSTEM_REG_LINE+1]; // Crea la nuova riga da scrivere 

    snprintf(line, sizeof(line), "%010u\n", value); // esempio "0000000025\n"

    fwrite(line, 1, SYSTEM_REG_LINE, f); // Sovrascrive la line con 11 byte

    fclose(f);
}

void pad_string(char* dest, const char* src, int fixed_length) {
    int src_len = src ? (int)strlen(src) : 0;
    /* Impedisce l'overflow troncando la stringa sorgente se eccede la lunghezza fissa */
    int copy_len = (src_len > fixed_length) ? fixed_length : src_len;
    
    if (copy_len > 0) {
        memcpy(dest, src, copy_len);
    }
    /* Riempie i byte rimanenti con caratteri spazio per bloccare la geometria hardware */
    for (int i = copy_len; i < fixed_length; i++) {
        dest[i] = ' ';
    }
    /* Chiude la stringa con il terminatore nullo di sicurezza per l'uso in RAM */
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
    
    /* Configura i campi stringa a larghezza rigidamente prefissata */
    pad_string(padded_user, user, MAX_USERNAME);
    pad_string(padded_pass, pass, MAX_PASSWORD);
    
    char role_char = (role == EMPLOYEE) ? 'D' : 'C';
    /* Genera la stringa anagrafica finale: 10 + MAX_USERNAME + MAX_PASSWORD + 1(ROLE)+ 1(/n) */
    sprintf(line_buffer, "%010d%s%s%c\n", id, padded_user, padded_pass, role_char);
}

void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role) {
    char raw_id[11] = {0};
    /* Estrazione tramite offset statici rigidi derivati dalla compressione anagrafica */
    memcpy(raw_id, line_buffer, 10);
    *id = atoi(raw_id);
    
    memcpy(user, line_buffer + 10, MAX_USERNAME);
    user[MAX_USERNAME] = '\0';
    trim_string(user); /* Rimuove gli spazi inseriti dal padding in fase di scrittura */
    
    memcpy(pass, line_buffer + 10 + MAX_USERNAME, MAX_PASSWORD);
    pass[MAX_PASSWORD] = '\0';
    trim_string(pass);
    
    /* Il carattere del ruolo risiede esattamente all'indice offset 10 + MAX_USERNAME + MAX_PASSWORD della riga */
    *role = (line_buffer[10 + MAX_USERNAME + MAX_PASSWORD] == 'D') ? EMPLOYEE : CITIZEN;
}

void report_to_line(char* line_buffer, Report r, char record_state){
    char temp_body[REPORT_BENCH_LINE] = {0};
	/* COMPRESSIONE AD ALTA DENSITÀ INFORMATIVA SENZA SPAZIATORI O TRATTINI AUSILIARI:
       - ID: 10 byte (%010u)         - USER_ID: 10 byte (%010u)    - USERNAME: MAX_USERNAME byte 
       - CATEGORY: 1 byte (%d)       - DESCRIPTION: MAX_DESC byte   - DATE: 11 byte 
       - URGENCY: 1 byte (%c)        - STATUS: 1 byte (%d)          - DISK_ROW: 10 byte (%010d)
       TOTALE CORPO RECORD = 10(REPORT_ID)+ 10(USER_ID) + MAX_USERNAME + 1(Report_category) + MAX_DESC + 10(Report_date) + 1 (Report_urgency)+ 1(Report_status) + 10(disk_row) byte puri. */
    int len = snprintf(temp_body, sizeof(temp_body),"%010u%010u%-*.*s%c%-*.*s%-10.10s%c%c%010d",
        get_report_id(r),
        get_report_user_id(r),
        MAX_USERNAME, MAX_USERNAME, //larghezza totale, max caratteri 
        get_report_citizen_name(r),
        (char)(get_report_category(r) + '0'),
        MAX_DESC, MAX_DESC, //larghezza totale, max caratteri 
        get_report_description(r),
        get_report_date(r),
        get_report_urgency(r),
        (char)(get_report_status(r) + '0'),
        get_report_disk_row(r)
    );
	
	if (len <= 0 || len >= (int)sizeof(temp_body))return; // controllo di sicurezza

    /* ------------------------------------------------------------
       MASTER LINE (con record_state)
       Layout:
       [temp_body ............][record_state]['\n']
       lunghezza = REPORT_MASTER_LINE
    ------------------------------------------------------------ */
    if (record_state != 0)
    {
        memset(line_buffer, ' ', REPORT_MASTER_LINE); // setting di line buffer a REPORT_MASTER_LINE ' '
        memcpy(line_buffer, temp_body, len);
        line_buffer[REPORT_MASTER_LINE - 2] = record_state; /* penultimo byte */
        line_buffer[REPORT_MASTER_LINE - 1] = '\n';          /* ultimo byte */
		line_buffer[REPORT_MASTER_LINE] = '\0';
    }
    /* ------------------------------------------------------------
       BENCH LINE (senza record_state)
       Layout:
       [temp_body ............]['\n']
       lunghezza = REPORT_BENCH_LINE
    ------------------------------------------------------------ */
    else
    {
        memset(line_buffer, ' ', REPORT_BENCH_LINE); // setting di line buffer a REPORT_BENCH_LINE ' '
        memcpy(line_buffer, temp_body, len);
        line_buffer[REPORT_BENCH_LINE - 1] = '\n'; /* ultimo byte */
		line_buffer[REPORT_BENCH_LINE] = '\0';
    }
}

Report line_to_report(const char* line_buffer, char* record_state)
{
    char raw_id[11] = {0}; // '\0'
    char raw_user_id[11] = {0};
    char raw_name[MAX_USERNAME + 1] = {0};
    char raw_desc[MAX_DESC + 1] = {0};
    char raw_date[11] = {0};
    char raw_row[11] = {0};
	
	//REPORT_MASTER_LINE: 10(REPORT_ID)+ 10(USER_ID) + MAX_USERNAME + 1(Report_category) + MAX_DESC + 11(Report_date) + 1 (Report_urgency)+ 1(Report_status) + 10(disk_row) + 1(flag_cella)+ 1 byte di newline byte
    /* ---------------- REPORT ID ---------------- */
    memcpy(raw_id, line_buffer, 10); // copia di REPORT_ID
    unsigned int report_id = (unsigned int)strtoul(raw_id, NULL, 10); // conversione di stringa in unsigned int

    /* ---------------- USER ID ---------------- */
    memcpy(raw_user_id, line_buffer + 10, 10); // copia di USER_ID
    unsigned int user_id = (unsigned int)strtoul(raw_user_id, NULL, 10); // conversione di stringa in unsigned int 

    /* ---------------- USER NAME ---------------- */
    memcpy(raw_name, line_buffer + 20, MAX_USERNAME); // copia di USER_NAME
    raw_name[MAX_USERNAME] = '\0';
    trim_string(raw_name);

    /* ---------------- REPORT CATEGORY ---------------- */
    ReportCategory cat = (ReportCategory)(line_buffer[20+MAX_USERNAME] - '0'); // copia di REPORT_CATEGORY

    /* ---------------- REPORT DESCRIPTION ---------------- */
    memcpy(raw_desc, line_buffer + 20+MAX_USERNAME+1, MAX_DESC); // copia di REPORT_DESCRIPTION 
    raw_desc[MAX_DESC] = '\0';
    trim_string(raw_desc);

    /* ---------------- REPORT DATE ---------------- */
    memcpy(raw_date, line_buffer + 20+MAX_USERNAME+1+MAX_DESC, 10); // copia di record date 
    raw_date[10] = '\0';

    /* ---------------- URGENCY / STATUS ---------------- */
    char urgency = line_buffer[20+MAX_USERNAME+1+MAX_DESC+10]; // copia di urgency 
    ReportStatus status = (ReportStatus)(line_buffer[20+MAX_USERNAME+1+MAX_DESC+10+1] - '0'); // copia di status 

    /* ---------------- DISK ROW ---------------- */
    memcpy(raw_row, line_buffer + 20+MAX_USERNAME+1+MAX_DESC+10+1+1, 10); // copia di disk row
    raw_row[10] = '\0';
    unsigned int disk_row = (unsigned int)strtoul(raw_row, NULL, 10); // Conversione di stringa in unsigned int 

    /* ---------------- CELL FLAG ---------------- */
    if(record_state)*record_state = line_buffer[20+MAX_USERNAME+1+MAX_DESC+10+1+1+10]; // copia di cell flag 

    /* ---------------- REPORT CREATION ---------------- */
    Report r = create_report(report_id, user_id, raw_name, cat, raw_desc, raw_date, urgency);

    if (r != NULL)
    {
        update_report_status(r, status);
        set_report_disk_row(r, disk_row);
    }

    return r;
}
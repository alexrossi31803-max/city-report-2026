#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../models/user.h"
#include "../models/report.h"

/* Routine atomiche di I/O sul registro di stato centralizzato in O(1) */
unsigned int read_system_variable(int reg_index);
void write_system_variable(int reg_index, unsigned int value);

/* Utilità di sanificazione e riallineamento geometrico stringhe */
void pad_string(char* dest, const char* src, int fixed_length);
void trim_string(char* str);

/* Motori di parsing anagrafica utente (Geometria fissa 107 byte) */
void user_to_line(char* line_buffer, int id, const char* user, const char* pass, UserRole role);
void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role);

/* Motori di parsing segnalazione master (Geometria rigida 352 byte) */
void report_to_line(char* line_buffer, Report r, char record_state);
Report line_to_report_v2(const char* line_buffer, char* record_state);

/* Callback di esportazione simmetrica In-Order per i due indici AVL */
void write_avl_report_callback(FILE* f, unsigned int key_id, unsigned int opt_id, int row, char status);
void write_avl_user_callback(FILE* f, unsigned int user_id, unsigned int report_id, int row, char status);

#endif

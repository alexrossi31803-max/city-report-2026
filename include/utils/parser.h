#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../models/user.h"
#include "../models/report.h"

// Formatta una stringa inserendo spazi in coda per raggiungere la dimensione fissa desiderata
void pad_string(char* dest, const char* src, int fixed_length);

// Rimuove gli spazi in coda inseriti dal padding durante la lettura dal file .txt
void trim_string(char* str);

// PARSER UTENTE: Scrive e legge un utente in formato riga fissa .txt (107 char)
void user_to_line(char* line_buffer, int id, const char* user, const char* pass, UserRole role);
void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role);

// PARSER REPORT: Scrive e legge un report in formato riga fissa .txt (281 char)
void report_to_line(char* line_buffer, Report r, char record_state);
Report line_to_report(const char* line_buffer, char* record_state, int* citizen_id_out);

// Funzione callback per stampare un report tramite il BST o code
void write_report_callback(FILE* f_out, Report r);

#endif
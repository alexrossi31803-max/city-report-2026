#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../models/user.h"
#include "../models/report.h"

/* ========================================================================== */
/* UTILITÀ DI FORMATTAZIONE STRINGHE                                         */
/* ========================================================================== */

/**
 * @brief Riempie una stringa di spazi fino alla lunghezza desiderata.
 * @param dest Buffer di destinazione (deve essere fixed_length + 1)
 * @param src Stringa sorgente
 * @param fixed_length Lunghezza fissa desiderata
 */
void pad_string(char* dest, const char* src, int fixed_length);

/**
 * @brief Rimuove gli spazi di padding in coda a una stringa.
 */
void trim_string(char* str);

/* ========================================================================== */
/* PARSING ANAGRAFICA UTENTE (107 BYTE)                                      */
/* ========================================================================== */

/**
 * @brief Converte i dati utente in una riga fissa da 107 byte.
 */
void user_to_line(char* line_buffer, unsigned int id, const char* user, const char* pass, UserRole role);

/**
 * @brief Estrae i dati da una riga da 107 byte per creare un oggetto User.
 */
void line_to_user_data(const char* line_buffer, unsigned int* id, char* user, char* pass, UserRole* role);

/* ========================================================================== */
/* PARSING SEGNALAZIONI (351 BYTE)                                           */
/* ========================================================================== */

/**
 * @brief Serializza un report in una riga da 351 byte.
 * @param line_buffer Il buffer di output
 * @param r L'oggetto report
 * @param cell_status 'A' (Attivo), 'V' (Vuoto/Buco), 'E' (Fine file)
 */
void report_to_line(char* line_buffer, Report r, char cell_status);

/**
 * @brief Deserializza una riga da 351 byte in un oggetto Report RAM.
 * @param line_buffer La riga letta dal file
 * @param cell_status_out Ritorna lo stato della cella ('A', 'V', 'E')
 * @param row_out Ritorna l'indice di riga decodificato (per i file Master)
 */
Report line_to_report(const char* line_buffer, char* cell_status_out, int* row_out);

/* ========================================================================== */
/* PARSING INDICE AVL (CHIAVI NUMERICHE)                                     */
/* ========================================================================== */

/**
 * @brief Formatta una coppia di ID per la persistenza dell'albero AVL.
 * Formato: [KEY(10)][VALUE(10)]\n\0
 */
void avl_to_line(char* line_buffer, unsigned int key, int value);

#endif
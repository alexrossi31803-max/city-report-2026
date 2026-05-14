#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "../models/user.h"
#include "../models/report.h"

// ==============================================================================
//  UTILITÀ DI FORMATTAZIONE STRINGHE
// ==============================================================================

/**
 * @brief Formatta una stringa inserendo spazi in coda per raggiungere la dimensione fissa desiderata.
 * @pre dest deve essere preallocato con spazio sufficiente (fixed_length + 1).
 * @post dest conterrà src completata da spazi di padding e sigillata da \0.
 */
void pad_string(char* dest, const char* src, int fixed_length);

/**
 * @brief Rimuove gli spazi in coda (padding) inseriti durante la scrittura su file binarizzato.
 * @pre str deve essere una stringa valida terminata da \0.
 * @post Modifica str sul posto inserendo \0 al termine dei caratteri grafici effettivi.
 */
void trim_string(char* str);

// ==============================================================================
//  MOTORID DI PARSING ANAGRAFICA UTENTE (GEOMETRIA FIKSA 107 BYTE)
// ==============================================================================

void user_to_line(char* line_buffer, int id, const char* user, const char* pass, UserRole role);
void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role);

// ==============================================================================
//  MOTORI DI PARSING SEGNALAZIONE MASTER (GEOMETRIA FISSA 332 BYTE)
// ==============================================================================

/**
 * @brief Converte un oggetto Report in una riga a lunghezza fissa strutturata per il disco.
 * @pre line_buffer preallocato di dimensione minima REPORT_LINE_TOTAL + 1. r valido non NULL.
 * @post line_buffer conterrà esattamente 332 byte terminati da \0, inclusi i metadati geometrici.
 */
void report_to_line(char* line_buffer, Report r, char record_state);

/**
 * @brief Effettua il parsing inverso ricreando un oggetto Report da una riga binaria a dimensione fissa.
 * @pre line_buffer deve contenere una riga geometricamente integra da 332 caratteri.
 * @post Alloca in RAM un oggetto Report popolato e inietta nel record_state il flag letto.
 */
Report line_to_report(const char* line_buffer, char* record_state, int* disk_row_out);

/**
 * @brief Callback di sistema per la serializzazione in-order del BST dei report storici completi.
 * @pre f_out aperto in modalità scrittura binaria ("wb"), r istanza valida non NULL.
 * @post Scrive una riga da 332 byte sul canale di persistenza.
 */
void write_report_callback(FILE* f_out, Report r);

// ==============================================================================
//  MOTORI DI PARSING INDICE RIDOTTO UTENTE (GEOMETRIA CONTRATTA 12 BYTE)
// ==============================================================================

/**
 * @brief Converte la coppia ID Utente ed ID Report in una riga fissa minima per disaccoppiare lo storico.
 * @pre line_buffer preallocato con dimensione minima di 13 byte.
 * @post Genera sul buffer esattamente 12 byte compatti nel formato: [ID_USER(5)][ID_REPORT(5)]\n\0.
 */
void user_index_to_line(char* line_buffer, int id_user, int id_report);

/**
 * @brief Effettua il parsing della riga ridotta estraendo le sole chiavi numeriche per la triangolazione binaria.
 * @pre line_buffer deve contenere una riga geometricamente integra da 12 caratteri caricate via fread.
 * @post Inietta nelle variabili di output i rispettivi codici numerici interi decodificati.
 */
void line_to_user_index(const char* line_buffer, int* id_user_out, int* id_report_out);

/**
 * @brief Callback speciale per la serializzazione in-order dell'indice bst_by_user_id ridotto.
 * @pre f_out aperto in scrittura binaria ("wb"), r istanza di report valida.
 * @post Estrae le chiavi numeriche stabili e scrive sul disco una riga contratta da soli 12 byte.
 */
void write_user_bst_callback(FILE* f_out, Report r);

#endif


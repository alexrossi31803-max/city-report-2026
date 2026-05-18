#ifndef PARSER_H
#define PARSER_H

#include "../models/user.h"
#include "../models/report.h"
#include <stdio.h>

/**
 * @brief Legge una variabile numerica intera dal registro centrale in tempo costante O(1).
 * @param reg_index L'indice posizionale della riga nel registro (da 0 a 12).
 * @return Il valore unsigned int estratto dalla riga geometrica fissa da 11 byte.
 */
unsigned int read_system_variable(int reg_index);

/**
 * @brief Sovrascrive atomicamente una variabile nel registro centrale in O(1).
 * @param reg_index L'indice posizionale della riga da alterare (da 0 a 12).
 * @param value Il nuovo valore unsigned int da iniettare a larghezza fissa (10 cifre).
 */
void write_system_variable(int reg_index, unsigned int value);

/**
 * @brief Applica un riempimento statico di caratteri spazio ad una stringa (Padding).
 *        Indispensabile per bloccare la larghezza dei campi di testo sul disco a passi fissi.
 * @param dest Buffer di destinazione in RAM.
 * @param src Stringa sorgente di input.
 * @param fixed_length La lunghezza rigidamente prescritta per il campo.
 */
void pad_string(char* dest, const char* src, int fixed_length);

/**
 * @brief Rimuove i caratteri di spaziatura e i newline residui in coda ad una stringa (Trimming).
 *        Invocata subito dopo l'estrazione geometrica dal disco per ripristinare il testo puro.
 * @param str La stringa da sanificare in RAM.
 */
void trim_string(char* str);

/**
 * @brief Serializza i dati anagrafici dell'utente in una riga rigida da 107 byte.
 *        Geometria: 5(ID) + 50(User) + 50(Pass) + 1(Role) + 1(\n) = 107 byte puri.
 */
void user_to_line(char* line_buffer, int id, const char* user, const char* pass, UserRole role);

/**
 * @brief Esegue il parsing inverso deserializzando una riga da 107 byte in metadati utente.
 */
void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role);

/**
 * @brief Serializza un oggetto Report applicando la doppia configurazione geometrica asimmetrica.
 * @param line_buffer Buffer di RAM in cui depositare la stringa formattata.
 * @param r L'istanza dell'oggetto opaco Report da convertire.
 * @param record_state Se impostato a '\0' -> Configurazione BENCH: riga da 351 byte (\n al byte 350).
 *                     Se impostato ad 'A'/'N'/'E' -> Configurazione MASTER: riga da 352 byte (Flag al 350, \n al 351).
 */
void report_to_line(char* line_buffer, Report r, char record_state);

/**
 * @brief Esegue il parsing inverso deserializzando una riga a geometria fissa (351 o 352 byte).
 * @param line_buffer Buffer contenente la riga estratta fisicamente dal disco.
 * @param record_state Puntatore di output che intercetta il flag di cella (Active, Null, End).
 * @return Il puntatore opaco all'oggetto Report rigenerato in RAM con consistenza di offset.
 */
Report line_to_report_v2(const char* line_buffer, char* record_state);

/* --------------------------------==============================================
 *  CALLBACK DI FORMATTAZIONE PER LE ESPORTAZIONI INORDER DEI FILE D'INDICE
 * --------------------------------============================================== */
void write_avl_report_callback(FILE* f, unsigned int key_id, unsigned int opt_id, int row, char status);
void write_avl_user_callback(FILE* f, unsigned int user_id, unsigned int report_id, int row, char status);

#endif


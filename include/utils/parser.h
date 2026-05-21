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
 * @brief Serializza i dati anagrafici dell'utente in una riga rigida da USER_LINE_TOTAL byte.
 *        Geometria: 10(USER_ID) + MAX_USERNAME + MAX_PASSWORD + 1(Role) + 1(\n) byte puri.
 */
void user_to_line(char* line_buffer, int id, const char* user, const char* pass, UserRole role);

/**
 * @brief Esegue il parsing inverso deserializzando una riga da USER_LINE_TOTAL byte in metadati utente.
 */
void line_to_user_data(const char* line_buffer, int* id, char* user, char* pass, UserRole* role);

/**
 * @brief Serializza un oggetto Report applicando la doppia configurazione geometrica asimmetrica.
 * @param line_buffer Buffer di RAM in cui depositare la stringa formattata.
 * @param r L'istanza dell'oggetto opaco Report da convertire.
 * @param record_state Se impostato a '\0' -> Configurazione BENCH: riga da REPORT_BENCH_LINE byte (\n al byte REPORT_BENCH_LINE-1).
 *                     Se impostato ad 'A'/'N'/'E' -> Configurazione MASTER: riga da REPORT_MASTER_LINE byte (Flag al REPORT_MASTER_LINE-2, \n al REPORT_MASTER_LINE-1).
 */
void report_to_line(char* line_buffer, Report r, char record_state);

/**
 * @brief Esegue il parsing inverso deserializzando una riga a geometria fissa (REPORT_BENCH_LINE o REPORT_MASTER_LINE byte).
 * @param line_buffer Buffer contenente la riga estratta fisicamente dal disco.
 * @param record_state Puntatore di output che intercetta il flag di cella (Active, Null, End).
 * @return Il puntatore opaco all'oggetto Report rigenerato in RAM con consistenza di offset.
 */
Report line_to_report(const char* line_buffer, char* record_state);

#endif


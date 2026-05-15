#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <stdbool.h>
#include "../config.h"

/**
 * @brief Verifica che una stringa non sia NULL e non contenga solo spazi.
 * @param str La stringa da analizzare.
 * @return true se valida, false se vuota o composta solo da whitespace.
 */
bool validate_not_empty(const char* str);

/**
 * @brief Verifica che la stringa rientri nei limiti dimensionali del database.
 * @param str La stringa da controllare.
 * @param max_len Lunghezza massima consentita (inclusa in config.h).
 * @return true se la lunghezza è accettabile.
 */
bool validate_length(const char* str, int max_len);

/**
 * @brief Valida il formato della data "GG/MM/AAAA".
 * Controlla sia la struttura (posizioni dei '/') sia la coerenza numerica base.
 * @param date La stringa data.
 * @return true se il formato è corretto.
 */
bool validate_date_format(const char* date);

/**
 * @brief Verifica che il carattere di urgenza sia tra quelli ammessi.
 * @param urgency Carattere che rappresenta l'urgenza ('1', '2', '3').
 * @return true se l'urgenza è valida.
 */
bool validate_urgency_range(char urgency);

/**
 * @brief Verifica se un nome utente rispetta i criteri minimi (es. no caratteri speciali).
 * @param username L'username da validare.
 */
bool validate_alphanumeric(const char* username);

#endif
#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <stdbool.h>

/**
 * @brief Verifica che una stringa non sia vuota o composta da soli spazi.
 *        Indispensabile per impedire descrizioni o username nulli.
 * @param str La stringa da sottoporre a verifica in RAM.
 * @return true se la stringa contiene almeno un carattere valido, false altrimenti.
 */
bool validate_not_empty(const char* str);

/**
 * @brief Valida formalmente il formato stringa di una data (Formato richiesto: GG/MM/AAAA).
 *        Esegue un controllo sintattico sui separatori ed uno semantico sui range di giorni e mesi.
 * @param date_str La stringa temporale da validare.
 * @return true se la data rispetta rigorosamente i vincoli del calendario, false altrimenti.
 */
bool validate_date_format(const char* date_str);

/**
 * @brief Valida che il carattere di urgenza sia confinato nel range scalare ammesso.
 *        Range accettato: '0' (Bassa), '1' (Media), '2' (Alta).
 * @param urgency_char Il carattere inviato dall'interfaccia utente.
 * @return true se il carattere è valido, false in caso di violazione dei vincoli.
 */
bool validate_urgency_range(char urgency_char);

#endif

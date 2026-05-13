#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <stdbool.h>
#include "../config.h"

// Verifica che una stringa non sia vuota o composta solo da spazi
bool validate_not_empty(const char* str);

// Verifica la lunghezza massima di una stringa (incluso il terminatore)
bool validate_length(const char* str, int max_len);

// Verifica il formato della data "GG/MM/AAAA"
bool validate_date_format(const char* date);

// Verifica che il livello di urgenza sia compreso tra 1 (Bassa) e 3 (Alta)
bool validate_urgency_range(int urgency);

#endif
#include "../../include/utils/validators.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

bool validate_not_empty(const char* str) {
    if (!str || strlen(str) == 0) return false;
    
    // Controlla se contiene almeno un carattere non-spazio
    while (*str) {
        if (!isspace((unsigned char)*str)) return true;
        str++;
    }
    return false;
}

bool validate_length(const char* str, int max_len) {
    if (!str) return false;
    return (int)strlen(str) <= max_len;
}

bool validate_date_format(const char* date) {
    if (!date || strlen(date) != 10) return false;

    // Formato atteso: GG/MM/AAAA
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) {
            if (date[i] != '/') return false;
        } else {
            if (!isdigit((unsigned char)date[i])) return false;
        }
    }

    // Validazione logica base (giorni e mesi)
    int g = (date[0] - '0') * 10 + (date[1] - '0');
    int m = (date[3] - '0') * 10 + (date[4] - '0');
    int a = atoi(date + 6);

    if (m < 1 || m > 12) return false;
    if (g < 1 || g > 31) return false;
    if (a < 2000 || a > 2100) return false;

    return true;
}

bool validate_urgency_range(char urgency) {
    // Accettiamo i caratteri '1', '2', '3' come da specifiche
    return (urgency >= '1' && urgency <= '3');
}

bool validate_alphanumeric(const char* str) {
    if (!str || strlen(str) == 0) return false;
    
    while (*str) {
        if (!isalnum((unsigned char)*str) && *str != '_' && *str != '.') {
            return false;
        }
        str++;
    }
    return true;
}
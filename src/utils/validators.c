#include "../../include/utils/validators.h"
#include <string.h>
#include <ctype.h>

bool validate_not_empty(const char* str) {
    if (str == NULL || strlen(str) == 0) return false;
    
    while (*str) {
        if (!isspace((unsigned char)*str)) return true;
        str++;
    }
    return false;
}

bool validate_length(const char* str, int max_len) {
    if (str == NULL) return false;
    return (int)strlen(str) < max_len;
}

bool validate_date_format(const char* date) {
    if (date == NULL || strlen(date) != 10) return false;
    
    // Verifica il pattern GG/MM/AAAA (es. 13/05/2026)
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) {
            if (date[i] != '/') return false;
        } else {
            if (!isdigit((unsigned char)date[i])) return false;
        }
    }
    
    int giorno = (date[0] - '0') * 10 + (date[1] - '0');
    int mese = (date[3] - '0') * 10 + (date[4] - '0');
    
    if (mese < 1 || mese > 12) return false;
    if (giorno < 1 || giorno > 31) return false;
    
    return true;
}

bool validate_urgency_range(char urgency) {
    return (urgency == '0' || urgency == '1' || urgency == '2');
}

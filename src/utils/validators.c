#include "../../include/utils/validators.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

bool validate_not_empty(const char* str) {
    /* CONTROLLO PREVENTIVO: Se il puntatore è nullo, la validazione fallisce immediatamente */
    if (!str) return false;
    
    /* Scansiona la stringa alla ricerca di almeno un carattere grafico che non sia uno spazio */
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return true; /* Trovato un carattere valido: stringa non vuota */
        }
        str++;
    }
    return false; /* La stringa è composta esclusivamente da spazi vuoti o tabulazioni */
}

bool validate_date_format(const char* date_str) {
    /* CONTROLLO PREVENTIVO: La stringa deve occupare esattamente 10 caratteri (GG/MM/AAAA) */
    if (!date_str || strlen(date_str) != 10) return false;

    /* 1. VERIFICA SINTATTICA DEI CARATTERI E DEI SEPARATORI SLANTED SLASHE '/' */
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) {
            if (date_str[i] != '/') return false; /* Errore: separatore mancante o errato */
        } else {
            if (!isdigit((unsigned char)date_str[i])) return false; /* Errore: atteso un carattere numerico */
        }
    }

    /* 2. ESTRAZIONE CHIRURGICA DEI PARAMETRI INTERI TRAMITE SOTTO-STRINGHE */
    char raw_day[3] = { date_str[0], date_str[1], '\0' };
    char raw_month[3] = { date_str[3], date_str[4], '\0' };
    char raw_year[5] = { date_str[6], date_str[7], date_str[8], date_str[9], '\0' };

    int day = atoi(raw_day);
    int month = atoi(raw_month);
    int year = atoi(raw_year);

    /* 3. VERIFICA SEMANTICA DEI RANGE CALENDARICI */
    if (year < 1900 || year > 2100) return false; /* Vincolo sugli anni accettati dal sistema comunale */
    if (month < 1 || month > 12) return false;    /* Un anno può contenere solo da 1 a 12 mesi */
    if (day < 1 || day > 31) return false;        /* Nessun mese può eccedere i 31 giorni */

    /* 4. VERIFICA CHIRURGICA DEI GIORNI SPECIFICI PER CIASCUN MESE (ESCLUSO ANNO BISESTILE PER SEMPLICITÀ) */
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        if (day > 30) return false; /* Aprile, Giugno, Settembre e Novembre hanno stabilmente 30 giorni */
    }
    if (month == 2) {
        if (day > 29) return false; /* Febbraio non può superare i 29 giorni nel caso massimo */
    }

    return true; /* La stringa supera tutti i controlli formali e semantici */
}

bool validate_urgency_range(char urgency_char) {
    /* Verifica che il carattere corrisponda esattamente a uno dei tre livelli scalari previsti */
    if (urgency_char == '0' || urgency_char == '1' || urgency_char == '2') {
        return true;
    }
    return false; /* Violazione del dominio: carattere non riconosciuto */
}

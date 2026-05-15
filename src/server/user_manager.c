#include "../../include/server/user_manager.h"
#include "../../include/utils/parser.h"
#include "../../include/utils/validators.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Ottiene l'ID per il prossimo utente leggendo l'ultima riga del file.
 */
static unsigned int get_next_user_id() {
    FILE* f = fopen(PATH_USERS, "r");
    if (!f) return 1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size < USER_LINE_TOTAL) {
        fclose(f);
        return 1;
    }

    // Salta all'inizio dell'ultima riga
    fseek(f, -USER_LINE_TOTAL, SEEK_END);
    char line[USER_LINE_TOTAL + 1];
    if (fgets(line, sizeof(line), f)) {
        unsigned int last_id;
        char u[MAX_USERNAME], p[MAX_PASSWORD];
        UserRole r;
        line_to_user_data(line, &last_id, u, p, &r);
        fclose(f);
        return last_id + 1;
    }

    fclose(f);
    return 1;
}

bool is_username_taken(const char* username) {
    FILE* f = fopen(PATH_USERS, "r");
    if (!f) return false;

    char line[USER_LINE_TOTAL + 1];
    unsigned int id;
    char u[MAX_USERNAME], p[MAX_PASSWORD];
    UserRole r;

    while (fgets(line, sizeof(line), f)) {
        line_to_user_data(line, &id, u, p, &r);
        if (strcmp(u, username) == 0) {
            fclose(f);
            return true;
        }
    }

    fclose(f);
    return false;
}

User login_user(const char* username, const char* password) {
    if (!validate_not_empty(username) || !validate_not_empty(password)) return NULL;

    FILE* f = fopen(PATH_USERS, "r");
    if (!f) return NULL;

    char line[USER_LINE_TOTAL + 1];
    unsigned int id;
    char u[MAX_USERNAME], p[MAX_PASSWORD];
    UserRole r;

    while (fgets(line, sizeof(line), f)) {
        line_to_user_data(line, &id, u, p, &r);
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            fclose(f);
            return create_user(id, u, p, r);
        }
    }

    fclose(f);
    return NULL;
}

bool register_user(const char* username, const char* password, UserRole role) {
    if (!validate_alphanumeric(username) || is_username_taken(username)) return false;
    if (!validate_length(password, MAX_PASSWORD - 1)) return false;

    FILE* f = fopen(PATH_USERS, "a");
    if (!f) return false;

    unsigned int new_id = get_next_user_id();
    char line[USER_LINE_TOTAL + 1];
    
    // Trasforma i dati in riga fissa tramite parser
    user_to_line(line, new_id, username, password, role);
    
    if (fputs(line, f) == EOF) {
        fclose(f);
        return false;
    }

    fclose(f);

    // Aggiornamento indice semplificato (Append ID su users_idx.txt)
    FILE* f_idx = fopen(PATH_USERS_IDX, "a");
    if (f_idx) {
        fprintf(f_idx, "%05u\n", new_id);
        fclose(f_idx);
    }

    return true;
}

unsigned int get_user_count() {
    FILE* f = fopen(PATH_USERS, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    return (unsigned int)(size / USER_LINE_TOTAL);
}

#include "../../include/server/user_manager.h"
#include "../../include/utils/parser.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Algoritmo di hashing DJB2 (sviluppato da Daniel J. Bernstein).
 *        Mappa una stringa alfanumerica (username) in un intero unsigned long.
 *        Garantisce un'eccellente distribuzione uniforme riducendo al minimo le collisioni.
 */
static unsigned long djb2_hash(const char* str) {
    unsigned long hash = 5381; /* Valore iniziale di moltiplicazione (numero primo di partenza) */
    int c;
    /* Scorre la stringa carattere per carattere applicando lo shift bit a bit (hash * 33 + c) */
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int get_users_hash_capacity() {
    FILE* f = fopen(PATH_USERS_IDX, "rb");
    if (!f) return 0;
    int lines = 0;
    char buffer[10];
    /* Scansione sequenziale per contare il numero totale di slot allocati nell'indice utenti */
    while (fgets(buffer, sizeof(buffer), f)) {
        lines++;
    }
    fclose(f);
    return lines;
}

/**
 * @brief Espande la tabella hash allocando BLOCK_SIZE_USERS nuovi slot vuoti per scongiurare la saturazione.
 *        Scrive la stringa fissa "-1   \n" per preservare il passo di 6 byte (USER_IDX_LINE).
 */
static bool expand_users_index() {
    FILE* f = fopen(PATH_USERS_IDX, "ab");
    if (!f) return false;
    for (int i = 0; i < BLOCK_SIZE_USERS; i++) {
        fputs("-1   \n", f);
    }
    fclose(f);
    return true;
}

/**
 * @brief Calcola quanti utenti sono registrati dividendo la dimensione del file per il passo a USER_LINE_TOTAL byte.
 */
static int count_registered_users() {
    FILE* f = fopen(PATH_USERS, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    return (int)(size / USER_LINE_TOTAL);
}

bool register_user(const char* username, const char* password, UserRole role) {
    int capacity = get_users_hash_capacity();
    int size_users = count_registered_users();
    
    /* REQUISITO DI SCALABILITÀ: Se la tabella hash supera il Load Factor del 70%, innesca l'espansione */
    if (capacity == 0 || size_users >= capacity * 0.7) {
        if (!expand_users_index()) return false;
        capacity = get_users_hash_capacity(); 
    }

    /* Calcolo dello slot iniziale tramite l'operatore modulo sulla capacita attuale */
    unsigned long hash_val = djb2_hash(username);
    int start_index = hash_val % capacity;
    int current_index = start_index;
    
    FILE* f_idx = fopen(PATH_USERS_IDX, "rb+");
    if (!f_idx) return false;
    
    char idx_buffer[USER_IDX_LINE + 3];
    int assigned_data_row = -1;
    bool slot_found = false;

    /* Algoritmo di Linear Probing: scansiona i blocchi adiacenti in caso di collisione */
    for (int i = 0; i < capacity; i++) {
        /* Salto millimetrico calibrato sul passo USER_IDX_LINE (6 byte per riga) */
        fseek(f_idx, (long)current_index * USER_IDX_LINE, SEEK_SET); 
        if (!fgets(idx_buffer, sizeof(idx_buffer), f_idx)) break;
        
        int slot_value = atoi(idx_buffer);
        
        /* SCENARIO A: Lo slot e vuoto (-1). Abbiamo trovato la posizione in cui inserire */
        if (slot_value == -1) {
            assigned_data_row = size_users; 
            fseek(f_idx, (long)current_index * USER_IDX_LINE, SEEK_SET);
            /* GEOMETRIA CRITICA SANIFICATA: Scrive solo i 5 byte numerici, escludendo il newline \n.
               Questo sovrascrive il "-1   " lasciando intatto l'originale \n del file. */
            fprintf(f_idx, "%010d", assigned_data_row);
            slot_found = true;
            break;
        }
        
        /* SCENARIO B: Lo slot e occupato. Verifichiamo se l'username coincide per bloccare i duplicati */
        FILE* f_usr = fopen(PATH_USERS, "rb");
        if (f_usr) {
            char user_line[USER_LINE_TOTAL + 3];
            /* Salto geometrico nel database anagrafico a passo USER_LINE_TOTAL (USER_LINE_TOTAL byte) */
            fseek(f_usr, (long)slot_value * USER_LINE_TOTAL, SEEK_SET);
            
            if (fgets(user_line, sizeof(user_line), f_usr)) {
                int tmp_id; 
                char tmp_user[MAX_USERNAME + 1]; 
                char tmp_pass[MAX_PASSWORD + 1]; 
                UserRole tmp_role;
                
                line_to_user_data(user_line, &tmp_id, tmp_user, tmp_pass, &tmp_role);
                
                /* BLOCCO ANTI-DUPLICATI: Se l'username esiste gia nel database, abortisce in O(1) */
                if (strcmp(tmp_user, username) == 0) {
                    fclose(f_usr);
                    fclose(f_idx);
                    return false;
                }
            }
            fclose(f_usr);
        }
        
        /* Linear Probing: incremento circolare dell'indice per esaminare il blocco successivo */
        current_index = (current_index + 1) % capacity;
    }

    fclose(f_idx);
    if (!slot_found) return false;

    /* Append finale dei dati anagrafici a USER_LINE_TOTAL byte nel database principale */
    FILE* f_data = fopen(PATH_USERS, "ab");
    if (!f_data) return false;
    
    char final_line[USER_LINE_TOTAL + 3];
    user_to_line(final_line, assigned_data_row, username, password, role); 
    fputs(final_line, f_data);
    fclose(f_data);
    
    return true;
}

User login_user(const char* username, const char* password) {
    int capacity = get_users_hash_capacity();
    if (capacity == 0) return NULL;

    /* Calcolo immediato dello slot teorico in tempo costante O(1) */
    unsigned long hash_val = djb2_hash(username);
    int start_index = hash_val % capacity;
    int current_index = start_index;

    FILE* f_idx = fopen(PATH_USERS_IDX, "rb");
    if (!f_idx) return NULL;

    char idx_buffer[USER_IDX_LINE + 3];
    User authenticated_user = NULL;

    /* Scansione ad indirizzamento aperto: scorre finche non intercetta un match o una riga vuota */
    for (int i = 0; i < capacity; i++) {
        fseek(f_idx, (long)current_index * USER_IDX_LINE, SEEK_SET);
        if (!fgets(idx_buffer, sizeof(idx_buffer), f_idx)) break;

        int slot_value = atoi(idx_buffer);
        if (slot_value == -1) break; /* Se lo slot e vuoto, l'account non esiste (Fine scansione immediata) */

        FILE* f_usr = fopen(PATH_USERS, "rb");
        if (f_usr) {
            char user_line[USER_LINE_TOTAL + 3];
            fseek(f_usr, (long)slot_value * USER_LINE_TOTAL, SEEK_SET);
            
            if (fgets(user_line, sizeof(user_line), f_usr)) {
                int u_id; 
                char u_user[MAX_USERNAME + 1]; 
                char u_pass[MAX_PASSWORD + 1]; 
                UserRole u_role;
                
                line_to_user_data(user_line, &u_id, u_user, u_pass, &u_role);

                /* Se l'username coincide, verifica la password per validare la sessione */
                if (strcmp(u_user, username) == 0) {
                    if (strcmp(u_pass, password) == 0) {
                        /* Istanziazione dell'oggetto User opaco restituito come sessione client */
                        authenticated_user = create_user(slot_value, u_user, u_pass, u_role);
                    }
                    fclose(f_usr);
                    break; 
                }
            }
            fclose(f_usr);
        }
        current_index = (current_index + 1) % capacity;
    }

    fclose(f_idx);
    return authenticated_user; /* Restituisce il puntatore opaco o NULL se rifiutato */
}




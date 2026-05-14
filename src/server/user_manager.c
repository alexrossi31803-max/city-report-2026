#include "../../include/server/user_manager.h"
#include "../../include/utils/parser.h"
#include "../../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Algoritmo di hashing DJB2 per convertire l'username in un intero positivo
static unsigned long djb2_hash(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Calcola quante righe (slot) sono presenti in users_idx.txt
int get_users_hash_capacity() {
    FILE* f = fopen(PATH_USERS_IDX, "rb");
    if (!f) return 0;
    int lines = 0;
    char buffer[16];
    while (fgets(buffer, sizeof(buffer), f)) lines++;
    fclose(f);
    return lines;
}

// Espande il file indice di altri 50 slot inserendo "-1   \n" (esattamente 6 byte per riga)
static bool expand_users_index() {
    FILE* f = fopen(PATH_USERS_IDX, "ab");
    if (!f) return false;
    for (int i = 0; i < BLOCK_SIZE_USERS; i++) {
        fputs("-1   \n", f);
    }
    fclose(f);
    return true;
}

// Calcola istantaneamente in O(1) quanti utenti stabili sono registrati sfruttando la dimensione del file
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
    
    // Controllo del fattore di carico (70%) per l'espansione dinamica dell'indice
    if (size_users >= capacity * 0.7) {
        if (!expand_users_index()) return false;
        capacity = get_users_hash_capacity(); 
    }

    unsigned long hash_val = djb2_hash(username);
    int start_index = hash_val % capacity;
    int current_index = start_index;
    
    FILE* f_idx = fopen(PATH_USERS_IDX, "rb+");
    if (!f_idx) return false;
    
    char idx_buffer[16];
    int riga_dati_assegnata = -1;
    bool slot_trovato = false;

    // Linear Probing sul file indice (Ogni slot occupa stabilmente 6 byte: 5 cifre + \n)
    for (int i = 0; i < capacity; i++) {
        fseek(f_idx, current_index * 6, SEEK_SET); 
        if (!fgets(idx_buffer, sizeof(idx_buffer), f_idx)) break;
        
        int valore_slot = atoi(idx_buffer);
        
        // Slot vuoto trovato: procediamo alla registrazione
        if (valore_slot == -1) {
            riga_dati_assegnata = size_users; 
            fseek(f_idx, current_index * 6, SEEK_SET);
            fprintf(f_idx, "%05d\n", riga_dati_assegnata);
            slot_trovato = true;
            break;
        }
        
        // INTERCETTAZIONE DUPLICATI IN O(1): Salto mirato sul file dati senza scansione lineare
        FILE* f_usr = fopen(PATH_USERS, "rb");
        if (f_usr) {
            char user_line[USER_LINE_TOTAL + 3];
            fseek(f_usr, valore_slot * USER_LINE_TOTAL, SEEK_SET);
            
            if (fgets(user_line, sizeof(user_line), f_usr)) {
                int tmp_id; 
                char tmp_user[MAX_USERNAME + 1]; 
                char tmp_pass[MAX_PASSWORD + 1]; 
                UserRole tmp_role;
                
                line_to_user_data(user_line, &tmp_id, tmp_user, tmp_pass, &tmp_role);
                
                if (strcmp(tmp_user, username) == 0) {
                    // Duplicato rilevato immediatamente
                    fclose(f_usr);
                    fclose(f_idx);
                    return false;
                }
            }
            fclose(f_usr);
        }
        
        current_index = (current_index + 1) % capacity;
    }

    fclose(f_idx);
    if (!slot_trovato) return false;

    // Scrittura in Append del nuovo record anagrafico a riga fissa
    FILE* f_data = fopen(PATH_USERS, "ab");
    if (!f_data) return false;
    
    char final_line[USER_LINE_TOTAL + 3];
    user_to_line(final_line, riga_dati_assegnata + 1, username, password, role); 
    fputs(final_line, f_data);
    fclose(f_data);
    
    return true;
}

User login_user(const char* username, const char* password) {
    int capacity = get_users_hash_capacity();
    if (capacity == 0) return NULL;

    unsigned long hash_val = djb2_hash(username);
    int start_index = hash_val % capacity;
    int current_index = start_index;

    FILE* f_idx = fopen(PATH_USERS_IDX, "rb");
    if (!f_idx) return NULL;

    char idx_buffer[16];
    User utente_autenticato = NULL;

    // Ricerca in Linear Probing dell'username inserito
    for (int i = 0; i < capacity; i++) {
        fseek(f_idx, current_index * 6, SEEK_SET);
        if (!fgets(idx_buffer, sizeof(idx_buffer), f_idx)) break;

        int valore_slot = atoi(idx_buffer);
        if (valore_slot == -1) break; // Slot vuoto: l'utente non esiste nel sistema

        // AUTENTICAZIONE ISTANTANEA IN O(1): Salto diretto alla riga calcolata
        FILE* f_usr = fopen(PATH_USERS, "rb");
        if (f_usr) {
            char user_line[USER_LINE_TOTAL + 3];
            fseek(f_usr, valore_slot * USER_LINE_TOTAL, SEEK_SET);
            
            if (fgets(user_line, sizeof(user_line), f_usr)) {
                int u_id; 
                char u_user[MAX_USERNAME + 1]; 
                char u_pass[MAX_PASSWORD + 1]; 
                UserRole u_role;
                
                line_to_user_data(user_line, &u_id, u_user, u_pass, &u_role);

                if (strcmp(u_user, username) == 0) {
                    if (strcmp(u_pass, password) == 0) {
                        utente_autenticato = create_user(u_id, u_user, u_pass, u_role);
                    }
                    fclose(f_usr);
                    break; // Utente trovato (credenziali verificate o errate, interrompe la ricerca)
                }
            }
            fclose(f_usr);
        }
        current_index = (current_index + 1) % capacity;
    }

    fclose(f_idx);
    return utente_autenticato;
}

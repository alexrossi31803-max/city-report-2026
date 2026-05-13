#include "../../include/server/user_manager.h"
#include "../../include/utils/parser.h"
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
    FILE* f = fopen(PATH_USERS_IDX, "r");
    if (!f) return 0;
    int lines = 0;
    char buffer[16];
    while (fgets(buffer, sizeof(buffer), f)) lines++;
    fclose(f);
    return lines;
}

// Funzione interna per espandere il file indice di altri 50 slot inserendo "-1   \n"
static bool expand_users_index() {  //int current_capacity
    FILE* f = fopen(PATH_USERS_IDX, "a");
    if (!f) return false;
    for (int i = 0; i < BLOCK_SIZE_USERS; i++) {
        fputs("-1   \n", f);
    }
    fclose(f);
    return true;
}

// Conta quanti record reali sono presenti nel file users.txt per determinare il prossimo ID progressivo
static int count_registered_users() {
    FILE* f = fopen(PATH_USERS, "r");
    if (!f) return 0;
    int count = 0;
    char line[128];
    while (fgets(line, sizeof(line), f)) count++;
    fclose(f);
    return count;
}
/*
bool register_user(const char* username, const char* password, UserRole role) {
    int capacity = get_users_hash_capacity();
    int size_users = count_registered_users();
    
    // Se la tabella hash supera la soglia critica di riempimento, espandiamo a blocchi di 50
    if (size_users >= capacity * 0.7) {
        if (!expand_users_index()) return false;
        capacity = get_users_hash_capacity(); 
    }

    // Calcolo dell'indice hash iniziale tramite algoritmo DJB2
    unsigned long hash_val = djb2_hash(username);
    int start_index = hash_val % capacity;
    int current_index = start_index;
    
    FILE* f_idx = fopen(PATH_USERS_IDX, "r+");
    if (!f_idx) return false;
    
    char idx_buffer[16];
    int riga_dati_assegnata = -1;
    bool slot_trovato = false;

    // Linear Probing sul file indice per allocare lo slot o intercettare duplicati
    for (int i = 0; i < capacity; i++) {
        fseek(f_idx, current_index * 6, SEEK_SET); 
        if (!fgets(idx_buffer, sizeof(idx_buffer), f_idx)) break;
        
        int valore_slot = atoi(idx_buffer);
        if (valore_slot == -1) {
            // Slot libero individuato con successo
            riga_dati_assegnata = size_users; 
            fseek(f_idx, current_index * 6, SEEK_SET);
            fprintf(f_idx, "%05d\n", riga_dati_assegnata);
            slot_trovato = true;
            break;
        }
        
        // Controllo anti-duplicazione: estrazione e confronto accurato dell'username
        FILE* f_usr = fopen(PATH_USERS, "r");
        if (f_usr) {
            char user_line[128];
            fseek(f_usr, valore_slot * 107, SEEK_SET);
            if (fgets(user_line, sizeof(user_line), f_usr)) {
                int tmp_id; 
                char tmp_user[MAX_USERNAME + 1]; 
                char tmp_pass[MAX_PASSWORD + 1]; 
                UserRole tmp_role;
                
                // Inizializzazione a zero della memoria per evitare falsi negativi in strcmp
                memset(tmp_user, 0, sizeof(tmp_user));
                memset(tmp_pass, 0, sizeof(tmp_pass));
                
                line_to_user_data(user_line, &tmp_id, tmp_user, tmp_pass, &tmp_role);
                if (strcmp(tmp_user, username) == 0) {
                    // Username identico rilevato, interrompiamo bloccando il duplicato
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

    // Append finale del nuovo record anagrafico a riga fissa in users.txt
    FILE* f_data = fopen(PATH_USERS, "a");
    if (!f_data) return false;
    
    char final_line[128];
    user_to_line(final_line, riga_dati_assegnata + 1, username, password, role); 
    fputs(final_line, f_data);
    fclose(f_data);
    
    return true;
}
*/
bool register_user(const char* username, const char* password, UserRole role) {
    int capacity = get_users_hash_capacity();
    int size_users = count_registered_users();
    
    if (size_users >= capacity * 0.7) {
        if (!expand_users_index()) return false;
        capacity = get_users_hash_capacity(); 
    }

    unsigned long hash_val = djb2_hash(username);
    int start_index = hash_val % capacity;
    int current_index = start_index;
    
    FILE* f_idx = fopen(PATH_USERS_IDX, "r+");
    if (!f_idx) return false;
    
    char idx_buffer[16];
    int riga_dati_assegnata = -1;
    bool slot_trovato = false;

    // Linear Probing sul file indice
    for (int i = 0; i < capacity; i++) {
        fseek(f_idx, current_index * 6, SEEK_SET); 
        if (!fgets(idx_buffer, sizeof(idx_buffer), f_idx)) break;
        
        int valore_slot = atoi(idx_buffer);
        if (valore_slot == -1) {
            riga_dati_assegnata = size_users; 
            fseek(f_idx, current_index * 6, SEEK_SET);
            fprintf(f_idx, "%05d\n", riga_dati_assegnata);
            slot_trovato = true;
            break;
        }
        
        // Controllo anti-duplicazione tramite lettura sequenziale sicura per riga
        FILE* f_usr = fopen(PATH_USERS, "r");
        if (f_usr) {
            char user_line[150];
            int riga_corrente = 0;
            bool riga_letta = false;
            
            // Scorre il file riga per riga fino a raggiungere la posizione esatta dello slot
            while (fgets(user_line, sizeof(user_line), f_usr)) {
                if (riga_corrente == valore_slot) {
                    riga_letta = true;
                    break;
                }
                riga_corrente++;
            }
            fclose(f_usr);
            
            if (riga_letta) {
                int tmp_id; 
                char tmp_user[MAX_USERNAME + 1]; 
                char tmp_pass[MAX_PASSWORD + 1]; 
                UserRole tmp_role;
                
                memset(tmp_user, 0, sizeof(tmp_user));
                memset(tmp_pass, 0, sizeof(tmp_pass));
                
                line_to_user_data(user_line, &tmp_id, tmp_user, tmp_pass, &tmp_role);
                
                if (strcmp(tmp_user, username) == 0) {
                    // Duplicato intercettato con certezza matematica
                    fclose(f_idx);
                    return false;
                }
            }
        }
        
        current_index = (current_index + 1) % capacity;
    }

    fclose(f_idx);
    if (!slot_trovato) return false;

    FILE* f_data = fopen(PATH_USERS, "a");
    if (!f_data) return false;
    
    char final_line[150];
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

    FILE* f_idx = fopen(PATH_USERS_IDX, "r");
    if (!f_idx) return NULL;

    char idx_buffer[16];
    User utente_autenticato = NULL;

    for (int i = 0; i < capacity; i++) {
        fseek(f_idx, current_index * 6, SEEK_SET);
        if (!fgets(idx_buffer, sizeof(idx_buffer), f_idx)) break;

        int valore_slot = atoi(idx_buffer);
        if (valore_slot == -1) break;

        FILE* f_usr = fopen(PATH_USERS, "r");
        if (f_usr) {
            char user_line[150];
            int riga_corrente = 0;
            bool riga_letta = false;
            
            while (fgets(user_line, sizeof(user_line), f_usr)) {
                if (riga_corrente == valore_slot) {
                    riga_letta = true;
                    break;
                }
                riga_corrente++;
            }
            fclose(f_usr);

            if (riga_letta) {
                int u_id; char u_user[MAX_USERNAME + 1] = {0}; char u_pass[MAX_PASSWORD + 1] = {0}; UserRole u_role;
                line_to_user_data(user_line, &u_id, u_user, u_pass, &u_role);

                if (strcmp(u_user, username) == 0) {
                    if (strcmp(u_pass, password) == 0) {
                        utente_autenticato = create_user(u_id, u_user, u_pass, u_role);
                    }
                    break; 
                }
            }
        }
        current_index = (current_index + 1) % capacity;
    }

    fclose(f_idx);
    return utente_autenticato;
}


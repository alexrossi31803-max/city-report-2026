#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "../models/user.h"
#include <stdbool.h>

#define PATH_USERS "database/Master_Files/users.txt"
#define PATH_USERS_IDX "database/Master_Files/users_idx.txt"
#define BLOCK_SIZE_USERS 50

// Restituisce la capacità attuale della tabella hash (leggendo il numero di righe di users_idx.txt)
int get_users_hash_capacity();

// Registra un nuovo utente nel sistema gestendo l'espansione dinamica dell'indice
bool register_user(const char* username, const char* password, UserRole role);

// Esegue il login O(1). Se le credenziali sono corrette, alloca e restituisce l'oggetto User caricato dal file
User login_user(const char* username, const char* password);

#endif

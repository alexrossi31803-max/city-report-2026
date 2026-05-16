#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "../models/user.h"
#include <stdbool.h>

// Restituisce la capacità attuale della tabella hash leggendo il file indice
int get_users_hash_capacity();

// Registra un nuovo utente nel sistema gestendo il linear probing e l'espansione
bool register_user(const char* username, const char* password, UserRole role);

// Esegue l'autenticazione istantanea in O(1) caricando l'oggetto User
User login_user(const char* username, const char* password);

#endif


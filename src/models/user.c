#include "../../include/models/user.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct User {
    int id;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    UserRole role;
};

User create_user(int id, const char* username, const char* password, UserRole role) {
    User u = (User)malloc(sizeof(struct User));
    if (u == NULL) return NULL;

    u->id = id;
    
    // Inizializzazione sicura con azzeramento della memoria
    memset(u->username, 0, MAX_USERNAME);
    memset(u->password, 0, MAX_PASSWORD);

    strncpy(u->username, username, MAX_USERNAME - 1);
    strncpy(u->password, password, MAX_PASSWORD - 1);
    
    u->role = role;

    return u;
}

void free_user(User u) {
    if (u != NULL) {
        free(u);
    }
}

const char* get_role_string(UserRole role) {
    if (role == EMPLOYEE) return "Dipendente Comunale";
    return "Cittadino";
}

// Implementazione dei Getters
int get_user_id(User u) {
    if (u == NULL) return -1;
    return u->id;
}

const char* get_user_username(User u) {
    if (u == NULL) return "";
    return u->username;
}

const char* get_user_password(User u) {
    if (u == NULL) return "";
    return u->password;
}

UserRole get_user_role(User u) {
    if (u == NULL) return CITIZEN; 
    return u->role;
}

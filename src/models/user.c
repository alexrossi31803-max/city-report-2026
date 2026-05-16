#include "../../include/models/user.h"
#include <stdlib.h>
#include <string.h>

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
    
    memset(u->username, 0, MAX_USERNAME);
    memset(u->password, 0, MAX_PASSWORD);

    if (username) strncpy(u->username, username, MAX_USERNAME - 1);
    if (password) strncpy(u->password, password, MAX_PASSWORD - 1);
    
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

int get_user_id(User User) {
    if (User == NULL) return -1;
    return User->id;
}

const char* get_user_username(User User) {
    if (User == NULL) return "";
    return User->username;
}

const char* get_user_password(User User) {
    if (User == NULL) return "";
    return User->password;
}

UserRole get_user_role(User User) {
    if (User == NULL) return CITIZEN; 
    return User->role;
}


#include "../../include/models/user.h"
#include <stdlib.h>
#include <string.h>

struct User {
    unsigned int id;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    UserRole role;
};

User create_user(unsigned int id, const char* username, const char* password, UserRole role) {
    User u = (User)malloc(sizeof(struct User));
    if (!u) return NULL;

    u->id = id;
    u->role = role;
    
    strncpy(u->username, username, MAX_USERNAME - 1);
    u->username[MAX_USERNAME - 1] = '\0';
    
    strncpy(u->password, password, MAX_PASSWORD - 1);
    u->password[MAX_PASSWORD - 1] = '\0';

    return u;
}

void free_user(User u) {
    if (u) free(u);
}

unsigned int get_user_id(User u) { return u->id; }
const char* get_user_username(User u) { return u->username; }
const char* get_user_password(User u) { return u->password; }
UserRole get_user_role(User u) { return u->role; }
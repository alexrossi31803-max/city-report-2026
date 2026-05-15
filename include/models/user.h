#ifndef USER_H
#define USER_H

#include "../config.h"

typedef struct User* User;

User create_user(unsigned int id, const char* username, const char* password, UserRole role);
void free_user(User u);

/* Getters */
unsigned int get_user_id(User u);
const char* get_user_username(User u);
const char* get_user_password(User u);
UserRole get_user_role(User u);

#endif
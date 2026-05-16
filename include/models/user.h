#ifndef USER_H
#define USER_H

#include "../config.h"

typedef struct User* User;

User create_user(int id, const char* username, const char* password, UserRole role);
void free_user(User u);

const char* get_role_string(UserRole role);

int get_user_id(User u); 
const char* get_user_username(User u);
const char* get_user_password(User u);
UserRole get_user_role(User u);

#endif


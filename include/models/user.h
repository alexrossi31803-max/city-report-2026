#ifndef USER_H
#define USER_H

#include "../config.h"

/*
The internal structure is hidden to enforce information hiding.
*/
typedef struct User* User;
//USER OPERATIONS 

/*
create_user -> It creates a new user 
Preconditions: 
- id must be valid (> 0)
- username and password must not be NULL

Postconditions:
- returns initialized User structure address
*/

User create_user(int id, const char* username, const char* password, UserRole role);

/*
authenticate_user -> It checks user credentials

Preconditions:
- users array must be valid
- size must be > 0

Postconditions:
- returns index of user if found
- returns -1 if authentication fails
*/
int authenticate_user(const char* username, const char* password, User user, int size);

#endif
/*
STRUCTURE USER -> This ADT represents a system user (Citizen or Employee) 
and encapsulates authentication data and role management
*/
struct User {
    int id;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    UserRole role;
};
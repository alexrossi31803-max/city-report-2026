#ifndef AUTH_H
#define AUTH_H

#include "../models/user.h"

/*
AUTH_H - Authentication Module 

This module handles user authentication within the system
*/
/*
login -> Prompts the user for credentials, compares username 
and password with stored users, and return the authenticated user if valid
Preconditions:
 - size >0;
 - users -> not NULL; 
Postconditions:
 - if authentication fails return NULL 
 - if authentication succeeds returns a pointer to a valid User struct 
   representing the logged-in user
Side effects: 
 - Reads from stdin
Complexity: O(n) in the worst case due to linear search over users 
 */
User login(User user, int size);

#endif
#ifndef VALIDATORS_H
#define VALIDATORS_H

/*
  VALIDATORS ADT - Input Validation Module
 
  This module is responsible for validating user input across the system.
  It ensures that data received from the user or external sources
  respects the required constraints before being processed by the system.
 
  This layer helps maintain data integrity and prevents invalid states
  inside the application.
 */

#include <stdbool.h>

/*
  validate_id -> Checks whether the provided ID is valid according to system rules
  Preconditions : id must be an integer value provided by user or system
  Postconditions: returns true if id is valid (positive and non-zero)
                   returns false otherwise
 */
bool validate_id(int id);

/*
validate_string -> Ensures the string is not NULL and not empty.
  Preconditions:
  str must be a valid pointer or NULL is allowed but treated as invalid
  Postconditions:
  returns true if string is not NULL and not empty
  returns false otherwise
 */
bool validate_string(const char* str);

/*
  validate_urgency -> Validate urgency level
 
  Preconditions:
  level must be an integer value provided by user input
  Postconditions:
  returns true if level is within allowed range
  returns false otherwise
 */
bool validate_urgency(int level);

#endif
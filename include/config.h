#ifndef CONFIG_H
#define CONFIG_H

/*
CONFIGURATION FILE 

Contains global constants and shared types
used across the entire system.

This file centralizes configuration to
ensure consistency and maintainability.
*/

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_NAME 50
#define MAX_DESC 256
/*
REPORT STATUS -> Represents the lyfecycle of a report 
*/
typedef enum {
    OPEN,
    IN_PROGRESS,
    CLOSED
} ReportStatus;

/*
USER ROLE -> Defines system access level
*/
typedef enum {
    CITIZEN,
    EMPLOYEE
} UserRole;

/*
REPORT CATEGORY -> Defines type of urban issue reported
*/
typedef enum {
    ROAD,
    LIGHTING,
    WASTE,
    INFRASTRUCTURE,
    OTHER
} ReportCategory;

#endif
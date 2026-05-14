#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_NAME 50
#define MAX_DESC 256

/* Limiti Operativi del Server */
#define LIMIT_BENCH 50
#define SOGLIA_FLUSH 50
#define BLOCK_SIZE_USERS 50

/* Geometrie di Riga Fisse (Testo + Flag Cella Spaziale + Newline) */
#define USER_LINE_TOTAL 107   // 5(ID) + 50(User) + 50(Pass) + 1(Role) + 1(\n)
#define REPORT_LINE_TOTAL 332 // 5(ID) + 5(C_ID) + 2(Cat) + 1(Urg) + 1(Stat) + 10(Date) + 50(Name) + 256(Desc) + 1(Flag) + 1(\n)

/* Mappa dei Percorsi Database binarizzati */
#define PATH_USERS "database/Master_Files/users.txt"
#define PATH_USERS_IDX "database/Master_Files/users_idx.txt"
#define PATH_BENCH "database/Derived_Files/reports_bench.txt"
#define PATH_OPEN_MASTER "database/Master_Files/open_reports.txt"
#define PATH_PROGRESS_MASTER "database/Master_Files/in_progress_reports.txt"
#define PATH_CLOSED_MASTER "database/Master_Files/closed_reports.txt"
#define PATH_BST_REPORT_ID "database/Derived_Files/report_BST_BY_REPORT_ID.txt"
#define PATH_BST_USER_ID "database/Derived_Files/report_BST_BY_USER_ID.txt"
#define PATH_PRIORITY_FILE "database/Derived_Files/reports_by_priority.txt"
#define PATH_SEQUENCE "database/Master_Files/system_total_report.txt"

typedef enum {
    OPEN,
    IN_PROGRESS,
    CLOSED
} ReportStatus;

typedef enum {
    CITIZEN,
    EMPLOYEE
} UserRole;

typedef enum {
    ROAD,
    LIGHTING,
    WASTE,
    INFRASTRUCTURE,
    OTHER
} ReportCategory;

#endif


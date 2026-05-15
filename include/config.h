#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/* --- Limiti Geometrici e Operativi --- */
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_NAME 50
#define MAX_DESC 256
#define LIMIT_BENCH 50
#define SOGLIA_FLUSH 50

/* * Geometria fissa di riga (350 byte dati + 1 byte \n = 351 totali)
 * ID(10)+UID(10)+NAME(50)+CAT(1)+DESC(256)+DATE(11)+URG(1)+STAT(1)+ROW(10)+CELL(1)
 */
#define REPORT_LINE_DATA 350
#define REPORT_LINE_TOTAL 351

/* Geometria riga Utente (es. 107 byte) */
#define USER_LINE_TOTAL 107

/* --- Definizioni enumerative per la logica di business --- */
typedef enum { ROAD = 'R', LIGHTING = 'L', WASTE = 'W', INFRASTRUCTURE = 'I', OTHER = 'O' } ReportCategory;
typedef enum { OPEN = 'O', IN_PROGRESS = 'P', CLOSED = 'C', DESTROYED = 'D' } ReportStatus;
typedef enum { CITIZEN = 'C', EMPLOYEE = 'E' } UserRole;

/* ========================================================================== */
/* MAPPA PERCORSI (TREE STRUCTURE)                       */
/* ========================================================================== */

/* --- Master Files (Database Centrale) --- */
#define PATH_USERS              "database/Master_Files/users.txt"
#define PATH_USERS_IDX          "database/Master_Files/users_idx.txt"
#define PATH_SYS_STATS          "database/Master_Files/system_total_report.txt"

/* File Master divisi per stato */
#define PATH_OPEN               "database/Master_Files/open_latest.txt"
#define PATH_PROGRESS           "database/Master_Files/in_progress_latest.txt"
#define PATH_CLOSED             "database/Master_Files/closed_latest.txt"

/* Gestione Buchi (LIFO) */
#define HOLES_OPEN              "database/Master_Files/open_null_pointer.txt"
#define HOLES_PROGRESS          "database/Master_Files/in_progress_null_pointer.txt"
#define HOLES_CLOSED            "database/Master_Files/closed_null_pointer.txt"

/* --- Derived Files (Indici e Cache) --- */
#define PATH_BENCH              "database/Derived_Files/reports_bench"
#define PATH_PRIORITY           "database/Derived_Files/reports_by_priority.txt"
#define PATH_AVL_ID             "database/Derived_Files/report_AVL_BY_REPORT_ID.txt"
#define PATH_AVL_USER           "database/Derived_Files/report_AVL_BY_USER_ID.txt"

#endif


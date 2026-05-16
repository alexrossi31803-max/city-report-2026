#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/* Limiti dimensionali stringhe */
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_NAME 50
#define MAX_DESC 256

/* Limiti Operativi della Cache Operativa (Server) */
#define LIMIT_BENCH 50
#define SOGLIA_FLUSH 50
#define BLOCK_SIZE_USERS 50

/* 
   Geometrie di Riga Fisse (Testo + Flag Cella Spaziale + Newline)
   - USER_LINE_TOTAL: 5(ID) + 50(User) + 50(Pass) + 1(Role) + 1(\n) = 107 byte
   - REPORT_LINE_TOTAL: 10(ID) + 10(USER_ID) + 50(Name) + 1(Cat) + 256(Desc) + 11(Date) + 1(Urg) + 1(Stat) + 10(Row) + 1(FlagCella) + 1(\n) = 352 byte
   - SYSTEM_REG_LINE: Ogni riga del registro centrale occupa stabilmente 11 byte: %010u\n
*/
#define USER_LINE_TOTAL 107   
#define REPORT_LINE_TOTAL 352 
#define SYSTEM_REG_LINE 11    

/* Mappa dei Percorsi Database Binarizzati */
#define PATH_USERS "database/Master_Files/users.txt"
#define PATH_USERS_IDX "database/Master_Files/users_idx.txt"
#define PATH_BENCH "database/Derived_Files/reports_bench.txt"
#define PATH_OPEN_MASTER "database/Master_Files/open_reports.txt"
#define PATH_PROGRESS_MASTER "database/Master_Files/in_progress_reports.txt"
#define PATH_CLOSED_MASTER "database/Master_Files/closed_reports.txt"

/* Stack LIFO dei Buchi per il Riciclo Geometrico O(1) */
#define PATH_OPEN_HOLES "database/Master_Files/open_holes.txt"
#define PATH_PROGRESS_HOLES "database/Master_Files/in_progress_holes.txt"
#define PATH_CLOSED_HOLES "database/Master_Files/closed_holes.txt"

/* Nuovi Percorsi Indici Strutturati ad Albero Bilanciato AVL e Code */
#define PATH_AVL_REPORT_ID "database/Derived_Files/report_AVL_BY_REPORT_ID.txt"
#define PATH_AVL_USER_ID "database/Derived_Files/report_AVL_BY_USER_ID.txt"
#define PATH_PRIORITY_FILE "database/Derived_Files/reports_by_priority.txt"
#define PATH_SEQUENCE "database/Master_Files/system_total_report.txt"

/* Mappatura Posizionale dei Contatori Statistici in system_total_report.txt */
#define REG_IDX_GLOBAL_ID        0  /* ID incrementale globale dei report */
#define REG_IDX_COUNTER_BENCH    1  /* Contatore corrente elementi in bench */
#define REG_IDX_NM_REPORT        2  /* Numero totale di report attivi nel sistema */
#define REG_IDX_STAT_OPEN        3  /* Totale pratiche in stato OPEN */
#define REG_IDX_STAT_PROGRESS    4  /* Totale pratiche in stato IN_PROGRESS */
#define REG_IDX_STAT_CLOSED      5  /* Totale pratiche in stato CLOSED */
#define REG_IDX_CAT_ROAD         6  /* Totale anomalie stradali */
#define REG_IDX_CAT_LIGHTING     7  /* Totale anomalie illuminazione */
#define REG_IDX_CAT_WASTE        8  /* Totale anomalie rifiuti */
#define REG_IDX_CAT_INFRASTRUCT  9  /* Totale anomalie impianti pubblici */
#define REG_IDX_CAT_OTHER        10 /* Totale anomalie generiche */

typedef enum {
    OPEN,
    IN_PROGRESS,
    CLOSED,
    DESTROYED
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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/* Limiti dimensionali stringhe modificabili */
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_DESC 256

/* Limiti Operativi della Cache Operativa (Server) modificabili  */
#define LIMIT_BENCH 2
#define BLOCK_SIZE_USERS 50

/* 
   Geometrie di Riga Fisse (Testo + Flag Cella Spaziale + Newline) non modificabili 
   - USER_LINE_TOTAL: 10(USER_ID) + MAX_USERNAME + MAX_PASSWORD + 1(Role) + 1(\n) byte 
   - USER_IDX_LINE: Ogni riga dell'indice utenti occupa stabilmente 6 byte
   - REPORT_BENCH_LINE: 10(REPORT_ID)+ 10(USER_ID) + MAX_USERNAME + 1(Report_category) + MAX_DESC + 10(Report_date) + 1 (Report_urgency)+ 1(Report_status) + 10(disk_row) + 1 byte di newline byte
   - REPORT_MASTER_LINE: 10(REPORT_ID)+ 10(USER_ID) + MAX_USERNAME + 1(Report_category) + MAX_DESC + 10(Report_date) + 1 (Report_urgency)+ 1(Report_status) + 10(disk_row) + 1(flag_cella)+ 1 byte di newline byte
   - SYSTEM_REG_LINE: Ogni riga del registro dei buchi (%010u\n) = 11 byte
   - AVL_REPORT_ID_LINE: 10(REPORT_ID) + 1(status) + 10(disk row) + 1(\n) = 22 byte
   - AVL_USER_ID_LINE: 10(USER_ID) + 10(REPORT_ID) + 1(\n) = 21 byte
*/
#define USER_LINE_TOTAL (10 + MAX_USERNAME + MAX_PASSWORD + 1 + 1)
#define USER_IDX_LINE 6
#define REPORT_BENCH_LINE (10 + 10 + MAX_USERNAME + 1 + MAX_DESC + 10 + 1 + 1 + 10 + 1)
#define REPORT_MASTER_LINE (10 + 10 + MAX_USERNAME + 1 + MAX_DESC + 10 + 1 + 1 + 10 + 1 + 1)
#define SYSTEM_REG_LINE 11
#define AVL_REPORT_ID_LINE 22
#define AVL_USER_ID_LINE 21

/* Mappa dei Percorsi Database Binarizzati non modificabili */
#define PATH_USERS "database/Master_Files/users.txt"
#define PATH_USERS_IDX "database/Master_Files/users_idx.txt"
#define PATH_BENCH "database/Derived_Files/reports_bench.txt"
#define PATH_OPEN_MASTER "database/Master_Files/open_reports.txt"
#define PATH_PROGRESS_MASTER "database/Master_Files/in_progress_reports.txt"
#define PATH_CLOSED_MASTER "database/Master_Files/closed_reports.txt"

/* Stack LIFO dei Buchi per il Riciclo Geometrico O(1) non modificabili */
#define PATH_OPEN_HOLES "database/Master_Files/open_holes.txt"
#define PATH_PROGRESS_HOLES "database/Master_Files/in_progress_holes.txt"
#define PATH_CLOSED_HOLES "database/Master_Files/closed_holes.txt"

/* Nuovi Percorsi Indici Strutturati ad Albero Bilanciato AVL e Code non modificabili*/
#define PATH_AVL_REPORT_ID "database/Derived_Files/report_AVL_BY_REPORT_ID.txt"
#define PATH_AVL_USER_ID "database/Derived_Files/report_AVL_BY_USER_ID.txt"
#define PATH_PRIORITY_FILE "database/Derived_Files/reports_by_priority.txt"
/* NOTA NOTIFICA: Percorso non utilizzato per la persistenza su disco nella v5.1. 
   La Coda a Priorita viene compilata dinamicamente "On-Demand" in RAM all'atto 
   della richiesta del Dipendente per azzerare l'I/O ridondante. Preservato 
   esclusivamente per scalabilita e modularita futura del sistema. */
#define PATH_SEQUENCE "database/Master_Files/system_total_report.txt"

/* Mappatura Posizionale dei Contatori Statistici in system_total_report.txt non modificabili*/
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
#define REG_IDX_AVL_REP_COUNT    11 /* Numero di nodi/righe in AVL Report ID */
#define REG_IDX_AVL_USR_COUNT    12 /* Numero di nodi/righe in AVL User ID */
/*SYSTEM_REG_COUNT: 13 linee del file system_total_report.txt non modificabile*/
#define SYSTEM_REG_COUNT 13
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


#ifndef REPORT_H
#define REPORT_H
#include "../config.h"
/*
The internal structure is hidden to enforce information hiding.
*/
typedef struct Report* Report; 
//REPORT OPERATIONS 
/*
create_report -> It creates a new report

Preconditions:
- all string inputs must be valid (not NULL)
- id must be unique

Postconditions:
- returns initialized Report object
*/

Report create_report(int id, const char* name, ReportCategory cat,
                      const char* desc, const char* date, int urgency);
/*
update_report_status -> It updates report status

Preconditions:
- report pointer must be valid

Postconditions:
- report status is updated
*/
void update_report_status(Report r, ReportStatus new_status);

/*
get_category_string -> It returns textual representation of category.

Preconditions:
- valid category enum

Postconditions:
- returns constant string (read-only)
*/
const char* get_category_string(ReportCategory c);

/*
get_status_string -> It returns textual representation of status.

Preconditions:
- valid status enum

Postconditions:
- returns constant string (read-only)
*/
const char* get_status_string(ReportStatus s);
#endif
/*
STRUCTURE REPORT -> Represents a municipal report submitted
by a citizen. Core entity of the system.
*/
struct Report {
    int id;
    char citizen_name[MAX_NAME];
    ReportCategory category;
    char description[MAX_DESC];
    char date[11]; // dd-mm-yyyy
    int urgency;   // 1-5
    ReportStatus status;
};
#include "../../include/models/report.h"
#include <stdlib.h>
#include <string.h>

struct Report {
    unsigned int id;
    unsigned int user_id;
    char citizen_name[MAX_NAME];
    ReportCategory category;
    char description[MAX_DESC];
    char date[11];
    char urgency; 
    ReportStatus status;
    int disk_row; // Posizione nel file master (-1 se in Bench/RAM)
};

Report create_report(unsigned int id, unsigned int user_id, const char* name, 
                     ReportCategory cat, const char* desc, const char* date, 
                     char urgency, ReportStatus status) {
    Report r = (Report)malloc(sizeof(struct Report));
    if (!r) return NULL;

    r->id = id;
    r->user_id = user_id;
    r->category = cat;
    r->urgency = urgency;
    r->status = status;
    r->disk_row = -1;

    strncpy(r->citizen_name, name, MAX_NAME - 1);
    r->citizen_name[MAX_NAME - 1] = '\0';
    
    strncpy(r->description, desc, MAX_DESC - 1);
    r->description[MAX_DESC - 1] = '\0';
    
    strncpy(r->date, date, 10);
    r->date[10] = '\0';

    return r;
}

void free_report(Report r) {
    if (r) free(r);
}

/* Implementazione Getters */
unsigned int get_report_id(Report r) { return r->id; }
unsigned int get_report_user_id(Report r) { return r->user_id; }
char get_report_urgency(Report r) { return r->urgency; }
ReportStatus get_report_status(Report r) { return r->status; }
const char* get_report_citizen_name(Report r) { return r->citizen_name; }
ReportCategory get_report_category(Report r) { return r->category; }
const char* get_report_description(Report r) { return r->description; }
const char* get_report_date(Report r) { return r->date; }
int get_report_disk_row(Report r) { return r->disk_row; }

/* Implementazione Setters */
void set_report_status(Report r, ReportStatus s) { if(r) r->status = s; }
void set_report_disk_row(Report r, int row) { if(r) r->disk_row = row; }
void set_report_urgency(Report r, char new_urgency) { if(r) r->urgency = new_urgency; }

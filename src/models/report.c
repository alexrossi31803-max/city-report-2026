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
    int disk_row; 
};

Report create_report(unsigned int id, unsigned int user_id, const char* name, ReportCategory cat, const char* desc, const char* date, char urgency) {
    Report r = (Report)malloc(sizeof(struct Report));
    if (r == NULL) return NULL;

    r->id = id;
    r->user_id = user_id;
    
    memset(r->citizen_name, 0, MAX_NAME);
    memset(r->description, 0, MAX_DESC);
    memset(r->date, 0, 11);

    if (name) strncpy(r->citizen_name, name, MAX_NAME - 1);
    r->category = cat;
    if (desc) strncpy(r->description, desc, MAX_DESC - 1);
    if (date) strncpy(r->date, date, 10);
    
    r->urgency = urgency;
    r->status = OPEN; 
    r->disk_row = -1; 

    return r;
}

void update_report_status(Report r, ReportStatus new_status) {
    if (r != NULL) {
        r->status = new_status;
    }
}

void free_report(Report r) {
    if (r != NULL) {
        free(r);
    }
}

const char* get_category_string(ReportCategory c) {
    switch(c) {
        case ROAD:           return "Buca Stradale";
        case LIGHTING:       return "Illuminazione Pubblica";
        case WASTE:          return "Rifiuti Abbandonati";
        case INFRASTRUCTURE: return "Guasto Impianto";
        default:             return "Altro";
    }
}

const char* get_status_string(ReportStatus s) {
    switch(s) {
        case OPEN:        return "Aperta";
        case IN_PROGRESS: return "In Lavorazione";
        case CLOSED:      return "Chiusa";
        case DESTROYED:   return "Eliminata/Destroyed";
        default:          return "Sconosciuto";
    }
}

unsigned int get_report_id(Report r) { return r ? r->id : 0; }
unsigned int get_report_user_id(Report r) { return r ? r->user_id : 0; }
ReportStatus get_report_status(Report r) { return r ? r->status : OPEN; }
char get_report_urgency(Report r) { return r ? r->urgency : '0'; }
const char* get_report_citizen_name(Report r) { return r ? r->citizen_name : ""; }
ReportCategory get_report_category(Report r) { return r ? r->category : OTHER; }
const char* get_report_description(Report r) { return r ? r->description : ""; }
const char* get_report_date(Report r) { return r ? r->date : ""; }

int get_report_disk_row(Report r) { return r ? r->disk_row : -1; }
void set_report_disk_row(Report r, int row) { if (r) r->disk_row = row; }


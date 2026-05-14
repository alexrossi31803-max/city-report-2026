#include "../../include/models/report.h"
#include <stdlib.h>
#include <string.h>

struct Report {
    int id;
    char citizen_name[MAX_NAME];
    ReportCategory category;
    char description[MAX_DESC];
    char date[11];
    int urgency;   
    ReportStatus status;
    int disk_row; // Iniezione indice fisico di riga (-1 alla nascita)
};

Report create_report(int id, const char* name, ReportCategory cat, const char* desc, const char* date, int urgency) {
    Report r = (Report)malloc(sizeof(struct Report));
    if (r == NULL) return NULL;

    r->id = id;
    memset(r->citizen_name, 0, MAX_NAME);
    memset(r->description, 0, MAX_DESC);
    memset(r->date, 0, 11);

    strncpy(r->citizen_name, name, MAX_NAME - 1);
    r->category = cat;
    strncpy(r->description, desc, MAX_DESC - 1);
    strncpy(r->date, date, 10);
    
    r->urgency = urgency;
    r->status = OPEN; 
    r->disk_row = -1; // Flag di inizializzazione base di default

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
        default:          return "Sconosciuto";
    }
}

int get_report_id(Report r) { return r->id; }
ReportStatus get_report_status(Report r) { return r->status; }
int get_report_urgency(Report r) { return r->urgency; }
const char* get_report_citizen_name(Report r) { return r->citizen_name; }
ReportCategory get_report_category(Report r) { return r->category; }
const char* get_report_description(Report r) { return r->description; }
const char* get_report_date(Report r) { return r->date; }

int get_report_disk_row(Report r) { return r ? r->disk_row : -1; }
void set_report_disk_row(Report r, int row) { if (r) r->disk_row = row; }

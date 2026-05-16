#ifndef REPORT_H
#define REPORT_H

#include "../config.h"

typedef struct Report* Report;

/* Costruttore aggiornato con tipi senza segno e urgenza compressa a carattere */
Report create_report(unsigned int id, unsigned int user_id, const char* name, ReportCategory cat, const char* desc, const char* date, char urgency);
void update_report_status(Report r, ReportStatus new_status);
void free_report(Report r);

const char* get_category_string(ReportCategory c);
const char* get_status_string(ReportStatus s);

/* Metodi di estrazione (Getter) aggiornati con le nuove specifiche */
unsigned int get_report_id(Report r);
unsigned int get_report_user_id(Report r);
ReportStatus get_report_status(Report r);
char get_report_urgency(Report r);
const char* get_report_citizen_name(Report r);
ReportCategory get_report_category(Report r);
const char* get_report_description(Report r);
const char* get_report_date(Report r);

/* Metodi per l'indirizzamento geometrico e il riciclo dei buchi */
int get_report_disk_row(Report r);
void set_report_disk_row(Report r, int row);

#endif


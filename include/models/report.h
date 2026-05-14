#ifndef REPORT_H
#define REPORT_H

#include "../config.h"

typedef struct Report* Report;

Report create_report(int id, const char* name, ReportCategory cat, const char* desc, const char* date, int urgency);
void update_report_status(Report r, ReportStatus new_status);
void free_report(Report r);

const char* get_category_string(ReportCategory c);
const char* get_status_string(ReportStatus s);

int get_report_id(Report r);
ReportStatus get_report_status(Report r);
int get_report_urgency(Report r);
const char* get_report_citizen_name(Report r);
ReportCategory get_report_category(Report r);
const char* get_report_description(Report r);
const char* get_report_date(Report r);

/* Nuovi metodi per l'indirizzamento fisico O(1) */
int get_report_disk_row(Report r);
void set_report_disk_row(Report r, int row);

#endif


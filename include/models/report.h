#ifndef REPORT_H
#define REPORT_H

#include "../config.h"

/* Tipo opaco per garantire l'incapsulamento */
typedef struct Report* Report;

/**
 * @brief Crea un oggetto Report in RAM.
 * @param id ID univoco generato dal sistema
 * @param user_id ID dell'utente che invia la segnalazione
 * @param urgency Valore char ('1', '2', '3')
 */
Report create_report(unsigned int id, unsigned int user_id, const char* name, 
                     ReportCategory cat, const char* desc, const char* date, 
                     char urgency, ReportStatus status);

void free_report(Report r);

/* --- Getters --- */
unsigned int get_report_id(Report r);
unsigned int get_report_user_id(Report r);
char get_report_urgency(Report r);
ReportStatus get_report_status(Report r);
const char* get_report_citizen_name(Report r);
ReportCategory get_report_category(Report r);
const char* get_report_description(Report r);
const char* get_report_date(Report r);
int get_report_disk_row(Report r);

/* --- Setters --- */
void set_report_status(Report r, ReportStatus s);
void set_report_disk_row(Report r, int row);
void set_report_urgency(Report r, char new_urgency);

#endif


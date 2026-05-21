#ifndef REPORT_H
#define REPORT_H

#include "../config.h"

/**
 * @brief Struttura opaca (Opaque Pointer) per l'oggetto Report.
 *        Garantisce il rispetto del principio dell'Information Hiding oscurando 
 *        le variabili membro private ed impedendo modifiche non autorizzate dalla UI.
 */
typedef struct ReportStruct* Report;

/**
 * @brief Costruttore dell'ADT Report. Alloca la memoria RAM e valorizza i campi della segnalazione.
 * @param id Identificatore unico globale ed incrementale senza segno (unsigned int).
 * @param user_id ID numerico senza segno del cittadino titolare della segnalazione.
 * @param citizen_name Stringa dell'username del cittadino che ha inviato il problema.
 * @param category Enumerazione della classificazione dell'anomalia (ROAD, LIGHTING, ecc.).
 * @param description Stringa testuale descrittiva del problema riscontrato.
 * @param date Stringa della data di inserimento standardizzata nel formato "GG/MM/AAAA".
 * @param urgency Carattere scalare ('0'=Bassa, '1'=Media, '2'=Alta) che indica la gravita.
 * @return Un'istanza valida di tipo Report allocata in RAM, o NULL in caso di fallimento della malloc.
 */
Report create_report(
    unsigned int id, 
    unsigned int user_id, 
    const char* citizen_name, 
    ReportCategory category, 
    const char* description, 
    const char* date, 
    char urgency
);

/**
 * @brief Distruttore dell'ADT Report. Libera in modo sicuro e pulito la memoria RAM occupata.
 * @param r Il puntatore all'istanza dell'oggetto Report da distruggere.
 */
void free_report(Report r);

/* --------------------------------==============================================
 *  METODI GETTER (PROTEZIONE INCAPSULAMENTO IN SOLA LETTURA)
 * --------------------------------============================================== */

unsigned int get_report_id(Report r);
unsigned int get_report_user_id(Report r);
const char* get_report_citizen_name(Report r);
ReportCategory get_report_category(Report r);
const char* get_report_description(Report r);
const char* get_report_date(Report r);
char get_report_urgency(Report r);
ReportStatus get_report_status(Report r);
int get_report_disk_row(Report r);

/* --------------------------------==============================================
 *  METODI SETTER (MUTATORI CONTROLLATI INTERNI PER IL SERVER)
 * --------------------------------============================================== */

/**
 * @brief Modifica lo stato interno della segnalazione (OPEN, IN_PROGRESS, CLOSED, DESTROYED).
 * @param r L'istanza dell'oggetto Report.
 * @param new_status Il nuovo valore enumerato dello stato da assegnare.
 */
void update_report_status(Report r, ReportStatus new_status);

/**
 * @brief Inietta o aggiorna l'indice di riga fisica occupato dal report nei file Master.
 * @param r L'istanza dell'oggetto Report.
 * @param row L'indice numerico intero della riga. Impostato a -1 se volatile (RAM o BENCH).
 */
void set_report_disk_row(Report r, int row);

/* --------------------------------==============================================
 *  METODI DI CONVERSIONE AGGIUNTIVI (STRING TRANSLATIONS)
 * --------------------------------============================================== */

const char* get_category_string(ReportCategory category);
const char* get_status_string(ReportStatus status);

#endif



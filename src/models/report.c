#include "../../include/models/report.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Struttura interna dell'oggetto Report (Definizione fisica nascosta all'esterno).
 *        L'allocazione statica interna degli array di caratteri previene la frammentazione 
 *        della RAM ed evita la gestione onerosa di puntatori multipli.
 */
struct ReportStruct {
    unsigned int id;                  /* ID unico globale ed incrementale senza segno (10 byte su file) */
    unsigned int user_id;             /* ID unico senza segno dell'utente proprietario (10 byte su file) */
    char citizen_name[MAX_USERNAME + 1];  /* Array statico protetto per l'username (MAX_USERNAME byte su file) */
    ReportCategory category;         /* Categoria dell'anomalia (1 byte su file) */
    char description[MAX_DESC + 1];   /* Array statico per il corpo descrittivo (MAX_DESC byte su file) */
    char date[12];                    /* Array statico per la data "GG/MM/AAAA\0" (10 byte su file) */
    char urgency;                     /* Carattere scalare della priorita '0', '1', '2' (1 byte su file) */
    ReportStatus status;              /* Stato corrente della segnalazione (1 byte su file) */
    int disk_row;                     /* Indice riga fisica occupata nel file Master (10 byte su file) */
};

Report create_report(
    unsigned int id, 
    unsigned int user_id, 
    const char* citizen_name, 
    ReportCategory category, 
    const char* description, 
    const char* date, 
    char urgency
) {
    /* Allocazione dinamica protetta dello spazio della struttura privata in RAM */
    Report r = (Report)malloc(sizeof(struct ReportStruct));
    if (!r) return NULL; /* Ritorno preventivo di sicurezza per intercettare fallimenti della malloc */
	memset(r, 0, sizeof(struct ReportStruct));
    r->id = id;
    r->user_id = user_id;
    
    /* Esecuzione dell'hard copy bloccata delle stringhe per isolare l'oggetto logico */
    strncpy(r->citizen_name, citizen_name, MAX_USERNAME);
    r->citizen_name[MAX_USERNAME] = '\0'; /* Sigillo di chiusura stringa obbligatorio in C */
    
    r->category = category;
    
    strncpy(r->description, description, MAX_DESC);
    r->description[MAX_DESC] = '\0';
    
    strncpy(r->date, date, 11);
    r->date[11] = '\0';
    
    r->urgency = urgency;
    r->status = OPEN;  /* Ogni nuova segnalazione nasce nativamente ed obbligatoriamente in stato OPEN */
    r->disk_row = -1;  /* Un report appena creato non occupa alcuna cella master, riga impostata a -1 */

    return r; /* Restituisce il puntatore alla cella di memoria opaca configurata */
}

void free_report(Report r) {
    /* Previene tentativi di doppia deallocazione distruttiva della memoria (Double Free) */
    if (r != NULL) {
        free(r); /* Cancella fisicamente il blocco strutturale allocato dal costruttore */
    }
}

/* --------------------------------==============================================
 *  IMPLEMENTAZIONE DEI METODI GETTER (INCAPSULAMENTO IN SOLA LETTURA)
 * --------------------------------============================================== */

unsigned int get_report_id(Report r) {
    return r ? r->id : 0;
}

unsigned int get_report_user_id(Report r) {
    return r ? r->user_id : 0;
}

const char* get_report_citizen_name(Report r) {
    return r ? r->citizen_name : NULL;
}

ReportCategory get_report_category(Report r) {
    return r ? r->category : OTHER;
}

const char* get_report_description(Report r) {
    return r ? r->description : NULL;
}

const char* get_report_date(Report r) {
    return r ? r->date : NULL;
}

char get_report_urgency(Report r) {
    return r ? r->urgency : '0';
}

ReportStatus get_report_status(Report r) {
    return r ? r->status : OPEN;
}

int get_report_disk_row(Report r) {
    return r ? r->disk_row : -1;
}

/* --------------------------------==============================================
 *  IMPLEMENTAZIONE DEI METODI SETTER (MUTATORI CONTROLLATI)
 * --------------------------------============================================== */

void update_report_status(Report r, ReportStatus new_status) {
    if (r) {
        r->status = new_status; /* Aggiornamento controllato dello stato logico */
    }
}

void set_report_disk_row(Report r, int row) {
    if (r) {
        r->disk_row = row; /* Iniezione dell'offset geometrico della cella master assegnata */
    }
}

/* --------------------------------==============================================
 *  FUNZIONI AUSILIARIE DI TRADUZIONE PER L'INTERFACCIA UTENTE (STRING TRANSLATIONS)
 * --------------------------------============================================== */

const char* get_category_string(ReportCategory category) {
    switch (category) {
        case ROAD:           return "Buca Stradale";
        case LIGHTING:       return "Illuminazione Pubblica";
        case WASTE:          return "Rifiuti Abbandonati";
        case INFRASTRUCTURE: return "Guasto Impianto";
        case OTHER:          return "Altro / Generico";
        default:             return "Sconosciuta";
    }
}

const char* get_status_string(ReportStatus status) {
    switch (status) {
        case OPEN:        return "APERTA";
        case IN_PROGRESS: return "IN LAVORAZIONE";
        case CLOSED:      return "CHIUSA";
        case DESTROYED:   return "ELIMINATA LOGICAMENTE";
        default:          return "SCONOSCIUTO";
    }
}



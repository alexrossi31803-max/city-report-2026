#include "../../include/adt/report_stack.h"
#include "../../include/config.h"
#include <stdlib.h>

/**
 * @brief Struttura interna dello stack statico ad Information Hiding.
 *        Implementa un array fisso a 10 elementi per limitare l'uso di memoria in sessione.
 */
struct ReportStack {
    Report data[10];    /* Array statico di puntatori opaci preallocato in base ai vincoli */
    int top;            /* Indice di cima dello stack: punta all'ultimo elemento inserito */
};

ReportStack create_stack(void) {
    ReportStack s = (ReportStack)malloc(sizeof(struct ReportStack));
    if (s) {
        s->top = -1; /* Lo stack nasce vuoto, l'indice di cima viene inizializzato a -1 */
    }
    return s;
}

void free_stack(ReportStack s) {
    if (!s) return;
    /* Svuota ricorsivamente i report rimasti nello stack per azzerare i memory leak */
    while (s->top >= 0) {
        free_report(s->data[s->top]);
        s->top--;
    }
    free(s); /* Libera la struttura di controllo dell'ADT */
}

bool stack_push(ReportStack s, Report r) {
    /* CONTROLLO DI SICUREZZA: Verifica la saturazione strutturale dell'array statico (max 10) */
    if (!s || !r || s->top >= 9) {
        return false; /* Stack saturo o parametri non validi, inserimento abortito */
    }

    /* CLONAZIONE PROFONDA (DEEP COPY): Isola il backup dalle alterazioni della sessione RAM.
       Estrae i metadati nativi (ID senza segno e urgenza char) per rigenerare un record speculare. */
    Report backup_clone = create_report(
        get_report_id(r),
        get_report_user_id(r),
        get_report_citizen_name(r),
        get_report_category(r),
        get_report_description(r),
        get_report_date(r),
        get_report_urgency(r)
    );

    if (!backup_clone) return false;

    /* Sincronizza lo stato e la disk_row d'origine per preservare l'integrità del record storico */
    update_report_status(backup_clone, get_report_status(r));
    set_report_disk_row(backup_clone, get_report_disk_row(r));

    /* Incrementa l'indice di cima e deposita il clone profondo nello slot dell'array */
    s->top++;
    s->data[s->top] = backup_clone;
    return true;
}

Report stack_pop(ReportStack s) {
    /* CONTROLLO DI SICUREZZA: Impedisce l'estrazione illegale da uno stack vuoto (Underflow) */
    if (!s || stack_is_empty(s)) {
        return NULL;
    }

    /* Estrae il puntatore opaco dalla cima dello stack */
    Report r = s->data[s->top];
    s->top--; /* Decrementa l'indice spostando la cima verso il basso */
    return r; /* Restituisce l'oggetto clonato pronto per il ripristino (Revert) */
}

Report stack_top(ReportStack s) {
    if (!s || stack_is_empty(s)) {
        return NULL;
    }
    /* Restituisce il puntatore corrente senza alterare l'indice di cima */
    return s->data[s->top];
}

bool stack_is_empty(ReportStack s) {
    /* Se top è fermo a -1 significa che nessuna operazione di push è attiva */
    return (s == NULL || s->top == -1);
}

int stack_size(ReportStack s) {
    /* La dimensione effettiva è data dall'indice corrente aumentato di 1 unità */
    return s ? (s->top + 1) : 0;
}




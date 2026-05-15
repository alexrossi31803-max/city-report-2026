#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/server/user_manager.h"
#include "../include/server/report_manager.h"
#include "../include/adt/report_list.h"
#include "../include/adt/report_stack.h"
#include "../include/utils/validators.h"

/* --- Prototipi Interfaccia --- */
void menu_citizen(User current_user);
void menu_employee(User current_user);
void create_new_report(User u, ReportList session_list, ReportStack undo_stack);

int main() {
    init_report_manager();
    int choice;
    User logged_user = NULL;

    while (1) {
        printf("\n=== CITY REPORT SYSTEM 2026 ===\n");
        printf("1. Login\n2. Registrazione\n0. Esci\nScelta: ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            char u[MAX_USERNAME], p[MAX_PASSWORD];
            printf("Username: "); scanf("%s", u);
            printf("Password: "); scanf("%s", p);
            logged_user = login_user(u, p);

            if (logged_user) {
                if (get_user_role(logged_user) == CITIZEN) menu_citizen(logged_user);
                else menu_employee(logged_user);
                free_user(logged_user);
            } else printf("Credenziali errate!\n");
        } 
        else if (choice == 2) {
            char u[MAX_USERNAME], p[MAX_PASSWORD];
            int role_choice;
            printf("Nuovo Username: "); scanf("%s", u);
            printf("Password: "); scanf("%s", p);
            printf("Ruolo (0: Cittadino, 1: Dipendente): "); scanf("%d", &role_choice);
            
            if (register_user(u, p, (UserRole)(role_choice + '0'))) printf("Registrazione completata!\n");
            else printf("Errore: username occupato o dati non validi.\n");
        }
        else if (choice == 0) break;
    }

    shutdown_report_manager();
    return 0;
}

/* --- Area Cittadino --- */
void menu_citizen(User u) {
    ReportList session_list = create_list();
    ReportStack undo_stack = create_stack();
    int choice;

    while (1) {
        printf("\n--- AREA CITTADINO (%s) ---\n", get_username(u));
        printf("1. Nuova Segnalazione\n2. Visualizza Sessione RAM\n3. Undo Ultima Azione\n4. Logout (Flush su Disco)\nScelta: ");
        scanf("%d", &choice);

        if (choice == 1) create_new_report(u, session_list, undo_stack);
        else if (choice == 2) {
            list_rewind(session_list);
            Report r;
            while ((r = list_next(session_list))) {
                printf("[%u] %s - %s\n", get_report_id(r), get_report_date(r), get_report_description(r));
            }
        }
        else if (choice == 3) {
            Report last = stack_pop(undo_stack);
            if (last) {
                list_remove(session_list, get_report_id(last));
                printf("Ultima segnalazione annullata.\n");
            } else printf("Nulla da annullare.\n");
        }
        else if (choice == 4) {
            // Eseguiamo il Flush di tutto ciò che è nella lista di sessione
            list_rewind(session_list);
            Report r;
            while ((r = list_next(session_list))) save_report_to_bench(r);
            break; 
        }
    }
    free_list(session_list);
    free_stack(undo_stack);
}

void create_new_report(User u, ReportList session_list, ReportStack undo_stack) {
    char desc[MAX_DESC], date[11], urgency;
    int cat;

    printf("Categoria (0: AMBIENTE, 1: TRASPORTI, 2: VERDE, 3: ALTRO): "); scanf("%d", &cat);
    printf("Descrizione (max 255 car): "); scanf(" %[^\n]s", desc);
    printf("Data (GG/MM/AAAA): "); scanf("%s", date);
    printf("Urgenza (1: Bassa, 2: Media, 3: Alta): "); scanf(" %c", &urgency);

    if (validate_date_format(date) && validate_urgency_range(urgency)) {
        Report r = create_report(get_next_report_id(), get_user_id(u), get_username(u), 
                                 (ReportCategory)(cat + '0'), desc, date, urgency, OPEN);
        list_insert(session_list, r);
        stack_push(undo_stack, r);
        printf("Segnalazione creata (ID: %u). Sarà salvata al logout.\n", get_report_id(r));
    } else printf("Dati non validi!\n");
}

/* --- Area Dipendente --- */
void menu_employee(User u) {
    printf("\n--- DASHBOARD DIPENDENTE (%s) ---\n", get_username(u));
    // Qui andrebbe l'inizializzazione della Priority Queue caricando i report OPEN dai file Master
    // Per brevità simuliamo l'accesso
    printf("Funzionalità: Visualizzazione segnalazioni per priorità... (In Sviluppo)\n");
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/config.h"
#include "../include/models/user.h"
#include "../include/models/report.h"
#include "../include/adt/report_list.h"
#include "../include/adt/report_stack.h"
#include "../include/utils/validators.h"
#include "../include/utils/parser.h"
#include "../include/server/user_manager.h"
#include "../include/server/report_manager.h"
#include "../include/tests/test_suite.h"
// Funzioni per la gestione dei Menu e delle Viste UI
void menu_cittadino(User logged_in_user);
void menu_dipendente(User logged_in_user);
void mostra_segnalazioni_paginate(const char* file_path);
void esegui_casi_test();
void genera_statistiche_comunali();

int main() {
    int scelta;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];

    printf("===================================================\n");
    printf("     SISTEMA DI SEGNALAZIONI MUNICIPALI BARONISSI   \n");
    printf("===================================================\n");

    while (1) {
        printf("\n--- MENU PRINCIPALE ---\n");
        printf("1. Accedi (Login)\n");
        printf("2. Registrati (Nuovo Utente)\n");
        printf("3. Esegui Casi di Test Automatizzati\n");
        printf("4. Esci dal Programma\n");
        printf("Seleziona un'opzione: ");
        if (scanf("%d", &scelta) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); // Pulisce il buffer

        switch (scelta) {
            case 1:
                printf("\n--- ACCESSO AL SISTEMA ---\n");
                printf("Username: ");
                fgets(username, sizeof(username), stdin);
                trim_string(username);
                printf("Password: ");
                fgets(password, sizeof(password), stdin);
                trim_string(password);

                User u = login_user(username, password);
                if (u != NULL) {
                    printf("\n[OK] Autenticazione riuscita! Benvenuto %s.\n", get_user_username(u));
                    if (get_user_role(u) == EMPLOYEE) {
                        menu_dipendente(u);
                    } else {
                        menu_cittadino(u);
                    }
                    free_user(u); // Libera la sessione utente al logout
                } else {
                    printf("\n[ERRORE] Credenziali errate o utente non trovato.\n");
                }
                break;

            case 2:
                printf("\n--- REGISTRAZIONE UTENTE ---\n");
                printf("Scegli un Username (max 12 caratteri): ");
                fgets(username, sizeof(username), stdin);
                trim_string(username);
                printf("Scegli una Password (max 12 caratteri): ");
                fgets(password, sizeof(password), stdin);
                trim_string(password);

                int ruolo_scelta;
                printf("Seleziona il ruolo (0 = Cittadino, 1 = Dipendente Comunale): ");
                if (scanf("%d", &ruolo_scelta) != 1) {
                    while (getchar() != '\n');
                    printf("[ERRORE] Input non valido.\n");
                    break;
                }
                while (getchar() != '\n');

                UserRole r = (ruolo_scelta == 1) ? EMPLOYEE : CITIZEN;
                if (register_user(username, password, r)) {
                    printf("\n[OK] Registrazione completata con successo! Ora puoi accedere.\n");
                } else {
                    printf("\n[ERRORE] Registrazione fallita. Username gia' in uso o errore di sistema.\n");
                }
                break;

            case 3:
                esegui_casi_test();
                break;

            case 4:
                printf("\nGrazie per aver utilizzato il sistema municipale. Arrivederci!\n");
                return 0;

            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
    return 0;
}

void menu_cittadino(User logged_in_user) {
    ReportList ram_list = create_list();
    ReportStack revert_stack = create_stack();
    int scelta;

    //char cat_str[MAX_NAME];
    char desc_str[MAX_DESC];
    char data_str[11];
    int urgenza;

    while (1) {
        printf("\n--- AREA CITTADINO (%s) ---\n", get_user_username(logged_in_user));
        printf("1. Inserisci Nuova Segnalazione (In RAM)\n");
        printf("2. Visualizza lo Storico delle mie Segnalazioni\n");
        printf("3. Modifica una Segnalazione (In RAM)\n");
        printf("4. Annulla Ultima Modifica (Revert/Undo Stack)\n");
        printf("5. Esci ed Invia Segnalazioni al Comune (Logout & Flush)\n");
        printf("Seleziona un'opzione: ");
        if (scanf("%d", &scelta) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        switch (scelta) {
            case 1:
                printf("\n--- COMPILA SEGNALAZIONE ---\n");
                int cat_scelta;
                printf("Categoria (0=Buca, 1=Illuminazione, 2=Rifiuti, 3=Impianto, 4=Altro): ");
                if (scanf("%d", &cat_scelta) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                printf("Descrizione del problema: ");
                fgets(desc_str, sizeof(desc_str), stdin);
                trim_string(desc_str);

                printf("Data di oggi (GG/MM/AAAA): ");
                fgets(data_str, sizeof(data_str), stdin);
                trim_string(data_str);

                printf("Livello di Urgenza (1=Bassa, 2=Media, 3=Alta): ");
                if (scanf("%d", &urgenza) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                if (!validate_not_empty(desc_str) || !validate_date_format(data_str) || !validate_urgency_range(urgenza)) {
                    printf("\n[ERRORE] Dati inseriti non validi o non conformi.\n");
                    break;
                }

                int global_id = generate_global_report_id() + list_size(ram_list);
                Report new_r = create_report(global_id, get_user_username(logged_in_user), (ReportCategory)cat_scelta, desc_str, data_str, urgenza);
                list_insert(ram_list, new_r);
                printf("\n[OK] Segnalazione inserita nella sessione locale RAM (Codice: %05d).\n", global_id);
                break;

            case 2:
                printf("\n===================================================\n");
                printf("           ELENCO SEGNALAZIONI ATTIVE               \n");
                printf("===================================================\n");
                
                // 1. Mostra elementi attualmente presenti in RAM per la sessione corrente
                int blocco_counter = 1;
                list_rewind(ram_list);
                Report r_ram = list_next(ram_list);
                if (r_ram != NULL) printf("\n--- SEGNALAZIONI NELLA SESSIONE ATTUALE (RAM) ---\n");
                while (r_ram != NULL) {
                    printf("[ Blocco N. %d ]\n", blocco_counter++);
                    printf("Codice Segnalazione: %05d\n", get_report_id(r_ram));
                    printf("Categoria: %s\n", get_category_string(get_report_category(r_ram)));
                    printf("Data Inserimento: %s\n", get_report_date(r_ram));
                    printf("Urgenza: %d\n", get_report_urgency(r_ram));
                    printf("Descrizione: %s\n", get_report_description(r_ram));
                    printf("------------------------------------------------------\n");
                    r_ram = list_next(ram_list);
                }

                // 2. Legge e mostra gli elementi salvati precedentemente nel Bench File per questo utente
                FILE* f_bench = fopen(PATH_BENCH, "r");
                if (f_bench) {
                    char line[335];
                    bool primo_bench = true;
                    while (fgets(line, sizeof(line), f_bench)) {
                        if (line[0] != ' ' && line[0] != '\n') {
                            char state; int c_id;
                            Report r_b = line_to_report(line, &state, &c_id);
                            if (state == 'A' && strcmp(get_report_citizen_name(r_b), get_user_username(logged_in_user)) == 0) {
                                if (primo_bench) { printf("\n--- SEGNALAZIONI SALVATE NELLA CACHE SERVER (BENCH) ---\n"); primo_bench = false; }
                                printf("[ Blocco N. %d ]\n", blocco_counter++);
                                printf("Codice Segnalazione: %05d\n", get_report_id(r_b));
                                printf("Categoria: %s\n", get_category_string(get_report_category(r_b)));
                                printf("Stato Pratica: %s\n", get_status_string(get_report_status(r_b)));
                                printf("Descrizione: %s\n", get_report_description(r_b));
                                printf("------------------------------------------------------\n");
                            }
                            free_report(r_b);
                        }
                    }
                    fclose(f_bench);
                }
                
                if (blocco_counter == 1) {
                    printf("\nNon ci sono segnalazioni attive associate al tuo profilo.\n");
                }
                break;

            case 3:
                printf("\nInserisci il Codice della segnalazione da modificare (presente in RAM): ");
                int target_id;
                if (scanf("%d", &target_id) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                Report r_mod = list_find(ram_list, target_id);
                if (r_mod != NULL && get_report_status(r_mod) == OPEN) {
                    // Salva lo stato corrente nello Stack di Revert prima di sovrascrivere
                    stack_push(revert_stack, r_mod);

                    printf("Nuova descrizione: ");
                    fgets(desc_str, sizeof(desc_str), stdin);
                    trim_string(desc_str);
                    
                    // Sostituisce la descrizione aggiornando l'oggetto nascosto
                    Report clonizzato = create_report(get_report_id(r_mod), get_report_citizen_name(r_mod), 
                                                      get_report_category(r_mod), desc_str, get_report_date(r_mod), get_report_urgency(r_mod));
                    list_remove(ram_list, target_id);
                    list_insert(ram_list, clonizzato);
                    
                    printf("\n[OK] Segnalazione aggiornata in RAM. Stato precedente salvato nello Stack.\n");
                } else {
                    printf("\n[ERRORE] Segnalazione non trovata in RAM o non modificabile.\n");
                }
                break;

            case 4:
                if (!stack_is_empty(revert_stack)) {
                    Report vecchio_stato = stack_pop(revert_stack);
                    int old_id = get_report_id(vecchio_stato);
                    
                    // Rimuove la versione alterata e ripristina la precedente estratta dallo stack
                    list_remove(ram_list, old_id);
                    list_insert(ram_list, vecchio_stato);
                    printf("\n[OK] Azione annullata! Ripristinato lo stato precedente del report %05d.\n", old_id);
                } else {
                    printf("\n[AVVISO] Nessuna azione da annullare nello Stack.\n");
                }
                break;

            case 5:
                printf("\nSalvataggio e sincronizzazione in corso...\n");
                flush_session_to_bench(ram_list);
                free_list(ram_list);
                free_stack(revert_stack);
                printf("[OK] Sessione chiusa correttamente. Dati inviati alla coda del server.\n");
                return;

            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
}

void menu_dipendente(User logged_in_user) {
    int scelta;
    int rep_id, current_st, new_st;

    while (1) {
        printf("\n--- AREA DIPENDENTE (%s) ---\n", get_user_username(logged_in_user));
        printf("1. Visualizza Segnalazioni APERTE (Paginazione 5 alla volta)\n");
        printf("2. Visualizza Segnalazioni IN LAVORAZIONE\n");
        printf("3. Visualizza Segnalazioni CHIUSE\n");
        printf("4. Modifica Stato di una Segnalazione (Avanzamento Pratica)\n");
        printf("5. Visualizza Elenco delle Priorita' ed Urgenze (Server Queue)\n");
        printf("6. Genera Report Statistico Comunale\n");
        printf("7. Disconnetti (Logout)\n");
        printf("Seleziona un'opzione: ");
        if (scanf("%d", &scelta) != 1) {
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        switch (scelta) {
            case 1:
                printf("\n--- ELENCO PRATICHE APERTE (Master) ---\n");
				process_and_flush_bench();
                mostra_segnalazioni_paginate(PATH_OPEN_LATEST);
                break;
            case 2:
                printf("\n--- ELENCO PRATICHE IN LAVORAZIONE (Master) ---\n");
				process_and_flush_bench();
                mostra_segnalazioni_paginate(PATH_PROGRESS_LATEST);
                break;
            case 3:
                printf("\n--- ELENCO PRATICHE CHIUSE (Master) ---\n");
				process_and_flush_bench();
                mostra_segnalazioni_paginate(PATH_CLOSED_LATEST);
                break;
            case 4:
                printf("\n--- CAMBIO STATO SEGNALAZIONE ---\n");
                printf("Inserisci il Codice numerico del Report da aggiornare: ");
                if (scanf("%d", &rep_id) != 1) { while (getchar() != '\n'); break; }
                printf("Stato attuale (0=OPEN, 1=IN_PROGRESS): ");
                if (scanf("%d", &current_st) != 1) { while (getchar() != '\n'); break; }
                printf("Nuovo stato desiderato (1=IN_PROGRESS, 2=CLOSED): ");
                if (scanf("%d", &new_st) != 1) { while (getchar() != '\n'); break; }
                while (getchar() != '\n');

                if (update_report_state_server(rep_id, (ReportStatus)current_st, (ReportStatus)new_st)) {
                    printf("\n[OK] Stato della pratica %05d modificato con successo nel database.\n", rep_id);
                } else {
                    printf("\n[ERRORE] Impossibile aggiornare la pratica. Verifica codice o stato di partenza.\n");
                }
                break;
            case 5:
                printf("\n--- CRONOLOGIA DELLE PRIORITA' INTEGRATE ED AGGIORNATE ---\n");
                // Forza un riallineamento preventivo dei file e genera l'array ordinato
                process_and_flush_bench();
                rebuild_priority_file();
                mostra_segnalazioni_paginate(PATH_PRIORITY_FILE);
                break;
            case 6:
                genera_statistiche_comunali();
                break;
            case 7:
                printf("\nDisconnessione effettuata.\n");
                return;
            default:
                printf("\n[ERRORE] Opzione non valida.\n");
        }
    }
}

void mostra_segnalazioni_paginate(const char* file_path) {
    FILE* f = fopen(file_path, "r");
    if (!f) {
        printf("Nessun dato memorizzato o archivio vuoto.\n");
        return;
    }

    char line[335];
    int counter = 0;
    char input_pag;

    while (fgets(line, sizeof(line), f)) {
        if (line[0] != ' ' && line[0] != '\n') {
            char rec_state; int cit_id;
            Report r = line_to_report(line, &rec_state, &cit_id);
            
            if (rec_state == 'A') {
                printf("------------------------------------------------------\n");
                printf("Codice: %05d | Cittadino: %s | Categoria: %s\n", 
                       get_report_id(r), get_report_citizen_name(r), get_category_string(get_report_category(r)));
                printf("Data: %s | Urgenza: %d | Stato: %s\n", 
                       get_report_date(r), get_report_urgency(r), get_status_string(get_report_status(r)));
                printf("Descrizione: %s\n", get_report_description(r));
                counter++;

                // Paginazione interattiva: blocca l'output ogni 5 segnalazioni attive
                if (counter % 5 == 0) {
                    printf("------------------------------------------------------\n");
                    printf("Mostrati 5 elementi. Premi [INVIO] per caricare ancora o 'q' per fermarti: ");
                    input_pag = getchar();
                    if (input_pag == 'q' || input_pag == 'Q') {
                        free_report(r);
                        fclose(f);
                        return;
                    }
                    if (input_pag != '\n') while (getchar() != '\n');
                }
            }
            free_report(r);
        }
    }
    printf("------------------------------------------------------\n");
    printf("Fine dell'elenco. %d segnalazioni attive caricate.\n", counter);
    fclose(f);
}

void genera_statistiche_comunali() {
    int totali = 0, aperte = 0, lavorazione = 0, chiuse = 0;
    int cat_counts[5] = {0};
    char line[335];

    const char* paths[] = { PATH_OPEN_LATEST, PATH_PROGRESS_LATEST, PATH_CLOSED_LATEST };
    for (int i = 0; i < 3; i++) {
        FILE* f = fopen(paths[i], "r");
        if (f) {
            while (fgets(line, sizeof(line), f)) {
                if (line[0] != ' ' && line[0] != '\n') {
                    char state; int c_id;
                    Report r = line_to_report(line, &state, &c_id);
                    if (state == 'A') {
                        totali++;
                        cat_counts[(int)get_report_category(r)]++;
                        if (get_report_status(r) == OPEN) aperte++;
                        else if (get_report_status(r) == IN_PROGRESS) lavorazione++;
                        else if (get_report_status(r) == CLOSED) chiuse++;
                    }
                    free_report(r);
                }
            }
            fclose(f);
        }
    }

    printf("\n===================================================\n");
    printf("         REPORT STATISTICO COMUNALE GENERATO        \n");
    printf("===================================================\n");
    printf("Numero totale di segnalazioni registrate: %d\n", totali);
    printf("  - Pratiche Aperte (OPEN): %d\n", aperte);
    printf("  - Pratiche In Lavorazione (IN_PROGRESS): %d\n", lavorazione);
    printf("  - Pratiche Risolte/Chiuse (CLOSED): %d\n", chiuse);
    printf("\nSuddivisione analitica per Categoria:\n");
    for (int i = 0; i < 5; i++) {
        printf("  - %-25s: %d\n", get_category_string((ReportCategory)i), cat_counts[i]);
    }
    
    int max_idx = 0;
    for (int i = 1; i < 5; i++) {
        if (cat_counts[i] > cat_counts[max_idx]) max_idx = i;
    }
    printf("\nTipologia di problema piu' frequente: %s\n", get_category_string((ReportCategory)max_idx));
    printf("===================================================\n");
}

void esegui_casi_test() {
	run_all_tests(); 
}

/*
Stavo riguardando il codice allora adesso cominciamo a rifinire log in. All'inizio del programma ci sta il login oppure la registrazione. La registrazione 
avviene praticamente andando a popolare il file users.txt. Adesso concentriamoci sul menu utente
dopo il login o registrazione l'utente può inserire dei report. I report che l'utente inserisce vengono caricati sui data file. Qui bisogna fare attenzione
al come vengono inseriti. Localmente l'utente ha in RAM tramite lista concatenata tutte le sue segnalazioni fatte durante la sessione. L'utente può visionare
tutte le sue segnalazioni, vedrà sia la lista caricata in RAM della sessione e sia tutte le sue segnalazioni che si trovano sui data. Se lo stato della 
segnalazione è OPEN (submitted) l'utente ha pieno controllo della segnalazione, può modificare i dati inseriti e può eliminarla. Se lo stato è IN_PROGRESS
non può più fare niente se non visualizzarla stessa cosa per CLOSED.  

--------------------------------------------
📝 Descrizione del Modulo di Autenticazione (Versione File .txt)I dati di login vengono gestiti attraverso un'architettura ottimizzata basata su due 
file di testo semplici che lavorano in modo complementare per garantire la massima velocità di esecuzione: users.txt (il file contenente i dati reali 
degli utenti) e users_idx.txt (il file che funge da tabella di indicizzazione hash). Questa struttura accoglie indistintamente sia i profili dei cittadini 
sia quelli dei lavoratori comunali.🗂️ Struttura e Crescita dei File .txtIl File Dati (users.txt): Memorizza le informazioni degli utenti (ID, Username, 
Password, Ruolo) in righe di testo a lunghezza fissa (es. ogni riga occupa esattamente 40 caratteri, riempiti con spazi " " alla fine). Ogni volta che una 
registrazione va a buon fine, la nuova riga viene appesa in coda al file.Il File Indice (users_idx.txt): È il motore della velocità del sistema. 
Viene inizializzato a blocchi fissi. Il primo blocco contiene esattamente 50 slot di testo. Ogni slot occupa una dimensione fissa (es. 4 caratteri) ed è 
inizializzato con il valore "-1  " (uno spazio vuoto). Quando il numero di utenti registrati raggiunge la capacità limite, il file si estende aggiungendo 
un nuovo blocco di testo da 50 slot (portando la capacità a 100, poi 150, ecc.).📥 Logica di Registrazione IntelligenteIl caricamento e la verifica degli 
account avvengono tramite l'analisi dell'username, che deve essere univoco:Il sistema prende l'username e, tramite una funzione hash(), genera un indice 
numerico calcolato in base alla capacità attuale del file indice (es. tra 0 e 49 nel primo blocco).Il sistema calcola la posizione nel file di testo users_
idx.txt moltiplicando l'indice per la dimensione fissa dello slot e legge il valore:Se lo slot contiene "-1", significa che la posizione è disponibile. Il
 sistema scrive in quel punto il numero di riga sequenziale dell'utente e scrive i dati reali in coda al file users.txt.Se lo slot è già occupato da un altro
 utente, l'algoritmo verifica se l'username coincide: in caso di collisione tra stringhe diverse (es. hash identico), applica una scansione lineare muovendosi
 in avanti di un indice alla volta (0 -> 1 -> 2 -> 3...) fino a trovare il primo slot che contiene "-1". Se invece l'username esiste già, il sistema segnala
 che non è disponibile.La generazione degli indici si adatta dinamicamente all'estensione del file di testo: se la capacità attuale è 50, l'hash distribuisce
 i valori tra 0 e 49. Se il file viene esteso di altri 50 slot, l'algoritmo sposta proporzionalmente l'intervallo includendo i nuovi indici da 50 a 99.
 🔐 Login Istantaneo in \(O(1)\)Grazie a questa separazione su file di testo a dimensione fissa, la fase di login riduce a zero i tempi di attesa. 
 Quando un utente inserisce username e password, il sistema non deve scansionare l'intero database riga per riga \(O(n)\).L'algoritmo calcola l'hash 
 dell'username, interroga direttamente lo slot corrispondente nel file di testo users_idx.txt e salta istantaneamente alla riga esatta nel file users.txt. 
 L'accesso avviene in tempo costante \(O(1)\), rendendo l'intero sistema altamente ottimizzato ed efficiente.Nota di progetto: Per mantenere l'applicazione 
 snella e focalizzata sugli obiettivi principali, il sistema non prevede alcuna logica di modifica dell'username, recupero o cancellazione degli account 
 utenti.
----------------------------------


Adesso invece parliamo dei report, prima di tutto seppure i dipendenti hanno molti permessi non possono incerire nuovi report, ovviamente può farlo il 
dipendente intesa come persona ma usando un account citizen. La difficoltà qui sta nel fatto che il sistema deve gestire i report nel miglior modo possibile.
Il sistema praticamente riceve tutta una serie di report, che vanno a finire all'interno di un file nella cartella Master_Files che possiamo chiamare 
reports_open.txt praticamente qui vengono caricate tutti report in progress che gli utenti possono attivamente modificare a loro piacimento e anche eliminare.
La gestione di questo data file deve essere simiile a quella degli users il file viene è inizializzato ad una dimensione iniziale tot, l'hash() genera gli indici
sempre in base alla dimensione del file. In questa maniera gli utenti possono velocemente interagire con i loro report O(1) gli indici devono essere creati
sempre con il criterio reports_open ha 50 posti per report genera indici da 0 a 49, se viene aumentata da 50 a 99 e così via. Come vengono generati questi indici
io stavo pensando che usa id cittadino e id report poi vedi tu la strategia migliore l'importante è che sia report -> indice univoco aligning non ammesso. Per quanto
riguarda l'UI cittadino, deve poter vedere tutte le sue segnalazioni, ognunga in un blocco separato da ---------------- lineette e numerate 1-2-3 poi può scegliere
con quale interagire, ovviamente modifica e cancellazione solo se il report (segnalazione) è in stato OPEN. Una volta che i report vengono messi in lavorazione
vanno nel file reports_in_progress e una volta che sono chiuse nel file reports_closed. Quindi in totale il sistema tiene 3 file principali dove amministra
segnalazioni OPEN, IN_PROGRESS, CLOSED. La criticità qui è proprio la hash() deve essere deterministica, dare un indirizzo corretto, in base alla dimensione
dei file in primis, e id degi oggetti report e user. Mentre nel file user non ci preoccupiamo della eliminazione per semplicità qui invece è obbligo pensarla.
Quello che stavo pensando è che il sistema ha sempre una variabile che tiene traccia dei report che sono tecnicamente elimati per esempio una segnalazione finsice
all'interno di open.txt poi viene aggiornata dal dipendente che la mette in progress, oppure eliminata dal cittadino che l'ha pubblicata quindi deve essere eliminata



I dipendendti devono poter visualizzare tutte le segnalazioni -> Un dipendente visualizzerà tutte le segnalazioni, l'ui mostra 5 segnalazioni alla volta e poi chiede
carica ancora per caricare le altre. 
I dipendenti devono aggiornare lo stato di una segnalazione -> Un dipendente aggiorna lo stato di una segnalzione che comporta un cambiamento all'inerno dei 
file data dei report, open, closed, in progress.
I dipendenti possono cercare una segnalazione tramite codice o categoria -> 
I dipendenti possono visualizzare le OPEN, IN_PROGRESS, CLOSED ->
I dipendneti possono visualizzare le segnalazioni per ordine di urgenza -> Il sistema che idealmente opera nei server prende tutte le segnalazioni open e in_progress
e le ordina in base all'urgenza. L'ordine si basa su urgenza e sulla data, più è vecchia più ha urgenza alta. Problema il sistema deve visionare tutte le segnalazioni
nei file open e in_progress 
I dipendenti possono elimare una segnalazione ->

I dipendneti possono richiedere la generazione di un report -> Il report ha numero totale di segnalazioni, segnalazioni per categoria, segnalazioni 
aperte e chiuse, segnalazioni più frequenti per tipologia



● Registrazione di una nuova segnalazione con: 
○ codice identificativo  ok- 
○ nome del cittadino   ok-
○ categoria del problema ok-
○ descrizione  ok-
○ data di inserimento  ok-
○ livello di urgenza  ok-
○ stato della segnalazione  ok-
● Visualizzazione di tutte le segnalazioni registrate 
● Ricerca di una segnalazione tramite codice o categoria 
● Aggiornamento dello stato di una segnalazione 
● Visualizzazione delle segnalazioni: 
○ aperte 
○ in lavorazione 
○ chiuse 
● Visualizzazione delle segnalazioni più urgenti 
● Eliminazione di una segnalazione (se consentito) 
● Generazione di un report con: 
○ numero totale di segnalazioni 
○ segnalazioni per categoria 
○ segnalazioni aperte e chiuse 
○ segnalazioni più frequenti per tipologia 
*/
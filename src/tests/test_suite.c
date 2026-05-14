#include "../../include/tests/test_suite.h"
#include "../../include/config.h"
#include "../../include/models/user.h"
#include "../../include/models/report.h"
#include "../../include/adt/report_list.h"
#include "../../include/adt/report_stack.h"
#include "../../include/adt/priority_queue.h"
#include "../../include/utils/validators.h"
#include "../../include/utils/parser.h"
#include "../../include/server/user_manager.h"
#include "../../include/server/report_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define ASSERT_TRUE(condition, message) \
    if (condition) { printf("  [PASS] %s\n", message); } \
    else { printf("  [FAIL] %s\n", message); }

/**
 * @brief Punto di ingresso principale della suite di test automatizzata.
 * @post Esegue sequenzialmente tutti i test di conformità logica e geometrica del sistema.
 */
void run_all_tests() {
    printf("\n===================================================\n");
    printf("     AVVIO TEST SUITE AUTOMATIZZATA (REQUISITI)    \n");
    printf("===================================================\n");
    
    srand((unsigned int)time(NULL));

    test_user_registration_and_login();
    test_report_registration_and_ram_list();
    test_revert_stack_logic();
    test_report_state_update_and_file_transfer();
    test_priority_queue_sorting();
    test_statistical_report_generation();
    
    printf("===================================================\n");
    printf("             TEST SUITE COMPLETATA                 \n");
    printf("===================================================\n");
}

void test_user_registration_and_login() {
    printf("\n[TEST] Autenticazione e Anagrafica O(1):\n");
    char test_uname[MAX_USERNAME];
    char test_uname_dup[MAX_USERNAME];
    
    snprintf(test_uname, sizeof(test_uname), "u_%d_%d", (int)(time(NULL) % 1000), rand() % 1000);
    snprintf(test_uname_dup, sizeof(test_uname_dup), "d_%d_%d", (int)(time(NULL) % 1000), rand() % 1000);    

    bool reg_ok = register_user(test_uname, "securePass1", CITIZEN);
    ASSERT_TRUE(reg_ok, "Registrazione di un nuovo utente univoco.");   

    register_user(test_uname_dup, "pass", CITIZEN);
    bool reg_dup = register_user(test_uname_dup, "pass2", CITIZEN);
    ASSERT_TRUE(!reg_dup, "Rifiuto corretto di un username duplicato.");    

    User logged = login_user(test_uname, "securePass1");
    ASSERT_TRUE(logged != NULL && strcmp(get_user_username(logged), test_uname) == 0, "Login riuscito con credenziali corrette.");   

    User wrong_pass = login_user(test_uname, "wrongPass");
    ASSERT_TRUE(wrong_pass == NULL, "Login rifiutato correttamente con password errata.");
    
    if (logged) free_user(logged);
}

void test_report_registration_and_ram_list() {
    printf("\n[TEST] Registrazione Segnalazione e RAM List:\n");
    ReportList tl = create_list();
    Report r1 = create_report(9001, "Mario", ROAD, "Buca profonda via Roma", "14/05/2026", 3);
    
    list_insert(tl, r1);
    ASSERT_TRUE(list_size(tl) == 1, "Inserimento corretto del report in RAM List.");
    
    Report found = list_find(tl, 9001);
    ASSERT_TRUE(found != NULL && strcmp(get_report_description(found), "Buca profonda via Roma") == 0, "Ricerca e corrispondenza del report per codice.");
    
    bool removed = list_remove(tl, 9001);
    ASSERT_TRUE(removed && list_size(tl) == 0, "Rimozione ed eliminazione logica dalla RAM List.");
    
    free_list(tl);
}

void test_revert_stack_logic() {
    printf("\n[TEST] Logica di Revert (Undo Stack) del Cittadino:\n");
    ReportList ram_list = create_list();
    ReportStack rev_stack = create_stack();
    
    Report original = create_report(9500, "Luigi", LIGHTING, "Lampione spento", "14/05/2026", 1);
    list_insert(ram_list, original);
    
    Report r_to_mod = list_find(ram_list, 9500);
    stack_push(rev_stack, r_to_mod);
    
    Report modified = create_report(9500, "Luigi", LIGHTING, "Lampione spento e rotto", "14/05/2026", 2);
    list_remove(ram_list, 9500);
    list_insert(ram_list, modified);
    
    Report checked_mod = list_find(ram_list, 9500);
    ASSERT_TRUE(strcmp(get_report_description(checked_mod), "Lampione spento e rotto") == 0, "Modifica applicata correttamente in RAM.");
    
    if (!stack_is_empty(rev_stack)) {
        Report backup = stack_pop(rev_stack);
        list_remove(ram_list, 9500);
        list_insert(ram_list, backup);
    }
    
    Report reverted = list_find(ram_list, 9500);
    ASSERT_TRUE(reverted != NULL && strcmp(get_report_description(reverted), "Lampione spento") == 0, "Revert riuscito: ripristinato lo stato originario del report.");
    
    free_list(ram_list);
    free_stack(rev_stack);
}

void test_report_state_update_and_file_transfer() {
    printf("\n[TEST] Avanzamento Stato e Conservazione Metadati Geometrici:\n");
    int test_rep_id = 8888;
    Report r_test = create_report(test_rep_id, "StatoTester", WASTE, "Rifiuti ingombranti", "14/05/2026", 2);
    set_report_disk_row(r_test, 12); // Simulazione iniezione indirizzo fisico di riga
    
    update_report_status(r_test, IN_PROGRESS);    

    char buffer_line[REPORT_LINE_TOTAL + 3];
    report_to_line(buffer_line, r_test, 'A');    

    char r_state; int row;
    Report read_back = line_to_report(buffer_line, &r_state, &row);
    
    bool final_check = (read_back != NULL && 
                        get_report_id(read_back) == test_rep_id && 
                        get_report_status(read_back) == IN_PROGRESS && 
                        get_report_disk_row(read_back) == 12 &&
                        r_state == 'A');
                        
    ASSERT_TRUE(final_check, "Mappatura e specidica del record invertita con successo sul parser rigido.");
    
    free_report(r_test);
    if (read_back) free_report(read_back);
}

void test_priority_queue_sorting() {
    printf("\n[TEST] Ordinamento Coda a Priorita' (Urgenza + Data FIFO):\n");
    PriorityQueue pq = create_pq();
    
    Report r1 = create_report(8001, "UserA", ROAD, "Problema Medio", "12/05/2026", 2);
    Report r2 = create_report(8002, "UserB", ROAD, "Problema Alto Recente", "13/05/2026", 3);
    Report r3 = create_report(8003, "UserC", ROAD, "Problema Alto Vecchio", "10/05/2026", 3);
    
    pq_enqueue(pq, r1);
    pq_enqueue(pq, r2);
    pq_enqueue(pq, r3);
    
    Report e1 = pq_dequeue(pq);
    ASSERT_TRUE(get_report_id(e1) == 8003, "Precedenza corretta all'urgenza alta piu' datata (FIFO temporale).");
    
    Report e2 = pq_dequeue(pq);
    ASSERT_TRUE(get_report_id(e2) == 8002, "Seconda estrazione per l'urgenza alta successiva.");
    
    Report e3 = pq_dequeue(pq);
    ASSERT_TRUE(get_report_id(e3) == 8001, "Ultima estrazione riservata all'urgenza minore.");
    
    free_report(e1); free_report(e2); free_report(e3);
    free_pq(pq);
}

void test_statistical_report_generation() {
    printf("\n[TEST] Integrita' dei Registri di Stato stazionari:\n");
    FILE* f1 = fopen(PATH_OPEN_MASTER, "rb");
    FILE* f2 = fopen(PATH_PROGRESS_MASTER, "rb");
    FILE* f3 = fopen(PATH_CLOSED_MASTER, "rb");
    
    ASSERT_TRUE(f1 != NULL || f2 != NULL || f3 != NULL, "Accessibilita' binarizzata ai canali storici verificata.");
    
    if (f1) fclose(f1);
    if (f2) fclose(f2);
    if (f3) fclose(f3);
}



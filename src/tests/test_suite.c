#include "../../include/tests/test_suite.h"
#include "../../include/config.h"
#include "../../include/models/user.h"
#include "../../include/models/report.h"
#include "../../include/adt/report_list.h"
#include "../../include/adt/report_stack.h"
#include "../../include/adt/report_avl.h"
#include "../../include/adt/priority_queue.h"
#include "../../include/utils/parser.h"
#include "../../include/server/user_manager.h"
#include "../../include/server/report_manager.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/* ==============================================================================
 *  PROTOTIPI INTERNI DEI TEST CASE ISOLATI
 * ============================================================================== */
static void test_user_hashing_and_registration(void);
static void test_ram_list_and_volatile_session(void);
static void test_undo_stack_lifo_revert(void);
static void test_dual_geometry_parsing_mechanisms(void);
static void test_priority_queue_cross_sorting(void);
static void test_disk_binary_search_indices(void);
static void test_statistical_registers_o1(void);

void run_all_tests(void) {
    printf("\n===================================================\n");
    printf("     AVVIO SUITE DI VALIDAZIONE AUTOMATIZZATA      \n");
    printf("===================================================\n");

    test_user_hashing_and_registration();
    test_ram_list_and_volatile_session();
    test_undo_stack_lifo_revert();
    test_dual_geometry_parsing_mechanisms();
    test_priority_queue_cross_sorting();
    test_disk_binary_search_indices();
    test_statistical_registers_o1();

    printf("\n===================================================\n");
    printf("  [OK] TUTTI I TEST SONO STATI SUPERATI CON SUCCESS0! \n");
    printf("===================================================\n");
}

/**
 * @brief Valida la registrazione O(1) tramite Hashing DJB2 e Linear Probing.
 *        Accerta il passo a 6 byte dell'indice e la protezione anti-duplicati.
 */
static void test_user_hashing_and_registration(void) {
    printf("[TEST] Validazione Hashing DJB2 e Linear Probing... ");
    
    /* Pulisce i file per garantire l'isolamento del test case */
    remove(PATH_USERS);
    remove(PATH_USERS_IDX);

    /* Test 1: Inserimento primo utente inedito */
    bool status1 = register_user("cittadino_test", "password123", CITIZEN);
    assert(status1 == true);

    /* Test 2: Tentativo di duplicazione dello stesso username (Deve fallire in O(1)) */
    bool status2 = register_user("cittadino_test", "altra_pass", CITIZEN);
    assert(status2 == false);

    /* Test 3: Verifica correttezza login e recupero puntatore opaco */
    User u = login_user("cittadino_test", "password123");
    assert(u != NULL);
    assert(strcmp(get_user_username(u), "cittadino_test") == 0);
    assert(get_user_role(u) == CITIZEN);
    
    free_user(u);
    printf("SUPERATO.\n");
}

/**
 * @brief Valida la Linked List dinamica per la memorizzazione volatile in RAM.
 */
static void test_ram_list_and_volatile_session(void) {
    printf("[TEST] Validazione ReportList locale di sessione RAM... ");
    ReportList list = create_list();
    assert(list != NULL);

    Report r1 = create_report(100, 1, "Alex", ROAD, "Buca", "18/05/2026", '1');
    list_insert(list, r1);
    assert(list_size(list) == 1);

    Report found = list_find(list, 100);
    assert(found != NULL);
    assert(get_report_user_id(found) == 1);

    bool removed = list_remove(list, 100);
    assert(removed == true);
    assert(list_size(list) == 0);

    free_list(list);
    printf("SUPERATO.\n");
}

/**
 * @brief Valida la logica LIFO profonda dello stack statico di Undo (max 10).
 */
static void test_undo_stack_lifo_revert(void) {
    printf("[TEST] Validazione Undo Stack LIFO (Clonazione Profonda)... ");
    ReportStack stack = create_stack();
    assert(stack != NULL);

    Report original = create_report(200, 2, "Alex", LIGHTING, "Guasto", "18/05/2026", '2');
    
    /* Esegue il push salvando lo stato integro primordiale */
    bool pushed = stack_push(stack, original);
    assert(pushed == true);
    assert(stack_size(stack) == 1);

    /* Modifica fittizia del record originale in RAM */
    update_report_status(original, IN_PROGRESS);

    /* Estrae il punto di ripristino (Pop LIFO) */
    Report restored = stack_pop(stack);
    assert(restored != NULL);
    /* Accerta che il backup conservi lo stato primordiale OPEN (Isolamento Deep Copy) */
    assert(get_report_status(restored) == OPEN);
    assert(get_report_id(restored) == 200);

    free_report(original);
    free_report(restored);
    free_stack(stack);
    printf("SUPERATO.\n");
}

/**
 * @brief Valida le scomposizioni asimmetriche separate dei parser a 351 e 352 byte.
 */
static void test_dual_geometry_parsing_mechanisms(void) {
    printf("[TEST] Validazione Parser Asimmetrico (351 Bench vs 352 Master)... ");
    
    Report r = create_report(555, 99, "UserTest", WASTE, "Rifiuti", "18/05/2026", '0');
    set_report_disk_row(r, 15);

    /* Test 1: Configurazione geometrica MASTER a 352 byte */
    char master_line[REPORT_MASTER_LINE + 1];
    report_to_line(master_line, r, 'A');
    assert(strlen(master_line) == REPORT_MASTER_LINE);
    assert(master_line[350] == 'A'); /* Flag Active al byte offset 350 */
    assert(master_line[351] == '\n');

    /* Verifiche del parsing inverso di riga Master */
    char extracted_state;
    Report r_master = line_to_report_v2(master_line, &extracted_state);
    assert(r_master != NULL);
    assert(extracted_state == 'A');
    assert(get_report_id(r_master) == 555);
    assert(get_report_disk_row(r_master) == 15);
    free_report(r_master);

    /* Test 2: Configurazione geometrica BENCH a 351 byte */
    char bench_line[REPORT_BENCH_LINE + 1];
    report_to_line(bench_line, r, '\0');
    assert(strlen(bench_line) == REPORT_BENCH_LINE);
    assert(bench_line[350] == '\n'); /* Sulla BENCH il newline risiede direttamente al byte 350 */

    free_report(r);
    printf("SUPERATO.\n");
}

/**
 * @brief Valida i criteri di ordinamento incrociato (Urgenza decrescente + Data FIFO).
 */
static void test_priority_queue_cross_sorting(void) {
    printf("[TEST] Validazione Coda a Priorita (Urgenza Decrescente + Data FIFO)... ");
    PriorityQueue pq = create_pq();

    /* Inserimento di tre casi con criticita asimmetriche */
    Report low_urg = create_report(1, 10, "A", ROAD, "Desc", "15/05/2026", '0');
    Report high_urg_recent = create_report(2, 10, "B", ROAD, "Desc", "18/05/2026", '2');
    Report high_urg_old = create_report(3, 10, "C", ROAD, "Desc", "10/05/2026", '2'); /* Massima priorita (Piu vecchia) */

    pq_enqueue(pq, low_urg);
    pq_enqueue(pq, high_urg_recent);
    pq_enqueue(pq, high_urg_old);

    /* Estrazione 1: Deve estrarre high_urg_old perche pari urgenza '2' ma data piu remota */
    Report e1 = pq_dequeue(pq);
    assert(get_report_id(e1) == 3);
    free_report(e1);

    /* Estrazione 2: Estrae high_urg_recent (Urgenza '2' rimanente) */
    Report e2 = pq_dequeue(pq);
    assert(get_report_id(e2) == 2);
    free_report(e2);

    /* Estrazione 3: Estrae low_urg (Urgenza '0') */
    Report e3 = pq_dequeue(pq);
    assert(get_report_id(e3) == 1);
    free_report(e3);

    free_pq(pq);
    printf("SUPERATO.\n");
}

/**
 * @brief Valida la Ricerca Binaria (Dicotomica) su disco per gli indici Inorder Array (21 e 22 byte).
 */
static void test_disk_binary_search_indices(void) {
    printf("[TEST] Validazione Ricerca Dicotomica su Disco (Inorder Arrays 21/22 byte)... ");
    
    /* Configura dei file temporanei d'indice puliti e simulati */
    remove(PATH_AVL_REPORT_ID);
    remove(PATH_AVL_USER_ID);
    remove(PATH_SEQUENCE);

    /* Scrive due variabili di controllo nel file system simulato */
    write_system_variable(REG_IDX_AVL_REP_COUNT, 1);
    write_system_variable(REG_IDX_AVL_USR_COUNT, 2);

    /* 1. Generazione fisica controllata di un record inorder di test per l'indice Report ID (22 byte) */
    FILE* f_rep = fopen(PATH_AVL_REPORT_ID, "wb");
    assert(f_rep != NULL);
    /* ID Report = 0000000777, Status = 1 (PROGRESS), Disk Row = 0000000045 */
    fprintf(f_rep, "%010u%c%010d\n", 777, '1', 45);
    fclose(f_rep);

    /* Test di ricerca dicotomica diretta su file via findReportId */
    int calculated_row = findReportId(777);
    assert(calculated_row == 45);
    
    int not_found_row = findReportId(999);
    assert(not_found_row == -1);

    /* 2. Generazione di un record a duplicazione contigua per l'indice User ID (21 byte) */
    FILE* f_usr = fopen(PATH_AVL_USER_ID, "wb");
    assert(f_usr != NULL);
    /* Utente 0000000088 possiede due report associati (0000000001 e 0000000002) */
    fprintf(f_usr, "%010u%010u\n", 88, 1);
    fprintf(f_usr, "%010u%010u\n", 88, 2);
    fclose(f_usr);

    unsigned int collected_results[10];
    /* Test di ricerca dicotomica con espansione bilaterale via findUserId */
    int total_found_matches = findUserId(88, collected_results);
    assert(total_found_matches == 2);
    assert(collected_results[0] == 1);
    assert(collected_results[1] == 2);

    printf("SUPERATO.\n");
}

/**
 * @brief Valida la consistenza e l'aggiornamento atomico dei registri statistici in O(1).
 */
static void test_statistical_registers_o1(void) {
    printf("[TEST] Validazione Indicatori Statistici Istantanei O(1)... ");
    
    remove(PATH_SEQUENCE);
    
    /* Configura inizialmente una variabile a zero */
    write_system_variable(REG_IDX_NM_REPORT, 0);
    unsigned int initial_val = read_system_variable(REG_IDX_NM_REPORT);
    assert(initial_val == 0);

    /* Effettua un incremento simulato per riflettere un inserimento di backend */
    write_system_variable(REG_IDX_NM_REPORT, initial_val + 5);
    unsigned int updated_val = read_system_variable(REG_IDX_NM_REPORT);
    assert(updated_val == 5);

    printf("SUPERATO.\n");
}





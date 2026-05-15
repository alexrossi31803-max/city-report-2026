#include "test_suite.h"
#include "../include/utils/parser.h"
#include "../include/adt/priority_queue.h"
#include "../include/adt/report_avl.h"
#include <stdio.h>
#include <string.h>

void print_test_result(const char* test_name, bool result) {
    printf("[TEST] %-30s %s\n", test_name, result ? "✅ PASSATO" : "❌ FALLITO");
}

/* 1. Test Geometria Parser (351 Byte) */
bool test_parser_geometry() {
    Report r = create_report(1, 101, "Test User", TRASPORTI, "Descrizione", "15/05/2026", '3', OPEN);
    char buffer[400]; // Buffer abbondante per sicurezza
    
    report_to_line(buffer, r, 'A');
    
    // Verifica lunghezza riga (350 caratteri + \n = 351)
    size_t len = strlen(buffer);
    bool length_ok = (len == 351);
    
    // Verifica presenza del terminatore di riga in posizione corretta
    bool newline_ok = (buffer[350] == '\n');
    
    free_report(r);
    return length_ok && newline_ok;
}

/* 2. Test Logica Priority Queue (Urgenza + FIFO) */
bool test_priority_queue_logic() {
    PriorityQueue pq = create_pq();
    
    // R1: Urgenza 2, Data più vecchia
    Report r1 = create_report(1, 10, "U1", AMBIENTE, "D1", "01/01/2026", '2', OPEN);
    // R2: Urgenza 3, Data più recente
    Report r2 = create_report(2, 20, "U2", AMBIENTE, "D2", "10/01/2026", '3', OPEN);
    // R3: Urgenza 2, Data più recente (deve andare dopo R1)
    Report r3 = create_report(3, 30, "U3", AMBIENTE, "D3", "05/01/2026", '2', OPEN);

    pq_enqueue(pq, r1);
    pq_enqueue(pq, r2);
    pq_enqueue(pq, r3);

    // Il primo estratto deve essere R2 (Urgenza 3)
    Report first = pq_dequeue(pq);
    bool first_ok = (get_report_id(first) == 2);

    // Il secondo estratto deve essere R1 (Urgenza 2, ma più vecchio di R3)
    Report second = pq_dequeue(pq);
    bool second_ok = (get_report_id(second) == 1);

    free_pq(pq);
    return first_ok && second_ok;
}

/* 3. Test AVL Search */
bool test_avl_search() {
    ReportAVL tree = create_avl();
    Report r = create_report(500, 1, "User", VERDE, "Desc", "15/05/2026", '1', OPEN);
    set_report_disk_row(r, 42); // Simuliamo che sia alla riga 42 del file master

    avl_insert_by_id(tree, r);
    
    int found_row = avl_search_disk_row(tree, 500);
    bool found_ok = (found_row == 42);
    bool not_found_ok = (avl_search_disk_row(tree, 999) == -1);

    free_avl(tree);
    free_report(r);
    return found_ok && not_found_ok;
}

/* --- Runner --- */
void run_all_tests() {
    printf("\n=== AVVIO TEST UNITARI DEL SISTEMA ===\n");
    
    print_test_result("Geometria Parser 351B", test_parser_geometry());
    print_test_result("Logica Priorita' (Urgenza+FIFO)", test_priority_queue_logic());
    print_test_result("Ricerca AVL", test_avl_search());
    
    printf("=======================================\n\n");
}

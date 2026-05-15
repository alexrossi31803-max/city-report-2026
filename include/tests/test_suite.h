#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <stdbool.h>

/**
 * @brief Esegue tutti i test del sistema.
 */
void run_all_tests();

/* --- Test Moduli Specifici --- */

/**
 * @brief Testa la geometria a 351 byte del Parser.
 */
bool test_parser_geometry();

/**
 * @brief Testa il bilanciamento dell'AVL (Rotazioni LL, RR).
 */
bool test_avl_balancing();

/**
 * @brief Testa l'ordinamento Urgenza + FIFO della Priority Queue.
 */
bool test_priority_queue_logic();

/**
 * @brief Testa il caricamento e la ricerca degli utenti.
 */
bool test_user_manager();

/**
 * @brief Testa il meccanismo di Flush e gestione buchi (null_pointer.txt).
 */
bool test_report_flush_logic();

/* --- Utility per i Test --- */
void print_test_result(const char* test_name, bool result);

#endif

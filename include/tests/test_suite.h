#ifndef TEST_SUITE_H
#define TEST_SUITE_H

/**
 * @brief Esegue l'intera suite di test di conformità per le nuove geometrie AVL.
 * @post Stampa a video l'esito formale di ciascun test case logico.
 */
void run_all_tests();

/* Singoli casi di test mappati sui requisiti e le geometrie a 352 byte */
void test_user_registration_and_login();
void test_report_registration_and_ram_list();
void test_revert_stack_logic();
void test_report_state_update_and_file_transfer();
void test_priority_queue_sorting();
void test_report_avl_indexing();
void test_statistical_report_generation();

#endif


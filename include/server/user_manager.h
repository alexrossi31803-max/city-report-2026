#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "../models/user.h"
#include <stdbool.h>

/**
 * @brief Tenta il login di un utente.
 * Carica i dati dal file Master_Files/users.txt verificando username e password.
 * * @param username Nome utente inserito
 * @param password Password inserita
 * @return Oggetto User se le credenziali sono corrette, NULL altrimenti.
 */
User login_user(const char* username, const char* password);

/**
 * @brief Registra un nuovo utente nel sistema.
 * Controlla l'unicità dell'username, assegna un ID incrementale e 
 * scrive i dati in coda al file users.txt. Aggiorna anche users_idx.txt.
 * * @param username Nome utente desiderato
 * @param password Password scelta
 * @param role Ruolo assegnato (CITIZEN o EMPLOYEE)
 * @return true se la registrazione ha successo, false se l'utente esiste già.
 */
bool register_user(const char* username, const char* password, UserRole role);

/**
 * @brief Verifica se un username è già presente nel database.
 * @return true se occupato, false se disponibile.
 */
bool is_username_taken(const char* username);

/**
 * @brief Restituisce il numero totale di utenti registrati.
 */
unsigned int get_user_count();

#endif

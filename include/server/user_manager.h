#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "../models/user.h"
#include <stdbool.h>

/**
 * @brief Restituisce la capacita attuale (numero totale di slot allocati) della tabella hash.
 *        Legge l'indice binarizzato calcolando le righe fisse da 6 byte (USER_IDX_LINE).
 * @return Numero di slot totali preallocati nell'indice.
 */
int get_users_hash_capacity(void);

/**
 * @brief Registra un nuovo utente nel sistema municipale gestendo l'anagrafica.
 *        Calcola la posizione tramite hashing DJB2 in O(1) e applica il Linear Probing
 *        su blocchi rigidi da 6 byte per intercettare all'istante i duplicati.
 * @param username Stringa dell'identificatore account scelto dal cittadino/dipendente.
 * @param password Stringa della chiave d'accesso scelta.
 * @param role Enumerazione del ruolo istituzionale (CITIZEN o EMPLOYEE).
 * @return true ad avvenuta registrazione senza collisioni anagrafiche, false se duplicato.
 */
bool register_user(const char* username, const char* password, UserRole role);

/**
 * @brief Esegue l'autenticazione istantanea nel sistema in tempo costante medio O(1).
 *        Effettua un salto geometrico mirato sull'indice e sul file dati a 107 byte,
 *        istanziando l'oggetto User opaco in caso di corrispondenza delle credenziali.
 * @param username Stringa dell'account inserito nella UI.
 * @param password Stringa della password inserita nella UI.
 * @return Puntatore opaco alla struttura User se le credenziali sono valide, NULL altrimenti.
 */
User login_user(const char* username, const char* password);

#endif



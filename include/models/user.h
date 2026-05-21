#ifndef USER_H
#define USER_H

#include "../config.h"

/**
 * @brief Struttura opaca (Opaque Pointer) per l'oggetto User.
 *        Realizza l'Information Hiding oscurando le variabili membro private all'esterno.
 */
typedef struct UserStruct* User;

/**
 * @brief Costruttore dell'ADT User. Alloca la memoria e valorizza i campi dell'utente loggato.
 * @param id Indice numerico finto database che identifica l'utente.
 * @param username Stringa dell'account utente.
 * @param password Stringa della password utente.
 * @param role Enumerazione del livello istituzionale di accesso (CITIZEN o EMPLOYEE).
 * @return Un'istanza valida di tipo User inizializzata in RAM, o NULL in caso di fallimento malloc.
 */
User create_user(int id, const char* username, const char* password, UserRole role);

/**
 * @brief Distruttore dell'ADT User. Esegue la deallocazione atomica della memoria RAM occupata.
 * @param u Il puntatore all'istanza dell'oggetto User da distruggere.
 */
void free_user(User u);

/* --------------------------------==============================================
 *  METODI GETTER PRIVILEGIATI (ESPORTANO I DATI IN SOLA LETTURA)
 * --------------------------------============================================== */

/**
 * @brief Restituisce l'ID numerico interno dell'utente.
 * @param u L'istanza dell'oggetto User.
 * @return L'identificatore numerico intero.
 */
int get_user_id(User u);

/**
 * @brief Restituisce il puntatore alla stringa dell'username dell'utente.
 * @param u L'istanza dell'oggetto User.
 * @return Stringa costante (sola lettura) memorizzata nel nodo.
 */
const char* get_user_username(User u);

/**
 * @brief Restituisce il puntatore alla stringa della password dell'utente.
 * @param u L'istanza dell'oggetto User.
 * @return Stringa costante (sola lettura) memorizzata nel nodo.
 */
const char* get_user_password(User u);

/**
 * @brief Restituisce il ruolo associato al profilo (CITIZEN o EMPLOYEE).
 * @param u L'istanza dell'oggetto User.
 * @return L'enumerazione UserRole di controllo accessi.
 */
UserRole get_user_role(User u);

#endif


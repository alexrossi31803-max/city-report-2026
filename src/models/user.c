#include "../../include/models/user.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Struttura interna dell'oggetto User (Definizione fisica nascosta).
 *        L'allocazione statica interna delle stringhe impedisce frammentazione della RAM.
 */
struct UserStruct {
    int id;                       /* L'indice progressivo geometrico sul finto database */
    char username[MAX_USERNAME + 1];  /* Array statico protetto dimensionato da config.h più \0 */
    char password[MAX_PASSWORD + 1];  /* Array statico protetto dimensionato da config.h più \0 */
    UserRole role;                /* Ruolo istituzionale (EMPLOYEE o CITIZEN) */
};

User create_user(int id, const char* username, const char* password, UserRole role) {
    /* Allocazione dinamica protetta dello spazio della struttura privata in RAM */
    User u = (User)malloc(sizeof(struct UserStruct));
    if (!u) return NULL; /* Ritorno preventivo di sicurezza in caso di crash della memoria */

    u->id = id;
    
    /* Hard copy bloccata delle stringhe per isolare l'oggetto da vettori volatili dell'interfaccia */
    strncpy(u->username, username, MAX_USERNAME);
    u->username[MAX_USERNAME] = '\0'; /* Sigillo obbligatorio del terminatore di stringa */
    
    strncpy(u->password, password, MAX_PASSWORD);
    u->password[MAX_PASSWORD] = '\0';

    u->role = role;
    return u; /* Restituisce il puntatore alla cella di memoria opaca */
}

void free_user(User u) {
    /* Previene tentativi di doppia deallocazione distruttiva della memoria (Double Free) */
    if (u != NULL) {
        free(u); /* Libera l'intero blocco strutturale allocato dal costruttore */
    }
}

/* --------------------------------==============================================
 *  IMPLEMENTAZIONE DEI METODI GETTER (PROTEZIONE INCAPSULAMENTO)
 * --------------------------------============================================== */

int get_user_id(User u) {
    /* Restituisce la copia del valore isolando la variabile d'istanza privata */
    return u ? u->id : -1;
}

const char* get_user_username(User u) {
    /* Restituisce il puntatore costante in sola lettura impedendo alterazioni esterne maliziose */
    return u ? u->username : NULL;
}

const char* get_user_password(User u) {
    return u ? u->password : NULL;
}

UserRole get_user_role(User u) {
    return u ? u->role : CITIZEN;
}



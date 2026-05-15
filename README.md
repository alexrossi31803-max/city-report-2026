# City Report 2026 - Sistema di Gestione Segnalazioni Urbane

City Report 2026 è un sistema gestionale sviluppato in **C** per la gestione delle segnalazioni cittadine. Il software permette ai cittadini di inviare segnalazioni e ai dipendenti comunali di gestirle in base a criteri di urgenza e cronologia, garantendo persistenza dei dati e performance ottimali tramite indici AVL.

---

##  Funzionalità Principali

###  Area Cittadino
* **Registrazione e Login**: Accesso sicuro al sistema.
* **Invio Segnalazioni**: Creazione di segnalazioni con categoria, descrizione e livello di urgenza.
* **Undo System**: Possibilità di annullare le ultime azioni eseguite durante la sessione corrente tramite logica Stack (LIFO).
* **Session Persistence**: I dati vengono salvati permanentemente solo al momento del logout.

###  Area Dipendente
* **Dashboard Prioritaria**: Visualizzazione delle segnalazioni pendenti ordinate per **Urgenza** (Alta > Media > Bassa) e, a parità di urgenza, per **Data di invio** (FIFO).
* **Gestione Stati**: Avanzamento delle segnalazioni da "Aperta" a "In Lavorazione" fino a "Chiusa".
* **Ricerca Rapida**: Accesso istantaneo ai report tramite ID grazie all'indicizzazione AVL.

---

##  Architettura Tecnica
Il progetto implementa concetti avanzati di strutture dati e gestione del file system:
* **Persistenza a 351 Byte**: Record a lunghezza fissa per accesso casuale $O(1)$ tramite `fseek`.
* **Indici AVL**: Ricerca in tempo logaritmico $O(\log n)$ degli ID e degli utenti.
* **Gestione dei Buchi**: Recupero dello spazio dei record cancellati tramite file `null_pointer.txt`.
* **Layered Design**: Separazione netta tra Modelli, ADT, Utility e Logica Server.

---

##  Struttura del Progetto
```text
.
├── include/           # Header files (.h)
│   ├── adt/           # Strutture dati (AVL, PQ, List, Stack)
│   ├── models/        # Modelli dati (Report, User)
│   ├── server/        # Logica di gestione (ReportManager, UserManager)
│   └── utils/         # Validatori e Parser
├── src/               # Sorgenti (.c)
├── database/          # File di persistenza (.txt)
│   ├── Master_Files/  # Database principali (Open, InProgress, Closed)
│   └── Derived_Files/ # Indici, Bench e File dei buchi
├── tests/             # Suite di test unitari
├── docs/              # Documentazione tecnica (Analysis, Architecture, ADT)
└── main.c             # Entry point del programma
```
## Installazione e Compilazione
###Clona la repository:

Bash ```
git clone [https://github.com/alexrossi31803-max/city-report-2026.git](https://github.com/alexrossi31803-max/city-report-2026.git)
cd city-report-2026
```
---
###Compilazione:
È possibile compilare il progetto utilizzando gcc:

Bash ```
gcc -o city_report main.c src/**/*.c -I./include
```
---
###Esecuzione:

Bash```
./city_report
```
---
## Documentazione Approfondita
Per maggiori dettagli tecnici, consulta la cartella docs/:

* Analisi Tecnica

* Architettura del Sistema

* Panoramica ADT

---
## Autore
* **Alessandro Rossi** - Sviluppo e Progettazione

* **Anno Accademico**: 2025/2026

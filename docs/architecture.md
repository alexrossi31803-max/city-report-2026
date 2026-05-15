# Architettura del Software - City Report 2026

Il sistema è basato su un'architettura **modulare a livelli**, progettata per garantire la separazione delle responsabilità (Separation of Concerns) e facilitare la manutenzione.

---

## 1. Schema dei Livelli (Layered Architecture)

### 1.1 Livello di Presentazione (UI/Main)
* **Modulo:** `main.c`
* **Responsabilità:** Gestione dei menu interattivi, acquisizione input utente, gestione della sessione corrente.
* **Interazione:** Comunica esclusivamente con il livello *Server Logic*.

### 1.2 Livello Server Logic (Managers)
* **Moduli:** `user_manager.c`, `report_manager.c`
* **Responsabilità:** * `UserManager`: Autenticazione e registrazione.
    * `ReportManager`: Orchestrazione del database, gestione della Bench, esecuzione del Flush e coordinamento degli indici AVL.

### 1.3 Livello di Supporto (ADT & Utils)
* **Moduli:** `avl.c`, `priority_queue.c`, `parser.c`, `validators.c`
* **Responsabilità:** * Fornire strutture dati ottimizzate per la ricerca e la priorità.
    * Gestire la trasformazione dei dati (Serializzazione/Deserializzazione) e la loro validità.

---

## 2. Flusso di Controllo: Il Ciclo di una Segnalazione

L'architettura gestisce la segnalazione attraverso quattro stati principali, garantendo l'integrità del database anche in caso di errori utente.

1.  **Input & Validation**: L'utente inserisce i dati; il `main` invoca i `validators` per garantire la compatibilità con la geometria a 351 byte.
2.  **Session Buffer (RAM)**: Il report viene inserito in una `ReportList` e in un `ReportStack`. Qui l'utente può eseguire l'operazione di **Undo**.
3.  **Transit Stage (Bench)**: Al logout, il report viene "parcheggiato" nel file `reports_bench.txt`.
4.  **Final Commit (Flush)**: Il `ReportManager` sposta i dati dalla Bench ai file Master, aggiornando simultaneamente l'albero AVL in memoria.

---

## 3. Gestione della Memoria e Persistenza

### 3.1 Indici AVL (Memory-Mapped Logic)
Per evitare scansioni lineari dei file master (operazione $O(n)$), l'architettura prevede il caricamento degli indici in RAM all'avvio:
* `AVL_ID`: Mappa `ID_Report` -> `Riga_Disco`.
* `AVL_USER`: Mappa `ID_User` -> `Lista_ID_Report`.

### 3.2 Geometria del File Master
L'architettura impone che ogni file master (`open.txt`, `progress.txt`, `closed.txt`) sia un array di record di dimensione fissa. 
* **Offset Calculation**: `Posizione = Indice_Riga * 351`.
* Questo permette l'aggiornamento "in place" dello stato di una segnalazione senza dover riscrivere l'intero file.

---

## 4. Diagramma delle Dipendenze

```text
[ main.c ]
    |
    +--> [ user_manager.h ]  --> [ user.h ]
    |
    +--> [ report_manager.h ] --> [ report.h ]
               |
               +--> [ report_avl.h ]
               +--> [ parser.h ]
               +--> [ validators.h ]
```
---
## 5. Gestione degli Errori e Robustezza
* **Null Pointer Management**: L'architettura prevede file dedicati (null_pointer.txt) per tracciare i record cancellati, permettendo al sistema di auto-riparare i "buchi" nel database durante il normale funzionamento.

* **Opaque Pointers**: Tutte le strutture dati (ADT) utilizzano puntatori opachi per nascondere l'implementazione interna ai manager, riducendo l'accoppiamento (Coupling).
# Analisi Tecnica del Sistema "City Report 2026"

## 1. Architettura del Sistema
Il sistema adotta un'architettura **stratificata** per separare la logica di presentazione (UI), la logica di business (Server/Managers) e la persistenza (Filesystem). 

### 1.1 Flusso dei Dati
Il ciclo di vita di una segnalazione segue tre stadi di memoria:
1.  **Volatile (RAM)**: Memorizzazione in una `ReportList` durante la sessione attiva dell'utente. Permette operazioni di **Undo** tramite un `ReportStack`.
2.  **Transitoria (Bench)**: Al logout, i dati vengono scritti in `reports_bench.txt`. Questo agisce come buffer per ottimizzare le operazioni di I/O.
3.  **Persistente (Master)**: Al raggiungimento della soglia critica, avviene il **Flush**. I dati vengono smistati nei file master (`open.txt`, `progress.txt`, `closed.txt`) e gli indici AVL vengono aggiornati.

---

## 2. Geometria del Database (Fixed-Length Records)
Per garantire performance $O(1)$ nell'accesso ai file tramite `fseek`, è stata definita una geometria rigorosa a **351 byte** per record.

| Campo | Dimensione (Byte) | Tipo/Formato | Descrizione |
| :--- | :--- | :--- | :--- |
| ID | 10 | `%010u` | Identificativo univoco report |
| UserID | 10 | `%010u` | ID dell'autore |
| Nome | 50 | `%-50s` | Nome del cittadino (padding spazi) |
| Categoria | 1 | `char` | Codice categoria (0-3) |
| Descrizione | 256 | `%-256s` | Testo della segnalazione |
| Data | 11 | `%-11s` | Formato GG/MM/AAAA |
| Urgenza | 1 | `char` | Livello 1, 2 o 3 |
| Stato | 1 | `char` | O (Open), P (Progress), C (Closed) |
| Riga Disco | 10 | `%010d` | Indice fisico nel file master |
| Cell Status | 1 | `char` | A (Active), V (Void/Buco), E (End) |
| Terminator | 1 | `\n` | Carattere di fine riga |

---

## 3. Gestione Efficiente dello Spazio
### 3.1 Meccanismo dei "Buchi" (Hole Recovery)
Invece di cancellare fisicamente i record (operazione $O(n)$ che richiederebbe la riscrittura del file), il sistema:
1.  Marca la cella come 'V' (Void).
2.  Salva l'indice della riga nel file `null_pointer.txt` relativo allo stato.
3.  In fase di inserimento, consulta il file dei buchi con logica **LIFO** (Last-In First-Out) per riutilizzare lo spazio.

---

## 4. Strutture Dati e Complessità Algoritmica

### 4.1 AVL Tree (Indici di Ricerca)
Utilizzati per mappare gli ID alle posizioni fisiche su disco.
* **Search**: $O(\log n)$.
* **Insert**: $O(\log n)$ con rotazioni per il bilanciamento.
* **Memory**: Mantiene solo ID e Puntatore a riga, minimizzando l'occupazione RAM.

### 4.2 Priority Queue (Dashboard Dipendente)
Gestisce l'ordine di intervento basato su un criterio incrociato:
1.  **Priorità Primaria**: Urgenza (3 > 2 > 1).
2.  **Priorità Secondaria**: FIFO (Data più vecchia prima).
* Implementata come lista ordinata per facilità di gestione delle stringhe data.

---

## 5. Requisiti di Sicurezza e Integrità
* **Validazione**: Ogni input è filtrato da `validators.c` per prevenire Buffer Overflow e corruzione della geometria del file.
* **Atomicità**: Il Flush avviene solo su record integri. Se un'operazione di scrittura fallisce, la Bench conserva i dati originali.


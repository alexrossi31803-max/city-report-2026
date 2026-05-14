#  Schema Strutturale, Diagrammi dei Flussi e Ciclo di Vita dei Report

Questo documento descrive l'architettura logica e fisica del sistema, illustrando visivamente come i dati transitano e si sincronizzano tra i vari livelli di memoria.

---

## 1. Schema Architetturale dei Componenti

Il sistema adotta un disaccoppiamento rigido per impedire la manipolazione incontrollata dei file da parte dell'interfaccia utente, sfruttando gli **Opaque Pointers**:

```text
+-------------------------------------------------------+

|                       MAIN.C                          |
|  (Interfaccia Utente Testuale / Menu Cittadino e Dip) |
+---------------------------+---------------------------+

                            |
                            | (Passa solo Opaque Pointers)
                            v
+-------------------------------------------------------+

|               CONTROLLORI SERVER                      |
|   (user_manager.c  <--->  report_manager.c)           |
+-----------+----------------+----------------+---------+

            |                |                |
            v                v                v
+--------------+     +--------------+     +-------------+

| UTILS/       |     | ADT/         |     | DATABASE/   |
| (parser.c,   |     | (lists, bst, |     | (.txt fisse |
|  validators) |     |  stack, pq)  |     |  binari)    |
+--------------+     +--------------+     +-------------+
```

---

## 2. Diagramma Visivo del Ciclo di Vita di un Report

La tabella e il flusso sottostanti illustrano la migrazione di una segnalazione attraverso le strutture dati e la gestione geometrica dei file.

###  Mappa dei Livelli Hardware di Persistenza


| Fase Operativa / Indice | Struttura Dati / File | Dimensione Record | Logica di Accesso | Stato e Indicatori |
| :--- | :--- | :---: | :---: | :--- |
| **1. RAM Locale** | `ReportList` (Dinamica) | Puntatore Opaco | \(O(1)\) in testa | Stato iniziale. Riga disco = `-1`. |
| **2. Cache Server**| `reports_bench.txt` (Flat) | 332 Byte (Fisso) | \(O(1)\) via `counter` | Buffer statico da 50 slot. Incremento solo su aggiunte. |
| **3. DB Master Storico** | `open / progress / closed` | 332 Byte (Fisso) | \(O(1)\) via `disk_row` | Flag Cella: `'A'` (Attivo), `'V'` (Vuoto), `'E'` (Sentinella). |
| **4. Indice Report ID (BST)**| `report_BST_BY_REPORT_ID`| 332 Byte (Fisso) | \(O(\log n)\) binario | Punto di Verità. Rigenerato a blocchi fissi `fread`. |
| **5. Indice User ID (BST)**  | `report_BST_BY_USER_ID`  | **12 Byte (Fisso)** | \(O(\log n)\) binario | Rigenerato in logica contratta `[ID_USER(5)][ID_REPORT(5)]\n`. |

###  Diagramma di Flusso Geometrico (RAM -> Cache -> Master)

```text
  [ CITTADINO COMPILA ]

            |
            v
+-----------------------+
|   ReportList (RAM)    | <---> [ ReportStack (RAM) ] (Undo / Revert LIFO)
+-----------------------+

            |
            | (Logout: flush_session_to_bench)
            v
+-------------------------------------------------------------------------+

|                      REPORTS_BENCH.TXT (CACHE DISCO)                    |
| [Slot 0] [Slot 1] ... [Slot 49]  <-- Gestito da counter circolare (0-49)|
+-------------------------------------------------------------------------+

            |
            | (Raggiungimento Quota 50: process_and_flush_bench)
            v
    [ANALISI RECORDFLAG]

            |
            +---> Se record storico modificato (Riga != -1)
            |     |
            |     v

            |     Apre il vecchio file master tramite lo stato originale ricavato dal BST,
            |     esegue salto fseek(Riga * 332) e scrive 'V' (Genera Buco in O(1)).
            |
            +---> Per tutti i record attivi da consolidare

                  |
                  v
                  Trova 'V' (Primo Buco) o 'E' (EOF) nel NUOVO file master di stato.
                  Scrive il report, imposta flag 'A', inietta la riga nel report
                  e risigilla spostando la sentinella 'E' di fine dati logici.
```

---

## 3. Diagramma Generale del Flusso di Programma

Questo diagramma riassume i percorsi logici eseguiti dall'applicazione a seconda della sessione utente e dei meccanismi di sincronizzazione degli indici.

```text
               +----------------------------------+
               |         AVVIO APPLICAZIONE       |
               +----------------+-----------------+

                                |
                                v
               +----------------------------------+
               |          MENU PRINCIPALE         |
               +----------------+-----------------+

                                |
        +-----------------------+-----------------------+
        |                                               |
        v (Login Cittadino)                             v (Login Dipendente)
+-----------------------+                       +-----------------------+

|     AREA CITTADINO    |                       |    AREA DIPENDENTE    |
+-----------------------+                       +-----------------------+

        |                                               |
        +---> 1. Inserisce in RAM (Id, Riga=-1)         +---> 1. Viste Filtrate (1, 2, 3)

        |                                               |      (Scansiona BENCH [O(counter)]
        +---> 2. Modifica in RAM                        |       + File Master con fread)

        |     (Deep copy in Undo Stack)                 |
        |                                               +---> 2. Modifica Stato Report
        +---> 3. Visualizza Storico                     |      |

        |     |                                         |      v
        |     +-- Cerca in RAM / BENCH [O(n)]           |      Cerca in BENCH [O(50)].
        |     |                                         |      Se assente, cerca in
        |     +-- Interroga bst_by_user_id [O(log n)]   |      bst_by_report_id [O(log n)]
        |     |   per estrarre array di ID ridotti      |      e lo copia nella BENCH.
        |     |                                         |      Applica cambio stato.
        |     +-- Risolve ogni ID in tempo reale        |      Inietta la modifica nel nodo del
        |         su bst_by_report_id [O(log n)]        |      bst_by_report_id [O(log n)].
        |                                               |      Il counter BENCH NON cresce.
        +---> 4. Logout (Flush in BENCH)                |

              |                                         +---> 3. Richiesta Coda Priorità (5)
              v                                                |
              Incrementa counter BENCH +1.                     v
              Se counter == 50:                                Forza Flush BENCH.
              |                                                Carica file master attivi.
              v                                                Popola PriorityQueue.
        [ FLUSH PESANTE ]                                      Genera file pre-ordinato
        1. Svuota BENCH riciclando i buchi 'V'.                reports_by_priority.txt.
        2. Rigenera bst_by_report_id (Da file master).
        3. Rigenera bst_by_user_id (Solo ID Utente/Report).
        4. Riposiziona counter BENCH = 0.
```

---

###  Generazione ID Univoci tramite Registro di Testo Binarizzato O(1)

Per azzerare il rischio di collisioni indotto dal riciclo geometrico dei buchi dei file di stato, il server centralizza l'assegnazione dei codici nel file di testo binarizzato `system_total_report.txt` (posizionato in `database/Master_Files/`). 

Ogni volta che viene creata una nuova segnalazione in RAM, il sistema esegue una lettura diretta del contatore storico in tempo costante **\(O(1)\)**, applica un incremento atomico e risalta sul disco per memorizzare il nuovo valore (es. passando da `0` a `1`). Questo sblocca un disaccoppiamento totale dai flussi transitori della BENCH o dei file master, garantendo l'unicità matematica di ciascun `report_id` emesso nella storia del Comune.

---

## 4. Analisi d'Integrità e Vantaggi del Modello Geometrico

1. **Integrità negli Spostamenti:** Separando i file fisici per stato, il rischio storico era di perdere record o sfasare gli indici durante i passaggi di stato. Iniettando il numero di riga fisica nel report, il server cancella il vecchio record in **\(O(1)\)** e ricicla lo spazio tramite l'algoritmo del primo buco libero, mantenendo i file compatti.
2. **Prevenzione della Corruzione del Disco:** La sentinella speciale `'E'` arresta immediatamente i cicli di scansione ed analisi `fread` o `fgets`. Qualsiasi frammento di spazzatura o residuo binario rimasto oltre la fine logica dei dati non viene mai interpretato dal parser, blindando l'I/O del server.
3. **Consistenza dello Stato per il Cittadino:** Il fatto che l'albero utente (`bst_by_user_id`) memorizzi solo vettori di codici contratti da 12 byte e non oggetti completi costringe il sistema a risolvere gli attributi del problema sul BST principale del Report ID. Di conseguenza, se un dipendente chiude una pratica, il cittadino ne visualizza all'istante lo stato aggiornato, azzerando le asimmetrie informative.



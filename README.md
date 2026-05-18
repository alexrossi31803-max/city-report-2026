# Sistema di Gestione Segnalazioni Municipali - Comune di Baronissi (v5.0)

Ecosistema software modulare sviluppato in standard **C99** per la gestione, lo sfoltimento, l'indicizzazione e il riciclo geometrico delle segnalazioni di anomalia urbana.

L'architettura simula un **Database Relazionale a geometria rigida** basato su file piatti di testo, escludendo qualsiasi degradamento prestazionale indotto da scansioni orizzontali in `O(n)` sugli archivi storici e implementando un totale isolamento dei dati tramite **Opaque Pointers (Information Hiding)**.

---

#  Caratteristiche Tecniche Principali

## Persistenza Hardware ad Asimmetria Fisica

- Separazione netta tra:
  - cache operativa volatile → **351 byte**
  - archivio Master stazionario → **352 byte**
- Presenza del flag di attivazione cella al byte offset `350`

---

## Algoritmo di Riciclo Chirurgico dei Buchi in `O(1)`

- I record modificati o rimossi generano celle vuote marcate `'N'`
- Gli indici fisici vengono salvati in uno:
  - **Stack LIFO** ausiliario
- Il server può:
  - sovrascrivere i buchi
  - riutilizzare gli slot
  - evitare scansioni orizzontali

Prestazione:
```text
Tempo costante O(1)
```

---

## Ricerca Dicotomica su File d'Indice in `O(log n)`

Gli indici prodotti dagli AVL auto-bilancianti vengono salvati come:

- `Inorder Arrays`
- sequenze continue su disco

Ricerca eseguita tramite:

- salti dicotomici
- accesso diretto ai file indice

Geometrie:

| File indice | Dimensione Riga |
|---|---|
| User ID | 21 byte |
| Report ID | 22 byte |

---

## Dashboard Statistica Atomica in `O(1)`

Registro centrale statico:

- 11 byte per riga
- 13 indicatori di sistema

Traccia:

- ID progressivo
- saturazione cache
- contatori per stato
- contatori per categoria
- consistenza AVL

Prestazione:

```text
Rendering istantaneo O(1)
```

---

# 📂 Organizzazione dei File e Geometrie Rigide

```text
database/
├── Master_Files/
│   ├── users.txt
│   │   └── Anagrafica Utenti (107 byte fissi)
│   │
│   ├── users_idx.txt
│   │   └── Indice Hash Utenti (6 byte fissi)
│   │
│   ├── open_reports.txt
│   │   └── Master Segnalazioni Aperte (352 byte)
│   │
│   ├── in_progress_reports.txt
│   │   └── Master Segnalazioni in Lavorazione (352 byte)
│   │
│   ├── closed_reports.txt
│   │   └── Master Segnalazioni Chiuse (352 byte)
│   │
│   ├── open_holes.txt
│   │   └── Stack LIFO Buchi Open (11 byte)
│   │
│   ├── in_progress_holes.txt
│   │   └── Stack LIFO Buchi Progress (11 byte)
│   │
│   ├── closed_holes.txt
│   │   └── Stack LIFO Buchi Closed (11 byte)
│   │
│   └── system_total_report.txt
│       └── Registro Statistico Centrale (13 righe da 11 byte)
│
└── Derived_Files/
    ├── reports_bench.txt
    │   └── Cache Operativa Server (50 slot da 351 byte)
    │
    ├── report_AVL_BY_REPORT_ID.txt
    │   └── Indice Ordinato per Report ID (22 byte)
    │
    └── report_AVL_BY_USER_ID.txt
        └── Indice Ordinato per User ID (21 byte)
```

---

#  Architettura della Memoria a Tre Livelli

## 1. RAM Volatile di Sessione (Cittadino)

Gestita tramite:

- `ReportList`
  - lista concatenata semplice
  - inserimento in testa

- `ReportStack`
  - array statico
  - undo profondo
  - massimo 10 elementi

Le modifiche RAM avvengono solo sui report:

```text
OPEN
```

---

## 2. Cache Vettoriale Statica (BENCH)

Al logout:

- i record vengono riversati in:
  - `reports_bench.txt`

Geometria:

```text
351 byte per slot
```

Comportamento:

- se il report esiste:
  - la cella viene riscritta sul posto
- gli incrementi anagrafici vengono azzerati

Al raggiungimento di:

```text
50 slot
```

si attiva automaticamente:

```text
Flush Pesante
```

---

## 3. Database Master e Rigenerazione Indici

Il flush:

- smista i record validi nei Master
- ricicla i buchi tramite Stack LIFO
- elimina fisicamente i record `DESTROYED`

Successivamente:

- il server rilegge i Master
- rigenera gli indici AVL
- esegue visite:
  - simmetriche
  - `In-Order`

Aggiorna inoltre:

- contatori posizionali
- metadati statistici

---

# 🛠️ Compilazione ed Esecuzione

Il progetto adotta:

- architettura modulare
- standard rigoroso C99
- nessuna dipendenza esterna

## Compilazione

```bash
gcc -Wall -Wextra -pedantic -std=c99 -Iinclude src/main.c src/models/user.c src/models/report.c src/adt/report_list.c src/adt/report_stack.c src/adt/report_avl.c src/adt/priority_queue.c src/utils/validators.c src/utils/parser.c src/server/user_manager.c src/server/report_manager.c src/tests/test_suite.c -o segnalazioni_municipali.exe
```

---

## Esecuzione Interattiva

```bash
./segnalazioni_municipali.exe
```

---

## Validazione del Sistema

Per validare:

- integrità logica
- consistenza hardware
- correttezza asintotica
- strutture dati
- pipeline AVL
- riciclo geometrico

eseguire la suite di test integrata.

---




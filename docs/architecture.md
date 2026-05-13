# Questo documento illustra la struttura del sistema, i flussi operativi dei dati e il funzionamento dell'architettura a tre livelli di memoria tra RAM, file di supporto e database master.

---

# 🏢 1. Schema Architetturale dei Componenti

Il sistema è strutturato secondo una netta separazione dei ruoli per impedire la manipolazione incontrollata dei file:

```text
+-------------------------------------------------------+
|                       MAIN.C                          |
|  (Interfaccia Utente Testuale / Menu Cittadino e Dip)|
+---------------------------+---------------------------+
                            |
                            | (Passa solo Opaque Pointers)
                            v
+---------------------------+---------------------------+
|               CONTROLLORI SERVER                      |
|   (user_manager.c  <--->  report_manager.c)          |
+-----------+----------------+----------------+---------+
            |                |                |
            v                v                v
+--------------+     +--------------+     +-------------+
| UTILS/       |     | ADT/         |     | DATABASE/   |
| (parser.c)   |     | (lists, bst) |     | (.txt fisse)|
+--------------+     +--------------+     +-------------+
```

Usa il codice con cautela.

---

# 🔄 2. Ciclo di Vita dei Report (Meccanismo di Flushing e Soglia)

Il sistema mitiga il costo delle scritture su disco delegando le operazioni a strutture intermedie:

---

## Fase Locale (RAM)

Il cittadino compila una segnalazione.

Questa risiede esclusivamente nella `ReportList` in RAM.

Se l'utente commette errori, modifica il record; la versione precedente viene salvata nel `ReportStack`.

L'azione di Undo estrae dallo stack e ripristina la lista.

Il database non viene toccato.

---

## Fase Transitoria (Cache su File)

Al momento del logout, la lista RAM viene riversata nel file `reports_bench.txt` (capacità 50 righe a lunghezza fissa da 281 caratteri).

Questo file funge da "lavagna di supporto" comune.

Qui le segnalazioni `OPEN` sono ancora modificabili ed eliminabili (marcando l'ultimo carattere del record su `'E'`).

---

## Fase Consolidata (Flushing & Svuotamento)

Quando il `reports_bench.txt` è saturo (vicino a 50 elementi), il server avvia il flushing pesante:

- I record marcati con `'E'` vengono distrutti.
- I restanti record attivi vengono smistati in base al loro stato nei file sequenziali incrementali:
  - `open_latest.txt`
  - `in_progress_latest.txt`
  - `closed_latest.txt`

---

## Sincronizzazione Pesante ed Azzeramento (Soglia)

Il server mantiene un contatore cumulativo delle modifiche.

Ogni 50 elementi aggiunti nei file `_latest.txt`, il sistema azzera il contatore e avvia la rigenerazione di sfondo delle strutture derivate:

- Legge tutti i file master, crea in RAM il grande albero strutturato e sovrascrive il file `report_BST_ID_USER.txt` con la nuova vista In-Order, garantendo ricerche storiche in \(O(\log n)\).

- Filtra i record escludendo i casi `CLOSED`, li ordina nella coda e rigenera l'array lineare statico ordinato `reports_by_priority.txt` per la dashboard del dipendente.

---

# 🔐 3. Sicurezza e Accesso Diretto dei File di Testo

Tutti i file all'interno del sistema (anagrafiche ed elenchi) utilizzano stringhe a dimensione fissa integrate da caratteri di spazio `" "` (padding).

Questo approccio garantisce che:

- ogni riga utente occupi sempre 107 caratteri
- ogni riga report occupi sempre 281 caratteri

Grazie a questa geometria speculare sul disco, l'applicazione può usare la funzione:

```c
fseek(file, indice * dimensione_riga, SEEK_SET)
```

bypassando completamente la lettura sequenziale del testo.

Ciò assicura che il controllo delle credenziali e le interrogazioni per codice identificativo avvengano in tempo costante \(O(1)\), simulando il comportamento prestazionale di un database di livello enterprise su semplici file `.txt` leggibili.
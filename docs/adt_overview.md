# Panoramica degli Tipi Astratti di Dato (ADT Opachi v5.0 Definitiva)

Il sistema adotta una scomposizione modulare rigida basata sullo standard C99. Ogni Tipo Astratto di Dato (ADT) è esposto all'esterno unicamente tramite un **Puntatore Opaco** (*Opaque Pointer*), nascondendo la definizione fisica delle strutture concrete all'interno dei file `.c`. Questo garantisce il principio dell'**Information Hiding**, impedendo alterazioni maliziose della memoria da parte dell'interfaccia utente.

---

## 1. ADT ReportList (Linked List Dinamica)
* **File Header:** `include/adt/report_list.h`
* **File Sorgente:** `src/adt/report_list.c`
* **Definizione Fisica:** Struttura concatenata semplice con puntatore di controllo alla testa (`head`), iteratore interno per i cicli (`current`) e variabile pre-calcolata di dimensione (`size`).
* **Operazioni Chiave:**
  * `list_insert(ReportList l, Report r)`: Inserimento in testa in tempo costante $\mathcal{O}(1)$.
  * `list_remove(ReportList l, int report_id)`: Ricerca ed estrazione fisica del nodo in $\mathcal{O}(n)$, con decremento atomico della size.
  * `list_rewind(ReportList l)` / `list_next(ReportList l)`: Logica nativa dell'iteratore per scorrere sequenzialmente i record in RAM senza esporre i nodi della catena.

---

## 2. ADT ReportStack (Undo Buffer LIFO)
* **File Header:** `include/adt/report_stack.h`
* **File Sorgente:** `src/adt/report_stack.c`
* **Definizione Fisica:** Array statico preallocato limitato a un massimo di 10 slot gestito tramite un indice di cima (`top`).
* **Operazioni Chiave:**
  * `stack_push(ReportStack s, Report r)`: Esegue una clonazione profonda (*deep copy*) dell'oggetto Report originario per metterlo al sicuro in RAM, incrementando la cima in $\mathcal{O}(1)$.
  * `stack_pop(ReportStack s)`: Estrae l'ultimo punto di ripristino per innescare l'azione di *Revert* (annullamento modifiche) del cittadino, decrementando la cima in $\mathcal{O}(1)$.

---

## 3. ADT ReportAvl (Albero Auto-Bilanciante Polimorfico)
* **File Header:** `include/adt/report_avl.h`
* **File Sorgente:** `src/adt/report_avl.c`
* **Definizione Fisica:** Nodo bilanciato generico (`NodeAVL`) contenente un elemento puntatore opaco `void *elem`, l'indicatore di altezza (`height`), il contatore di sottoalbero (`size`) e il discriminante enumerato `Type_Avl`.
* **Operazioni Chiave:**
  * `insert(ReportAvl t, void *elem, Type_Avl type, int (*compare)(const void *, const void *))`: Inserimento polimorfico regolato da callback esterne. Gestisce i duplicati logici di User ID facendoli scivolare a destra, attivando le rotazioni LL, RR, LR, RL al rilevamento di un *Balance Factor* fuori dal range $[-1, 1]$.
  * `inorder(ReportAvl t, FILE *file)`: Visita simmetrica profonda che scarica i dati sul disco eliminando ogni spazio spurio, generando gli indici sequenziali ordinati (*Inorder Arrays*) da 21 e 22 byte.

---

## 4. ADT PriorityQueue (Coda con Ordinamento Incrociato)
* **File Header:** `include/adt/priority_queue.h`
* **File Sorgente:** `src/adt/priority_queue.c`
* **Definizione Fisica:** Catena di nodi ordinati in RAM durante la fase di inserimento in base a metriche combinate.
* **Operazioni Chiave:**
  * `pq_enqueue(PriorityQueue pq, Report r)`: Inserimento a scansione lineare ordinata. Applica la precedenza all'urgenza scalare decrescente ('2' $\rightarrow$ '1' $\rightarrow$ '0') e, in caso di perfetta parità di urgenza, invoca l'helper `compare_dates_fifo` per dare la precedenza alla segnalazione con la stringa temporale della data più remota (regola FIFO).
  * `pq_dequeue(PriorityQueue pq)`: Estrazione immediata in testa in tempo costante $\mathcal{O}(1)$ della segnalazione a massima criticità comunale assoluta.

---

## 5. Motori d'Indice Persistenti su Disco (File Interrogations)

Per massimizzare l'efficienza ed evitare l'occupazione impropria di memoria in RAM sul server municipale, le funzioni di ricerca per chiave **non lavorano sugli alberi residenti in memoria**, ma interrogano direttamente i file d'indice sequenziali ordinati generati dagli AVL tramite algoritmi di scansione hardware:

*   **`findReportId(unsigned int report_id)`**: Apre il file `report_AVL_BY_REPORT_ID.txt`, scansiona le righe a blocchi rigidi da 22 byte e, in caso di match con la sotto-stringa dell'ID, estrae l'indice numerico `disk_row` in tempo logaritmico, chiudendo il descrittore.
*   **`findUserId(unsigned int uid, unsigned int *results)`**: Apre il file `report_AVL_BY_USER_ID.txt` e legge il contatore di record in $\mathcal{O}(1)$ dal registro centrale. Applica un algoritmo di **Ricerca Binaria (Dicotomica)** saltando geometricamente a passi da 21 byte per localizzare il primo match in $\mathcal{O}(\log n)$. Trovato il punto di contatto, esegue un'espansione bilaterale contigua raccogliendo tutti i duplicati dell'utente all'interno del vettore dinamico di output `results`, restituendo il conteggio dei match.



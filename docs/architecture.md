# Schema Strutturale, Diagrammi dei Flussi e Ciclo di Vita (Versione AVL & Buchi LIFO)

Questo documento descrive l'architettura logica e fisica del sistema, illustrando come i dati transitano e si sincronizzano tra i livelli di memoria e i file di persistenza.

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
| (parser.c,   |     | (lists, avl, |     | (.txt fisse |
|  validators) |     |  stack, pq)  |     |  binari)    |
+--------------+     +--------------+     +-------------+
```

---

## 2. Mappa dei Livelli di Persistenza e Geometria Hardware


| Fase Operativa / Archivio | Struttura Dati / File Fisico | Dimensione Record | Logica di Accesso / Complessità | Stato e Indicatori di Cella |
| :--- | :--- | :---: | :---: | :--- |
| **1. RAM Locale** | `ReportList` (Dinamica) | Puntatore Opaco | $O(1)$ in testa | Stato iniziale sessione. Riga disco = `-1`. |
| **2. Cache Server** | `reports_bench.txt` (Flat) | **351 Byte (`REPORT_BENCH_LINE`)** | $O(1)$ via cursore | Buffer statico da 50 slot. Regola append circolare. |
| **3. DB Master Storico** | `open / progress / closed` | **352 Byte (`REPORT_MASTER_LINE`)** | $O(1)$ via `disk_row` | Cella: `'A'` (Attivo), `'N'` (Null/Buco), `'E'` (Sentinella). |
| **4. Indice Report ID (AVL)**| `report_AVL_BY_REPORT_ID.txt` | **24 Byte (Fisso)** | $O(\log n)$ bilanciato | Punto di Verità. Rigenerato ad albero bilanciato: `[ID_REPORT(10)] [STATUS(1)] [DISK_ROW(10)]\n`. |
| **5. Indice User ID (AVL)** | `report_AVL_BY_USER_ID.txt` | **21 Byte (Fisso)** | $O(\log n)$ bilanciato | Rigenerato in logica contratta accumulando n-report: `[ID_USER(10)][ID_REPORT(10)]\n`. |
| **6. Registro Centrale** | `system_total_report.txt`| **11 Byte (`SYSTEM_REG_LINE`)** | $O(1)$ diretto `fseek` | Memorizza 11 variabili anagrafiche pre-calcolate. |


---
## 3. Gestione Intelligente dei File Master: Stack LIFO dei Buchi

Per azzerare il degrado prestazionale causato dalle scansioni orizzontali in $O(n)$ nella ricerca di spazio libero, il server implementa un meccanismo di inserimento e riciclo geometrico in **$O(1)$**:

1. **Generazione del Buco:** Quando un dipendente cambia lo stato di una segnalazione (es. da `OPEN` a `IN_PROGRESS`), il server estrae la `disk_row` memorizzata nel record, effettua un salto `fseek` immediato sul file master di provenienza moltiplicando per `REPORT_MASTER_LINE` (352 byte) e sovrascrive il flag di cella al byte 350 impostandolo a `'N'` (Null). 
2. **Accatastamento LIFO:** Il numero di riga liberato viene immediatamente scritto in coda al file ausiliario corrispondente (es. `open_holes.txt`) in formato testo strutturato occupando esattamente i byte prescritti da `SYSTEM_REG_LINE` (`%010d\n`).
3. **Riciclo Chirurgico:** Al momento del flush della cache `BENCH`, prima di appendere un record in coda al master, il server interroga lo stack dei buchi leggendo l'ultima riga. Se presente, estrae l'indice, esegue il troncamento fisico (semantica Pop dello Stack LIFO) e sovrascrive direttamente lo slot vuoto nel master in tempo costante. Se lo stack dei buchi è vuoto, il record viene inserito in Append alla fine del file master.
---

## 4. Consistenza delle Interrogazioni e Flusso del Cittadino

Il cittadino visualizza esclusivamente il proprio storico. Al fine di evitare asimmetrie informative o ritardi di sincronizzazione, il server applica una logica di precedenza e sovrapposizione:

* **Passo 1 (RAM):** Carica i dati volatili presenti nella lista locale di sessione.
* **Passo 2 (BENCH):** Scansiona in $O(n)$ il file di cache fino al contatore corrente (`counter_bench`), estraendo i soli report non ancora flussati che presentano lo stesso ID Utente e stato diverso da `DESTROYED`.
* **Passo 3 (AVL):** Interroga in $O(\log n)$ l'indice `report_AVL_BY_USER_ID` estraendo la sequenza dei suoi report storici. Per ciascun codice, scarta gli elementi già estratti dalla BENCH (in quanto obsoleti) e interroga in $O(\log n)$ l'indice `report_AVL_BY_REPORT_ID` per ottenere la riga fisica. Esegue l'accesso diretto sul file master solo se la cella è marcata `'A'` (Attivo), escludendo i record contrassegnati a `'N'` che possiedono una modifica recente in cache.

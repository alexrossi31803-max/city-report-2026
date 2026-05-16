# Analisi Ingegneristica e Scelte Algoritmiche (Versione AVL & Registri O(1))

Questo documento descrive il razionale algoritmico e le complessità computazionali delle nuove strutture dati, dimostrando la conformità ai requisiti stringenti della traccia d'esame.

---

## 1. Gestione Utenti ed Autenticazione: Tabella Hash Geometrica su File
* **Requisito:** Login e registrazione istantanei con blocco dei duplicati in tempo costante.
* **Scelta Algoritmica:** Tabella Hash ad indirizzamento aperto (Linear Probing) su file indice binarizzato (`users_idx.txt`) accoppiata a un file dati ad anagrafica fissa (`users.txt`).
* **Complessità Computazionale:** Caso medio **$O(1)$**.
* **Razionale d'Uso:** L'username viene convertito in un intero tramite l'algoritmo **DJB2** per calcolare l'indice slot (`hash % capacità`). Il server esegue un salto `fseek` immediato nel file indice e, estratto il puntatore alla riga, effettua un secondo salto geometrico speculare sul file dei dati sfruttando la lunghezza rigida della riga (**107 byte**). Viene eliminata qualsiasi scansione sequenziale lineare, garantendo l'accesso e l'intercettazione dei duplicati in tempo costante.

---

## 2. Sessione Locale e Annullamento Azioni: Linked List + Stack Statico LIFO
* **Requisito:** Accumulo volatile delle segnalazioni del cittadino e meccanismo di Undo/Revert.
* **Scelta Algoritmica:** Lista Concatenata Dinamica (`ReportList`) accoppiata a uno Stack Statico a capacità fissata (`ReportStack`, max 10 elementi).
* **Complessità Computazionale:** Inserimento e Push in **$O(1)$**.
* **Razionale d'Uso:** La Linked List consente inserimenti in testa immediati senza preallocare memoria o conoscere il volume di problemi inviati. Lo Stack risponde alla semantica LIFO (Last In, First Out) per l'Undo: prima di alterare un report, il sistema esegue una **clonazione profonda** dei campi e spinge il backup nello stack. L'azione di annullamento estrae la copia e ripristina la lista RAM. Il disco non viene toccato, isolando le modifiche transitorie dal server.

---

## 3. Ricerca Operativa e Sfoltimento Master: Albero AVL per Codice Report
* **Requisito:** Ricerca e sfoltimento immediato di una segnalazione tramite codice identificativo senza degradamento prestazionale.
* **Scelta Algoritmica:** Albero Auto-Bilanciante AVL (`ReportAVL`) strutturato sulla chiave `report_id`, serializzato In-Order su file (`report_AVL_BY_REPORT_ID.txt`).
* **Complessità Computazionale:** Caso peggiore e medio strettamente limitato a **$O(\log n)$**.
* **Razionale d'Uso:** L'albero AVL introduce rotazioni singole e doppie (LL, RR, LR, RL) basate sul Fattore di Bilanciamento per mantenere la differenza di altezza tra sottoalberi $\le 1$, garantendo che non degradi mai in una lista. Il dipendente esegue ricerche rapide sul canale di verità del disco per localizzare e invalidare a `N` (Null) le vecchie celle fisiche master in tempo logaritmico certo. L'accesso al file master avviene saltando geometricamente tramite la macro rigida **`REPORT_MASTER_LINE` (352 byte)**.

---
## 4. Storico Personale del Cittadino: Albero AVL per User ID con Nodi ad Accumulo Dinamico
* **Requisito:** Estrazione dello storico del cittadino con isolamento delle modifiche ed elisione delle asimmetrie informative.
* **Scelta Algoritmica:** Albero Auto-Bilanciante AVL (`ReportAVL`) strutturato sulla chiave `user_id`, i cui nodi contengono vettori dinamici in grado di accumulare e aggregare n-chiavi di `report_id` per lo stesso utente.
* **Complessità Computazionale:** Ricerca in **$O(\log n)$** + Risoluzione codici in **$O(\log n)$**.
* **Razionale d'Uso:** Un cittadino può inviare più segnalazioni. Per rispettare l'Information Hiding e l'integrità dei dati, il nodo AVL utente non duplica gli oggetti Report complessi, ma mappa l'ID dell'utente e la sequenza numerica dei suoi codici. La ricerca logaritmica restituisce questi vettori compatti, che vengono poi incrociati con l'indice di verità AVL del Report ID, visualizzando lo stato aggiornato in tempo reale ed eliminando i record invalidati o in fase di lavorazione nella BENCH.

---

## 5. Dashboard Statistica e Indicatori Comunali: Registro Centrale su File
* **Requisito:** Generazione istantanea del report statistico senza scansioni orizzontali distruttive e onerose dei file storici.
* **Scelta Algoritmica:** Registro di Controllo Binarizzato statico con aggiornamento atomico (`system_total_report.txt`).
* **Complessità Computazionale:** Lettura, Scrittura e Aggiornamento in **$O(1)$**.
* **Razionale d'Uso:** Per azzerare i tempi di scansione sequenziale su archivi massivi, il server memorizza in anticipo 11 righe rigide da 11 byte per ospitare contatori numerici a 10 cifre (`%010u\n`), regolate dalla macro **`SYSTEM_REG_LINE`**. Ogni inserimento o eliminazione logica (`DESTROYED`) esegue un salto `fseek` atomico che aggiorna il contatore specifico. La cache della BENCH è invece isolata e protetta a **`REPORT_BENCH_LINE` (351 byte)** per non sprecare spazio sul disco e velocizzare l'I/O volatile delle sessioni dei cittadini.




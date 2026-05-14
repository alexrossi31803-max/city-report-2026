#  Analisi Ingegneristica e Scelte Algoritmiche Finali

Questo documento descrive il razionale algoritmico e le complessità computazionali delle strutture dati, dimostrando la conformità ai requisiti stringenti della traccia d'esame.

---

## 1. Gestione Utenti ed Autenticazione: Tabella Hash Geometrica su File
* **Requisito:** Login e registrazione istantanei con blocco dei duplicati in tempo costante.
* **Scelta Algoritmica:** Tabella Hash ad indirizzamento aperto (Linear Probing) su file indice binarizzato (`users_idx.txt`) accoppiata a un file dati ad anagrafica fissa (`users.txt`).
* **Complessità Computazionale:** Caso medio **\(O(1)\)**.
* **Razionale d'Uso:** L'username viene convertito in un intero tramite l'algoritmo **DJB2** per calcolare l'indice slot (`hash % capacità`). Il server esegue un salto `fseek` immediato nel file indice e, estratto il puntatore alla riga, effettua un secondo salto geometrico speculare sul file dei dati sfruttando la lunghezza rigida della riga (**107 byte**). Viene eliminata qualsiasi scansione sequenziale lineare, garantendo l'accesso e l'intercettazione dei duplicati in tempo costante.

---

## 2. Sessione Locale e Annullamento Azioni: Linked List + Stack Statico LIFO
* **Requisito:** Accumulo volatile delle segnalazioni del cittadino e meccanismo di Undo/Revert.
* **Scelta Algoritmica:** Lista Concatenata Dinamica (`ReportList`) accoppiata a uno Stack Statico a capacità fissata (`ReportStack`, max 10 elementi).
* **Complessità Computazionale:** Inserimento e Push in **\(O(1)\)**.
* **Razionale d'Uso:** La Linked List consente inserimenti in testa immediati senza preallocare memoria o conoscere il volume di problemi inviati. Lo Stack risponde alla semantica LIFO (Last In, First Out) per l'Undo: prima di alterare un report, il sistema esegue una **clonazione profonda** dei campi e spinge il backup nello stack. L'azione di annullamento estrae la copia e ripristina la lista RAM. Il disco non viene toccato, isolando le modifiche transitorie dal server.

---

## 3. Ricerca Operativa del Dipendente: BST per Codice Report
* **Requisito:** Ricerca immediata e puntuale di una segnalazione tramite codice identificativo univoco.
* **Scelta Algoritmica:** Albero Binario di Ricerca (`ReportBST`) strutturato sulla chiave `report_id`, serializzato In-Order su file (`report_BST_BY_REPORT_ID.txt`).
* **Complessità Computazionale:** Ricerca nel caso medio in **\(O(\log n)\)**.
* **Razionale d'Uso:** Per evitare scansioni orizzontali distruttive su archivi storici massivi, il server indicizza i record attivi in un BST usando il codice report come chiave. La navigazione binaria (`sinistra < radice < destra`) abbatte i tempi di localizzazione a livello logaritmico. Questo albero è il vero "Punto di Verità" (Single Source of Truth) dello stato dei dati: ogni modifica del dipendente viene riscritta all'istante su questo indice in RAM e sul disco, centralizzando la coerenza dell'anagrafica.

---

## 4. Storico Personale del Cittadino: BST per User ID a Triangolazione di Codici (12 Byte)
* **Requisito:** Visualizzazione dello storico del cittadino con garanzia di sincronizzazione degli stati ed isolamento.
* **Scelta Algoritmica:** Albero Binario di Ricerca (`ReportBST`) strutturato sulla chiave `user_id` (hash numerico a 5 cifre), i cui nodi stampano sul disco rigido una riga contratta a geometria fissa da **12 byte** complessivi nel formato `[ID_USER(5)][ID_REPORT(5)]\n`.
* **Complessità Computazionale:** Ricerca in **\(O(\log n)\)** + Risoluzione codici in **\(O(\log n)\)**.
* **Razionale d'Uso:** Memorizzare l'intero oggetto report in questo albero causerebbe disallineamenti di stato se il dipendente modificasse la segnalazione. Salvando nel nodo **esclusivamente l'ID dell'utente e la lista dei codici report a lui associati**, la navigazione simmetrica (Opzione 6) estrae in modo sicuro i soli codici numerici d'indice e li risolve tramite una seconda ricerca in \(O(\log n)\) sul `bst_by_report_id.txt`. Questo garantisce uno storico pulito con stati sempre aggiornati in tempo reale, azzerando le asimmetrie informative.

---

## 5. Dashboard Operativa delle Urgenze: Coda a Priorità su Richiesta
* **Requisito:** Ordinamento incrociato stocastico (Urgenza decrescente + Data FIFO più vecchia) per i dipendenti.
* **Scelta Algoritmica:** Coda a Priorità (`PriorityQueue`) alimentata in RAM ed estratta in un file flat ordinato.
* **Complessità Computazionale:** Inserimento ordinato in coda ed estrazione finale in **\(O(1)\)**.
* **Razionale d'Uso:** Trattandosi di un'operazione computazionalmente onerosa, la coda non viene aggiornata in background ma viene compilata **solo sotto esplicita richiesta del dipendente**. Il comando innesca un flush forzato di consolidamento della BENCH, scansiona i file master attivi escludendo i casi `CLOSED`, popola la struttura e riversa sul disco il file lineare pre-ordinato `reports_by_priority.txt` pronto per la lettura interattiva via `fread`.



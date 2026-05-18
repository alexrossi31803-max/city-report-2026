# Analisi dei Requisiti e Complessità Computazionale (v5.0 Definitiva)

Il sistema è stato progettato per simulare il comportamento di un Database Relazionale su file di testo a geometria fissa. L'obiettivo primario è l'elisione dei colli di bottiglia indotti dall'I/O sul disco, garantendo la massima reattività del finto server municipale attraverso strutture dati ottime.

---

## 1. Autenticazione e Anagrafica Utenti
* **Struttura Dati:** Tabella Hash ad indirizzamento aperto con risoluzione delle collisioni tramite Linear Probing (Ispezione Lineare), persistita su file binario (`users_idx.txt` e `users.txt`).
* **Dimensione Record:** L'indice hash occupa righe rigide regolate dalla macro `USER_IDX_LINE` (6 byte). Il database anagrafico occupa righe fisse regolate da `USER_LINE_TOTAL` (107 byte).
* **Algoritmo di Hashing:** DJB2 (Daniel J. Bernstein).
* **Complessità Computazionale:** 
  * Caso Medio: **$\mathcal{O}(1)$** sia per il login che per la registrazione.
  * Caso Peggiore (Tabella satura): $\mathcal{O}(k)$ dove $k$ è la capacità della tabella.
* **Razionale d'Uso:** L'indirizzamento aperto combinato con il Linear Probing azzera i puntatori RAM sul disco. La sovrascrittura protetta a 5 byte numerici impedisce la generazione di newline spuri. Il sistema monitora il Load Factor e innesca l'espansione automatica di 50 slot (`BLOCK_SIZE_USERS`) al superamento della soglia critica del 70%, mantenendo l'accesso in tempo costante.

---

## 2. Sessione Locale e Meccanismo di Undo (Cittadino)
* **Struttura Dati:** Lista concatenata semplice dinamica (`ReportList`) combinata con uno Stack statico preallocato (`ReportStack`).
* **Capacità Stack:** Limitata rigorosamente a un massimo di 10 elementi.
* **Complessità Computazionale:**
  * Inserimento nuova segnalazione in lista: **$\mathcal{O}(1)$** (inserimento nativo in testa).
  * Push/Pop nello Stack di Undo: **$\mathcal{O}(1)$** ad accesso diretto su array statico.
  * Modifica o Annullamento locale: $\mathcal{O}(m)$ nel caso peggiore per la ricerca lineare dell'ID nella lista RAM di sessione (dove $m \le 50$).
* **Razionale d'Uso:** L'isolamento della sessione in RAM evita scritture premature sul disco. L'operazione di `stack_push` esegue una clonazione profonda (*deep copy*) del report, isolando il punto di ripristino dalle successive alterazioni della lista prima del logout.

---

## 3. Ricerca ed Avanzamento Pratiche (Dipendente e Storico)
* **Struttura Dati:** Indici sequenziali piatti ordinati (*Inorder Arrays*) estratti da Alberi Auto-Bilanciati AVL (`PATH_AVL_REPORT_ID` e `PATH_AVL_USER_ID`).
* **Geometria Hardware:** 
  * Indice Report ID: Righe fisse da **22 byte (`AVL_REPORT_ID_LINE`)** in formato compresso contino senza spazi: `[REPORT_ID(10)][STATUS(1)][DISK_ROW(10)]\n`.
  * Indice User ID: Righe fisse da **21 byte (`AVL_USER_ID_LINE`)** in formato contratto senza spazi: `[USER_ID(10)][REPORT_ID(10)]\n`.
* **Complessità Computazionale:**
  * Ricerca univoca `findReportId`: **$\mathcal{O}(\log n)$** sul file d'indice.
  * Ricerca non univoca `findUserId`: **$\mathcal{O}(\log n)$** tramite Ricerca Binaria (Dicotomica) su disco integrata da espansione bilaterale contigua.
  * Accesso e modifica sul file Master: **$\mathcal{O}(1)$** tramite salto diretto `fseek` moltiplicato per `REPORT_MASTER_LINE` (352 byte).
* **Razionale d'Uso:** Gli indici vengono ricalcolati da zero tramite visite simmetriche *In-Order* solo al momento del flush. Il client non ricostruisce gli alberi in RAM (operazione distruttiva per l'I/O), ma esegue salti dicotomici direttamente sui file binari, estraendo la `disk_row` per agganciare i record in tempo logaritmico certo, escludendo le celle invalidate a `'N'`.

---

## 4. Gestione Spazio e Riciclo Chirurgico Master
* **Struttura Dati:** File ausiliari operanti con semantica di Stack LIFO (`open_holes.txt`, ecc.).
* **Complessità Computazionale:** **$\mathcal{O}(1)$** costante sia in fase di Push che di Pop.
* **Razionale d'Uso:** Per azzerare il degrado prestazionale causato dalla ricerca di spazi vuoti nel database storico, il server non scansiona mai orizzontalmente i file Master. Quando una riga viene marcata a `'N'`, il suo indice viene accatastato in coda al file dei buchi (Push). Al momento del flush pesante, il server preleva l'ultima riga del file buchi in $O(1)$, sovrascrive lo slot master in modo chirurgico e tronca il file ausiliario (Pop binaria), garantendo la scalabilità infinita dell'archivio.

---

## 5. Dashboard Statistica Comunale
* **Struttura Dati:** Registro di controllo binarizzato statico con aggiornamento atomico (`system_total_report.txt`).
* **Geometria Hardware:** Righe fisse regolate da `SYSTEM_REG_LINE` (11 byte per riga, formato `%010u\n`).
* **Variabili Mappate:** 13 variabili pre-calcolate (ID globale, counter cache, 3 indicatori di stato, 5 ripartizioni di categoria e 2 indicatori di consistenza nodi AVL in posizione 11 e 12).
* **Complessità Computazionale:** **$\mathcal{O}(1)$** costante per lettura, scrittura e rendering a video.
* **Razionale d'Uso:** Qualsiasi inserimento, sfoltimento logico (`DESTROYED`) o cambio di stato esegue un aggiornamento atomico sul posto tramite salto `fseek` combinato con la funzione helper `update_system_counters`. Il dipendente ottiene una dashboard istantanea in tempo costante, senza dover scansionare gigabyte di file storici sul disco.




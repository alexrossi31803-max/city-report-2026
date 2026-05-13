# Questo documento descrive le ragioni ingegneristiche e algoritmiche per cui le strutture dati selezionate garantiscono prestazioni ottimali rispetto ai requisiti della traccia.

---

## 👥 1. Gestione Utenti e Autenticazione: Tabella Hash su File

### Requisito:
Login e registrazione istantanei per cittadini e dipendenti comunali.

### Scelta Algoritmica:
Tabella Hash ad indirizzamento aperto (Linear Probing) implementata direttamente su file di testo `.txt` a righe a lunghezza fissa.

### Motivazione:
L'autenticazione standard richiede una scansione lineare \(O(n)\), inefficiente al crescere del numero di abitanti. Convertendo l'username in un intero tramite l'algoritmo DJB2 e calcolando l'indice tramite l'operatore modulo (`hash % capacità`), il sistema effettua un salto diretto `fseek` alla riga desiderata.

In questo modo si garantisce una complessità temporale in tempo costante \(O(1)\) sia nel caso medio di ricerca che in inserimento.

---

## 📱 2. Sessione RAM del Cittadino: Linked List + Stack statico

### Requisito:
Inserimento e modifica di segnalazioni in locale senza scrivere subito su disco, con possibilità di annullare l'ultima azione (Revert).

### Scelta Algoritmica:
Lista Concatenata Dinamica (`ReportList`) accoppiata a uno Stack Statico a capacità fissata (`ReportStack`, max 10 elementi).

### Motivazione:

La Linked List è ottimale in RAM per la sessione locale poiché le segnalazioni vengono inserite in tempo costante \(O(1)\) in testa, senza conoscere a priori quanti problemi notificherà il cittadino.

Lo Stack risponde nativamente alla semantica dell'operazione di Undo/Revert (politica LIFO - Last In, First Out). Memorizzando l'ultimo stato valido del report prima di una modifica, l'annullamento estrae l'elemento in testa in \(O(1)\) e ripristina lo stato precedente nella lista RAM tramite scansione \(O(n)\) mirata, isolando le modifiche volatili dal server.

---

## 🌲 3. Indicizzazione Storica del Server: Albero Binario di Ricerca (BST)

### Requisito:
Consentire al cittadino di visualizzare lo storico delle proprie segnalazioni (aperte, in lavorazione, chiuse) memorizzate nei file master a lungo termine.

### Scelta Algoritmica:
Albero Binario di Ricerca (`ReportBST`) serializzato su file tramite visita ordinata In-Order.

### Motivazione:
Per evitare di scansionare l'intero database del comune alla ricerca dei report di un singolo utente (operazione \(O(n)\) costosa), il server aggrega i record usando come chiave l'ID Utente.

L'organizzazione in BST riduce la complessità di ricerca a \(O(\log n)\).

La scrittura In-Order (Sinistra \(\rightarrow\) Radice \(\rightarrow\) Destra) sul file di testo `report_BST_ID_USER.txt` mantiene i dati ordinati matematicamente sul disco per ID, abilitando letture logaritmiche stabili.

---

## 🚨 4. Ordinamento delle Urgenze: Coda a Priorità (Priority Queue)

### Requisito:
Visualizzazione da parte dei dipendenti di tutte le segnalazioni aperte e in lavorazione ordinate per urgenza decrescente e, a parità di urgenza, per data più vecchia.

### Scelta Algoritmica:
Coda a Priorità (`PriorityQueue`) implementata con inserimento ordinato.

### Motivazione:
L'algoritmo richiede un ordinamento incrociato stocastico (Urgenza + FIFO temporale).

La Coda a Priorità è la struttura ideale perché ordina il report nel momento stesso in cui viene inserito (`pq_enqueue`).

Al momento della visualizzazione da parte del dipendente, l'estrazione (`pq_dequeue`) avviene in tempo costante \(O(1)\), restituendo l'array lineare definitivo `reports_by_priority.txt` già pronto per l'uso operativo.
#  Specifica Formale degli ADT e Operazioni Semantiche

Questo documento definisce la specifica sintattica e semantica completa di ciascuna operazione fornita dagli ADT creati, nel rispetto del principio dell'Information Hiding e dell'incapsulamento dei tipi.

---

##  ADT ReportList

## Tipo:
```c
typedef struct ReportList* ReportList;
```

(Opaque Pointer).

---

### Operazioni Formali:

#### `ReportList create_list();`
* **Input:** Nessuno.
* **Output:** Un’istanza valida di tipo `ReportList`.
* **Pre-condizioni:** Memoria di sistema disponibile per l'allocazione.
* **Post-condizioni:** Restituisce un puntatore a una struttura lista inizializzata con dimensione pari a `0` e puntatori di testa impostati a `NULL`.

#### `void free_list(ReportList l);`
* **Input:** La lista `l` da deallocare.
* **Pre-condizioni:** `l` inizializzato non NULL.
* **Post-condizioni:** Tutta la memoria associata ai nodi della lista e alle singole istanze di `Report` contenute viene interamente liberata in RAM.

#### `void list_insert(ReportList l, Report r);`
* **Input:** La lista `l`, il puntatore all'oggetto `Report r`.
* **Pre-condizioni:** `l` non deve essere `NULL`, `r` deve essere un'istanza di report valida e non nulla.
* **Post-condizioni:** `r` viene inserito in testa alla lista come nuovo nodo in tempo costatante \(O(1)\). La dimensione della lista aumenta di `1`.

#### `bool list_remove(ReportList l, int report_id);`
* **Input:** La lista `l`, l'ID numerico del report da eliminare.
* **Output:** Valore booleano (`true` se rimosso, `false` altrimenti).
* **Pre-condizioni:** `l` inizializzato e non vuoto.
* **Post-condizioni:** Se l'ID corrisponde a un report in lista, il nodo viene rimosso modificando i puntatori adiacenti, la memoria del report viene liberata, la dimensione decresce di `1` e ritorna `true`. Se non trovato, ritorna `false`.

#### `Report list_find(ReportList l, int report_id);`
* **Input:** La lista `l`, l'ID del report da cercare.
* **Output:** Il puntatore all'oggetto `Report` trovato, oppure `NULL`.
* **Pre-condizioni:** `l` inizializzato.
* **Post-condizioni:** Scansiona la lista. Se trova un report con ID corrispondente, ne restituisce il puntatore senza rimuoverlo o modificarlo. Altrimenti ritorna `NULL`.

---

##  ADT ReportStack

## Tipo:
```c
typedef struct ReportStack* ReportStack;
```

(Opaque Pointer).

---

### Operazioni Formali:

#### `bool stack_push(ReportStack s, Report r);`
* **Input:** Lo stack `s`, il report corrente `r` da salvare.
* **Output:** `true` se l'operazione riesce, `false` se lo stack è saturo.
* **Pre-condizioni:** `s` e `r` validi e non nulli. L'indice di cima `top` deve essere minore di `MAX_STACK - 1` (10).
* **Post-condizioni:** Effettua una clonazione profonda (deep copy) allocando un nuovo oggetto `Report` e copiando tutti i campi di `r`. Incrementa `top` di `1` e inserisce la copia in cima allo stack. Ritorna `true`.

#### `Report stack_pop(ReportStack s);`
* **Input:** Lo stack `s`.
* **Output:** Il puntatore al `Report` rimosso dalla cima, oppure `NULL` se vuoto.
* **Pre-condizioni:** `s` non vuoto (`top >= 0`).
* **Post-condizioni:** Estrae il report posizionato all'indice `top`, decrementa il contatore di cima di `1` e trasferisce la proprietà della memoria del report di backup clonato al chiamante.

---

##  ADT ReportBST

## Tipo:
```c
typedef struct ReportBST* ReportBST;
```

(Opaque Pointer).

---

### Operazioni Formali:

#### `void bst_insert(ReportBST t, int chiave, Report r);`
* **Input:** L'albero `t`, la chiave intera (`report_id` o `hash_user`), l'oggetto `Report r`.
* **Pre-condizioni:** `t` valido, `r` valido.
* **Post-condizioni:** Inserisce un nuovo nodo nell'albero rispettando la proprietà dei BST: i nodi con chiave minore vanno a sinistra, i maggiori o uguali a destra, garantendo una ricerca in tempo logaritmico **\(O(\log n)\)**.

#### `void bst_write_inorder(ReportBST t, FILE* f_out, void (*write_func)(FILE*, Report));`
* **Input:** L'albero `t`, il file aperto `f_out`, la funzione callback di formattazione `write_func`.
* **Pre-condizioni:** `t` inizializzato, `f_out` aperto in scrittura binarizzata, `write_func` non nulla.
* **Post-condizioni:** Attraversa l'albero tramite una visita in ordine simmetrico (In-Order: Sinistra $\rightarrow$ Radice $\rightarrow$ Destra) e scrive sul file un elenco serializzato matematicamente in ordine crescente per chiave.

---

##  ADT PriorityQueue

## Tipo:
```c
typedef struct PriorityQueue* PriorityQueue;
```

(Opaque Pointer).

---

### Operazioni Formali:

#### `void pq_enqueue(PriorityQueue pq, Report r);`
* **Input:** La coda `pq`, l'oggetto `Report r` da inserire.
* **Pre-condizioni:** `pq` e `r` validi e non nulli.
* **Post-condizioni:** Alloca un nodo per la coda e lo inserisce nella lista interna mantenendola ordinata. La precedenza viene calcolata tramite ordinamento incrociato stocastico: priorità assoluta al livello di urgenza decrescente (3 -> 2 -> 1) e, a parità di urgenza, viene applicato il criterio FIFO confrontando la stringa della data d'inserimento più vecchia.

#### `Report pq_dequeue(PriorityQueue pq);`
* **Input:** La coda `pq`.
* **Output:** Il puntatore al `Report` a massima priorità estratto, oppure `NULL`.
* **Pre-condizioni:** `pq` inizializzata e non vuota.
* **Post-condizioni:** Rimuove il primo nodo in testa alla lista interna (che rappresenta la priorità massima assoluta), dealloca il nodo di giunzione e restituisce la proprietà del report rimosso al chiamante in tempo costante \(O(1)\).




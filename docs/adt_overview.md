# Specifica Formale degli ADT e Operazioni Semantiche (Versione AVL)

Questo documento definisce la specifica sintattica e semantica completa di ciascuna operazione fornita dagli ADT creati, nel rispetto del principio dell'Information Hiding e dell'incapsulamento dei tipi.

---

## ADT ReportList

### Tipo:
```c
typedef struct ReportList* ReportList;
```
(Opaque Pointer).

### Operazioni Formali:

#### `ReportList create_list();`
* **Input:** Nessuno.
* **Output:** Un’istanza valida di tipo `ReportList`.
* **Pre-condizioni:** Memoria di sistema disponibile per l'allocazione.
* **Post-condizioni:** Restituisce un puntatore a una struttura lista inizializzata con dimensione pari a `0` e puntatori di testa e iteratore impostati a `NULL`.

#### `void free_list(ReportList l);`
* **Input:** La lista `l` da deallocare.
* **Pre-condizioni:** `l` inizializzato e non NULL.
* **Post-condizioni:** Tutta la memoria associata ai nodi della lista e alle singole istanze di `Report` contenute viene interamente liberata in RAM.

#### `void list_insert(ReportList l, Report r);`
* **Input:** La lista `l`, il puntatore all'oggetto `Report r`.
* **Pre-condizioni:** `l` non deve essere `NULL`, `r` deve essere un'istanza di report valida e non nulla.
* **Post-condizioni:** `r` viene inserito in testa alla lista come nuovo nodo in tempo costante $O(1)$. La dimensione della lista aumenta di `1`.

---

## ADT ReportStack

### Tipo:
```c
typedef struct ReportStack* ReportStack;
```
(Opaque Pointer).

### Operazioni Formali:

#### `bool stack_push(ReportStack s, Report r);`
* **Input:** Lo stack `s`, il report corrente `r` da salvare.
* **Output:** `true` se l'operazione riesce, `false` se lo stack è saturo.
* **Pre-condizioni:** `s` e `r` validi e non nulli. L'indice di cima `top` deve essere minore di `MAX_STACK - 1` (10).
* **Post-condizioni:** Effettua una clonazione profonda (deep copy) allocando un nuovo oggetto `Report` copiando tutti i campi aggiornati (ID senza segno e urgenza `char`). Incrementa `top` di `1` e inserisce la copia in cima. Ritorna `true`.

---

## ADT ReportAVL

### Tipo:
```c
typedef struct ReportAVL* ReportAVL;
```
(Opaque Pointer).

### Operazioni Formali:

#### `ReportAVL create_avl();`
* **Input:** Nessuno.
* **Output:** Un'istanza valida di tipo `ReportAVL`.
* **Pre-condizioni:** Memoria RAM disponibile.
* **Post-condizioni:** Restituisce un puntatore ad albero bilanciato con radice impostata a `NULL`.

#### `void avl_insert_by_report_id(ReportAVL t, unsigned int report_id, Report r);`
* **Input:** L'albero `t`, la chiave unica senza segno `report_id`, l'oggetto `Report r`.
* **Pre-condizioni:** `t` valido, `r` configurato con metadati fisici di riga disco coerenti.
* **Post-condizioni:** Inserisce un nuovo nodo rispettando la proprietà di ordinamento binario. Se il fattore di bilanciamento $|高度_{sinistra} - 高度_{destra}| > 1$, innesca automaticamente rotazioni singole o doppie (LL, RR, LR, RL) per ripristinare l'equilibrio della radice. Complessità limitata a $O(\log n)$.

#### `void avl_insert_by_user_id(ReportAVL t, unsigned int user_id, unsigned int report_id);`
* **Input:** L'albero `t`, la chiave di ricerca `user_id`, l'identificatore del report associato.
* **Pre-condizioni:** `t` inizializzato non NULL.
* **Post-condizioni:** Se la chiave utente è inedita, alloca un nuovo nodo memorizzando l'ID. Se la chiave collide (utente già presente), espande dinamicamente tramite `realloc` il vettore interno di accumulo, collezionando il nuovo codice in tempo logaritmico.

#### `void avl_write_inorder(ReportAVL t, FILE* f_out, void (*write_func)(FILE*, unsigned int, unsigned int, int, char));`
* **Input:** L'albero `t`, il file aperto `f_out`, la funzione callback di formattazione `write_func`.
* **Pre-condizioni:** `t` inizializzato, `f_out` aperto in scrittura binarizzata.
* **Post-condizioni:** Esegue un attraversamento in ordine simmetrico (Sinistra $\rightarrow$ Radice $\rightarrow$ Destra) e riversa su disco un archivio indici ordinato matematicamente per chiave crescente.

---

## ADT PriorityQueue

### Tipo:
```c
typedef struct PriorityQueue* PriorityQueue;
```
(Opaque Pointer).

### Operazioni Formali:

#### `void pq_enqueue(PriorityQueue pq, Report r);`
* **Input:** La coda `pq`, l'oggetto `Report r` da inserire.
* **Pre-condizioni:** `pq` e `r` validi e non nulli.
* **Post-condizioni:** Alloca un nodo di giunzione e lo inserisce nella lista interna mantenendola ordinata tramite confronto incrociato stocastico: precedenza assoluta all'urgenza decrescente ('2' -> '1' -> '0') e, a parità, ordinamento FIFO stringa temporale data.


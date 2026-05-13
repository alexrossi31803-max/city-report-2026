# Questo documento contiene la specifica formale delle operazioni fornite dagli ADT creati, nel pieno rispetto del principio dell'Information Hiding.

---

# 1️⃣ ADT ReportList

## Tipo:
```c
typedef struct ReportList* ReportList;
```

(Opaque Pointer).

---

## Operazioni:

### `ReportList create_list();`

#### Input:
Nessuno.

#### Output:
Puntatore a una lista inizializzata.

#### Pre-condizioni:
Memoria di sistema disponibile.

#### Post-condizioni:
Ritorna un'istanza di lista con dimensione pari a 0 e puntatori a NULL.

---

### `void list_insert(ReportList l, Report r);`

#### Input:
La lista `l`, il puntatore all'oggetto `Report r`.

#### Pre-condizioni:
`l` non deve essere `NULL`, `r` deve essere un report valido.

#### Post-condizioni:
`r` viene inserito in testa alla lista. La dimensione aumenta di 1.

---

### `bool list_remove(ReportList l, int report_id);`

#### Input:
La lista `l`, l'ID intero del report da rimuovere.

#### Output:
`true` se rimosso, `false` altrimenti.

#### Post-condizioni:
Se trovato, il nodo viene rimosso e deallocato. La dimensione diminuisce di 1.

#### Effetti collaterali:
Deallocazione della memoria del report.

---

# 2️⃣ ADT ReportStack

## Tipo:
```c
typedef struct ReportStack* ReportStack;
```

(Opaque Pointer).

---

## Operazioni:

### `bool stack_push(ReportStack s, Report r);`

#### Input:
Lo stack `s`, il report corrente `r`.

#### Output:
`true` se inserito, `false` se lo stack è pieno (max 10).

#### Post-condizioni:
Viene allocata una copia esatta di `r` (clonazione profonda) e inserita in cima allo stack.

---

### `Report stack_pop(ReportStack s);`

#### Input:
Lo stack `s`.

#### Output:
Il puntatore al report in cima, `NULL` se lo stack è vuoto.

#### Post-condizioni:
Il puntatore in cima (`top`) decresce di 1.

Restituisce la proprietà della memoria del report rimosso al chiamante.

---

# 3️⃣ ADT ReportBST

## Tipo:
```c
typedef struct ReportBST* ReportBST;
```

(Opaque Pointer).

---

## Operazioni:

### `void bst_insert(ReportBST t, int id_user, Report r);`

#### Input:
L'albero `t`, la chiave intera `id_user`, l'oggetto `Report r`.

#### Pre-condizioni:
`t` valido, `r` valido.

#### Post-condizioni:
Inserisce un nodo nell'albero rispettando la proprietà del BST:

- i nodi con `id_user` minore vanno a sinistra
- i maggiori o uguali a destra

---

### `void bst_write_inorder(ReportBST t, FILE* f_out, void (*write_func)(FILE*, Report));`

#### Input:
L'albero `t`, il puntatore al file aperto `f_out`, una funzione di callback per la formattazione.

#### Post-condizioni:
Visita l'albero in ordine simmetrico e scrive le righe a lunghezza fissa nel file di testo.

Non modifica la struttura dell'albero.

---

# 4️⃣ ADT PriorityQueue

## Tipo:
```c
typedef struct PriorityQueue* PriorityQueue;
```

(Opaque Pointer).

---

## Operazioni:

### `void pq_enqueue(PriorityQueue pq, Report r);`

#### Input:
La coda `pq`, il report `r`.

#### Post-condizioni:
Il report viene inserito nella posizione corretta della lista interna.

La precedenza è data:

1. dall'urgenza maggiore (`3 \rightarrow 2 \rightarrow 1`)
2. in seconda istanza, dalla stringa temporale della data più vecchia

---

### `Report pq_dequeue(PriorityQueue pq);`

#### Input:
La coda `pq`.

#### Output:
Il report in testa alla coda (priorità massima assoluta).

#### Post-condizioni:
Rimuove il primo nodo e restituisce il report.


# Panoramica delle Strutture Dati (ADT) - City Report 2026

Il successo del sistema City Report si basa sull'integrazione di diverse strutture dati astratte, ognuna scelta per ottimizzare un aspetto specifico del workflow: gestione della memoria, velocità di ricerca o ordinamento delle priorità.

---

## 1. Report AVL Tree (Indice ad Alte Prestazioni)
L'AVL è il "motore di ricerca" del sistema. Essendo un albero binario di ricerca bilanciato, garantisce che l'altezza dell'albero sia sempre proporzionale a $\log n$.

* **Scopo**: Mappare l'ID univoco di una segnalazione alla sua posizione fisica (riga) nel file master sul disco.
* **Perché AVL?**: Rispetto a un albero binario semplice, l'AVL evita che l'albero diventi una lista (degenerazione) in caso di inserimenti di ID sequenziali, mantenendo le ricerche istantanee.
* **Complessità**: 
    * Ricerca: $O(\log n)$
    * Inserimento: $O(\log n)$

## 2. Priority Queue (Dashboard Dipendente)
La Priority Queue gestisce l'ordine con cui i dipendenti devono processare le segnalazioni.

* **Scopo**: Estrarre sempre la segnalazione con la massima urgenza e, a parità di urgenza, quella arrivata per prima (FIFO).
* **Criterio di Ordinamento**: 
    1.  **Urgenza**: Valore numerico (3 > 2 > 1).
    2.  **Data**: Confronto tra stringhe in formato "AAAA/MM/GG" (normalizzato per il confronto).
* **Implementazione**: Lista concatenata ordinata per garantire un'estrazione $O(1)$ della segnalazione più urgente.

## 3. Report Stack (Gestione Undo)
Lo stack implementa la logica LIFO (Last-In, First-Out) per la sessione utente.

* **Scopo**: Memorizzare temporaneamente i riferimenti ai report creati o modificati durante la sessione corrente.
* **Funzionalità**: Se l'utente clicca su "Undo", l'ultimo report viene estratto dallo stack e rimosso dalla lista di sessione prima che avvenga il commit sul disco.
* **Complessità**: $O(1)$ per Push e Pop.

## 4. Report List (Session Buffer)
Una classica lista concatenata semplice che funge da memoria volatile.

* **Scopo**: Contenere tutti i report gestiti durante la sessione corrente prima del logout.
* **Interazione**: Al momento del logout (Flush), la lista viene iterata interamente per trasferire i dati nel file `reports_bench.txt`.
* **Vantaggio**: Permette di gestire un numero arbitrario di segnalazioni in RAM senza allocare staticamente grandi array.

---

## Tabella Comparativa delle Complessità

| ADT | Operazione Principale | Complessità (Average) | Complessità (Worst) |
| :--- | :--- | :--- | :--- |
| **AVL Tree** | Ricerca ID | $O(\log n)$ | $O(\log n)$ |
| **Priority Queue** | Estrazione Priorità | $O(1)$ | $O(1)$ |
| **Stack** | Undo (Pop) | $O(1)$ | $O(1)$ |
| **Linked List** | Inserimento | $O(1)$ | $O(1)$ |

---

## Implementazione e Information Hiding
Tutte le strutture seguono il principio dell'**incapsulamento**:
1.  I file `.h` espongono solo il tipo `typedef struct NomeADT* NomeADT`.
2.  I dettagli implementativi (nodi, puntatori `next`, `left`, `right`) sono nascosti nei file `.c`.
3.  Questo permette di cambiare l'implementazione interna (es. passare da una Lista Ordinata a un Heap per la Priority Queue) senza dover modificare il resto del sistema.



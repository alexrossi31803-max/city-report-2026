#  Sistema di Gestione delle Segnalazioni Municipali (PSD)

[![C Language](https://shields.io)](https://gnu.org)
[![Platform](https://shields.io)](https://github.com)

Un'applicazione avanzata in **C99** progettata per la gestione delle segnalazioni di problemi urbani inviate dai cittadini al Comune. Il sistema implementa il principio dell'**Information Hiding** tramite *Opaque Pointers* e simula un database relazionale ad alte prestazioni su file flat binarizzati, garantendo ricerche logaritmiche ed inserimenti intelligenti con riciclo dei buchi in tempo costante.

---

##  1. Architettura di Memoria a Tre Livelli e Gestione del Disco

Il ciclo di vita di una segnalazione attraversa tre stadi hardware per azzerare i colli di bottiglia causati dalle operazioni di I/O:

1. **Sessione Volatile (RAM):** Accumulo delle segnalazioni in testa a una Linked List in tempo costante \(O(1)\). Modifiche isolate tramite uno Stack statico LIFO per consentire operazioni di Undo/Revert profonde. I report nascono con indice riga fisso impostato a `-1`.
2. **Cache Vettoriale Statica (BENCH):** Al logout, i dati passano nel file a blocchi fissi `reports_bench.txt` (50 slot da **332 byte**). Un contatore traccia la prima riga disponibile. Al riempimento, il contatore si azzera (`counter = 0`), rendendo le celle disponibili ad essere sovrascritte come in un buffer circolare ad alta velocità.
3. **Database Centralizzato con Inserimento Intelligente (Master Files):** Al flush dei 50 elementi della cache, i record storici modificati vengono eliminati dal vecchio file di stato in **\(O(1)\)** eseguendo un salto `fseek` immediato sulla riga indicata dal report e settando il flag speciale su `'V'` (Vuoto/Buco). Il record aggiornato viene scritto nel nuovo file di stato occupando il primo buco noto per quel file e risigillando la sentinella di fine dati logici `'E'`. Il nuovo numero di riga fisica viene iniettato nel report, garantendo la consistenza assoluta.

---

##  2. Strutture Dati Astratte (ADT) e Sincronizzazione Indici

Al termine del flush pesante della BENCH, il server rigenera da zero i file d'indice estratti in modalità ordinata *In-Order*:

* **`report_BST_BY_REPORT_ID.txt` [O(log n)]:** Albero organizzato secondo la chiave `report_id`. È il vero Punto di Verità dello stato dei dati. Il dipendente vi effettua ricerche e modifiche istantanee in tempo logaritmico.
* **`report_BST_BY_USER_ID.txt` [O(log n)]:** Albero organizzato secondo la chiave `user_id`. I nodi contengono esclusivamente l'ID utente e l'array dei codici report a lui associati. Il cittadino estrae logaritmicamente i propri codici storici e li risolve sul BST dell'ID report, visualizzando lo storico con lo stato della pratica sempre perfettamente sincronizzato in tempo reale.
* **`reports_by_priority.txt` [O(1)]:** Coda a priorità ad ordinamento incrociato (Urgenza decrescente + Data FIFO più vecchia). Trattandosi di un'operazione costosa, viene compilata **solo sotto esplicita richiesta del dipendente**, eseguendo un flush forzato preventivo della cache e scansionando i soli file master attivi.

---

##  5. Compilazione ed Esecuzione

Il progetto include un `Makefile` configurato per la compilazione modulare separata tramite `gcc`.

### Compilazione Rapida tramite Makefile
```bash
make
```

### Compilazione Manuale tramite GCC
```bash
gcc -Wall -Wextra -pedantic -std=c99 -Iinclude src/main.c src/models/user.c src/models/report.c src/adt/report_list.c src/adt/report_stack.c src/adt/report_bst.c src/adt/priority_queue.c src/utils/validators.c src/utils/parser.c src/server/user_manager.c src/server/report_manager.c src/tests/test_suite.c -o segnalazioni_municipali.exe
```

### Avvio dell'Applicazione
Linux/macOS:
```bash
./segnalazioni_municipali.exe
```

Windows:
```bash
segnalazioni_municipali.exe
```

### Pulizia dei File Oggetto (.o)
```bash
make clean
```

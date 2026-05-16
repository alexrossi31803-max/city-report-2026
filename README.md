# Sistema di Gestione delle Segnalazioni Municipali (Architettura AVL)

Un'applicazione avanzata in **C99** progettata per la gestione e il monitoraggio delle segnalazioni di problemi urbani inviate dai cittadini al Comune. Il sistema implementa il principio dell'**Information Hiding** tramite *Opaque Pointers* e simula un database relazionale ad alte prestazioni su file flat binarizzati, garantendo ricerche bilanciate ed inserimenti intelligenti con riciclo dei buchi in tempo costante.

---

## 1. Architettura di Memoria e Flusso Geometrico dei Dati

Il ciclo di vita di una segnalazione attraversa tre stadi hardware per azzerare i colli di bottiglia causati dalle operazioni di I/O:

1. **Sessione Volatile (RAM):** Accumulo delle segnalazioni in testa a una Linked List in tempo costante $O(1)$. Modifiche isolate tramite uno Stack statico LIFO per consentire operazioni di Undo/Revert profonde. I report nascono con indice riga fisso impostato a `-1`.
2. **Cache Vettoriale Statica (BENCH):** Al logout, i dati passano nel file a blocchi fissi `reports_bench.txt` (50 slot regolati dalla macro **`REPORT_BENCH_LINE` a 351 byte**). Un contatore traccia la prima riga disponibile. Al riempimento, il contatore si azzera (`counter = 0`), rendendo le celle disponibili ad essere sovrascritte come in un buffer circolare ad alta velocità.
3. **Database Centralizzato con Inserimento Intelligente (Master Files):** Al flush dei 50 elementi della cache, i record storici modificati vengono eliminati dal vecchio file di stato in **$O(1)$** eseguendo un salto `fseek` immediato sulla riga indicata dal report moltiplicata per **`REPORT_MASTER_LINE` (352 byte)** e settando il flag speciale su `'N'` (Null). Il numero di riga liberato viene inserito in un file ausiliario gestito come uno stack LIFO. Il nuovo record aggiornato viene scritto nel nuovo file di stato occupando il primo buco estratto in tempo costante dallo stack LIFO, iniettando il nuovo numero di riga fisica nel report.
---

## 2. Strutture Dati Astratte (ADT) e Sincronizzazione Indici

Al termine del flush della BENCH, il server rigenera da zero i file d'indice estratti in modalità ordinata *In-Order*:

* **`report_AVL_BY_REPORT_ID.txt` [O(log n)]:** Albero organizzato secondo la chiave `report_id`. È il vero Punto di Verità dello stato dei dati. Il dipendente vi effettua ricerche e modifiche istantanee in tempo logaritmico garantito dalle rotazioni di bilanciamento.
* **`report_AVL_BY_USER_ID.txt` [O(log n)]:** Albero organizzato secondo la chiave `user_id`. I nodi contengono vettori dinamici ad accumulo locale per raccogliere più segnalazioni inviate dallo stesso cittadino.
* **`reports_by_priority.txt` [O(1)]:** Coda a priorità ad ordinamento incrociato (Urgenza decrescente + Data FIFO più vecchia). Trattandosi di un'operazione costosa, viene compilata **solo sotto esplicita richiesta del dipendente**, eseguendo un flush forzato della cache e scansionando i file master attivi.

---

## 3. Indicatori Statistici Istantanei O(1)

A differenza dei sistemi tradizionali, la generazione del report comunale non esegue scansioni sequenziali orizzontali distruttive sugli archivi storici. Il server aggiorna atomicamente 11 righe fisse a 11 byte nel file `system_total_report.txt` ad ogni cambio di stato o inserimento. La dashboard del dipendente esegue letture mirate tramite `fseek` visualizzando all'istante l'anagrafica statistica del Comune.

---

## 4. Compilazione ed Esecuzione Manuale tramite GCC

Il progetto è strutturato per essere compilato in modo modulare separato. 

### Comando di Compilazione Unificato
```bash
gcc -Wall -Wextra -pedantic -std=c99 -Iinclude src/main.c src/models/user.c src/models/report.c src/adt/report_list.c src/adt/report_stack.c src/adt/report_avl.c src/adt/priority_queue.c src/utils/validators.c src/utils/parser.c src/server/user_manager.c src/server/report_manager.c src/tests/test_suite.c -o segnalazioni_municipali.exe
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


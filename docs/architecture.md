# Architettura del Database Municipale e Ciclo di Vita del Report (v5.0 Definitiva)

Il sistema implementa un'architettura di persistenza a tre livelli per minimizzare i colli di bottiglia dell'I/O su disco, simulando il comportamento di un motore relazionale a tabelle fisse.

---

## 1. Mappa dei Livelli di Persistenza e Geometria Hardware

Ogni file all'interno del database adotta una geometria di riga rigida e immutabile. Questo consente l'accesso diretto tramite calcolo matematico dell'offset del cursore, escludendo le scansioni sequenziali orizzontali.


| Livello / Archivio | File Fisico Corrispondente | Dimensione Riga | Meccanismo di Accesso | Stato e Indicatori di Cella |
| :--- | :--- | :---: | :---: | :--- |
| **1. RAM Volatile** | Sessione Locale Cittadino | Puntatore Opaco | $\mathcal{O}(1)$ in testa / $\mathcal{O}(n)$ ricerca | Strutture dinamiche ad Information Hiding (`ReportList`, `ReportStack`). |
| **2. Cache Vettoriale** | `reports_bench.txt` | **351 Byte** (`REPORT_BENCH_LINE`) | $\mathcal{O}(n)$ sequenziale controllato fino al counter | Buffer circolare statico da 50 slot. Flag di cella al byte 350 (`\n`). |
| **3. Database Master** | `open_reports.txt`<br>`in_progress_reports.txt`<br>`closed_reports.txt` | **352 Byte** (`REPORT_MASTER_LINE`) | $\mathcal{O}(1)$ diretto tramite `disk_row` | Struttura finale. Byte 350: `'A'` (Attivo), `'N'` (Null), `'E'` (Sentinella). Byte 351: `\n`. |
| **4. Indice Report ID** | `report_AVL_BY_REPORT_ID.txt` | **22 Byte** (`AVL_REPORT_ID_LINE`) | $\mathcal{O}(\log n)$ Ricerca Binaria ad array ordinato | Generato via *In-Order* AVL. Formato: `[REPORT_ID(10)][STATUS(1)][DISK_ROW(10)]\n`. |
| **5. Indice User ID** | `report_AVL_BY_USER_ID.txt` | **21 Byte** (`AVL_USER_ID_LINE`) | $\mathcal{O}(\log n)$ Ricerca Dicotomica con range bilaterale | Generato via *In-Order* AVL. Formato: `[USER_ID(10)][REPORT_ID(10)]\n`. |
| **6. Registro Centrale**| `system_total_report.txt` | **11 Byte** (`SYSTEM_REG_LINE`) | $\mathcal{O}(1)$ diretto su riga fissa tramite `fseek` | Memorizza 13 contatori numerici globali atomici (`%010u\n`). |

---

## 2. Flusso Invertito Consistente: Storico del Cittadino

Per garantire l'allineamento assoluto dei dati del cittadino senza consumare risorse in RAM sul server, l'interfaccia esegue la compilazione della vista secondo la regola dell'**Inversione Temporale**:

## [1. FASE MASTER]

- AVL User ID (21B)
- Triangola AVL Report (22B) → `O(log n)`
- Estrae da Master (352B) → `O(1) fseek`
- Recupera il record solo se la cella è contrassegnata `'A'`

## [2. FASE CACHE BENCH]

- `reports_bench.txt` (351B)
- Scansione lineare → `O(n)` fino al counter attivo
- Se:
  - Match Utente
  - Cella `'A'`
- Allora:
  - Rimuove il vecchio nodo Master
  - Inserisce la variante Bench
  - Ignora i record logicamente `DESTROYED`

## [3. FASE RAM LOCAL]

- Lista RAM Volatile
- Inserimento in testa → `O(1)`
- Mantiene:
  - nuovi report di sessione
  - report non ancora flussati in cache

## [4. RENDERING]

- Rewind lista
- Mostra a video:
  - storico depurato
  - duplicati logici rimossi
  - record `DESTROYED` esclusi

## Se cella è contrassegnata 'A'.

1. **Iniezione Master:** Il client interroga direttamente l'indice `PATH_AVL_USER_ID` su disco tramite ricerca dicotomica logaritmica. Per ogni corrispondenza, triangola con l'indice Report ID ricavando la `disk_row`, salta sul rispettivo Master a 352 byte e, se la cella è contrassegnata come Active (`'A'`), inietta il record nella lista.
2. **Sovrascrittura Cache BENCH:** Subito dopo, scansiona la cache transitoria a passi da 351 byte fino al contatore corrente. Se rileva un record del cittadino attivo, invoca `list_remove` per sterminare la versione Master obsoleta, inserendo la variante BENCH aggiornata (a patto che non sia `DESTROYED`).
3. **Sincronizzazione RAM:** I report creati nella sessione locale corrente rimangono in cima alla lista. Il `list_rewind` finale mostra la verità assoluta del database.

---

## 3. Algoritmo di Sfoltimento ed Eliminazione del Dipendente

Quando il dipendente comunale utilizza la funzione `employee_change_report_status` per far avanzare una pratica o distruggerla (`DESTROYED`), il finto server applica un algoritmo di sfoltimento asincrono coordinato al singolo byte:

1. **Ricerca Orizzontale in Cache:** Il sistema verifica se il report risiede momentaneamente nella BENCH in $\mathcal{O}(n)$. Se lo trova, aggiorna i contatori nel registro in $\mathcal{O}(1)$ tramite la funzione helper `update_system_counters`, applica lo stato e rispiana la cella della cache a 351 byte.
2. **Ricerca Logaritmica e Invalidazione Master:** Se assente in BENCH, interroga l'array d'indice dei report su disco in $\mathcal{O}(\log n)$ tramite `findReportId`. Individuata la `disk_row`, il server esegue un salto `fseek` sul file master di provenienza e **sovrascrive il byte offset 350 impostandolo a 'N' (Null)**, distruggendo la cella e trasformandola in un buco.
3. **Accatastamento LIFO dei Buchi:** L'indice della riga liberata viene immediatamente spinto (operazione Push) in coda al file ausiliario dei buchi specifico (`open_holes.txt`, ecc.) occupando una riga da 11 byte.
4. **Immissione Controllata in Cache:** Il report modificato viene caricato in Append in fondo alla BENCH, portando il contatore della cache a `+1`, ma **senza mai incrementare il numero dei report totali del Comune**, in quanto la segnalazione era già censita nel sistema.

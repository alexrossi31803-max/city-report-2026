\# Municipal Report Management System



\## Problem Analysis



\### Objective



The objective of this project is to develop a municipal report management system in C capable of handling urban issue reports submitted by citizens.



The system allows users to create, monitor and manage reports related to:

\- road damage

\- public lighting failures

\- abandoned waste

\- public infrastructure faults



The application simulates a simplified municipal backend system using persistent files and in-memory data structures.



\---



\## System Goals



The system must allow:



\- citizen registration and authentication

\- employee authentication

\- creation of new reports

\- report visualization

\- report search by identifier or category

\- report status updates

\- report deletion

\- filtering by status and urgency level

\- generation of statistical reports

\- persistence of data after program termination



\---



\## Actors



\### Citizen



Citizens can:

\- authenticate into the system

\- create reports

\- visualize their own reports



Citizens have limited permissions and cannot modify reports created by other users.



\---



\### Municipal Employee



Municipal employees can:

\- visualize all reports

\- search reports

\- update report status

\- delete reports

\- generate system statistics

\- access filtered report visualizations



Employees represent the administrative component of the system.



\---



\## Functional Requirements



The system supports:



\- user authentication

\- report registration

\- report classification

\- report filtering

\- report persistence

\- report indexing

\- report statistics generation

\- urgency-based management of reports



All functionalities are implemented using Abstract Data Types (ADT) and modular C programming.



\---



\## Non-Functional Requirements



The project must guarantee:



\- modularity

\- maintainability

\- data persistence

\- abstraction of data structures

\- separation between interface and implementation

\- efficient search operations

\- dynamic memory management

\- ADT-based design



\---



\## Persistence Model



Persistent information is stored using text files inside the `data/` directory.



The system maintains:

\- users

\- employees

\- reports\_master

\- derived report files



The architecture distinguishes between:

\- primary persistent storage (master files)

\- automatically generated derived files



Data persists between different executions of the program.



\---



\## Runtime Model



At startup, the system loads persistent data into RAM structures.



The application simulates backend processing by:

\- loading reports into memory

\- generating auxiliary ADTs

\- creating filtered report views

\- rebuilding derived files after updates



This approach simulates a simplified server-side processing architecture while remaining entirely sequential and local.



\---



\## Data Classification



The system distinguishes between:



\### Persistent Data



Stored permanently on files:

\- users

\- employees

\- reports\_master



\### Volatile Data



Stored temporarily in RAM:

\- action history (stack ADT)

\- runtime indexes (hash table ADT)

\- temporary sorted structures

\- filtering structures



\---



\## Design Philosophy



The project emphasizes:



\- Abstract Data Types (ADT)

\- Information Hiding

\- modular programming

\- dynamic memory structures

\- computational complexity analysis

\- separation between persistent storage and runtime processing



The system is designed as a simulation of a municipal backend, where all processing is handled locally without external servers, using only C standard programming techniques.


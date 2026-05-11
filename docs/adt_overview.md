\# Abstract Data Types (ADT) Overview



\## Introduction



The system is designed using the Abstract Data Type (ADT) paradigm in order to achieve modularity, data abstraction, and separation between interface and implementation.



Each core component of the system is modeled as an ADT to encapsulate data structures and operations while hiding internal implementation details.



This approach follows the principles of:

\- Information Hiding

\- Data Abstraction

\- Modular Programming

\- Separation of Concerns



\---



\# Core ADTs in the System



\## 1. ADT User



\### Description



The User ADT represents both citizens and municipal employees interacting with the system.



\### Representation



Each user contains:



\- unique identifier

\- username

\- password

\- role (citizen or employee)



\### Operations



\- create user

\- authenticate user

\- validate credentials

\- retrieve user role



\### Design Motivation



The User ADT encapsulates authentication logic and role management, ensuring that user data is not directly manipulated by external modules.



\---



\## 2. ADT Report



\### Description



The Report ADT represents a municipal issue reported by a citizen.



It is the core entity of the system.



\### Representation



A report contains:



\- unique identifier

\- citizen name

\- category

\- description

\- creation date

\- urgency level

\- status



\### Operations



\- create report

\- update status

\- retrieve report data

\- validate report fields



\### Design Motivation



The Report ADT encapsulates all data related to a single report, ensuring consistency and preventing invalid modifications.



\---



\## 3. ADT Report List (Linked List)



\### Description



The Report List ADT manages a dynamic collection of reports using a linked list structure.



\### Operations



\- insert report

\- delete report

\- search by ID

\- search by category

\- traverse list

\- update report node



\### Design Motivation



A linked list is chosen because:

\- the number of reports is dynamic

\- frequent insertions and deletions are required

\- memory is allocated dynamically

\- sequential traversal is sufficient for most operations



\### Complexity



\- insertion: O(1)

\- search: O(n)

\- deletion: O(n)



\---



\## 4. ADT User List



\### Description



Manages a dynamic collection of users.



\### Operations



\- insert user

\- find user

\- authenticate user

\- traverse users



\### Design Motivation



A linked list is sufficient due to the limited number of users and simplicity of operations.



\---



\## 5. ADT Hash Table (Report Index)



\### Description



A hash table is used to provide fast access to reports by their unique identifier.



\### Operations



\- insert key-value pair (ID → Report)

\- search report by ID

\- delete entry

\- update entry



\### Design Motivation



The hash table significantly improves lookup performance compared to linear search in a linked list.



\### Complexity



\- average search: O(1)

\- worst case: O(n)



\---



\## 6. ADT Stack (Action History)



\### Description



The Stack ADT is used to store user actions during runtime.



\### Operations



\- push action

\- pop action

\- view last action



\### Use Cases



\- undo-like behavior simulation

\- logging recent operations

\- debugging support



\### Design Motivation



A stack is appropriate because it follows LIFO behavior, which matches the concept of undoing the most recent operation first.



\---



\## 7. (Optional) ADT Queue (Future Extension)



\### Description



A queue could be used to simulate processing order of incoming reports.



\### Operations



\- enqueue report

\- dequeue report

\- view front element



\### Design Motivation



A queue follows FIFO behavior and could be used to model real-world report processing pipelines.



This ADT is considered a possible extension and not mandatory in the current implementation.



\---



\# Global Design Considerations



\## Information Hiding



Each ADT exposes only its interface while hiding internal implementation details. This ensures:

\- modularity

\- maintainability

\- safe data manipulation



\---



\## Memory Management



All ADTs relying on dynamic memory (lists, stacks, hash tables) use:

\- malloc

\- free



Proper memory management is essential to avoid leaks.



\---



\## Computational Complexity



The system is designed with efficiency considerations:



\- Linked List → flexible but linear search

\- Hash Table → fast lookup

\- Stack → constant-time operations



The combination of ADTs allows balancing simplicity and performance.



\---



\## Architectural Role of ADTs



ADTs are used to separate responsibilities:



\- User ADT → authentication layer

\- Report ADT → core domain entity

\- Report List → data storage layer

\- Hash Table → indexing layer

\- Stack → runtime behavior tracking



This separation reflects a layered architecture similar to real backend systems.



\---



\## Conclusion



The use of ADTs in this project allows the simulation of a structured backend system using only procedural C programming.



Each ADT has been selected to balance:

\- simplicity

\- efficiency

\- educational value

\- consistency with course topics


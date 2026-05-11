\# System Architecture



\## General Architecture



The project simulates a simplified municipal backend system entirely implemented in standard C.



The application is composed of:

\- persistent storage files

\- volatile in-memory structures

\- simulated backend processing modules

\- user authentication and role management



The architecture follows a modular approach based on Abstract Data Types (ADT) and Information Hiding principles.



\---



\## System Workflow



The application follows this execution pipeline:



```text

Program Startup

&#x20;       ↓

Load Persistent Files

&#x20;       ↓

Build RAM Structures

&#x20;       ↓

Generate Runtime Indexes

&#x20;       ↓

Generate Derived Files

&#x20;       ↓

User Interaction

&#x20;       ↓

Data Update

&#x20;       ↓

Refresh Runtime Structures

&#x20;       ↓

Save Persistent Data

```



\## User Roles



The system distinguishes two main actors:



\### Citizen



Citizens can:



authenticate into the system

create reports

visualize their own reports



Citizens have limited permissions.



\### Municipal Employee



Employees can:



visualize all reports

search reports

update report status

delete reports

generate statistical reports

access filtered report views



Employees represent the administrative management component of the system.



\## Persistent Storage



Persistent data is stored inside the data/ directory using text files.



The project distinguishes between:



\### Master Files



Master files represent the primary persistent storage.



users.txt



Contains registered citizen accounts.



employees.txt



Contains municipal employee accounts.



reports\_master.txt



Contains all registered reports.



This file represents the main source of truth for report persistence.



\### Derived Files



Derived files are automatically generated from the master report file.



These files simulate backend indexing and preprocessing operations.



Examples:



reports\_open.txt

reports\_closed.txt

reports\_priority.txt

reports\_category.txt



Derived files are regenerated whenever report data changes.



\## Runtime Memory Model



At program startup, persistent files are loaded into RAM structures.



The system performs most operations in memory to improve efficiency and simplify data manipulation.



The runtime model distinguishes between:



persistent data

volatile runtime structures



\## Volatile Runtime Structures



Volatile structures exist only during program execution.



These include:



linked lists

hash indexes

action history stacks

temporary sorting structures

filtering structures



These structures are rebuilt at every program startup.



\## Simulated Backend Processing



The project simulates backend processing without using real servers or concurrency mechanisms.



Backend operations are implemented through sequential functions that:



load data

organize structures

regenerate indexes

rebuild derived files

refresh filtered views



This approach allows the system to simulate realistic backend behavior while remaining compatible with standard C programming techniques.



\## Refresh Workflow



Whenever the system performs a critical operation such as:



report insertion

report deletion

report status update



the application automatically executes a refresh process.



The refresh process:



updates RAM structures

rebuilds indexes

regenerates derived files

synchronizes persistent storage



This mechanism replaces the need for background services or periodic server synchronization.



\## Report Lifecycle



A report follows this lifecycle:



```text

Creation

&#x20;   ↓

Storage in Master File

&#x20;   ↓

Loading into RAM

&#x20;   ↓

Indexing and Classification

&#x20;   ↓

Visualization and Management

&#x20;   ↓

Status Update / Deletion

&#x20;   ↓

Refresh and Synchronization

```



\## Pagination System



The system avoids printing all reports simultaneously.



Report visualization uses pagination:



only a limited number of reports is shown at once

additional reports can be loaded incrementally



This improves:



readability

usability

testing quality

\## Data Structures Overview



The project combines multiple ADTs.



\### Linked Lists



Used for:



dynamic report storage

user storage

traversal operations

\### Hash Tables



Used for:



fast report lookup by identifier



Hashing improves average search complexity.



\### Stacks



Used for:



action history

undo-like operations

runtime activity tracking



Stacks are volatile and exist only in RAM.



\## Computational Considerations



The architecture prioritizes:



modularity

maintainability

efficient search operations

dynamic memory management



The system intentionally separates:



persistent storage

runtime processing

indexing structures



This improves scalability and organization.



\## Design Philosophy



The project aims to simulate a realistic municipal management platform while remaining compatible with:



procedural programming

standard C

ADT-oriented design

educational software engineering principles



The implementation emphasizes:



abstraction

modularity

information hiding

data persistence

computational complexity awareness










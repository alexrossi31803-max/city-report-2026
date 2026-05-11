#ifndef LIST_H
#define LIST_H

#include "../models/report.h"
/*
ADT REPORT LIST (LINKED LIST)

Represents the main in-memory dynamic
collection of reports.

This structure is loaded from persistent
storage (reports_master.txt) and used
during runtime operations.
*/
/*
The internal structure is hidden to enforce information hiding.
*/
typedef struct ReportList* ReportList;
//LIST OPERATIONS
/*
create_list -> Creates an empty list.

Preconditions:
- none

Postconditions:
- returns allocated ReportList
- head is initialized to NULL
*/
ReportList create_list();
/*
insert_report -> Inserts a report into the list.

Preconditions:
- list must be valid (not NULL)
- report must be valid

Postconditions:
- new node is added to the list
- list size increases by 1

Side effects:
- dynamic memory allocation (malloc)
*/
void insert_report(ReportList list, Report r);
/*
delete_report -> Deletes a report by ID.

Preconditions:
- list must be valid
- id may or may not exist

Postconditions:
- if found, node is removed and memory freed
- list integrity preserved
*/
void delete_report(ReportList list, int id);
/*
search_report -> Searches a report by ID.

Preconditions:
- list must be valid

Postconditions:
- returns pointer to report if found
- returns NULL if not found

Side effects:
- none (read-only operation)
*/
Report* search_report(ReportList list, int id);

#endif
/*
STRUCTURE NODE -> Represents a single element of the list
Invariants:
- data contains a valid Report
- next is either NULL or valid Node*
*/
 typedef struct Node {
    Report data;
    struct Node* next;
} Node;
/*
STRUCTURE REPORTLIST -> Encapsulates the head pointer of the list
Invariants:
- head == NULL → empty list
- otherwise points to first valid node
*/
struct ReportList {
    Node* head;
};
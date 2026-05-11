#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include "../models/report.h"
/*
ADT HASH TABLE (REPORT INDEX)
This ADT provides fast access to reports
using their unique identifier (key).

It acts as an indexing layer over the
primary ReportList structure.

Purpose:
- optimize search operations (O(1) average)
- avoid linear traversal of linked list
*/
#define TABLE_SIZE 100
/*
The internal structure is hidden to enforce information hiding.
*/
typedef struct HashTable* HashTable;
//HASH TABLE OPERATIONS
/*
create_hash_table -> Creates an empty hash table.

Preconditions:
- none

Postconditions:
- all buckets initialized to NULL
- returns allocated HashTable
*/
HashTable create_hash_table();
/*
insert_hash -> Inserts a key-value pair into the table.

Preconditions:
- ht must be valid
- key must be valid and unique (report ID)
- value must point to valid Report

Postconditions:
- new node inserted in correct bucket
- collision handled via chaining

Side effects:
- dynamic memory allocation
*/
void insert_hash(HashTable ht, int key, Report value);
/*
search_hash -> Searches a report by key.

Preconditions:
- ht must be valid

Postconditions:
- returns pointer to Report if found
- returns NULL otherwise

Side effects:
- none (read-only)
*/
Report search_hash(HashTable ht, int key);
/*
delete_hash -> Deletes entry by key.

Preconditions:
- ht must be valid

Postconditions:
- node removed if key exists
- memory freed
- table integrity preserved
*/
void delete_hash(HashTable ht, int key);

#endif
/*
STRUCTURE HASH NODE -> Represents a single entry in the hash table.

Invariants:
- key is unique report identifier
- value points to a valid Report
- next handles collision chaining
*/
typedef struct HashNode {
    int key;
    Report value;
    struct HashNode* next;
} HashNode;

/*
STRUCTURE HASHTABLE -> Array of buckets using separate chaining.

Invariants:
- each index may contain a linked list
- table[i] == NULL or valid chain
*/
struct HashTable {
    HashNode* table[TABLE_SIZE];
};
#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H
#include "../models/report.h"

typedef struct PriorityQueue* PriorityQueue;

PriorityQueue create_pq();
void free_pq(PriorityQueue pq);
void pq_enqueue(PriorityQueue pq, Report r);
Report pq_dequeue(PriorityQueue pq);
bool pq_is_empty(PriorityQueue pq);

#endif

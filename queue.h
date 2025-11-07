#ifndef QUEUE_H
#define QUEUE_H

#include "huffman_dynamic.h"

#define MAX_SIZE 512

typedef struct
{
    HuffmanDynamicCode *items[MAX_SIZE];
    int size;
} Queue;

void pq_init(Queue *pq);

void swap(HuffmanDynamicCode **a, HuffmanDynamicCode **b);

void heapify_up(Queue *pq, int idx);
void enqueue(Queue *pq, HuffmanDynamicCode *node);

void heapify_down(Queue *pq, int idx);
HuffmanDynamicCode *dequeue(Queue *pq);

HuffmanDynamicCode *peek(Queue *pq);

#endif

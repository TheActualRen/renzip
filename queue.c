#include "queue.h"

#include <stdio.h>

void pq_init(Queue *pq)
{
    pq->size = 0;
}

void swap(HuffmanDynamicCode **a, HuffmanDynamicCode **b)
{
    HuffmanDynamicCode *temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_up(Queue *pq, int idx)
{
    const int mid = (idx - 1) / 2;

    if (idx && pq->items[mid] > pq->items[idx])
    {
        swap(&pq->items[mid], &pq->items[idx]);
        heapify_up(pq, mid);
    }
}

void enqueue(Queue *pq, HuffmanDynamicCode *node)
{
    if (pq->size == MAX_SIZE)
    {
        printf("Priority Queue is full\n");
        return;
    }

    pq->items[pq->size] = node;
    pq->size++;

    heapify_up(pq, pq->size - 1);
}

void heapify_down(Queue *pq, int idx)
{
    int min = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < pq->size && pq->items[left] < pq->items[min])
    {
        min = left;
    }

    if (right < pq->size && pq->items[right] < pq->items[min])
    {
        min = right;
    }

    if (min != idx)
    {
        swap(&pq->items[idx], &pq->items[min]);
        heapify_down(pq, min);
    }
}

HuffmanDynamicCode *dequeue(Queue *pq)
{
    if (!pq->size)
    {
        printf("Priority queue is empty\n");
        return NULL;
    }

    HuffmanDynamicCode *item = pq->items[0];

    pq->items[0] = pq->items[pq->size - 1];
    pq->size--;

    heapify_down(pq, 0);

    return item;
}

HuffmanDynamicCode *peek(Queue *pq)
{
    if (!pq->size)
    {
        printf("Priority Queue is empty\n");
        return NULL;
    }

    return pq->items[0];
}

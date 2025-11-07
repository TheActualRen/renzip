#include "queue.h"
#include "huffman_dynamic.h"

#include <stdlib.h>

HuffmanDynamicCode *build_huffman_tree(uint32_t *freqs, int nsym)
{
    Queue pq;
    pq_init(&pq);

    for (int i = 0; i < nsym; i++)
    {
        if (freqs[i] > 0)
        {
            HuffmanDynamicCode *node = malloc(sizeof(HuffmanDynamicCode));

            node->symbol = i;
            node->freq = freqs[i];
            node->left = NULL;
            node->right = NULL;

            enqueue(&pq, node);
        }
    }

    // One symbol
    if (pq.size == 0) return NULL;

    if (pq.size == 1)
    {
        HuffmanDynamicCode *only = dequeue(&pq);
        HuffmanDynamicCode *dummy = malloc(sizeof(HuffmanDynamicCode));

        dummy->symbol = 0;
        dummy->freq = only->freq;
        dummy->left = only;
        dummy->right = NULL;

        return dummy;
    }

    while (pq.size > 1)
    {
        HuffmanDynamicCode *left = dequeue(&pq);
        HuffmanDynamicCode *right = dequeue(&pq);

        HuffmanDynamicCode *parent = malloc(sizeof(HuffmanDynamicCode));

        parent->symbol = 0; // internal nodes don't store symbols
        parent->freq = left->freq + right->freq;
        
        parent->left = left;
        parent->right = right;

        enqueue(&pq, parent);
    }

    return dequeue(&pq);
}

void generate_code_lengths(HuffmanDynamicCode *node, 
        uint8_t *code_lengths, int depth)
{
    if (!node) return;

    if (!node->left && !node->right)
    {
        code_lengths[node->symbol] = depth;
    }

    else 
    {
        generate_code_lengths(node->left, code_lengths, depth + 1);
        generate_code_lengths(node->right, code_lengths, depth + 1);
    }
}

void free_huffman_tree(HuffmanDynamicCode *node)
{
    if (!node) return;

    free_huffman_tree(node->left);
    free_huffman_tree(node->right);

    free(node);
}

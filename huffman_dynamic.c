#include "huffman_dynamic.h"

#include "queue.h"

#include <stdlib.h>
#include <string.h>

static HuffmanDynamicCode *new_node(uint16_t symbol, uint32_t freq)
{
    HuffmanDynamicCode *node = malloc(sizeof(HuffmanDynamicCode));
    if (!node)
    {
        return NULL;
    }

    node->symbol = symbol;
    node->freq = freq;
    node->left = NULL;
    node->right = NULL;
    return node;
}

HuffmanDynamicCode *build_huffman_tree(uint32_t *freqs, int nsym)
{
    Queue pq;
    pq_init(&pq);

    /* push only non-zero symbols */
    for (int i = 0; i < nsym; i++)
    {
        if (freqs[i] > 0)
        {
            HuffmanDynamicCode *node = new_node(i, freqs[i]);

            if (!node)
            {
                return NULL;
            }
            enqueue(&pq, node);
        }
    }

    /* no symbols */
    if (pq.size == 0)
    {
        return NULL;
    }

    /* single-symbol case (DEFLATE requires two leaves) */
    if (pq.size == 1)
    {
        HuffmanDynamicCode *only = dequeue(&pq);

        HuffmanDynamicCode *dummy = new_node(0, 0);
        HuffmanDynamicCode *parent = new_node(0, only->freq);

        if (!dummy || !parent)
        {
            return NULL;
        }

        parent->left = only;
        parent->right = dummy;

        return parent;
    }

    /* normal Huffman merge */
    while (pq.size > 1)
    {
        HuffmanDynamicCode *left = dequeue(&pq);
        HuffmanDynamicCode *right = dequeue(&pq);

        HuffmanDynamicCode *parent = new_node(0, left->freq + right->freq);

        if (!parent)
        {
            return NULL;
        }

        parent->left = left;
        parent->right = right;

        enqueue(&pq, parent);
    }

    return dequeue(&pq);
}

static void generate_code_lengths_rec(HuffmanDynamicCode *node,
                                      uint8_t *code_lengths, int depth)
{
    if (!node)
    {
        return;
    }

    if (depth > MAX_CODELEN)
    {
        depth = MAX_CODELEN;
    }

    if (!node->left && !node->right)
    {
        code_lengths[node->symbol] = (uint8_t)depth;
        return;
    }

    generate_code_lengths_rec(node->left, code_lengths, depth + 1);
    generate_code_lengths_rec(node->right, code_lengths, depth + 1);
}

void generate_code_lengths(HuffmanDynamicCode *root, uint8_t *code_lengths,
                           int nsym)
{
    memset(code_lengths, 0, nsym);
    generate_code_lengths_rec(root, code_lengths, 0);
}

HuffmanDynamicCode *build_huffman_tree_from_lengths(uint8_t *lengths, int nsym)
{
    int max_len = 0;

    for (int i = 0; i < nsym; i++)
    {
        if (lengths[i] > max_len)
        {
            max_len = lengths[i];
        }
    }

    if (max_len == 0)
    {
        return NULL;
    }

    int bl_count[MAX_CODELEN + 1] = {0};

    for (int i = 0; i < nsym; i++)
    {
        if (lengths[i] > 0)
        {
            bl_count[lengths[i]]++;
        }
    }

    int next_code[MAX_CODELEN + 1] = {0};
    int code = 0;

    for (int bits = 1; bits <= max_len; bits++)
    {
        code = (code + bl_count[bits - 1]) << 1;
        next_code[bits] = code;
    }

    HuffmanDynamicCode *root = new_node(0, 0);

    if (!root)
    {
        return NULL;
    }

    for (int sym = 0; sym < nsym; sym++)
    {
        int len = lengths[sym];

        if (len == 0)
        {
            continue;
        }

        int cur_code = next_code[len]++;
        HuffmanDynamicCode *node = root;

        for (int i = len - 1; i >= 0; i--)
        {
            int bit = (cur_code >> i) & 1;

            if (bit == 0)
            {
                if (!node->left)
                {
                    node->left = new_node(0, 0);
                }

                node = node->left;
            }

            else
            {
                if (!node->right)
                    node->right = new_node(0, 0);
                node = node->right;
            }

            if (!node)
            {
                return NULL;
            }
        }

        node->symbol = sym;
    }

    return root;
}

void free_huffman_tree(HuffmanDynamicCode *node)
{
    if (!node)
    {
        return;
    }

    free_huffman_tree(node->left);
    free_huffman_tree(node->right);
    free(node);
}

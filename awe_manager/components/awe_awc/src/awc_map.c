/* MIT License
**
** Copyright (c) 2024 DSP Concepts, Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/
#include "awc_map.h"
#include <math.h>
#include <stdbool.h>
#include <awosal_string.h>

#define AWC_MAP_SEED (241286)   // Any random number, different seed leads to different hash
#define AWC_MAP_RESIZE (1)      // If this is 1, then map will resize once the number of elemeents in map exceed a threshold
#define AWC_MAP_RESIZE_THRESHOLD (2.0) // Ratio of Map size (number of baskets) / Number of items in the map.

static bool is_prime(size_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (size_t i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

static size_t next_prime(size_t n) {
    while (!is_prime(n)) {
        n++;
    }
    return n;
}


static AwcMapNode *create_node(const char *key, void *value)
{
    if (!key || !value)
    {
        return NULL;
    }
    AwcMapNode *node = (AwcMapNode *)calloc(1, sizeof(AwcMapNode));
    if (!node)
    {
        return NULL;
    }
    node->key = strdup(key);
    node->value = value;
    node->next = NULL;
    return node;
}

#if AWC_MAP_RESIZE
static int awc_map_resize(AwcMap *map_p)
{
    if (!map_p)
    {
        return -1;
    }
    size_t new_size = next_prime(map_p->size + 50); // the map size next prime number , atleast  50 more elements
    AwcMapNode **new_buckets = (AwcMapNode **)calloc(new_size, sizeof(AwcMapNode *));
    if (!new_buckets)
    {
        return -1;
    }

    // Rehash all nodes
    for (size_t i = 0; i < map_p->size; i++)
    {
        AwcMapNode *node = map_p->buckets[i];
        while (node)
        {
            AwcMapNode *next = node->next;
            uint32_t hash = awc_map_hash(node->key, strlen(node->key), AWC_MAP_SEED) % new_size;
            node->next = new_buckets[hash];
            new_buckets[hash] = node;
            node = next;
        }
    }

    free(map_p->buckets);
    map_p->buckets = new_buckets;
    map_p->size = new_size;
    return 0;
}
#endif

// MurmurHash3 (32-bit)
uint32_t awc_map_hash(const char *key, size_t len, uint32_t seed)
{
    const uint8_t *data = (const uint8_t *)key;
    const int nblocks = len / 4;

    uint32_t h1 = seed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    // Process blocks
    const uint32_t *blocks = (const uint32_t *)(data + nblocks * 4);
    for (int i = -nblocks; i; i++)
    {
        uint32_t k1 = blocks[i];
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> (32 - 15)); // Rotate left 15
        k1 *= c2;

        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> (32 - 13)); // Rotate left 13
        h1 = h1 * 5 + 0xe6546b64;
    }

    // Tail
    const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3)
    {
    case 3:
        k1 ^= tail[2] << 16;
    case 2:
        k1 ^= tail[1] << 8;
    case 1:
        k1 ^= tail[0];
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> (32 - 15));
        k1 *= c2;
        h1 ^= k1;
    }

    // Finalization
    h1 ^= len;
    h1 ^= (h1 >> 16);
    h1 *= 0x85ebca6b;
    h1 ^= (h1 >> 13);
    h1 *= 0xc2b2ae35;
    h1 ^= (h1 >> 16);

    return h1;
}

AwcMap *awc_map_create(size_t initial_size)
{
    if(!initial_size)
        return NULL;
    
    initial_size = next_prime(initial_size);
    AwcMap *map_p = (AwcMap *)malloc(sizeof(AwcMap));
    if (map_p)
    {
        map_p->size = initial_size;
        map_p->count = 0;
        map_p->buckets = (AwcMapNode **)calloc(initial_size, sizeof(AwcMapNode *));
        if (!map_p->buckets)
        {
            free(map_p);
            map_p = NULL;
        }
    }
    return map_p;
}

int awc_map_free(AwcMap **map_pp)
{
    if (!map_pp || *map_pp == NULL)
    {
        return -1;
    }
    AwcMap *map_p = *map_pp;
    for (size_t i = 0; i < map_p->size; i++)
    {
        AwcMapNode *node = map_p->buckets[i];
        while (node)
        {
            AwcMapNode *temp = node;
            node = node->next;
            free(temp->key);
            free(temp);
        }
    }
    free(map_p->buckets);
    free(map_p);
    *map_pp = NULL;
    return 0;
}

int awc_map_insert(AwcMap *map_p, const char *key, void *value)
{
    if (!map_p || !key || !value)
    {
        return -1;
    }
#if AWC_MAP_RESIZE
    if ((double)map_p->count / map_p->size > AWC_MAP_RESIZE_THRESHOLD)
    {
        if (awc_map_resize(map_p) < 0)
        {
            return -1;
        }
    }
#endif
    uint32_t hash = awc_map_hash(key, strlen(key), AWC_MAP_SEED) % map_p->size;
    AwcMapNode *node = map_p->buckets[hash];

    while (node)
    {
        if (strcmp(node->key, key) == 0)
        {
            return -1;
        }
        node = node->next;
    }

    // Insert new node
    AwcMapNode *new_node = create_node(key, value);
    new_node->next = map_p->buckets[hash];
    map_p->buckets[hash] = new_node;
    map_p->count++;
    return 0;
}

void *awc_map_search(AwcMap *map_p, const char *key)
{
    if (!map_p || !key)
    {
        return NULL;
    }
    uint32_t hash = awc_map_hash(key, strlen(key), AWC_MAP_SEED) % map_p->size;
    AwcMapNode *node = map_p->buckets[hash];

    while (node)
    {
        if (strcmp(node->key, key) == 0)
        {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

int awc_map_delete(AwcMap *map_p, const char *key)
{
    if (!map_p || !key)
    {
        return -1;
    }
    uint32_t hash = awc_map_hash(key, strlen(key), AWC_MAP_SEED) % map_p->size;
    AwcMapNode *node = map_p->buckets[hash];
    AwcMapNode *prev = NULL;

    while (node)
    {
        if (strcmp(node->key, key) == 0)
        {
            if (prev)
            {
                prev->next = node->next;
            }
            else
            {
                map_p->buckets[hash] = node->next;
            }
            free(node->key);
            free(node);
            map_p->count--;
            return 0;
        }
        prev = node;
        node = node->next;
    }

    return -1;
}

/* MIT License
**
** Copyright (c) 2026 DSP Concepts, Inc.
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
#ifndef AWC_MAP_H
#define AWC_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

typedef struct AwcMapNode
{
    char *key;
    void *value;             // generic pointer
    struct AwcMapNode *next; // Linked list for collisions
} AwcMapNode;

typedef struct
{
    AwcMapNode **buckets; // Array of bucket pointers
    size_t size;          // Current table size (number of buckets)
    size_t count;         // Number of elements in the table
} AwcMap;

uint32_t awc_map_hash(const char *key, size_t len, uint32_t seed);
AwcMap *awc_map_create(size_t initial_size);
int awc_map_free(AwcMap **map);
int awc_map_insert(AwcMap *map, const char *key, void *value);
void *awc_map_search(AwcMap *map, const char *key);
int awc_map_delete(AwcMap *map, const char *key);

#ifdef __cplusplus
}
#endif
#endif // AWC_MAP_H

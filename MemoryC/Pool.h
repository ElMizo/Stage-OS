#ifndef POOL_H
#define POOL_H

#include <stddef.h>

typedef struct {
    void *pool;
    size_t object_size;
    size_t total_objects;
    size_t free_count;
    void **free_list;
} MemoryPool;

void pool_init(MemoryPool *pool, void *buffer, size_t object_size, size_t total_objects);
void* pool_alloc(MemoryPool *pool);
void pool_free(MemoryPool *pool, void *object);

#endif 

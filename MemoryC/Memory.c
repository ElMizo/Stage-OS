#include "Header.h"
#include <string.h>
#include <cstdint>

#define MEM_POOL_SIZE 1024 // Define the memory pool size (in bytes)

#define MIN_BLOCK_SIZE 16 // Define the minimum block size (in bytes)

typedef struct
{
    uint8_t *P_start;
    uint8_t *P_end;
    uint16_t block_size;
    uint16_t num_blocks;
    uint16_t free_blocks;
    uint8_t *free_list;
} memory_pool_t;

memory_pool_t mem_pool;
uint8_t mem_pool_buffer[MEM_POOL_SIZE];

void init_memory()
{
    mem_pool.P_start = mem_pool_buffer;
    mem_pool.P_end = mem_pool_buffer + MEM_POOL_SIZE;
    mem_pool.block_size = MIN_BLOCK_SIZE;
    mem_pool.num_blocks = (mem_pool.P_end - mem_pool.P_start) / mem_pool.block_size;
    mem_pool.free_blocks = mem_pool.num_blocks;
    mem_pool.free_list = mem_pool.P_start;
}

void *malloc(size_t size)
{

    uint16_t num_blocks = (size + mem_pool.block_size - 1) / mem_pool.block_size; // number of blocks needed

    if (num_blocks > mem_pool.free_blocks)
    {
        return NULL;
    }

    uint8_t *block = mem_pool.free_list;
    while (block < mem_pool.P_end)
    {
        if (*(uint16_t *)block == 0xFFFF)
        {
            *(uint16_t *)block = num_blocks;
            mem_pool.free_blocks -= num_blocks;
            return block + sizeof(uint16_t);
        }
        block += mem_pool.block_size;
    }

    return NULL;
}

void free(void *ptr)
{

    uint8_t *block = (uint8_t *)ptr - sizeof(uint16_t); // the block address

    uint16_t num_blocks = *(uint16_t *)block;

    *(uint16_t *)block = 0xFFFF; // Set the block as free

    mem_pool.free_blocks += num_blocks;

    mem_pool.free_list = block;
}

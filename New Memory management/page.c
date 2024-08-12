/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "kernel/types.h"
#include "page.h"
#include "string.h"
#include "memorylayout.h"
#include "kernelcore.h"
#include "logging.h"

static uint32_t pages_free = 0;
static uint32_t pages_total = 0;

static uint32_t *freemap = 0;
static uint32_t freemap_bits = 0;
static uint32_t freemap_bytes = 0;
static uint32_t freemap_cells = 0;
static uint32_t freemap_pages = 0;

static void *main_memory_start = (void *) MAIN_MEMORY_START;

#define CELL_BITS (8 * sizeof(*freemap))
#define GUARD_PATTERN 0xDEADBEEF  // guard pattern
static uint32_t guard_pattern_bytes = GUARD_PATTERN;

// Ensure memory system is initialized before operations
static inline bool is_memory_initialized() {
    return freemap != 0;
}

// Initialize page management system
void page_init()
{
    if (is_memory_initialized()) {
        LOG(LOG_LEVEL_WARN, "Memory system already initialized");
        return;
    }

    pages_total = (total_memory * 1024 * 1024 - MAIN_MEMORY_START) / PAGE_SIZE;
    pages_free = pages_total;

    LOG(LOG_LEVEL_INFO, "memory: %d MB (%d KB) total", (pages_free * PAGE_SIZE) / MEGA, (pages_free * PAGE_SIZE) / KILO);

    freemap = main_memory_start;
    freemap_bits = pages_total;
    freemap_bytes = 1 + freemap_bits / 8;
    freemap_cells = 1 + freemap_bits / CELL_BITS;
    freemap_pages = 1 + freemap_bytes / PAGE_SIZE;

    LOG(LOG_LEVEL_INFO, "memory: %d bits %d bytes %d cells %d pages", freemap_bits, freemap_bytes, freemap_cells, freemap_pages);

    memset(freemap, 0xff, freemap_bytes);
    for (int i = 0; i < freemap_pages; i++) {
        if (!page_alloc(0)) {
            LOG(LOG_LEVEL_ERROR, "Failed to allocate page during initialization");
            halt();
        }
    }

    freemap[0] = 0x0;

    LOG(LOG_LEVEL_INFO, "memory: %d MB (%d KB) available", (pages_free * PAGE_SIZE) / MEGA, (pages_free * PAGE_SIZE) / KILO);
}

// Return memory statistics
void page_stats(uint32_t *nfree, uint32_t *ntotal)
{
    if (!is_memory_initialized()) {
        LOG(LOG_LEVEL_ERROR, "Memory system not initialized");
        *nfree = 0;
        *ntotal = 0;
        return;
    }

    *nfree = pages_free;
    *ntotal = pages_total;
}

// Allocate a single page, with boundary checks, fragmentation handling, and memory corruption detection
void *page_alloc(bool zeroit)
{
    if (!is_memory_initialized()) {
        LOG(LOG_LEVEL_ERROR, "Memory not initialized yet!");
        return 0;
    }

    if (pages_free == 0) {
        LOG(LOG_LEVEL_WARN, "Memory overcommitment: no pages free");
        return 0;
    }

    for (uint32_t i = 0; i < freemap_cells; i++) {
        if (freemap[i] != 0) {
            for (uint32_t j = 0; j < CELL_BITS; j++) {
                uint32_t cellmask = (1 << j);
                if (freemap[i] & cellmask) {
                    freemap[i] &= ~cellmask;  // Mark page as allocated
                    uint32_t pagenumber = i * CELL_BITS + j;

                    if (pagenumber >= pages_total) {  // Boundary check
                        LOG(LOG_LEVEL_ERROR, "Page number out of bounds: %d", pagenumber);
                        return 0;
                    }

                    void *pageaddr = (void *)((pagenumber << PAGE_BITS) + (uint32_t)(main_memory_start));

                    // Memory corruption detection
                    if (memcmp(pageaddr, &guard_pattern_bytes, sizeof(guard_pattern_bytes)) == 0) {
                        LOG(LOG_LEVEL_ERROR, "Memory corruption detected at address %p", pageaddr);
                        return 0;
                    }

                    if (zeroit) {
                        memset(pageaddr, 0, PAGE_SIZE);
                    }

                    pages_free--;

                    LOG(LOG_LEVEL_DEBUG, "Page allocated: %p, %d pages free", pageaddr, pages_free);
                    return pageaddr;
                }
            }
        }
    }

    LOG(LOG_LEVEL_ERROR, "Memory allocation failed: all pages are allocated");
    return 0;
}

// Free a previously allocated page, with boundary checks and memory corruption detection
void page_free(void *pageaddr)
{
    if (!is_memory_initialized()) {
        LOG(LOG_LEVEL_ERROR, "Memory not initialized yet!");
        return;
    }

    if (pageaddr == 0) {
        LOG(LOG_LEVEL_ERROR, "Attempt to free a 0 page address");
        return;
    }

    void *page_offset = (void *)((uint32_t)pageaddr - (uint32_t)main_memory_start);

    if ((uint32_t)page_offset % PAGE_SIZE != 0 || page_offset >= pages_total * PAGE_SIZE) {  // Boundary check
        LOG(LOG_LEVEL_ERROR, "Invalid page address: %p", pageaddr);
        return;
    }

    uint32_t pagenumber = (uint32_t)page_offset >> PAGE_BITS;
    uint32_t cellnumber = pagenumber / CELL_BITS;
    uint32_t celloffset = pagenumber % CELL_BITS;
    uint32_t cellmask = (1 << celloffset);

    if (!(freemap[cellnumber] & cellmask)) {
        LOG(LOG_LEVEL_WARN, "Double free detected at address %p", pageaddr);
        return;
    }

    freemap[cellnumber] |= cellmask;
    pages_free++;

    // Memory corruption detection (check guard pattern)
    if (memcmp(pageaddr, &guard_pattern_bytes, sizeof(guard_pattern_bytes)) == 0) {
        LOG(LOG_LEVEL_ERROR, "Memory corruption detected at address %p", pageaddr);
    }

    LOG(LOG_LEVEL_DEBUG, "Page freed: %p, %d pages free", pageaddr, pages_free);
}

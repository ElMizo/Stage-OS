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

#define PAGE_SIZE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SIZE_SHIFT)
#define MAIN_MEMORY_START 0x100000

static uint32_t total_pages;
static uint32_t free_pages;
static uint32_t *page_map;
static uint32_t page_map_size;

void page_init() {
    total_pages = (total_memory * 1024 * 1024 - MAIN_MEMORY_START) / PAGE_SIZE;
    free_pages = total_pages;

    page_map_size = (total_pages + 7) / 8;
    page_map = (uint32_t *) MAIN_MEMORY_START;
    memset(page_map, 0xff, page_map_size);

    for (int i = 0; i < page_map_size / PAGE_SIZE; i++) {
        page_alloc(0);
    }

    page_map[0] = 0x0;     //not understood
}

void page_stats(uint32_t *nfree, uint32_t *ntotal) {
    *nfree = free_pages;
    *ntotal = total_pages;
}

void *page_alloc(bool zeroit) {
    uint32_t page_idx = 0;
    while (page_idx < page_map_size) {
        uint32_t page_mask = 1 << (page_idx % 8);
        if (page_map[page_idx / 8] & page_mask) {
            page_map[page_idx / 8] &= ~page_mask;
            void *page_addr = (page_idx << PAGE_SIZE_SHIFT) + MAIN_MEMORY_START;
            if (zeroit) {
                memset(page_addr, 0, PAGE_SIZE);
            }
            free_pages--;
            return page_addr;
        }
        page_idx++;
    }

    printf("memory: WARNING: everything allocated\n");
    halt();
    return 0;
}

void page_free(void *pageaddr) {
    uint32_t page_idx = (pageaddr - MAIN_MEMORY_START) >> PAGE_SIZE_SHIFT;
    uint32_t page_mask = 1 << (page_idx % 8);
    page_map[page_idx / 8] |= page_mask;
    free_pages++;
}

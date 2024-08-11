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

static uint32_t pages_free = 0;
static uint32_t pages_total = 0;

static uint32_t *freemap = 0; /*A pointer to an array (freemap) that keeps track of free and allocated pages. 
								Each bit in the array represents the state of a page (free or allocated). 
								Initialized to 0 (null pointer).*/ 
static uint32_t freemap_bits = 0 ; 
static uint32_t freemap_bytes = 0;
static uint32_t freemap_cells = 0;
static uint32_t freemap_pages = 0;

static void *main_memory_start = (void *) MAIN_MEMORY_START; /*A pointer to the start of the main memory region. 
															The MAIN_MEMORY_START macro should be defined elsewhere, 
															representing the starting address of the main memory. 
															This pointer is used to calculate the addresses of pages.*/
															
#define CELL_BITS (8*sizeof(*freemap)) /*A macro that calculates the number of bits in each cell of the freemap.
										 Since each cell is a uint32_t, this macro evaluates to 32 bits (assuming a 4-byte uint32_t). 
										This macro helps in calculating the bitwise operations within the freemap.*/
void page_init()
{
	int i;

	pages_total = (total_memory * 1024 * 1024 - MAIN_MEMORY_START) / PAGE_SIZE;
	pages_free = pages_total;
	printf("memory: %d MB (%d KB) total\n", (pages_free * PAGE_SIZE) / MEGA, (pages_free * PAGE_SIZE) / KILO);

	freemap = main_memory_start;
	freemap_bits = pages_total;
	freemap_bytes = 1 + freemap_bits / 8;
	freemap_cells = 1 + freemap_bits / CELL_BITS;
	freemap_pages = 1 + freemap_bytes / PAGE_SIZE;

	printf("memory: %d bits %d bytes %d cells %d pages\n", freemap_bits, freemap_bytes, freemap_cells, freemap_pages);

	memset(freemap, 0xff, freemap_bytes);
	for(i = 0; i < freemap_pages; i++)
		page_alloc(0);

	// This is ahack that I don't understand yet.
	// vmware doesn't like the use of a particular page
	// close to 1MB, but what it is used for I don't know.

	freemap[0] = 0x0;

	printf("memory: %d MB (%d KB) available\n", (pages_free * PAGE_SIZE) / MEGA, (pages_free * PAGE_SIZE) / KILO);
}

void page_stats( uint32_t *nfree, uint32_t *ntotal )
{
	*nfree = pages_free;
	*ntotal = pages_total;
}

void *page_alloc(bool zeroit) // Allocate a page of memory for the process structure and zero it out . 
{
	uint32_t i, j;
	uint32_t cellmask; 
	uint32_t pagenumber; // holds the index of the allocated page 
	void *pageaddr; // holds the address of the allocated page  

	if(!freemap) {     // If the "freemap" is not initialized, print an error message and return 0.
		printf("memory: not initialized yet!\n"); 
		return 0;
	}
	for(i = 0; i < freemap_cells; i++) {
		if(freemap[i] != 0) {
			for(j = 0; j < CELL_BITS; j++) {
				cellmask = (1 << j);
				if(freemap[i] & cellmask) {
					freemap[i] &= ~cellmask; // And between freemap[i] and not(cellmask) stored in freemap[i]
					pagenumber = i * CELL_BITS + j;
					pageaddr = (pagenumber << PAGE_BITS) + main_memory_start;
					if(zeroit)
						memset(pageaddr, 0, PAGE_SIZE);
					pages_free--;
					//printf("page: alloc %d\n",pages_free);
					return pageaddr;
				}
			}
		}
	}
	printf("memory: WARNING: everything allocated\n");
	halt();

	return 0;
}

void page_free(void *pageaddr)
{
	uint32_t pagenumber = (pageaddr - main_memory_start) >> PAGE_BITS;
	uint32_t cellnumber = pagenumber / CELL_BITS;
	uint32_t celloffset = pagenumber % CELL_BITS;
	uint32_t cellmask = (1 << celloffset);
	freemap[cellnumber] |= cellmask;
	pages_free++;
	//printf("page: free %d\n",pages_free);
}

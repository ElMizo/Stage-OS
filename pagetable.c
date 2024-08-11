/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "pagetable.h"
#include "page.h"
#include "string.h"
#include "kernelcore.h"

#define ENTRIES_PER_TABLE (PAGE_SIZE/4)

struct pageentry {
	unsigned present:1;	// 1 = present
	unsigned readwrite:1;	// 1 = writable
	unsigned user:1;	// 1 = user mode
	unsigned writethrough:1;	// 1 = write through

	unsigned nocache:1;	// 1 = no caching
	unsigned accessed:1;	// 1 = accessed
	unsigned dirty:1;	// 1 = dirty
	unsigned pagesize:1;	// leave to zero

	unsigned globalpage:1;	// 1 if not to be flushed
	unsigned avail:3;

	unsigned addr:20;
};

struct pagetable {
	struct pageentry entry[ENTRIES_PER_TABLE];
};

struct pagetable *pagetable_create() // create page table and return it the addreess 
{
	return page_alloc(1); 
}

void pagetable_init(struct pagetable *p)
{
	unsigned i, stop;
	stop = total_memory * 1024 * 1024; // totale_memory : is an unsigned short of 16 bit 
	for(i = 0; i < stop; i += PAGE_SIZE) {
		pagetable_map(p, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE); 
	}
	stop = (unsigned) video_buffer + video_xres * video_yres * 3;
	for(i = (unsigned) video_buffer; i <= stop; i += PAGE_SIZE) {
		pagetable_map(p, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE);
	}
}

// Retrieve the physical address and flags associated with a virtual address from the page table.
int pagetable_getmap(struct pagetable *p, unsigned vaddr, unsigned *paddr, int *flags)
{
    struct pagetable *q;          // Pointer to the next level page table
    struct pageentry *e;          // Pointer to a page table entry

    // Calculate the index into the first level page table.
    unsigned a = vaddr >> 22;     // Top 10 bits of the virtual address
    // Calculate the index into the second level page table.
    unsigned b = (vaddr >> 12) & 0x3ff; // Middle 10 bits of the virtual address

    e = &p->entry[a];             // Get the entry from the first level page table
    // Check if the entry is present.
    if(!e->present)
        return 0;                 // Return 0 if the entry is not present

    q = (struct pagetable *) (e->addr << 12); // Get the address of the second level page table

    e = &q->entry[b];             // Get the entry from the second level page table
    // Check if the entry is present.
    if(!e->present)
        return 0;                 // Return 0 if the entry is not present

    *paddr = e->addr << 12;       // Set the physical address

    // If flags pointer is provided, set the appropriate flags.
    if(flags) {
        *flags = 0;               // Initialize flags to 0
        if(e->readwrite)
            *flags |= PAGE_FLAG_READWRITE; // Set the readwrite flag if applicable
        if(e->avail & 0x01)
            *flags |= PAGE_FLAG_ALLOC;     // Set the allocation flag if applicable
        if(!e->user)
            *flags |= PAGE_FLAG_KERNEL;    // Set the kernel flag if applicable
    }

    return 1;                     // Return 1 to indicate success
}


/*The pagetable_map function configures a page table to map a virtual address (vaddr) to a physical address (paddr) with specified attributes.
 It sets up or reuses page tables as needed and ensures that the page entry has the correct permissions and flags based on the provided flags parameter.*/

int pagetable_map(struct pagetable *p, unsigned vaddr, unsigned paddr, int flags)
{
	struct pagetable *q; //Pointer to the new page table
	struct pageentry *e; //Pointer to the page entry in new page table 

	unsigned a = vaddr >> 22; // Index into the top-level-page table
	unsigned b = (vaddr >> 12) & 0x3ff; // Index into tohe second-level-page table (bits 12 - 21)

	if(flags & PAGE_FLAG_ALLOC) {
		paddr = (unsigned) page_alloc(flags & PAGE_FLAG_CLEAR);  // Allocates a physical page if the PAGE_FLAG_ALLOC flag is set.
		if(!paddr)
			return 0; // page allocation fails
	}

	e = &p->entry[a]; // Gets the pointer to the page entry in the top-level page table

	if(!e->present) {  // Checks if the top-level page table entry is not present.
		q = pagetable_create();  
		if(!q)
			return 0; //  creation fails
		e->present = 1;
		e->readwrite = 1;
		e->user = (flags & PAGE_FLAG_KERNEL) ? 0 : 1;
		e->writethrough = 0;
		e->nocache = 0;
		e->accessed = 0;
		e->dirty = 0;
		e->pagesize = 0;
		e->globalpage = (flags & PAGE_FLAG_KERNEL) ? 1 : 0;
		e->avail = 0;
		e->addr = (((unsigned) q) >> 12);
	} else {
		q = (struct pagetable *) (((unsigned) e->addr) << 12); // If the top-level page table entry is present, reuses the existing page table
	}

	e = &q->entry[b]; // Gets the pointer to the page entry in the second -level page table

	e->present = 1;
	e->readwrite = (flags & PAGE_FLAG_READWRITE) ? 1 : 0;
	e->user = (flags & PAGE_FLAG_KERNEL) ? 0 : 1;
	e->writethrough = 0;
	e->nocache = 0;
	e->accessed = 0;
	e->dirty = 0;
	e->pagesize = 0;
	e->globalpage = !e->user;
	e->avail = (flags & PAGE_FLAG_ALLOC) ? 1 : 0;
	e->addr = (paddr >> 12);

	return 1; // indicate that the mapping was successuful
}

// Unmaps a virtual address from the page table.
void pagetable_unmap(struct pagetable *p, unsigned vaddr)
{
    struct pagetable *q;      // Pointer to the next level page table
    struct pageentry *e;      // Pointer to a page table entry

    // Calculate the index into the first level page table.
    unsigned a = vaddr >> 22; // Top 10 bits of the virtual address
    // Calculate the index into the second level page table.
    unsigned b = vaddr >> 12 & 0x3ff; // Middle 10 bits of the virtual address

    e = &p->entry[a];         // Get the entry from the first level page table
    // Check if the entry is present.
    if(e->present) {
        // Get the address of the second level page table
        q = (struct pagetable *) (e->addr << 12);
        // Get the entry from the second level page table
        e = &q->entry[b];
        // Set the present bit to 0 to unmap the page
        e->present = 0;
    }
}

void pagetable_delete(struct pagetable *p)
{
	unsigned i, j;

	struct pageentry *e;
	struct pagetable *q;

	for(i = 0; i < ENTRIES_PER_TABLE; i++) {
		e = &p->entry[i];
		if(e->present) {
			q = (struct pagetable *) (e->addr << 12);
			for(j = 0; j < ENTRIES_PER_TABLE; j++) {
				e = &q->entry[j];
				if(e->present && e->avail) {
					void *paddr;
					paddr = (void *) (e->addr << 12);
					page_free(paddr);
				}
			}
			page_free(q);
		}
	}

	page_free(p);
}
// Allocate and map a range of virtual addresses to physical pages in a page table.
void pagetable_alloc(struct pagetable *p, unsigned vaddr, unsigned length, int flags)
{	
	// Calculate the number of pages required to cover the specified length.
	unsigned npages = length / PAGE_SIZE;

	if(length % PAGE_SIZE)  
		npages++;

	vaddr &= 0xfffff000;  // Align the starting virtual address to the nearest page boundary.

	while(npages > 0) {
		unsigned paddr;
		/* checks if the virtual address (vaddr) is already mapped to a physical address 
		 pagetable_getmap returns 0 if the address is not mapped */
		if(!pagetable_getmap(p, vaddr, &paddr, 0)) { 
			pagetable_map(p, vaddr, 0, flags | PAGE_FLAG_ALLOC);
		}
		vaddr += PAGE_SIZE;
		npages--;
	}
}

// Frees a range of pages from the page table starting at the given virtual address.
void pagetable_free(struct pagetable *p, unsigned vaddr, unsigned length)
{
    // Calculate the number of pages to free.
    unsigned npages = length / PAGE_SIZE;

    // If there's a remainder, account for the partial page as a full page.
    if(length % PAGE_SIZE)
        npages++;

    // Align the virtual address to the nearest page boundary.
    vaddr &= 0xfffff000;

    // Loop through each page and unmap it.
    while(npages > 0) {
        unsigned paddr;   // Physical address of the page
        int flags;        // Flags associated with the page

        // Check if the virtual address is mapped and get its physical address and flags.
        if(pagetable_getmap(p, vaddr, &paddr, &flags)) {
            // Unmap the page from the page table.
            pagetable_unmap(p, vaddr);

            // If the page was allocated with the PAGE_FLAG_ALLOC flag, free the physical page.
            if(flags & PAGE_FLAG_ALLOC)
                page_free((void *) paddr);
        }

        // Move to the next page.
        vaddr += PAGE_SIZE;
        npages--;
    }
}

struct pagetable *pagetable_load(struct pagetable *p)
{
	struct pagetable *oldp;
      asm("mov %%cr3, %0":"=r"(oldp));
	asm("mov %0, %%cr3"::"r"(p));
	return oldp;
}

void pagetable_refresh()
{
	asm("mov %cr3, %eax");
	asm("mov %eax, %cr3");
}

void pagetable_enable()
{
	asm("movl %cr0, %eax");
	asm("orl $0x80000000, %eax");
	asm("movl %eax, %cr0");
}

struct pagetable *pagetable_duplicate(struct pagetable *sp)
{
	unsigned i, j;

	struct pageentry *e;
	struct pagetable *q;

	struct pageentry *newe;
	struct pagetable *newq;
	struct pagetable *newp = pagetable_create();
	if(!newp)
		goto cleanup;

	for(i = 0; i < ENTRIES_PER_TABLE; i++) {
		e = &sp->entry[i];
		newe = &newp->entry[i];
		if(e->present) {
			q = (struct pagetable *) (e->addr << 12);
			newq = pagetable_create();
			if(!newq)
				goto cleanup;
			memcpy(newe, e, sizeof(struct pageentry));
			newe->addr = (((unsigned) newq) >> 12);
			for(j = 0; j < ENTRIES_PER_TABLE; j++) {
				e = &q->entry[j];
				newe = &newq->entry[j];
				memcpy(newe, e, sizeof(struct pageentry));
				if(e->present) {
					void *paddr;
					paddr = (void *) (e->addr << 12);
					void *new_paddr = 0;
					if(e->avail) {
						new_paddr = page_alloc(0);
						if(!new_paddr)
							goto cleanup;
						memcpy(new_paddr, paddr, PAGE_SIZE);
					} else {
						new_paddr = paddr;
					}
					newe->addr = (((unsigned) new_paddr) >> 12);
				}
			}
		}
	}
	return newp;
      cleanup:
	printf("Pagetable duplicate errors\n");
	if(newp) {
		pagetable_delete(newp);
	}
	return 0;
}

void pagetable_copy(struct pagetable *sp, unsigned saddr, struct pagetable *tp, unsigned taddr, unsigned length);

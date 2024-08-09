/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "pagetable.h"
#include "page.h"
#include "string.h"
#include "kernelcore.h"
#include "log.h"

#define ENTRIES_PER_TABLE (PAGE_SIZE / 4)

struct pageentry {
    unsigned present:1;
    unsigned readwrite:1;
    unsigned user:1;
    unsigned writethrough:1;
    unsigned nocache:1;
    unsigned accessed:1;
    unsigned dirty:1;
    unsigned pagesize:1;
    unsigned globalpage:1;
    unsigned avail:3;
    unsigned addr:20;
};

struct pagetable {
    struct pageentry entry[ENTRIES_PER_TABLE];
};

static int clock_front = 0;
static int clock_back = 0;
static int clock_bits[ENTRIES_PER_TABLE]; 

struct pagetable *pagetable_create()
{
	return page_alloc(1);
}

void pagetable_init(struct pagetable *p) {
   
    unsigned i, Max, End;
    Max = total_memory * 1024 * 1024;

    for (i = 0; i < Max; i += PAGE_SIZE) {
        if (!pagetable_map(p, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE)) {
            log_error("Failed to map page in pagetable_init.");
        }
    }

    End = (unsigned)video_buffer + video_xres * video_yres * 3;
    for (i = (unsigned)video_buffer; i <= End; i += PAGE_SIZE) {
        if (!pagetable_map(p, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE)) {
            log_error("Failed to map video buffer in pagetable_init.");
        }
    }
}

void pagetable_init_clock() {
    int i;
    for (i = 0; i < ENTRIES_PER_TABLE; i++) {
        clock_bits[i] = 0;
    }
    clock_front = 0;
    clock_back = 0;
}

int pagetable_getmap(struct pagetable *p, unsigned vaddr, unsigned *paddr, int *flags) {
    if (!p || !paddr) {
        log_error("Invalid parameters in pagetable_getmap.");
        return 0;
    }

    struct pagetable *q;
    struct pageentry *e;
    
    unsigned p_direc_indx = vaddr >> 22;
    unsigned p_tble_indx = (vaddr >> 12) & 0x3ff;

    e = &p->entry[p_direc_indx];
    if (!e->present) return 0;

    q = (struct pagetable *)(e->addr << 12);
    e = &q->entry[p_tble_indx];
    if (!e->present) return 0;

    *paddr = e->addr << 12;

    if (flags) {
        *flags = 0;
        if (e->readwrite) *flags |= PAGE_FLAG_READWRITE;
        if (e->avail & 0x01) *flags |= PAGE_FLAG_ALLOC;
        if (!e->user) *flags |= PAGE_FLAG_KERNEL;
    }

    return 1;
}

int pagetable_map(struct pagetable *p, unsigned vaddr, unsigned paddr, int flags) {
    if (!p) {
        log_error("Invalid pagetable in pagetable_map.");
        return 0;
    }

    struct pagetable *q;
    struct pageentry *e;
    
    unsigned p_direc_indx = vaddr >> 22;
    unsigned p_tble_indx = (vaddr >> 12) & 0x3ff;

    if (flags & PAGE_FLAG_ALLOC) {
        int victim_page = clock_paging_algorithm();
        if (victim_page != -1) {
            pagetable_unmap(p, victim_page);
            paddr = (unsigned)page_alloc(flags & PAGE_FLAG_CLEAR);
            if (!paddr) {
                log_error("Failed to allocate page in pagetable_map.");
                return 0;
            }
        }
    }

    e = &p->entry[p_direc_indx];

    if (!e->present) {
        q = pagetable_create();
        if (!q) {
            log_error("Failed to create page table in pagetable_map.");
            if (flags & PAGE_FLAG_ALLOC) page_free((void *)paddr);
            return 0;
        }
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
        e->addr = (((unsigned)q) >> 12);
    } else {
        q = (struct pagetable *)(((unsigned)e->addr) << 12);
    }

    e = &q->entry[p_tble_indx];
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

    return 1;
}

void pagetable_unmap(struct pagetable *p, unsigned vaddr) {
    if (!p) {
        log_error("Invalid pagetable in pagetable_unmap.");
        return;
    }

    struct pagetable *q;
    struct pageentry *e;
    unsigned p_direc_indx = vaddr >> 22;
    unsigned p_tble_indx = (vaddr >> 12) & 0x3ff;

    e = &p->entry[p_direc_indx];
    if (e->present) {
        q = (struct pagetable *)(e->addr << 12);
        e = &q->entry[p_tble_indx];
        e->present = 0;
        clock_bits[vaddr >> 12] = 0;  
    }
}

int clock_paging_algorithm() {
    int i;
    int victim_page = -1;

    for (i = clock_front; i < ENTRIES_PER_TABLE; i++) {
        if (clock_bits[i] == 1) {
            clock_bits[i] = 0;
        } else {
            victim_page = i;
            break;
        }
    }

    if (victim_page == -1) {
        for (i = 0; i < clock_front; i++) {
            if (clock_bits[i] == 1) {
                clock_bits[i] = 0;
            } else {
                victim_page = i;
                break;
            }
        }
    }

    if (victim_page == -1) {
        victim_page = 0;
    }

    clock_front = (clock_front + 1) % ENTRIES_PER_TABLE;
    clock_back = (clock_back + 1) % ENTRIES_PER_TABLE;

    return victim_page;
}

void pagetable_alloc(struct pagetable *p, unsigned vaddr, unsigned length, int flags) {
    if (!p) {
        log_error("Invalid pagetable in pagetable_alloc.");
        return;
    }

    unsigned npages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    vaddr &= 0xfffff000;

    while (npages > 0) {
        unsigned paddr;
        if (!pagetable_getmap(p, vaddr, &paddr, 0)) {
            if (!pagetable_map(p, vaddr, 0, flags | PAGE_FLAG_ALLOC)) {
                log_error("Failed to map page in pagetable_alloc.");
            }
        }
        vaddr += PAGE_SIZE;
        npages--;
    }
}

void pagetable_free(struct pagetable *p, unsigned vaddr, unsigned length) {
    if (!p) {
        log_error("Invalid pagetable in pagetable_free.");
        return;
    }

    unsigned npages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    vaddr &= 0xfffff000;

    while (npages > 0) {
        unsigned paddr;
        int flags;
        if (pagetable_getmap(p, vaddr, &paddr, &flags)) {
            pagetable_unmap(p, vaddr);
            if (flags & PAGE_FLAG_ALLOC) page_free((void *)paddr);
        }
        vaddr += PAGE_SIZE;
        npages--;
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

struct pagetable *pagetable_load(struct pagetable *p) {
    struct pagetable *oldp;
    asm("mov %%cr3, %0" : "=r"(oldp));
    asm("mov %0, %%cr3" :: "r"(p));
    return oldp;
}

void pagetable_unload(struct pagetable *p) {
    asm("mov %0, %%cr3" :: "r"(p));
}

void pagetable_switch(struct pagetable *p) {
    struct pagetable *oldp = pagetable_load(p);
    pagetable_unload(oldp);
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

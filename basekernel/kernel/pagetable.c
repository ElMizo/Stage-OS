/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "pagetable.h"   
#include "page.h"        
#include "string.h"      
#include "kernelcore.h"  

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


struct pagetable *pagetable_create() {
    return page_alloc(1);
}


void pagetable_init(struct pagetable *p) {
    unsigned i, Max, End;
    Max = total_memory * 1024 * 1024;

    for (i = 0; i < Max; i += PAGE_SIZE) {
        pagetable_map(p, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE);
    }

    End = (unsigned)video_buffer + video_xres * video_yres * 3;
    for (i = (unsigned)video_buffer; i <= End; i += PAGE_SIZE) {
        pagetable_map(p, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE);
    }
}


int pagetable_getmap(struct pagetable *p, unsigned vaddr, unsigned *paddr, int *flags) {
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
    struct pagetable *q;
    struct pageentry *e;
    unsigned p_direc_indx = vaddr >> 22;
    unsigned p_tble_indx = (vaddr >> 12) & 0x3ff;

    if (flags & PAGE_FLAG_ALLOC) {
        paddr = (unsigned)page_alloc(flags & PAGE_FLAG_CLEAR);
        if (!paddr) return 0;
    }

    e = &p->entry[p_direc_indx];

    if (!e->present) {
        q = pagetable_create();
        if (!q) return 0;
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
    struct pagetable *q;
    struct pageentry *e;
    unsigned p_direc_indx = vaddr >> 22;
    unsigned p_tble_indx = (vaddr >> 12) & 0x3ff;

    e = &p->entry[p_direc_indx];
    if (e->present) {
        q = (struct pagetable *)(e->addr << 12);
        e = &q->entry[p_tble_indx];
        e->present = 0;
    }
}


void pagetable_delete(struct pagetable *p) {
    unsigned i, j;
    struct pageentry *e;
    struct pagetable *q;

    for (i = 0; i < ENTRIES_PER_TABLE; i++) {
        e = &p->entry[i];
        if (e->present) {
            q = (struct pagetable *)(e->addr << 12);
            for (j = 0; j < ENTRIES_PER_TABLE; j++) {
                e = &q->entry[j];
                if (e->present && e->avail) {
                    void *paddr = (void *)(e->addr << 12);
                    page_free(paddr);
                }
            }
            page_free(q);
        }
    }

    page_free(p);
}


void pagetable_alloc(struct pagetable *p, unsigned vaddr, unsigned length, int flags) {
    unsigned npages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    vaddr &= 0xfffff000;

    while (npages > 0) {
        unsigned paddr;
        if (!pagetable_getmap(p, vaddr, &paddr, 0)) {
            pagetable_map(p, vaddr, 0, flags | PAGE_FLAG_ALLOC);
        }
        vaddr += PAGE_SIZE;
        npages--;
    }
}


void pagetable_free(struct pagetable *p, unsigned vaddr, unsigned length) {
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


struct pagetable *pagetable_load(struct pagetable *p) {
    struct pagetable *oldp;
    asm("mov %%cr3, %0" : "=r"(oldp)); 
    asm("mov %0, %%cr3" ::"r"(p));     
    return oldp;
}


void pagetable_refresh() {
    asm("mov %cr3, %eax");
    asm("mov %eax, %cr3");
}


void pagetable_enable() {
    asm("movl %cr0, %eax");
    asm("orl $0x80000000, %eax"); 
    asm("movl %eax, %cr0");
}

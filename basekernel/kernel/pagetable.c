/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "pagetable.h"
#include "page.h"
#include "string.h"
#include "kernelcore.h"
#include "logging.h"

#define ENTRIES_PER_TABLE (PAGE_SIZE / sizeof(struct pageentry))
#define VIDEO_BUFFER_SIZE (video_xres * video_yres * 3)
#define MAX_MEMORY_ADDRESS 0xFFFFFFFF

#define PAGE_TABLE_SUCCESS 1
#define PAGE_TABLE_FAILURE 0
#define PAGE_TABLE_INVALID_ADDR -1
#define PAGE_TABLE_OVERFLOW -2
#define PAGE_TABLE_PRIV_ESCALATION -3

struct pageentry {
    unsigned present:1;       // 1 = present
    unsigned readwrite:1;     // 1 = writable
    unsigned user:1;          // 1 = user mode
    unsigned writethrough:1;  // 1 = write through
    unsigned nocache:1;       // 1 = no caching
    unsigned accessed:1;      // 1 = accessed
    unsigned dirty:1;         // 1 = dirty
    unsigned pagesize:1;      // Leave to zero
    unsigned globalpage:1;    // 1 = not to be flushed
    unsigned avail:3;         // Available for system use
    unsigned addr:20;         // Physical page address
};

struct pagetable {
    struct pageentry entries[ENTRIES_PER_TABLE];
};

// Function to check if an address is within valid bounds
int validate_address(unsigned addr) {
    if (addr > MAX_MEMORY_ADDRESS) {
        LOG(LOG_LEVEL_ERROR, "Address 0x%x exceeds maximum allowed address 0x%x", addr, MAX_MEMORY_ADDRESS);
        return PAGE_TABLE_INVALID_ADDR;
    }
    return PAGE_TABLE_SUCCESS;
}

// Function to create a page table
struct pagetable *pagetable_create() {
    struct pagetable *pt = page_alloc(1);
    if (!pt) {
        LOG(LOG_LEVEL_ERROR, "Failed to allocate memory for page table");
    }
    return pt;
}

// Function to initialize a page table
int pagetable_init(struct pagetable *pt) {
    unsigned i;
    unsigned mem_limit = total_memory * 1024 * 1024;
    
    if (validate_address(mem_limit) == PAGE_TABLE_INVALID_ADDR) {
        return PAGE_TABLE_INVALID_ADDR;
    }

    for (i = 0; i < mem_limit; i += PAGE_SIZE) {
        if (pagetable_map(pt, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE) == PAGE_TABLE_FAILURE) {
            LOG(LOG_LEVEL_ERROR, "Failed to map memory address 0x%x", i);
            return PAGE_TABLE_FAILURE;
        }
    }
    
    unsigned video_limit = (unsigned) video_buffer + VIDEO_BUFFER_SIZE;
    if (validate_address(video_limit) == PAGE_TABLE_INVALID_ADDR) {
        return PAGE_TABLE_INVALID_ADDR;
    }

    for (i = (unsigned) video_buffer; i <= video_limit; i += PAGE_SIZE) {
        if (pagetable_map(pt, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE) == PAGE_TABLE_FAILURE) {
            LOG(LOG_LEVEL_ERROR, "Failed to map video buffer at address 0x%x", i);
            return PAGE_TABLE_FAILURE;
        }
    }

    return PAGE_TABLE_SUCCESS;
}

// Function to get the mapping of a virtual address to a physical address
int pagetable_getmap(struct pagetable *pt, unsigned vaddr, unsigned *paddr, int *flags) {
    unsigned pg_dirctry_idx = vaddr >> 22;
    unsigned pg_tble_idx = (vaddr >> 12) & 0x3FF;

    if (validate_address(vaddr) == PAGE_TABLE_INVALID_ADDR) {
        return PAGE_TABLE_INVALID_ADDR;
    }

    struct pageentry *entry = &pt->entries[pg_dirctry_idx];
    if (!entry->present) {
        LOG(LOG_LEVEL_WARN, "Page directory entry not present for address 0x%x", vaddr);
        return PAGE_TABLE_FAILURE;
    }

    struct pagetable *subtable = (struct pagetable *) (entry->addr << 12);
    entry = &subtable->entries[pg_tble_idx];
    if (!entry->present) {
        LOG(LOG_LEVEL_WARN, "Page table entry not present for address 0x%x", vaddr);
        return PAGE_TABLE_FAILURE;
    }

    *paddr = entry->addr << 12;

    if (flags) {
        *flags = 0;
        if (entry->readwrite) {
            *flags |= PAGE_FLAG_READWRITE;
        }
        if (entry->avail & 0x01) {
            *flags |= PAGE_FLAG_ALLOC;
        }
        if (!entry->user) {
            *flags |= PAGE_FLAG_KERNEL;
        }
    }

    return PAGE_TABLE_SUCCESS;
}

// Function to map a virtual address to a physical address in the page table
int pagetable_map(struct pagetable *pt, unsigned vaddr, unsigned paddr, int flags) {
    unsigned pg_dirctry_idx = vaddr >> 22;
    unsigned pg_tble_idx = (vaddr >> 12) & 0x3FF;

    if (validate_address(vaddr) == PAGE_TABLE_INVALID_ADDR || validate_address(paddr) == PAGE_TABLE_INVALID_ADDR) {
        return PAGE_TABLE_INVALID_ADDR;
    }

    if (flags & PAGE_FLAG_ALLOC) {
        paddr = (unsigned) page_alloc(flags & PAGE_FLAG_CLEAR);
        if (!paddr) {
            LOG(LOG_LEVEL_ERROR, "Failed to allocate page for address 0x%x", vaddr);
            return PAGE_TABLE_FAILURE;
        }
    }

    struct pageentry *entry = &pt->entries[pg_dirctry_idx];

    if (!entry->present) {
        struct pagetable *subtable = pagetable_create();
        if (!subtable) {
            LOG(LOG_LEVEL_ERROR, "Failed to create subpage table for address 0x%x", vaddr);
            return PAGE_TABLE_FAILURE;
        }
        entry->present = 1;
        entry->readwrite = 1;
        entry->user = !(flags & PAGE_FLAG_KERNEL);
        entry->writethrough = 0;
        entry->nocache = 0;
        entry->accessed = 0;
        entry->dirty = 0;
        entry->pagesize = 0;
        entry->globalpage = (flags & PAGE_FLAG_KERNEL) ? 1 : 0;
        entry->avail = 0;
        entry->addr = (unsigned) subtable >> 12;
    }

    struct pagetable *subtable = (struct pagetable *) (entry->addr << 12);
    entry = &subtable->entries[pg_tble_idx];

    // Check for privilege escalation
    if (entry->user && (flags & PAGE_FLAG_KERNEL)) {
        LOG(LOG_LEVEL_ERROR, "Privilege escalation attempt detected for address 0x%x", vaddr);
        return PAGE_TABLE_PRIV_ESCALATION;
    }

    entry->present = 1;
    entry->readwrite = (flags & PAGE_FLAG_READWRITE) ? 1 : 0;
    entry->user = !(flags & PAGE_FLAG_KERNEL);
    entry->writethrough = 0;
    entry->nocache = 0;
    entry->accessed = 0;
    entry->dirty = 0;
    entry->pagesize = 0;
    entry->globalpage = !entry->user;
    entry->avail = (flags & PAGE_FLAG_ALLOC) ? 1 : 0;
    entry->addr = paddr >> 12;

    return PAGE_TABLE_SUCCESS;
}

// Function to unmap a virtual address from the page table
void pagetable_unmap(struct pagetable *pt, unsigned vaddr) {
    unsigned pg_dirctry_idx = vaddr >> 22;
    unsigned pg_tble_idx = (vaddr >> 12) & 0x3FF;

    if (validate_address(vaddr) == PAGE_TABLE_INVALID_ADDR) {
        LOG(LOG_LEVEL_ERROR, "Invalid address 0x%x for unmapping", vaddr);
        return;
    }

    struct pageentry *entry = &pt->entries[pg_dirctry_idx];
    if (entry->present) {
        struct pagetable *subtable = (struct pagetable *) (entry->addr << 12);
        entry = &subtable->entries[pg_tble_idx];
        entry->present = 0;
    } else {
        LOG(LOG_LEVEL_WARN, "Attempted to unmap a non-present page at address 0x%x", vaddr);
    }
}

// Function to delete a page table
void pagetable_delete(struct pagetable *pt) {
    unsigned i, j;

    if (!pt) {
        LOG(LOG_LEVEL_ERROR, "0 page table passed to pagetable_delete");
        return;
    }

    for (i = 0; i < ENTRIES_PER_TABLE; i++) {
        struct pageentry *entry = &pt->entries[i];
        if (entry->present) {
            struct pagetable *subtable = (struct pagetable *) (entry->addr << 12);
            for (j = 0; j < ENTRIES_PER_TABLE; j++) {
                struct pageentry *subentry = &subtable->entries[j];
                if (subentry->present && subentry->avail) {
                    void *paddr = (void *) (subentry->addr << 12);
                    page_free(paddr);
                }
            }
            page_free(subtable);
        }
    }

    page_free(pt);
}

// Function to allocate pages in a page table
int pagetable_alloc(struct pagetable *pt, unsigned vaddr, unsigned length, int flags) {

    unsigned npages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    /*length + PAGE_SIZE - 1: This adds `PAGE_SIZE - 1` to the length.
	* This is done to ensure that any remainder when dividing by `PAGE_SIZE`
	* is accounted for, effectively rounding up to the next full page size
	* if there's any leftover.*/

    if (validate_address(vaddr) == PAGE_TABLE_INVALID_ADDR || validate_address(vaddr + length) == PAGE_TABLE_INVALID_ADDR) {
        return PAGE_TABLE_INVALID_ADDR;
    }

    vaddr &= 0xFFFFF000;

    while (npages > 0) {
        unsigned paddr;
        if (pagetable_getmap(pt, vaddr, &paddr, 0) == PAGE_TABLE_FAILURE) {
            if (pagetable_map(pt, vaddr, 0, flags | PAGE_FLAG_ALLOC) == PAGE_TABLE_FAILURE) {
                LOG(LOG_LEVEL_ERROR, "Failed to allocate page at virtual address 0x%x", vaddr);
                return PAGE_TABLE_FAILURE;
            }
        }
        vaddr += PAGE_SIZE;
        npages--;
    }

    return PAGE_TABLE_SUCCESS;
}

// Function to free pages in a page table
int pagetable_free(struct pagetable *pt, unsigned vaddr, unsigned length) {
    unsigned npages = (length + PAGE_SIZE - 1) / PAGE_SIZE;

    if (validate_address(vaddr) == PAGE_TABLE_INVALID_ADDR || validate_address(vaddr + length) == PAGE_TABLE_INVALID_ADDR) {
        return PAGE_TABLE_INVALID_ADDR;
    }

    vaddr &= 0xFFFFF000;

    while (npages > 0) {
        unsigned paddr;
        int flags;
        if (pagetable_getmap(pt, vaddr, &paddr, &flags) == PAGE_TABLE_SUCCESS) {
            pagetable_unmap(pt, vaddr);
            if (flags & PAGE_FLAG_ALLOC) {
                page_free((void *) paddr);
            }
        } else {
            LOG(LOG_LEVEL_WARN, "Failed to retrieve mapping for virtual address 0x%x", vaddr);
        }
        vaddr += PAGE_SIZE;
        npages--;
    }

    return PAGE_TABLE_SUCCESS;
}

// Function to load a page table
struct pagetable *pagetable_load(struct pagetable *pt) {
    struct pagetable *old_pt;
    asm ("mov %%cr3, %0" : "=r"(old_pt));
    asm ("mov %0, %%cr3" :: "r"(pt));
    return old_pt;
}

// Function to refresh the page table
void pagetable_refresh() {
    asm ("mov %cr3, %eax");
    asm ("mov %eax, %cr3");
}

// Function to enable paging
void pagetable_enable() {
    asm ("movl %cr0, %eax");
    asm ("orl $0x80000000, %eax");
    asm ("movl %eax, %cr0");
}

// Function to duplicate a page table
struct pagetable *pagetable_duplicate(struct pagetable *source_pt) {
    unsigned i, j;

    struct pagetable *new_pt = pagetable_create();
    if (!new_pt) {
        LOG(LOG_LEVEL_ERROR, "Failed to create new page table in pagetable_duplicate");
        return 0;
    }

    for (i = 0; i < ENTRIES_PER_TABLE; i++) {
        struct pageentry *src_entry = &source_pt->entries[i];
        struct pageentry *new_entry = &new_pt->entries[i];

        if (src_entry->present) {
            struct pagetable *src_subtable = (struct pagetable *) (src_entry->addr << 12);
            struct pagetable *new_subtable = pagetable_create();
            if (!new_subtable) {
                LOG(LOG_LEVEL_ERROR, "Failed to create new subpage table in pagetable_duplicate");
                pagetable_delete(new_pt);
                return 0;
            }

            memcpy(new_entry, src_entry, sizeof(struct pageentry));
            new_entry->addr = (unsigned) new_subtable >> 12;

            for (j = 0; j < ENTRIES_PER_TABLE; j++) {
                struct pageentry *src_subentry = &src_subtable->entries[j];
                struct pageentry *new_subentry = &new_subtable->entries[j];
                memcpy(new_subentry, src_subentry, sizeof(struct pageentry));

                if (src_subentry->present) {
                    void *src_paddr = (void *) (src_subentry->addr << 12);
                    void *new_paddr = src_subentry->avail ? page_alloc(0) : src_paddr;
                    if (src_subentry->avail && !new_paddr) {
                        LOG(LOG_LEVEL_ERROR, "Failed to allocate new page for duplicate");
                        pagetable_delete(new_pt);
                        return 0;
                    }
                    if (new_paddr != src_paddr) {
                        memcpy(new_paddr, src_paddr, PAGE_SIZE);
                    }
                    new_subentry->addr = (unsigned) new_paddr >> 12;
                }
            }
        }
    }

    return new_pt;
}

void pagetable_copy(struct pagetable *src_pt, unsigned src_addr, struct pagetable *dst_pt, unsigned dst_addr, unsigned length);

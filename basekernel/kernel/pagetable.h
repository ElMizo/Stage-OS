/****************************************************************
* Copyright (C) 2015-2019 The University of Notre Dame
* This software is distributed under the GNU General Public License.
* See the file LICENSE for details.
*****************************************************************/
/**
 * @file pagetable.h 
 * @brief memory management module for a kernel, by implementing paging
 *
 * The memory management module is a critical kernel component that divides
 * physical memory into pages and uses a hierarchical page table to map 
 * virtual addresses to physical addresses. It allocates and deallocates 
 * memory, handles page faults, and employs a page replacement algorithm 
 * to manage memory efficiently, ensuring system performance, security, 
 * and reliability.
 *
 * @author Khalid ElKoussami, Hamza Aarab, Abdelali Chattaoui, Anas Azouane, ElHassan Labyad
 * @date 04/08/2024
 *
 */

#ifndef PAGETABLE_H
#define PAGETABLE_H

#define PAGE_SIZE 4096

#define PAGE_FLAG_USER        0
#define PAGE_FLAG_KERNEL      1
#define PAGE_FLAG_EXISTS      0
#define PAGE_FLAG_ALLOC       2
#define PAGE_FLAG_READONLY    0
#define PAGE_FLAG_READWRITE   4
#define PAGE_FLAG_NOCLEAR     0
#define PAGE_FLAG_CLEAR       8

/*************************************************************************
                       Breakdown of a Virtual Address
**************************************************************************
* A 32-bit virtual address can be split into parts for indexing into a 
* two-level page table structure.
*
* Higher Bits: Used to index into the first-level page table.
* Middle Bits: Used to index into the second-level page table.
* Lower Bits: Represent the offset within the page.
* For a typical 32-bit address, if we use 10 bits for each level of 
* the page table and 12 bits for the offset, the virtual address can 
*be divided as follows:
* |   10 bits   |   10 bits   |   12 bits   |
* |  Directory  |    Table    |    Offset   |
*
* Shifting right by 12 bits removes the page offset, isolating the indices 
* needed for the page tables.
* Shifting left by 12 bits converts a page frame number to a physical address.
***************************************************************************/


/**
 * @brief Creates a new page table
 *
 * The pagetable_create() function creates a new page table, initializing its
 * internal structures and preparing it for use.
 *
 * @return a pointer to the newly created page table
 */
struct pagetable *pagetable_create();

/**
 * @brief Initializes a page table
 *
 * The pagetable_init() function initializes a page table, setting up its
 * internal structures and preparing it for use.
 *
 * @param p is a pointer to the page table to be initialized
 */
void pagetable_init(struct pagetable *p);

/**
 * @brief Maps a virtual address to a physical address
 *
 * The pagetable_map() function maps a virtual address to a physical address,
 * updating the page table and handling any necessary page faults.
 *
 * @param p is a pointer to the page table
 * @param vaddr is the virtual address to be mapped
 * @param paddr is the physical address to be mapped to
 * @param flags are the flags to be used for the mapping
 *
 * @return an integer indicating the success or failure of the mapping
 */
int pagetable_map(struct pagetable *p, unsigned vaddr, unsigned paddr, int flags);

/**
 * @brief Retrieves the physical address and flags associated with a virtual address
 *
 * The pagetable_getmap() function retrieves the physical address and flags
 * associated with a virtual address, using the page table to perform the
 * lookup.
 *
 * @param p is a pointer to the page table
 * @param vaddr is the virtual address to be looked up
 * @param paddr is a pointer to the physical address to be retrieved
 * @param flags is a pointer to the flags to be retrieved
 *
 * @return an integer indicating the success or failure of the lookup
 */
int pagetable_getmap(struct pagetable *p, unsigned vaddr, unsigned *paddr, int *flags);

/**
 * @brief Unmaps a virtual address
 *
 * The pagetable_unmap() function unmaps a virtual address, updating the page
 * table and handling any necessary page faults.
 *
 * @param p is a pointer to the page table
 * @param vaddr is the virtual address to be unmapped
 */
void pagetable_unmap(struct pagetable *p, unsigned vaddr);

/**
 * @brief Allocates a range of virtual addresses
 *
 * The pagetable_alloc() function allocates a range of virtual addresses,
 * updating the page table and handling any necessary page faults.
 *
 * @param p is a pointer to the page table
 * @param vaddr is the starting virtual address of the range to be allocated
 * @param length is the length of the range to be allocated
 * @param flags are the flags to be used for the allocation
 */
void pagetable_alloc(struct pagetable *p, unsigned vaddr, unsigned length, int flags);

/**
 * @brief Frees a range of virtual addresses
 *
 * The pagetable_free() function frees a range of virtual addresses,
 * updating the page table and handling any necessary page faults.
 *
 * @param p is a pointer to the page table
 * @param vaddr is the starting virtual address of the range to be freed
 * @param length is the length of the range to be freed
 */
void pagetable_free(struct pagetable *p, unsigned vaddr, unsigned length);

/**
 * @brief Deletes a page table
 *
 * The pagetable_delete() function deletes a page table, freeing any
 * associated resources and updating the internal structures.
 *
 * @param p is a pointer to the page table to be deleted
 */
void pagetable_delete(struct pagetable *p);
/**
 * @brief Loads a page table
 *
 * The pagetable_load() function loads a page table, preparing it for use
 * by the system.
 *
 * @param p is a pointer to the page table to be loaded
 *
 * @return a pointer to the loaded page table
 */
struct pagetable *pagetable_load(struct pagetable *p);
/**
 * @brief Enables paging
 *
 * The pagetable_enable() function enables paging, allowing the system to
 * use the page table for memory management.
 */
void pagetable_enable();

/**
 * @brief Refreshes the page table
 *
 * The pagetable_refresh() function refreshes the page table, updating its
 * internal structures and ensuring that it is consistent with the current
 * state of the system.
 */
void pagetable_refresh();

#endif

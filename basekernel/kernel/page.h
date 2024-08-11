/****************************************************************
* Copyright (C) 2015-2019 The University of Notre Dame
* This software is distributed under the GNU General Public License.
* See the file LICENSE for details.
*****************************************************************/
/**
 * @file page.h 
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

#ifndef PAGE_H
#define PAGE_H

#include "kernel/types.h"

void  page_init(); //initializes the page management system
/********************************************************************************************
 * @brief initializes the page management system
 *
 * The page_init() function initializes the page management system, setting up memory mapping,
 * page tables, allocation, and statistics, to prepare the system for memory management.
 *
 ********************************************************************************************/

void *page_alloc(bool zeroit); //allocates a new page of memory
/********************************************************************************************
 * @brief Allocates a new page of memory
 *
 * The page_alloc() function allocates a free memory page, managing page allocation, and returns
 * a pointer to the newly allocated page.
 *
 * @param  zeroit is a boolean that indicates whether the allocated page should be initialized
 * with zeros.
 * 
 * @return a pointer to the newly allocated page
 * 
 ********************************************************************************************/

void  page_free(void *addr); //frees a previously allocated page of memory
/********************************************************************************************
 * @brief frees a previously allocated page of memory
 *
 * The page_free() function frees a previously allocated memory page, updating page allocation 
 * statistics and making the page available for future allocations.
 * 
 * @param addr is the address of the page to be freed.
 * 
 ********************************************************************************************/

void  page_stats( uint32_t *nfree, uint32_t *ntotal ); //takes statistics about the page management system
/********************************************************************************************
 * @brief takes statistics about the page management system
 *
 * The page_stats() function returns statistics about the current state of the page allocator, such as 
 * the number of free pages, the number of allocated pages, and the total number of pages.
 * 
 * @param  nfree is a pointer to uint32_t variable that will hold the number of free pages.
 * @param  ntotal is a pointer to uint32_t variable that will hold the number of pages.

 ********************************************************************************************/


#endif


/****************************************************************
* Copyright (C) 2016-2019 The University of Notre Dame
* This software is distributed under the GNU General Public License.
* See the file LICENSE for details.
*****************************************************************/
/**
 * @file kmalloc.h 
 * @brief Kernel memory allocation module
 *
 * The kernel memory allocation module provides dynamic memory allocation
 * and deallocation functions for kernel space. It includes initialization
 * routines, memory allocation, and deallocation functions, as well as
 * debugging utilities to monitor memory usage and detect leaks.
 * 
 * This module ensures efficient memory usage and management within the
 * kernel, which is critical for system stability and performance.
 *
 * @author Khalid ElKoussami, Hamza Aarab, Abdelali Chattaoui, Anas Azouane, ElHassan Labyad
 * @date 04/08/2024
 *
 */

#ifndef KMALLOC_H
#define KMALLOC_H

/**
 * @brief Allocates memory in the kernel space
 *
 * The kmalloc() function allocates a block of memory of the specified length
 * in the kernel space.
 *
 * @param length is the length of the memory block to be allocated
 * @return a pointer to the allocated memory block, or NULL if allocation fails
 */
void *kmalloc(int length);

/**
 * @brief Frees previously allocated memory
 *
 * The kfree() function frees a block of memory previously allocated by kmalloc().
 *
 * @param ptr is a pointer to the memory block to be freed
 */
void kfree(void *ptr);

/**
 * @brief Initializes the kernel memory allocator
 *
 * The kmalloc_init() function initializes the kernel memory allocator with a given
 * starting address and length of the memory pool.
 *
 * @param start is the starting address of the memory pool
 * @param length is the length of the memory pool
 */
void kmalloc_init(char *start, int length);

/**
 * @brief Outputs debugging information for the kernel memory allocator
 *
 * The kmalloc_debug() function outputs debugging information about the current state
 * of the kernel memory allocator, including allocated blocks and any detected issues.
 */
void kmalloc_debug();

#endif // KMALLOC_H

# What's different
<p align="center">
  <img src="https://github.com/ElMizo/Stage-OS/blob/Khalid_branch/same.gif">
</p>
##So what are the actually differences
In the memory management section of our OS, our primary goal is to prioritize code clarity and ease of understanding while maintaining full functionality in memory management. We aim to make the code seamless and straightforward for any reader. To achieve this, we focused on enhancing `the page.c` and `pagetable.c` files through targeted improvements. Specifically, we implemented the Clock Paging Algorithm in the page replacement system to efficiently manage memory swaps. Additionally, we added a fault handler that provides feedback on the success of operations like page allocation and deallocation. Each file now includes detailed headers that explain every aspect of the code.

# Clock Paging Algorithm

The Clock Paging Algorithm is a page replacement strategy used in operating systems to manage the pages in memory. It is an approximation of the Least Recently Used (LRU) algorithm, designed to be more efficient in terms of time complexity.

## How It Works

1. **Circular List (Clock)**: Imagine the pages in memory are arranged in a circular list, much like the numbers on a clock. Each page has an associated "reference bit" that indicates whether the page has been accessed recently.

2. **Clock Hand**: There is a "clock hand" that points to one of the pages in this circular list. This hand moves around the circle as pages are accessed and replaced.

3. **Reference Bit**:
   - When a page is accessed, its reference bit is set to 1.
   - The clock hand checks the reference bit of the page it points to:
     - If the reference bit is 0, the page is not recently used, and it can be replaced.
     - If the reference bit is 1, it means the page was recently used. The algorithm sets the reference bit to 0 and moves the clock hand to the next page.

4. **Page Replacement**:
   - The algorithm continues to move the clock hand, clearing reference bits of pages that have been accessed recently.
   - It stops when it finds a page with a reference bit of 0, which means the page hasn't been accessed recently and can be replaced with a new page.

## Summary

- **Efficient**: The Clock Paging Algorithm is efficient because it doesn't require a complete traversal of all pages to find a replacement.
- **Approximation of LRU**: It approximates the Least Recently Used (LRU) algorithm by using reference bits to track recent usage.
- **Practical**: The algorithm is practical and widely used in operating systems due to its balance between performance and simplicity.

## Example Scenario

Imagine you have four pages in memory, and their reference bits are as follows (with the clock hand pointing to page 1):

Clock Hand -->
Page 1 [reference bit: 1]                                                                                                                                                                                                             
 Clock Hand + 1 -->Page 2 [reference bit: 1]                                                                                                                                                                                                          
 Clock Hand + 2 -->Page 3 [reference bit: 0]                                                                                                                                                                                                                
 Clock Hand + 3 -->Page 4 [reference bit: 0]

- If a new page needs to be loaded, the clock hand will check Page 1. Since its reference bit is 1, it sets the bit to 0 and moves to Page 2.
- Page 2 also has a reference bit of 1, so the algorithm sets it to 0 and moves to Page 3.
- Page 3 has a reference bit of 0, so it selects this page for replacement.

This way, the algorithm ensures that pages that have been accessed recently are less likely to be replaced, approximating an LRU strategy.

# log_error

## Overview

The `log_error` function is used to log error messages in the `pagetable.c` code. It helps with debugging by recording issues that occur during the execution of the code.

## Purpose

The primary purpose of the log_error function is to record error messages whenever an error condition is encountered in the code. This helps in debugging and monitoring the system.

## Implementation method 

### Creation of log files
- We developed C and header files that include a function for invoking the 'printk' function, a fundamental logging mechanism in the Linux kernel. This function allows developers to print messages to log files with varying levels of importance. It accepts three arguments: an integer representing the log level (e.g., LOG_LEVEL_ERR), a string format specifier (fmt), and a variable number of arguments to populate the format specifier. The function first determines the appropriate log level string based on the level argument. It then uses the vsprintf function to format the string with the provided format specifier and arguments. Finally, the formatted string, including the log level string, is printed to the log file.
# Memory management
## Paging
Paging is a memory management scheme that eliminates the need for contiguous allocation of physical memory. This allows the physical address space of a process to be noncontiguous, making efficient use of available memory.
### Paging in simple terms:
Paging breaks down memory into fixed-size blocks called "pages" for the process's logical memory, and into blocks of the same size called "frames" for the physical memory. When a process is to be executed, its pages are loaded into any available memory frames from the disk.
### Key Concepts
- **Page:** A fixed-length contiguous block of virtual memory.
- **Frame:** A fixed-length block of physical memory, of the same size as a page.
- **Page Table:** A data structure used by the operating system to keep track of the mapping between virtual addresses and physical addresses.
### Illustration of Paging
            Logical Address Space          Physical Address Space
         +------------------------+       +------------------------+
         | Page 0 | Page 1 | ...  |       | Frame 0| Frame 1| ...  |
         +--------+--------+----- +       +--------+--------+------+
         |        |        |      |       |        |        |      |
         |  Data  |  Data  | ...  |       |  Data  |  Data  | ...  |
         |        |        |      |       |        |        |      |
         +--------+--------+------+       +--------+--------+------+

           Logical Address Space:                   Page Table:
         +--------+--------+------+       +-------------+--------------+
         | Page 0 | Page 1 | ...  |       | Page Number | Frame Number |
         +--------+--------+------+       +-------------+--------------+
         |        |        |      |       | Page 0      | Frame 2      |
         |  Data  |  Data  | ...  |       | Page 1      | Frame 4      |
         |        |        |      |       | Page 2      | Frame 5      |
         +--------+--------+------+       +-------------+--------------+


### Memory Management Diagram

    +---------------+          +---------------+           +----------------+           +---------------+           +---------------+           +---------------+           +---------------+
    |               |          |               |           |                |           |               |           |               |           |               |           |               |
    |   Process     |   --->   |  Memory       |   --->    |  Paging        |   --->    |  Page Table   |   --->    |  Page Table   |   --->    |  Physical     |   --->    |  Page Presence|
    |  (Virtual     |          |  Management   |           |  Directory     |           |  (PT)         |           |  Entry (PTE)  |           |  Memory       |           |  Bit (PPB)    |
    |   Address     |          |  Unit (MMU)   |           |  (PD)          |           |               |           |               |           |  (RAM)        |           |               |
    |   Space)      |          |  (Translation |           |  (Contains     |           |  (Contains    |           |  (Contains    |           |  (Divided into|           |  (Indicates   |
    |               |          |   of Virtual  |           |   1024 4-byte  |           |   1024 4-byte |           |   Page Frame  |           |   4 KiB Page  |           |   whether the |
    |               |          |   Address)    |           |   entries,     |           |   entries,    |           |   Number,     |           |   Frames)     |           |   page is in  |
    |               |          |               |           |   each pointing|           |   each pointing|          |   Page Status,|           |               |           |   physical    |
    |               |          |               |           |   to a Page    |           |   to a 4 KiB  |           |   Access      |           |               |           |   memory)     |
    +---------------+          +---------------+           |   table)       |           |   Physical    |           |   Control,    |           +---------------+           +---------------+
                                                           |                |           |   Page Frame) |           |   Cache       |                                                |
                                                           +----------------+           +---------------+           |   Control)    |                                                |
                                                                                                                    +---------------+                                                |
             ________________________________________________________________________________________________________________________________________________________________________|
            |
           \_/ 
    +---------------+          +---------------+           +---------------+           +---------------+           +----------------+           
    |               |          |               |           |               |           |               |           |                |         
    |  Page Fault   |   --->   |  Page Fault   |   --->    |  Page         |   --->    |  Disk         |   --->    |  Disk I/O      |       
    |  (Trap to     |          |  Handler      |           |  Replacement  |           |  Storage      |           |  (Read/Write)  |       
    |   Operating   |          |  (Determines  |           |  Algorithm    |           |  (Hard Drive) |           |                |       
    |   System)     |          |   cause of    |           |  (Selects     |           |  (Stores      |           |  (Reads page   |        
    |               |          |   page fault, |           |   victim page |           |   pages that  |           |   from disk    |         
    |               |          |   selects     |           |   to replace) |           |   are not in  |           |   storage into |          
    |               |          |   replacement |           |               |           |   physical    |           |   physical     |        
    |               |          |   page)       |           |               |           |   memory)     |           |   memory, or   |       
    +---------------+          +---------------+           +---------------+           +---------------+           |   writes page  |         
                                                                                                                   |   from physical|
                                                                                                                   |   memory to    |
                                                                                                                   |   disk storage |
                                                                                                                   +----------------+
                                                                                                                             |
                                                                                                                             |
             ________________________________________________________________________________________________________________|
            |
           \_/ 
    +---------------+           +---------------+
    |               |           |               |
    |  Page Table   |   --->    |  Process      |
    |  Update       |           |  (Continued)  |
    |  (Updates     |           |  (Resumes     |
    |   page table  |           |   execution   |
    |   entry to    |           |   with updated|
    |   reflect new |           |   page table) |
    |   mapping of  |           |               |
    |   virtual page|           |               |
    |   to physical |           |               |
    |   page)       |           |               |
    +---------------+           +---------------+
## Conclusion
Paging is a critical concept in modern operating systems that helps manage memory efficiently by breaking logical memory into fixed-size pages and mapping them to physical frames.

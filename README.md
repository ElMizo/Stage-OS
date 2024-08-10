<h1 align="center" style="font-family: Georgia, serif;">WHAT'S DIFFERENT IN THE MEMORY MANAGEMENT SECTION</h1>

<p align="center">
  <img src="https://github.com/ElMizo/Stage-OS/blob/Khalid_branch/same.gif">
</p>

# So what are the actual differences
In the memory management section of our OS, our main objective is to enhance code clarity and readability while ensuring the memory management remains fully functional. We aim to create code that is intuitive and easy for any developer to follow. To accomplish this, we focused on refining the `page.c` and `pagetable.c` files with specific improvements. In particular, page.c has been enhanced with Memory Corruption Detection, Enhanced Boundary Checks, Error Handling and Reporting, and Logging, as detailed below:
## Memory Corruption Detection

Memory corruption occurs when a program unintentionally modifies memory, leading to unpredictable behavior or security vulnerabilities. To detect memory corruption, this system uses a **guard pattern** (`GUARD_PATTERN`) during both page allocation and deallocation.

- **During Allocation (`page_alloc`)**: Before allocating a page, the system checks if the memory at the target address matches the guard pattern. If it does, an error is logged, and the allocation is aborted, as this indicates potential memory corruption.
- **During Deallocation (`page_free`)**: When a page is freed, the system checks the memory for the guard pattern again. If detected, an error is logged, signaling possible memory corruption.

This detection mechanism helps catch issues like buffer overflows or unintended memory modifications, improving the security and reliability of the system.

## Enhanced Boundary Checks

Boundary checks ensure that memory accesses stay within valid bounds, preventing out-of-bounds errors that could lead to crashes or undefined behavior.

- **During Allocation (`page_alloc`)**: After identifying a free page, the system checks if the page number is within valid memory limits. If it exceeds the bounds, an error is logged, and the operation is aborted.
- **During Deallocation (`page_free`)**: The system checks if the address being freed is within the valid memory range and properly aligned. If the address is invalid, an error is logged, and the deallocation is halted.

These checks prevent memory access violations, ensuring the stability and safety of the memory management system.

## Error Handling and Reporting

The system integrates robust error handling and reporting mechanisms to ensure that issues are detected and addressed promptly.

- **Error Reporting**: The system uses a logging mechanism to report errors at different severity levels:
  - **Warnings** for non-critical issues that may indicate potential problems (e.g., memory overcommitment).
  - **Errors** for critical issues that require immediate attention (e.g., memory corruption, invalid operations).
  
- **Handling Initialization**: Functions like `page_alloc` and `page_free` include checks to ensure that the memory management system is initialized before performing operations. If the system is not initialized, an error is logged, and the operation is prevented.

These mechanisms ensure that the system remains stable and that problems are identified and resolved quickly.

## Reporting and Logging

Logging is essential for tracking the behavior of the memory management system and diagnosing issues.

- **Logging Mechanism**: The system uses a `LOG` macro to log messages at various levels, including informational messages during initialization, warnings for potential issues, and errors for critical failures.
- **Detailed Logging**: Logs capture details such as the total memory available, the number of free pages, and specific errors like invalid page addresses or memory corruption.

These logs are invaluable for understanding system behavior, troubleshooting issues, and maintaining system stability.

# Memory management
## Paging
<p align="center">
  <img src="Paging_Structure.gif">
</p>

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

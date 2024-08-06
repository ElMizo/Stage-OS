# Stage-OS

## Basekernel
Made by : Douglas Thain (dthain)

Modified by : Khalid ElKoussami, Hamza Aarab, Abdelali Chattaoui, Anas Azouane, ElHassan Labyad.


# memory management:
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


+---------------+          +---------------+        +----------------+          +----------------+
|   Process     |          |  Memory       |        |  Paging        |          |  Page Table    |
|   (Virtual    |          |  Management   |        |  Directory     |          |  (PT)          |
|    Address    |          |  Unit (MMU)   |        |  (PD)          |          |                |
|    Space)     |          |  (Translation |        |                |          |  (Contains     |
+---------------+          |   of Virtual  |        |  (Contains     |          |   1024 4-byte  |
                           |   Address)    |        |   1024 4-byte  |          |   entries,     |
                           +---------------+        |   entries,     |          |   each pointing|
                                                    |   each pointing|          |   to a 4 KiB   |
                                                    |   to a Page    |          |   Physical     |
                                                    |   table)       |          |   Page Frame)  |
                                                    +----------------+          +----------------+
          _ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ |
         |
        \_/
+---------------+           +---------------+       +---------------+           +---------------+
|  Page Table   |           |  Physical     |       |  Page Presence|           |  Page Fault   |
|  Entry (PTE)  |           |  Memory       |       |  Bit (PPB)    |           |  (Trap to     |
|               |           |  (RAM)        |       |               |           |   Operating   |
|  (Contains    |=========> |               |=====> |  (Indicates   |==========>|   System)     |
|   Page Frame  |           |  (Divided into|       |   whether the |           +---------------+   
|   Number,     |           |   4 KiB Page  |       |   page is in  |                    |
|   Page Status,|           |   Frames)     |       |   physical    |                    |
|   Access      |           +---------------+       |   memory)     |                    |           
|   Control,    |                                   +---------------+                    |           
|   Cache       |                                                                        |           
|   Control)    |                                                                        |           
+---------------+                                                                        |
          _ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ |
         |
        \_/
+---------------+           +---------------+       +---------------+           +----------------+
|  Page fault   |           |  Page         |       |  Disk Storage |           |  Disk I/O      |
|  Handler      |           |  Replacement  |       |  (Hard Drive) |           |  (Read/Write)  |
|               |           |  Algorithm    |       |               |           |                |
|  (Determines  |=========> |               |=====> |  (Stores      |==========>|  (Reads page)  |
|   Cause of    |           |  (Selects     |       |   pages that  |           |   from disk    |   
|   page fault  |           |   victim page |       |   are not in  |           |   storage into |
|   selects     |           |   to replace  |       |   physical    |           |   physical     |
|   replacement |           +---------------+       |   memory)     |           |   memory, or   |
|   page)       |                                   +---------------+           |   writes page  |
+---------------+                                                               |   from physical|
                                                                                |   memory to    |
                                                                                |   disk storage |     
                                                                                +----------------+
                                                                                         |
          _ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ __ _ |
         |
        \_/

+---------------+           +---------------+
|  Page Table   |           |  Process      |
|  Update       |           |  (Continued)  |
|               |           |               |
|  (Updates     |=========> |  (Resumes     |
|   page table  |           |   execution   |
|   entry to    |           |   with updated|
|   reflect new |           |   page table) |
|   mapping of  |           +---------------+
|   virtual page|           
|   to physical |           
|   page)       |           
+---------------+           

## Conclusion
Paging is a critical concept in modern operating systems that helps manage memory efficiently by breaking logical memory into fixed-size pages and mapping them to physical frames.

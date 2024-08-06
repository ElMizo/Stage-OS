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


### Memory Management Diagram

                                      +---------------+
                                      |  Process      |
                                      |  (Virtual     |
                                      |   Address     |
                                      |   Space)      |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Memory       |
                                      |  Management   |
                                      |  Unit (MMU)   |
                                      |               |
                                      |  (Translation |
                                      |   of Virtual  |
                                      |   Address)    |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Paging       |
                                      |  Directory    |
                                      |  (PD)         |
                                      |               |
                                      |  (Contains    |
                                      |   1024 4-byte |
                                      |   entries,    |
                                      |   each        |
                                      |   pointing to |
                                      |   a Page      |
                                      |   Table)      |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Page Table   |
                                      |  (PT)         |
                                      |               |
                                      |  (Contains    |
                                      |   1024 4-byte |
                                      |   entries,    |
                                      |   each        |
                                      |   pointing to |
                                      |   a 4 KiB     |
                                      |   Physical    |
                                      |   Page Frame) |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Page Table   |
                                      |  Entry (PTE)  |
                                      |               |
                                      |  (Contains    |
                                      |   Page Frame  |
                                      |   Number,     |
                                      |   Page Status,|
                                      |   Access      |
                                      |   Control,    |
                                      |   Cache       |
                                      |   Control)    |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Physical     |
                                      |  Memory       |
                                      |  (RAM)        |
                                      |               |
                                      |  (Divided into|
                                      |   4 KiB Page  |
                                      |   Frames)     |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Page Presence|
                                      |  Bit (PPB)    |
                                      |               |
                                      |  (Indicates   |
                                      |   whether the |
                                      |   page is in  |
                                      |   physical    |
                                      |   memory)     |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Page Fault   |
                                      |  (Trap to     |
                                      |   Operating   |
                                      |   System)     |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Page Fault   |
                                      |  Handler      |
                                      |               |
                                      |  (Determines  |
                                      |   cause of    |
                                      |   page fault, |
                                      |   selects     |
                                      |   replacement |
                                      |   page)       |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Page         |
                                      |  Replacement  |
                                      |  Algorithm    |
                                      |               |
                                      |  (Selects     |
                                      |   victim page |
                                      |   to replace) |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Disk Storage |
                                      |  (Hard Drive) |
                                      |               |
                                      |  (Stores      |
                                      |   pages that  |
                                      |   are not in  |
                                      |   physical    |
                                      |   memory)     |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Disk I/O     |
                                      |  (Read/Write) |
                                      |               |
                                      |  (Reads page  |
                                      |   from disk   |
                                      |   storage into|
                                      |   physical    |
                                      |   memory, or  |
                                      |   writes page |
                                      |   from        |
                                      |   physical    |
                                      |   memory to   |
                                      |   disk storage)|
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Page Table   |
                                      |  Update       |
                                      |               |
                                      |  (Updates     |
                                      |   page table  |
                                      |   entry to    |
                                      |   reflect new |
                                      |   mapping of  |
                                      |   virtual page|
                                      |   to physical |
                                      |   page)       |
                                      +---------------+
                                             |
                                             |
                                             v
                                      +---------------+
                                      |  Process      |
                                      |  (Continued)  |
                                      |               |
                                      |  (Resumes     |
                                      |   execution   |
                                      |   with        |
                                      |   updated     |
                                      |   page table) |
                                      +---------------+

## Conclusion
Paging is a critical concept in modern operating systems that helps manage memory efficiently by breaking logical memory into fixed-size pages and mapping them to physical frames.

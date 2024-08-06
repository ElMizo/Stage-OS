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

# Memory Management Diagram

# Memory Management Diagram

          Process                  Memory Management            Paging Directory                Page Table                Page Table Entry (PTE)         Physical Memory               Page Presence Bit (PPB)
       (Virtual Address                Unit (MMU)                       (PD)                           (PT)                           (Contains)                       (RAM)                        (Indicates
            Space)                (Translation of Virtual      (Contains 1024 4-byte          (Contains 1024 4-byte           Page Frame Number,                  (Divided into               whether the page
                                      Address)                    entries, each                entries, each pointing        Page Status, Access                 4 KiB Page Frames)            is in physical
                                                               pointing to a Page Table)         to a 4 KiB Physical           Control, Cache Control)                                             memory)
                                                                                                     Page Frame)

    +---------------+          +---------------+           +---------------+           +---------------+           +---------------+           +---------------+           +---------------+
    |               |          |               |           |               |           |               |           |               |           |               |           |               |
    |   Process     |   --->   |  Memory       |   --->    |  Paging       |   --->    |  Page Table   |   --->    |  Page Table   |   --->    |  Physical     |   --->    |  Page Presence|
    |  (Virtual     |          |  Management   |           |  Directory    |           |  (PT)         |           |  Entry (PTE)  |           |  Memory       |           |  Bit (PPB)    |
    |   Address     |          |  Unit (MMU)   |           |  (PD)         |           |               |           |               |           |  (RAM)        |           |               |
    |   Space)      |          |  (Translation |           |  (Contains    |           |  (Contains    |           |  (Contains    |           |  (Divided into|           |  (Indicates   |
    |               |          |   of Virtual  |           |   1024 4-byte |           |   1024 4-byte |           |   Page Frame  |           |   4 KiB Page  |           |   whether the |
    |               |          |   Address)    |           |   entries,    |           |   entries,    |           |   Number,     |           |   Frames)     |           |   page is in  |
    |               |          |               |           |   each pointing|          |   each pointing|           |   Page Status,|           |               |           |   physical    |
    |               |          |               |           |   to a Page   |           |   to a 4 KiB  |           |   Access      |           |               |           |   memory)     |
    +---------------+          +---------------+           |   table)      |           |   Physical    |           |   Control,    |           +---------------+           +---------------+
                                                          |               |           |   Page Frame) |           |   Cache       |
                                                          +---------------+           +---------------+           |   Control)    |
                                                                                                                +---------------+

                                  Page Fault                    Page Fault                   Page Replacement               Disk Storage                      Disk I/O
                                  (Trap to                      Handler                      Algorithm                      (Hard Drive)                      (Read/Write)
                                  Operating                     (Determines                  (Selects victim                 (Stores pages                    (Reads page from
                                  System)                       cause of page                page to replace)                that are not in                   disk storage into
                                                                fault, selects                                                 physical memory)                 physical memory, or
                                                                replacement page)                                                                               writes page from
                                                                                                                                                               physical memory to
                                                                                                                                                               disk storage)
 
    +---------------+          +---------------+           +---------------+           +---------------+           +---------------+           +---------------+
    |               |          |               |           |               |           |               |           |               |           |               |
    |  Page Fault   |   --->   |  Page Fault   |   --->    |  Page         |   --->    |  Disk         |   --->    |  Disk I/O     |           |               |
    |  (Trap to     |          |  Handler      |           |  Replacement  |           |  Storage      |           |  (Read/Write) |           |               |
    |   Operating   |          |  (Determines  |           |  Algorithm    |           |  (Hard Drive) |           |               |           |               |
    |   System)     |          |   cause of    |           |  (Selects     |           |  (Stores      |           |  (Reads page  |           |               |
    |               |          |   page fault, |           |   victim page |           |   pages that  |           |   from disk   |           |               |
    |               |          |   selects     |           |   to replace) |           |   are not in  |           |   storage into|           |               |
    |               |          |   replacement |           |               |           |   physical    |           |   physical    |           |               |
    |               |          |   page)       |           |               |           |   memory)     |           |   memory, or  |           |               |
    +---------------+          +---------------+           +---------------+           +---------------+           |   writes page |           +---------------+
                                                                                                                |   from physical|
                                                                                                                |   memory to    |
                                                                                                                |   disk storage |
                                                                                                                +---------------+

                                 Page Table Update               Process
                                 (Updates page table entry       (Continued)
                                 to reflect new mapping of       (Resumes execution with
                                 virtual page to physical page)  updated page table)

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

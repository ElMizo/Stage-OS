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
    |               |          |               |           |   each pointing|          |   each pointing|          |   Page Status,|           |               |           |   physical    |
    |               |          |               |           |   to a Page   |           |   to a 4 KiB  |           |   Access      |           |               |           |   memory)     |
    +---------------+          +---------------+           |   table)      |           |   Physical    |           |   Control,    |           +---------------+           +---------------+
                                                           |               |           |   Page Frame) |           |   Cache       |
                                                           +---------------+           +---------------+           |   Control)    |
                                                                                                                   +---------------+

## Conclusion
Paging is a critical concept in modern operating systems that helps manage memory efficiently by breaking logical memory into fixed-size pages and mapping them to physical frames.

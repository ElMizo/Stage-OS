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

+---------------++---------------++----------------+
|   Process     ||  Memory       ||  Paging        |
|   (Virtual    ||  Management   ||  Directory     |
|    Address    ||  Unit (MMU)   ||  (PD)          |
|    Space)     ||  (Translation ||                |
+---------------+|   of Virtual  ||  (Contains     |
                 |   Address)    ||   1024 4-byte  |
                 +---------------+|   entries,     |
                                  |   each pointing|
                                  |   to a Page    |
                                  |   table)       |
                                  +----------------+

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
+---------------++---------------++----------------+
|   Process     ||  Memory       ||  Paging        |
|   (Virtual    ||  Management   ||  Directory     |
|    Address    ||  Unit (MMU)   ||  (PD)          |
|    Space)     ||  (Translation ||                |
+---------------+|   of Virtual  ||  (Contains     |
                 |   Address)    ||   1024 4-byte  |
                 +---------------+|   entries,     |
                                  |   each pointing|
                                  |   to a Page    |
                                  |   table)       |
                                  +----------------+


## Conclusion
Paging is a critical concept in modern operating systems that helps manage memory efficiently by breaking logical memory into fixed-size pages and mapping them to physical frames.

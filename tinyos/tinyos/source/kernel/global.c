/* ========================================================================= */
/* File Name : source/kernel/global.c                                        */
/* File Date : 3 February 2015                                               */
/* Author(s) : Michael Collins                                               */
/* ------------------------------------------------------------------------- */
/* global variables and structures, and general bad practise.                */
/* ========================================================================= */

/*The global.c file contains global variables and structures for memory management. 
It defines memory maps for the system and debug purposes, including low and high RAM buffers and an E820 memory map. 
This setup provides a detailed overview of the system's memory layout, crucial for the operating system to manage memory effectively.*/

#include "../tinyos.h"    // Include the header file for tinyOS definitions and functions
#include "../internal.h"  // Include the header file for internal type definitions

/* ------------------------------------------------------------------------- */
/* source/library/malloc.c                                                   */
/* ------------------------------------------------------------------------- */
MEMMAP memmap[MEMMAPSIZE] = {
    { (char*)0x00000000, 0x000A0000 },  // Memory map entry: base address 0x00000000, size 0x000A0000 (640KB)
    { (char*)0x00100000, 0x47670000 },  // Memory map entry: base address 0x00100000, size 0x47670000 (1.1GB)
};

#ifndef NDEBUG
char loram[0x000A0000];  // Low RAM buffer, size 0x000A0000 (640KB)
char hiram[0x47670000];  // High RAM buffer, size 0x47670000 (1.1GB)
E820 e820debug[] = {
    {     (__int64)loram,      sizeof(loram), 0x00000001, 0x00000001 }, /* (0x01 Memory Available To OS) */
    { 0x00000000000F0000, 0x0000000000010000, 0x00000002, 0x00000001 }, /* (0x02 Reserved, Not Available (e.g. System ROM, Memory-Mapped Devices) */
    {     (__int64)hiram,      sizeof(hiram), 0x00000001, 0x00000001 }, /* (0x01 Memory Available To OS) */
    { 0x0000000047770000, 0x0000000000002000, 0x00000004, 0x00000001 }, /* (0x04 ACPI NVS Memory OS Is Required To Save This Memory Between NVS Sessions) */
    { 0x0000000047772000, 0x0000000000021000, 0x00000003, 0x00000001 }, /* (0x03 ACPI Reclaim Memory Usable After Reading ACPI Tables) */
    { 0x0000000047793000, 0x000000000006D000, 0x00000002, 0x00000001 }, /* (0x02 Reserved, Not Available (e.g. System ROM, Memory-Mapped Devices) */
    { 0x00000000FED20000, 0x0000000000070000, 0x00000002, 0x00000001 }, /* (0x02 Reserved, Not Available (e.g. System ROM, Memory-Mapped Devices) */
    { 0x00000000FECF0000, 0x0000000000001000, 0x00000002, 0x00000001 }, /* (0x02 Reserved, Not Available (e.g. System ROM, Memory-Mapped Devices) */
    { 0x00000000FEC00000, 0x0000000000010000, 0x00000002, 0x00000001 }, /* (0x02 Reserved, Not Available (e.g. System ROM, Memory-Mapped Devices) */
    { 0x00000000FEE00000, 0x0000000000010000, 0x00000002, 0x00000001 }, /* (0x02 Reserved, Not Available (e.g. System ROM, Memory-Mapped Devices) */
    { 0x00000000FFB00000, 0x0000000000500000, 0x00000002, 0x00000001 }, /* (0x02 Reserved, Not Available (e.g. System ROM, Memory-Mapped Devices) */
    { 0x0000000000000000, 0x0000000000000000, 0x00000000, 0x00000000 }  /* End of e820 memory map */
};
#endif

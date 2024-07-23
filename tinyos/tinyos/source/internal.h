/* ========================================================================= */
/* File Name : source/internal.h                                             */
/* File Date : 3 February 2015                                               */
/* Author(s) : Michael Collins                                               */
/* ------------------------------------------------------------------------- */
/* Internal type definitions.                                                */
/* ========================================================================= */

#ifndef internal_h
#define internal_h

                          
/* ------------------------------------------------------------------------- */
/* source/library/malloc.c                                                   */
/* ------------------------------------------------------------------------- */

/* Memory Mapping Structures */

typedef struct E820 { 
    unsigned __int64 base;      /*  Base address of the memory region. */
    unsigned __int64 size;	/* Size of the memory region. */
    unsigned __int32 flag;	/* Flags indicating the type of memory */
    unsigned __int32 meta; 	/* Metadata related to the memory region. */
} E820;

#define MEMMAPSIZE (100)
typedef struct MEMMAP {
	char *addr;
	int   size;
} MEMMAP;

int memmap_init(void);   /* Function to initialize the memory map.*/  


/* ------------------------------------------------------------------------- */
/* source/devices/terminal.c                                                 */
/* ------------------------------------------------------------------------- */
char _cdecl tty_getc(void);                                                                                        /*_cdecl (C Declaration): The _cdecl (or __cdecl) calling convention is commonly used in C and C++ programs on the x86 architecture. Here are its characteristics:
															Arguments Order: Arguments are passed on the stack from right to left.
															Stack Cleanup: The caller is responsible for cleaning up the stack after the function call.
															Register Usage: The EAX, ECX, and EDX registers are considered volatile, meaning they can be freely used by the function, while other registers must be preserved.*/
void _cdecl tty_putc(char c);
int  _cdecl tty_open(char *path, char *access);
int  _cdecl tty_close(void);


/* ------------------------------------------------------------------------- */
/* source/devices/keyboard.c                                                 */
/* ------------------------------------------------------------------------- */
char _cdecl kbrd_getc(void);
void _cdecl kbrd_putc(char c);
int  _cdecl kbrd_open(char *path, char *access);
int  _cdecl kbrd_close(void);


/* ------------------------------------------------------------------------- */
/* source/devices/serial.c                                                   */
/* ------------------------------------------------------------------------- */
char _cdecl s0_getc(void);
void _cdecl s0_putc(char c);
int  _cdecl s0_open(char *path, char *access);
int  _cdecl s0_close(void);


/* ------------------------------------------------------------------------- */
/* source/kernel/init.c                                                      */
/* ------------------------------------------------------------------------- */
int init(void);


/* ------------------------------------------------------------------------- */
/* source/kernel/shell.c                                                     */
/* ------------------------------------------------------------------------- */
int shell_main(USER *user);


#endif

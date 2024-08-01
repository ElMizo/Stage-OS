/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "process.h"           // Include process management definitions
#include "kobject.h"           // Include kernel object definitions
#include "page.h"              // Include paging and memory management definitions
#include "string.h"            // Include string manipulation functions
#include "list.h"              // Include linked list definitions
#include "x86.h"               // Include x86 architecture specific definitions
#include "interrupt.h"         // Include interrupt handling definitions
#include "memorylayout.h"      // Include memory layout definitions
#include "kmalloc.h"           // Include kernel memory allocation definitions
#include "kernel/types.h"      // Include kernel type definitions
#include "kernelcore.h"        // Include core kernel functions
#include "main.h"              // Include main kernel entry definitions
#include "keyboard.h"          // Include keyboard handling definitions
#include "clock.h"             // Include clock and timing definitions

struct process *current = 0;                     // Pointer to the current process
struct list ready_list = { 0, 0 };               // List of ready processes
struct list grave_list = { 0, 0 };               // List of terminated processes
struct list grave_watcher_list = { 0, 0 };       // List of parent processes waiting for children
struct process *process_table[PROCESS_MAX_PID] = { 0 };  // Table of all processes

/*
 * Initialize the process system by creating the first process,
 * loading its page table, enabling paging, and setting its state to ready.
 */
void process_init()
{
    current = process_create();                  // Create the first process

    pagetable_load(current->pagetable);          // Load the process's page table
    pagetable_enable();                          // Enable paging

    current->state = PROCESS_STATE_READY;        // Set the process state to ready

    current->waiting_for_child_pid = 0;          // Initialize waiting_for_child_pid to 0
}

/*
 * Reset the kernel stack for a process. This function sets up the initial
 * values of the stack for a process, including the instruction pointer,
 * stack pointer, and segment registers.
 */
void process_kstack_reset(struct process *p, unsigned entry_point)
{
    struct x86_stack *s;

    p->state = PROCESS_STATE_CRADLE;             // Set the process state to cradle

    s = (struct x86_stack *) p->kstack_ptr;      // Get the stack pointer

    s->regs2.ebp = (uint32_t) (p->kstack_ptr + 28);  // Set the base pointer
    s->old_ebp = (uint32_t) (p->kstack_ptr + 32);    // Set the old base pointer
    s->old_eip = (unsigned) intr_return;             // Set the old instruction pointer
    s->fs = 0;                                      // Set fs segment
    s->gs = 0;                                      // Set gs segment
    s->es = X86_SEGMENT_USER_DATA;                  // Set es segment
    s->ds = X86_SEGMENT_USER_DATA;                  // Set ds segment
    s->cs = X86_SEGMENT_USER_CODE;                  // Set cs segment
    s->eip = entry_point;                           // Set the instruction pointer to the entry point
    s->eflags.interrupt = 1;                        // Enable interrupts
    s->eflags.iopl = 3;                             // Set I/O privilege level
    s->esp = PROCESS_STACK_INIT;                    // Initialize stack pointer
    s->ss = X86_SEGMENT_USER_DATA;                  // Set stack segment
}

/*
 * Copy the kernel stack from the parent process to the child process.
 * This function is used during process creation (fork) to ensure the
 * child process has a copy of the parent's stack.
 */
void process_kstack_copy(struct process *parent, struct process *child)
{
    child->kstack_top = child->kstack + PAGE_SIZE - 8;        // Set top of the child's kernel stack
    child->kstack_ptr = child->kstack_top - sizeof(struct x86_stack);  // Set child's stack pointer

    struct x86_stack *child_regs = (struct x86_stack *) child->kstack_ptr;  // Child's stack registers
    struct x86_stack *parent_regs = (struct x86_stack *) (parent->kstack_top - sizeof(struct x86_stack));  // Parent's stack registers

    *child_regs = *parent_regs;  // Copy parent's registers to child

    child_regs->regs2.ebp = (uint32_t) (child->kstack_ptr + 28);  // Set child's base pointer
    child_regs->old_ebp = (uint32_t) (child->kstack_ptr + 32);    // Set child's old base pointer
    child_regs->old_eip = (unsigned) intr_return;                 // Set child's old instruction pointer
    child_regs->regs1.eax = 0;                                    // Set child's eax register to 0
}

/*
 * Allocate a process ID (pid). This function finds an available pid
 * from the process table and returns it. If no pids are available,
 * it returns 0.
 */
static int process_allocate_pid()
{
    static int last = 0;  // Last allocated pid

    int i;

    for(i = last + 1; i < PROCESS_MAX_PID; i++) {  // Loop through PIDs
        if(!process_table[i]) {  // Check if the PID is available
            last = i;  // Update last
            return i;  // Return the PID
        }
    }

    for(i = 1; i < last; i++) {  // Loop through PIDs again if needed
        if(!process_table[i]) {  // Check if the PID is available
            last = i;  // Update last
            return i;  // Return the PID
        }
    }

    return 0;  // Return 0 if no PID is available
}

/*
 * Selectively inherit file descriptors from the parent process to the child process.
 * This function copies specific file descriptors from the parent to the child based
 * on the provided array of file descriptors.
 */
void process_selective_inherit(struct process *parent, struct process *child, int * fds, int length)
{
    int i;

    for (i = 0; i < length; i++) {  // Loop through file descriptors
        if(fds[i] > -1) {  // Check if the file descriptor is valid
            child->ktable[i] = kobject_copy(parent->ktable[fds[i]]);  // Copy the kernel object
        } else {
            child->ktable[i] = 0;  // Set to null if not valid
        }
    }

    child->ppid = parent->pid;  // Set child's parent PID
}

/*
 * Inherit all file descriptors from the parent process to the child process.
 * This function copies all valid file descriptors from the parent to the child.
 */
void process_inherit(struct process *parent, struct process *child)
{
    /* Child inherits everything parent inherits */
    int i;
    int * fds = kmalloc(sizeof(int) * PROCESS_MAX_OBJECTS);  // Allocate memory for file descriptors
    for (i = 0

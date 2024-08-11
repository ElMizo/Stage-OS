#ifndef PROCESS_H
#define PROCESS_H

#include "kernel/types.h"
#include "kernel/stats.h"
#include "list.h"
#include "pagetable.h"
#include "kobject.h"
#include "x86.h"
#include "fs.h"

// Process states
#define PROCESS_STATE_CRADLE  0   // Process is newly created
#define PROCESS_STATE_READY   1   // Process is ready to run
#define PROCESS_STATE_RUNNING 2   // Process is currently running
#define PROCESS_STATE_BLOCKED 3   // Process is blocked, waiting for an event
#define PROCESS_STATE_GRAVE   4   // Process has terminated

#define PROCESS_MAX_OBJECTS 32 // Maximum number of kernel objects (such as files, sockets, etc.) a process can hold.

#define PROCESS_MAX_PID 1024 //Maximum number of processes that can be created.

// Exit reasons
#define PROCESS_EXIT_NORMAL   0   // Process exited normally
#define PROCESS_EXIT_KILLED   1   // Process was killed

// Process structure
struct process {
    struct list_node node;          // Node for linking processes in lists
    int state;                      // Current state of the process
    int exitcode;                   // Exit code of the process
    int exitreason;                 // Reason for the process exit
    struct pagetable *pagetable;    // Pointer to the process's page table
    char *kstack;                   // Kernel stack base pointer
    char *kstack_top;               // Kernel stack top pointer
    char *kstack_ptr;               // Current kernel stack pointer
    struct kobject *ktable[PROCESS_MAX_OBJECTS]; // Table of kernel objects (files, sockets, etc.) used by the process
    struct process_stats stats;     // Statistics about the process (e.g., CPU time, memory usage)
    uint32_t pid;                   // Process ID
    uint32_t ppid;                  // Parent process ID
    uint32_t vm_data_size;          // Size of the process's data segment
    uint32_t vm_stack_size;         // Size of the process's stack segment
    uint32_t waiting_for_child_pid; // PID of the child process this process is waiting for
};

// Function declarations

// Initialize the process system
void process_init();

// Create a new process
struct process *process_create();

// Delete a process
void process_delete(struct process *p);

// Launch a process
void process_launch(struct process *p);

// Pass arguments to a process
void process_pass_arguments(struct process *p, int argc, char **argv);

// Inherit properties from the parent process to the child process
void process_inherit(struct process *parent, struct process *child);

// Selectively inherit file descriptors from the parent process to the child process
void process_selective_inherit(struct process *parent, struct process *child, int * fds, int fd_len);

// Reset the stack of a process
void process_stack_reset(struct process *p, unsigned size);

// Reset the kernel stack of a process
void process_kstack_reset(struct process *p, unsigned entry_point);

// Copy the kernel stack from the parent process to the child process
void process_kstack_copy(struct process *parent, struct process *child);

// Set the data size of a process
int process_data_size_set(struct process *p, unsigned size);

// Set the stack size of a process
int process_stack_size_set(struct process *p, unsigned size);

// Find an available file descriptor for a process
int process_available_fd(struct process *p);

// Get the maximum number of kernel objects for a process
int process_object_max(struct process *p);

// Yield the processor to another process
void process_yield();

// Preempt the current process
void process_preempt();

// Exit the current process
void process_exit(int code);

// Dump information about a process
void process_dump(struct process *p);

// Wait for a process to change state
void process_wait(struct list *q);

// Wake up a process
void process_wakeup(struct list *q);

// Wake up the parent process
void process_wakeup_parent(struct list *q);

// Wake up all processes in the list
void process_wakeup_all(struct list *q);

// Reap all terminated processes
void process_reap_all();

// Kill a process by PID
int process_kill(uint32_t pid);

// Wait for a child process to change state
int process_wait_child(uint32_t pid, struct process_info *info, int timeout);

// Reap a terminated child process
int process_reap(uint32_t pid);

// Get statistics about a process
int process_stats(int pid, struct process_stats *stat);

// Pointer to the currently running process
extern struct process *current;

#endif // PROCESS_H

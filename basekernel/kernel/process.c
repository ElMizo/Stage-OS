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

	child->ppid = parent->pid;
}

void process_inherit(struct process *parent, struct process *child)
{
	/* Child inherits everything parent inherits */
	int i;
	int * fds = kmalloc(sizeof(int)*PROCESS_MAX_OBJECTS);
	for (i = 0; i < PROCESS_MAX_OBJECTS; i++)
	{
		if (parent->ktable[i]) {
			fds[i] = i;
		} else {
			fds[i] = -1;
		}
	}
	process_selective_inherit(parent, child, fds, PROCESS_MAX_OBJECTS);
	kfree(fds);
}

int process_data_size_set(struct process *p, unsigned size)
{
	// XXX check valid ranges
	// XXX round up to page size

	if(size % PAGE_SIZE) {
		size += (PAGE_SIZE - size % PAGE_SIZE);
	}

	if(size > p->vm_data_size) {
		uint32_t start = PROCESS_ENTRY_POINT + p->vm_data_size;
		pagetable_alloc(p->pagetable, start, size, PAGE_FLAG_USER | PAGE_FLAG_READWRITE | PAGE_FLAG_CLEAR);
	} else if(size < p->vm_data_size) {
		uint32_t start = PROCESS_ENTRY_POINT + size;
		pagetable_free(p->pagetable, start, p->vm_data_size);
	} else {
		// requested size is equal to current.
	}

	p->vm_data_size = size;
	pagetable_refresh();

	return 0;
}

int process_stack_size_set(struct process *p, unsigned size)
{
	// XXX check valid ranges
	// XXX round up to page size

	if(size > p->vm_stack_size) {
		uint32_t start = -size;
		pagetable_alloc(p->pagetable, start, size - p->vm_stack_size, PAGE_FLAG_USER | PAGE_FLAG_READWRITE | PAGE_FLAG_CLEAR);
	} else {
		uint32_t start = -p->vm_stack_size;
		pagetable_free(p->pagetable, start, p->vm_stack_size - size);
	}

	p->vm_stack_size = size;
	pagetable_refresh();

	return 0;
}

void process_stack_reset(struct process *p, unsigned size)
{
	process_stack_size_set(p, size);
	memset((void *) -size, size, 0);
}

struct process *process_create()
{
	struct process *p;

	p = page_alloc(1);

	p->pid = process_allocate_pid();
	process_table[p->pid] = p;

	p->pagetable = pagetable_create();
	pagetable_init(p->pagetable);

	p->vm_data_size = 0;
	p->vm_stack_size = 0;

	process_data_size_set(p, 2 * PAGE_SIZE);
	process_stack_size_set(p, 2 * PAGE_SIZE);

	p->kstack = page_alloc(1);
	p->kstack_top = p->kstack + PAGE_SIZE - 8;
	p->kstack_ptr = p->kstack_top - sizeof(struct x86_stack);

	process_kstack_reset(p, PROCESS_ENTRY_POINT);

	// XXX table should be allocated
	int i;
	for(i = 0; i < PROCESS_MAX_OBJECTS; i++) {
		p->ktable[i] = 0;
	}

	p->state = PROCESS_STATE_RUNNING;
	printf("%d\n", p->pid);

	return p;
}

void process_delete(struct process *p)
{
	int i;
	for(i = 0; i < PROCESS_MAX_OBJECTS; i++) {
		if(p->ktable[i]) {
			kobject_close(p->ktable[i]);
		}
	}
	pagetable_delete(p->pagetable);
	page_free(p->kstack);
	page_free(p);
	process_table[p->pid] = 0;
}

void process_launch(struct process *p)
{
	list_push_tail(&ready_list, &p->node);
}

static void process_switch(int newstate)
{
	interrupt_block();

	if(current) {
		if(current->state != PROCESS_STATE_CRADLE) {
			asm("pushl %ebp");
			asm("pushl %edi");
			asm("pushl %esi");
			asm("pushl %edx");
			asm("pushl %ecx");
			asm("pushl %ebx");
			asm("pushl %eax");
		      asm("movl %%esp, %0":"=r"(current->kstack_ptr));
		}

		interrupt_stack_pointer = (void *) INTERRUPT_STACK_TOP;
		current->state = newstate;

		if(newstate == PROCESS_STATE_READY) {
			list_push_tail(&ready_list, &current->node);
		}
		if(newstate == PROCESS_STATE_GRAVE) {
			list_push_tail(&grave_list, &current->node);
		}
	}

	current = 0;

	while(1) {
		current = (struct process *) list_pop_head(&ready_list);
		if(current)
			break;

		interrupt_unblock();
		interrupt_wait();
		interrupt_block();
	}

	current->state = PROCESS_STATE_RUNNING;
	interrupt_stack_pointer = current->kstack_top;

	asm("movl %0, %%cr3"::"r"(current->pagetable));
	asm("movl %0, %%esp"::"r"(current->kstack_ptr));

	asm("popl %eax");
	asm("popl %ebx");
	asm("popl %ecx");
	asm("popl %edx");
	asm("popl %esi");
	asm("popl %edi");
	asm("popl %ebp");

	interrupt_unblock();
}

int allow_preempt = 1;

void process_preempt()
{
	if(allow_preempt && current && ready_list.head) {
		process_switch(PROCESS_STATE_READY);
	}
}

void process_yield()
{
	/* no-op if process module not yet initialized. */
	if(!current) return;
	process_switch(PROCESS_STATE_READY);
}

void process_exit(int code)
{

  printf("process %d exiting with status %d...\n", current->pid, code); //--> transport to kshell run

	current->exitcode = code;
	current->exitreason = PROCESS_EXIT_NORMAL;
	process_wakeup_parent(&grave_watcher_list);	// On exit, wake up parent if need be
	process_switch(PROCESS_STATE_GRAVE);
}

void process_wait(struct list *q)
{
	list_push_tail(q, &current->node);
	process_switch(PROCESS_STATE_BLOCKED);
}

void active_proc(){  //added by anas
	for (int i=0; i<PROCESS_MAX_PID; i++){
		if(process_table[i]){
			printf("%d\n", process_table[i]->pid);
		}
	}
}



void process_wakeup(struct list *q)
{
	struct process *p;
	p = (struct process *) list_pop_head(q);
	if(p) {
		p->state = PROCESS_STATE_READY;
		list_push_tail(&ready_list, &p->node);
	}
}

void process_reap_all()
{
	struct process *p;
	while((p = (struct process *) list_pop_head(&grave_list))) {
		process_delete(p);
	}
}

/* Wakes up parent off of the corresponding list*/
void process_wakeup_parent(struct list *q)
{
	struct process *p = (struct process *) q->head;
	// Loop through all the waiting parents to see if one needs to be woken up
	while(p) {
		if(p->pid == current->ppid && (p->waiting_for_child_pid == 0 || p->waiting_for_child_pid == current->pid)) {
			p->state = PROCESS_STATE_READY;
			p->waiting_for_child_pid = 0;
			list_remove(&p->node);
			list_push_tail(&ready_list, &p->node);
			break;
		}
		p = (struct process *) (&p->node)->next;
	}
}

void process_wakeup_all(struct list *q)
{
	struct process *p;
	while((p = (struct process *) list_pop_head(q))) {
		p->state = PROCESS_STATE_READY;
		list_push_tail(&ready_list, &p->node);
	}
}

void process_dump(struct process *p)
{
	struct x86_stack *s = (struct x86_stack *) (INTERRUPT_STACK_TOP - sizeof(*s));
	printf("kstack: %x\n", p->kstack);
	printf("stackp: %x\n", p->kstack_ptr);
	printf("eax: %x     cs: %x\n", s->regs1.eax, s->cs);
	printf("ebx: %x     ds: %x\n", s->regs1.ebx, s->ds);
	printf("ecx: %x     ss: %x\n", s->regs1.ecx, s->ss);
	printf("edx: %x eflags: %x\n", s->regs1.edx, s->eflags);
	printf("esi: %x\n", s->regs1.esi);
	printf("edi: %x\n", s->regs1.edi);
	printf("ebp: %x\n", s->regs1.ebp);
	printf("esp: %x\n", s->esp);
	printf("eip: %x\n", s->eip);
}

int process_available_fd(struct process *p)
{
	struct kobject **fdtable = current->ktable;
	int i;
	for(i = 0; i < PROCESS_MAX_OBJECTS; i++) {
		if(fdtable[i] == 0)
			return i;
	}
	return -1;
}

int process_object_max(struct process *p)
{
	struct kobject **fdtable = current->ktable;
	int i;
	// Because of 0-indexing, PROCESS_MAX_OBJECTS is the size and
	// therefor 1 offset, don't look for 0 there.
	for(i = PROCESS_MAX_OBJECTS - 1; i > -1; i--) {
		if(fdtable[i] != 0)
			return i;
	}
	return -1;
}

void process_make_dead(struct process *dead)
{
	int i;
	for(i = 0; i < PROCESS_MAX_PID; i++) {
		if(process_table[i] && process_table[i]->ppid == dead->pid) {
			process_make_dead(process_table[i]);
		}
	}
	dead->exitcode = 0;
	dead->exitreason = PROCESS_EXIT_KILLED;
	if(dead == current) {
		process_switch(PROCESS_STATE_GRAVE);
	} else {
		list_remove(&dead->node);
		list_push_tail(&grave_list, &dead->node);
	}
}

int process_kill(uint32_t pid)
{
	if(pid > 0 && pid <= PROCESS_MAX_PID) {
		struct process *dead = process_table[pid];
		if(dead) {
			printf("process killed\n");
			process_make_dead(dead);
			return 0;
		} else {
			return 1;
		}
	} else {
		return 1;
	}
}

int process_wait_child(uint32_t pid, struct process_info *info, int timeout)
{
	clock_t start, elapsed;
	uint32_t total;

	if(!info)
		return -1;

	start = clock_read();

	do {
		struct process *p = (struct process *) (grave_list.head);
		while(p) {
			struct process *next = (struct process *) p->node.next;
			if((pid != 0 && p->pid == pid) || (p->ppid == current->pid)) {
				info->exitcode = p->exitcode;
				info->exitreason = p->exitreason;
				info->pid = p->pid;
				return p->pid;
			}
			p = next;
		}

		current->waiting_for_child_pid = pid;
		process_wait(&grave_watcher_list);

		elapsed = clock_diff(start, clock_read());
		total = elapsed.millis + elapsed.seconds * 1000;
	} while(total < timeout || timeout < 0);

	return 0;
}

int process_reap(uint32_t pid)
{
	struct process *p = (struct process *) (grave_list.head);
	while(p) {
		struct process *next = (struct process *) p->node.next;
		if(p->pid == pid) {
			list_remove(&p->node);
			process_delete(p);
			return 0;
		}
		p = next;
	}
	return 1;
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

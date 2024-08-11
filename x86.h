#ifndef X86_H
#define X86_H

#include "kernel/types.h"

/* Define the x86 segment selectors used in the segment descriptor */
#define X86_SEGMENT_SELECTOR( seg, rpl )  (((seg)<<3)+(rpl))

#define X86_SEGMENT_KERNEL_CODE  X86_SEGMENT_SELECTOR(1,0)  // Kernel code segment selector
#define X86_SEGMENT_KERNEL_DATA  X86_SEGMENT_SELECTOR(2,0)  // Kernel data segment selector
#define X86_SEGMENT_USER_CODE    X86_SEGMENT_SELECTOR(3,3)  // User code segment selector
#define X86_SEGMENT_USER_DATA    X86_SEGMENT_SELECTOR(4,3)  // User data segment selector
#define X86_SEGMENT_TSS          X86_SEGMENT_SELECTOR(5,0)  // Task State Segment (TSS) selector

/* Structure representing the EFLAGS register in x86 architecture */
struct x86_eflags {
	unsigned carry:1;        // Carry flag
	unsigned reserved0:1;    // Reserved bit
	unsigned parity:1;       // Parity flag
	unsigned reserved1:1;    // Reserved bit

	unsigned auxcarry:1;     // Auxiliary carry flag (BCD operations)
	unsigned reserved2:1;    // Reserved bit
	unsigned zero:1;         // Zero flag
	unsigned sign:1;         // Sign flag

	unsigned trap:1;         // Trap flag (single-step mode)
	unsigned interrupt:1;    // Interrupt flag (interrupts enabled)
	unsigned direction:1;    // Direction flag (string operations)
	unsigned overflow:1;     // Overflow flag

	unsigned iopl:2;         // I/O Privilege Level
	unsigned nested:1;       // Reserved bit
	unsigned reserved3:1;    // Reserved bit

	unsigned resume:1;       // Reserved bit
	unsigned v86:1;          // Virtual 8086 mode flag
	unsigned align:1;        // Alignment check flag
	unsigned vinterrupt:1;   // Virtual interrupt flag

	unsigned vpending:1;     // Virtual interrupt pending
	unsigned id:1;           // ID flag (CPUID instruction supported)
};

/* Structure representing general-purpose registers in x86 architecture */
struct x86_regs {
	int32_t eax;  // Accumulator register
	int32_t ebx;  // Base register
	int32_t ecx;  // Count register
	int32_t edx;  // Data register
	int32_t esi;  // Source index register
	int32_t edi;  // Destination index register
	int32_t ebp;  // Base pointer register
};

/* Structure representing the state of the stack in x86 architecture */
struct x86_stack {
	struct x86_regs regs2;   // Saved general-purpose registers (during context switch)
	int32_t old_ebp;         // Previous base pointer (saved during context switch)
	int32_t old_eip;         // Previous instruction pointer (saved during context switch)
	struct x86_regs regs1;   // Registers saved by interrupt handler
	int32_t gs;              // GS segment register
	int32_t fs;              // FS segment register
	int32_t es;              // ES segment register
	int32_t ds;              // DS segment register
	int32_t intr_num;        // Interrupt number (saved by interrupt handler)
	int32_t intr_code;       // Interrupt error code (saved by interrupt handler)
	int32_t eip;             // Instruction pointer (saved by x86 hardware)
	int32_t cs;              // Code segment register (saved by x86 hardware)
	struct x86_eflags eflags;// Flags register (saved by x86 hardware)
	int32_t esp;             // Stack pointer (saved by x86 hardware)
	int32_t ss;              // Stack segment register (saved by x86 hardware)
};

/* Structure representing a segment descriptor in the Global Descriptor Table (GDT) */
struct x86_segment {
	uint16_t limit0;    // Lower 16 bits of the segment limit
	uint16_t base0;     // Lower 16 bits of the base address
	uint8_t base1;      // Middle 8 bits of the base address

	unsigned type:4;   // Segment type
	unsigned stype:1;  // Descriptor type
	unsigned dpl:2;    // Descriptor Privilege Level
	unsigned present:1;// Segment present

	unsigned limit1:4; // Upper 4 bits of the segment limit
	unsigned avail:1; // Available for system use
	unsigned zero:1;  // Reserved bit
	unsigned size:1;  // Segment size (32-bit or 16-bit)
	unsigned granularity:1; // Granularity of segment limit

	uint8_t base2;     // Upper 8 bits of the base address
};

/* Structure representing the Task State Segment (TSS) in x86 architecture */
struct x86_tss {
	int16_t prev;           // Previous TSS selector
	int16_t reserved;       // Reserved (16 bits)
	int32_t esp0;           // Stack pointer for ring 0
	int16_t ss0;            // Stack segment selector for ring 0
	int16_t reserved0;      // Reserved (16 bits)
	int32_t esp1;           // Stack pointer for ring 1
	int16_t ss1;            // Stack segment selector for ring 1
	int16_t reserved1;      // Reserved (16 bits)
	int32_t esp2;           // Stack pointer for ring 2
	int16_t ss2;            // Stack segment selector for ring 2
	int16_t reserved2;      // Reserved (16 bits)
	int32_t cr3;            // Page Directory Base Register
	int32_t eip;            // Instruction pointer
	int32_t eflags;        // Flags register
	int32_t eax;            // Accumulator register
	int32_t ecx;            // Count register
	int32_t edx;            // Data register
	int32_t ebx;            // Base register
	int32_t esp;            // Stack pointer
	int32_t ebp;            // Base pointer
	int32_t esi;            // Source index
	int32_t edi;            // Destination index
	int16_t es;             // Data segment selector
	int16_t reserved3;      // Reserved (16 bits)
	int16_t cs;             // Code segment selector
	int16_t reserved4;      // Reserved (16 bits)
	int16_t ss;             // Stack segment selector
	int16_t reserved5;      // Reserved (16 bits)
	int16_t ds;             // Data segment selector
	int16_t reserved6;      // Reserved (16 bits)
	int16_t fs;             // FS segment selector
	int16_t reserved7;      // Reserved (16 bits)
	int16_t gs;             // GS segment selector
	int16_t reserved8;      // Reserved (16 bits)
	int16_t ldt;            // Local Descriptor Table (LDT) selector
	int16_t reserved9;      // Reserved (16 bits)
	int16_t t;              // Task priority (not widely used)
	int16_t iomap;         // I/O map base address (16 bits)
};

/* Structure used to initialize the Global Descriptor Table (GDT) */
struct x86_gdt_init {
	int16_t size;            // Size of the GDT in bytes
	struct x86_segment *base; // Pointer to the base address of the GDT
};

#endif
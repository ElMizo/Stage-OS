.code16
.section .text
.global _start
_start:
    ljmp $0x07c0, $_start2

_start2:
    sti                     # Enable interrupts
    cld                     # Clear the direction flag
    movw %cs, %ax           # Set all segments to code
    movw %ax, %ds
    movw %ax, %es
    movw $0x0000, %ax       # Set up the stack
    movw %ax, %ss
    movw $0xfff0, %sp

    movb %dl, disk_number   # Save the disk number
    movw $partition_status, %di # Set the partition table as dest
    movw $12, %cx           # Copy 12 bytes from si to di
    rep movsb

    movw $loadmsg, %si      # Print initial message
    call bios_putstring

    movb $0x00, %ah         # Set video mode
    movb $0x03, %al         # 80x25 text mode
    int $0x10


printingcredits:
    movw $credits_inpt, %si   # Print credits
    call bios_putstring

done:
    movb $0x00, %ah         # BIOS function for waiting for a key press
    int $0x16               # BIOS interrupt for keyboard service
    cmpb $0x1C, %ah        # Compare key scan code with 0x1C (Enter key)
    jne done                # If not Enter key, repeat waiting

    xorb %ah, %ah           # Reset the disk system
    int $0x13

    movb $0x08, %ah         # Get the drive geometry
    int $0x13
    andb $0x3f, %cl         # Mask off high tracks
    movb %cl, disk_sectors
    movb %ch, disk_cylinders
    movb %dh, disk_heads

    movw $0x1000, %ax       # Load happens at es:bx
    movw %ax, %es           # Which we set to
    movw $0x0000, %bx       # KERNEL_SEGMENT:KERNEL_OFFSET

    # Disk parameters:
    movb disk_number, %dl   # Device
    xorb %ch, %ch           # Cylinder 0
    xorb %dh, %dh           # Head 0
    movb $2, %cl            # Sector 2

loadsector:
    movb $1, %al            # Load 1 sector
    movb $0x02, %ah         # Load command
    int $0x13               # Execute load

    movb $'.', %al          # Display a dot
    call bios_putchar       # For each sector loaded

    movw sectors_left, %ax  # How many sectors left?
    cmpw $0xffff, %ax       # Has it been initialized?
    jne gotsectors          # Yes - use the value
    movl %es:20, %eax       # No - get size of kernel
    shrl $9, %eax           # Convert into blocks
    incl %eax               # Add one for good measure

gotsectors:
    decw %ax                # Remove one block
    movw %ax, sectors_left  # Store the value
    cmpw $0, %ax            # Are we done?
    je loaddone             # Yes - jump to bottom

checksegment:
    addw $512, %bx          # Move data pointer by 512 bytes
    cmpw $0, %bx            # Did we reach segment end?
    jnz nextsector          # No - find next sector
    movw %es, %ax           # Yes - retrieve seg register
    addw $0x1000, %ax       # Move to next 64k block
    movw %ax, %es           # Store segment register

nextsector:
    incb %cl                # Advance by one sector
    movb disk_sectors, %al  # What is the maximum sector?
    cmpb %al, %cl           # Is this the last sector?
    jle loadsector          # No - load the next sector
    movb $1, %cl            # Yes - go to sector zero..

    incb %dh                # Advance to next head
    movb disk_heads, %al    # What is the maximum head?
    cmpb %al, %dh           # Is this the last head?
    jle loadsector          # No - read the next sector
    xorb %dh, %dh           # Yes - go to head zero

    incb %ch                # Advance to next cylinder
    movb disk_cylinders, %al# What is the maximum cylinder?
    cmpb %al, %ch           # Is this the last cylinder?
    jle loadsector          # No - read the next sector
                            # Yes - fall through here

loaddone:
    xorb %ah, %ah           # Reset the disk system
    int $0x13

    movw $bootmsg, %si      # Print boot message
    call bios_putstring

    movw $0x1000, %ax       # Jump to the kernel code
    movw %ax, %ds
    ljmp $0x1000, $0x0000

bios_putstring:              # Routine to print an entire string
    movb (%si), %al
    cmpb $0, %al
    jz bios_putstring_done
    call bios_putchar
    incw %si
    jmp bios_putstring

	
bios_putstring_done:
    ret

bios_putchar:                # Routine to print a single char
    pushw %ax
    pushw %bx
    movb $14, %ah
    movb $1, %bl
    int $0x10
    popw %bx
    popw %ax
    ret

loadmsg:
    .asciz "bootblock: loading kernel...\r\n"
bootmsg:
    .asciz "\r\nbootblock: booting kernel...\r\n"

disk_number:
    .byte 0
disk_cylinders:
    .byte 0
disk_heads:
    .byte 0
disk_sectors:
    .byte 0
sectors_left:
    .word 0xffff

partition_status:
    .byte 0
partition_start_chs:
    .byte 0, 0, 0
partition_type:
    .byte 0
partition_stop_chs:
    .byte 0, 0, 0
partition_start_lba:
    .long 0
partition_length:
    .long 0
credits_inpt:
    .asciz "\nThis work was done by : \n\r   \t-Hamza Aarab\n\r   \t-Hassan Labyad\n\r   \t-Khalid ElKoussami\n\r   \t-Abdelaali Chattaoui\n\r   \t-Anas Azouane"

.org 510
bootflag:
    .word 0xaa55


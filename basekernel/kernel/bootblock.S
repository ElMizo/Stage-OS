<<<<<<< HEAD
# Copyright (C) 2015 The University of Notre Dame
# This software is distributed under the GNU General Public License.
# See the file LICENSE for details.

# This is the raw bootblock code, a 512-byte chunk of assembly
# found on the first sector of the boot disk.  The BIOS is responsible
# for starting the machine, loading this sector into memory,
# and then transferring control here.  The bootblock must call
# to the BIOS to load the remaining sectors containing the
# kernel code, and then jump there.

# Constants describing our basic memory layout are in this
# header file, which is shared between C and assembly modules:
	
#include "memorylayout.h"
	
# When we receive control from the BIOS, the following are set:
# %dl - the device number this block was loaded from
# %es:%si - the partition table entry we were loaded from

# To set the code segment appropriately, the first thing we
# do is a long jump to _start2, which sets cs=BOOTBLOCK_SEGMENT
	
.code16
.text
.global _start
_start:
	ljmp	$BOOTBLOCK_SEGMENT,$_start2

# Now we begin setting up the execution environment
# for loading the rest of the kernel.

_start2:	
	sti				# disable interrupts
	cld				# clear the direction flag
	mov	%cs, %ax		# set all segments to code
	mov	%ax, %ds
	mov	%ax, %es
	mov	$INTERRUPT_STACK_SEGMENT, %ax     # set up the stack
	mov	%ax, %ss
	mov	$INTERRUPT_STACK_OFFSET, %sp

	mov	%dl, (disk_number)	# save the disk number
	mov	partition_status,%di	# set the partition table as dest
	mov	$12, %cx		# copy 12 bytes from si to di
	rep	movsb
		
	mov	$(loadmsg),%si		# print initial message
        call	bios_putstring

	mov	$0,%ah			# reset the disk system
	int	$0x13

	mov	$0x08, %ah		# get the drive geometry
	int	$0x13
	and	$0x3f, %cl		# mask off high tracks
	mov	%cl, (disk_sectors)
	mov	%ch, (disk_cylinders)
	mov	%dh, (disk_heads)
	
	mov	$KERNEL_SEGMENT,%ax	# load happens at es:bx
	mov	%ax, %es		# which we set to
	mov	$KERNEL_OFFSET,%bx	# KERNEL_SEGMENT:KERNEL_OFFSET

					# disk parameters:
	mov	(disk_number), %dl	#	device
	mov	$0,%ch			#	cylinder 0
	mov	$0,%dh			#	head 0
	mov	$2,%cl			#	sector 2

loadsector:
	mov	$1,%al			# load 1 sector
	mov	$0x02, %ah		# load command
	int	$0x13			# execute load
	
	mov	$'.', %al		# display a dot
	call	bios_putchar		# for each sector loaded
	
	mov	(sectors_left),%ax	# how many sectors left?
	cmp	$0xffff, %ax		# has it been initialized?
	jne	gotsectors		# yes - use the value
	mov	%es:(KERNEL_SIZE_OFFSET),%eax	# no - get size of kernel
	shr	$9, %eax		# convert into blocks
	inc	%eax			# add one for good measure
	
gotsectors:		
	dec	%ax			# remove one block
	mov	%ax,(sectors_left)	# store the value
	cmp	$0, %ax			# are we done?
	je	loaddone		# yes - jump to bottom
			
checksegment:
	add	$512,%bx		# move data pointer by 512 bytes
	cmp	$0, %bx			# did we reach segment end?
	jnz	nextsector		# no - find next sector
	mov	%es, %ax		# yes - retrieve seg register
	add	$0x1000, %ax		# move to next 64k block
	mov	%ax, %es		# store segment register

nextsector:
	inc	%cl			# advance by one sector
	mov	(disk_sectors),%al	# what is the maximum sector?
	cmp	%al, %cl		# is this the last sector?
	jle	loadsector		# no - load the next sector
	mov	$1,%cl			# yes - go to sector zero..

	inc	%dh			# advance to next head
	mov	(disk_heads), %al	# what is the maximum head?
	cmp	%al, %dh		# is this the last head?
	jle	loadsector		# no - read the next sector
	mov	$0,%dh			# yes - go to head zero

	inc	%ch			# advance to next cylinder
	mov	(disk_cylinders), %al	# what is the maximum cylinder?
	cmp	%al, %ch		# is this the last cylinder?
	jle	loadsector		# no - read the next sector
					# yes - fall through here
loaddone:
	mov	$0,%ah			# reset the disk system
	int	$0x13
	
	mov	$(bootmsg),%si		# print boot message
        call	bios_putstring
	
	mov	$KERNEL_SEGMENT, %ax    # jump to the kernel code
	mov	%ax, %ds
	ljmp	$KERNEL_SEGMENT, $KERNEL_OFFSET
	
bios_putstring:				# routine to print an entire string
	mov	(%si), %al
	cmp	$0, %al
	jz	bios_putstring_done
        call	bios_putchar
	inc	%si
	jmp	bios_putstring
bios_putstring_done:
        ret

bios_putchar:				# routine to print a single char
	push	%ax
	push	%bx
        mov	$14,%ah
        mov	$1,%bl
        int	$0x10
	pop	%bx
	pop	%ax
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
	.byte 0
	.byte 0
	.byte 0
partition_type:	
	.byte 0
partition_stop_chs:	
	.byte 0
	.byte 0
	.byte 0
partition_start_lba:
	.long 0
partition_length:
	.long 0
				
# A bootblock must have 0xaa55 in its two final bytes.
# The .org directive forces this data to that point.

.org 510
bootflag:
	.word 0xaa55
=======
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
    movw $credits, %si   # Print credits
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
credits:
    .asciz "this work was done by Hamza Aarab, Hassan Labyad, Khalid Elkoussami, Abdelaali Chattaoui, Anas Azouane"

.org 510
bootflag:
    .word 0xaa55

>>>>>>> origin/anas_branch

[org 0x7c00]
KERNEL_LOCATION equ 0x1000
credits:
        db "this work was done by Hamza Aarab, Khalid Elkoussami, Hassan Labyad, Abdelali Chattaoui, Anas Azouane", 0


mov [BOOT_DISK], dl  ;dl conventionally contains the disk number during booting


mov ax, 0
mov es, ax
mov ds, ax
mov bp, 0x8000
mov sp, bp  ;setting up the stack

mov dh, 2

mov bx, KERNEL_LOCATION
mov ah, 0x02
mov al, dh
mov ch, 0x00
mov dh, 0x00
mov cl, 0x02
mov dl, [BOOT_DISK]
int 0x13  ;bios interrupt to read from the disk and load into memoryy.
jc error_reading


mov ah, 0x0
mov al, 0x3
int 0x10  ; text mode

mov si, credits

printing_credits:
        lodsb
        cmp al, 0
        je done
        mov ah, 0x0E
        mov bh, 0x00
        mov bl, 0x07
        int 0x10
        jmp printing_credits
done:
        mov ah, 0x00  ; bios function for waiting for key press
        int 0x16  ;bios interrupt for keyboard service
        cmp ah, 0x1C  ;waiting for enter key press
        jne done

mov ah, 0x0
mov al, 0x3
int 0x10  ;clears the screen

CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

cli
lgdt [GDT_descriptor]
mov eax, cr0
or eax, 1
mov cr0, eax
jmp CODE_SEG:start_protected_mode

jmp $

BOOT_DISK: db 0

GDT_start:
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10011010
        db 0b11001111
        db 0x0

    GDT_data:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start


[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
        mov ds, ax
        mov ss, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov ebp, 0x90000   ; 32 bit stack base pointer
        mov esp, ebp

    jmp KERNEL_LOCATION
error_reading:
        mov ah, 0x0E
        push ax
        mov al, 'e'
        int 0x10
        pop ax
        mov al, 'r'
        int 0x10
        pop ax
        mov al, 'r'
        int 0x10
        pop ax
        mov al, 'r'
        int 0x10
        pop ax
        mov al, 'o'
        int 0x10
        pop ax
        mov al, 'r'
        int 0x10
        pop ax


times 510-($-$$) db 0
dw 0xaa55

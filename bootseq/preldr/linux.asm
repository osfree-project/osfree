;
; linux.asm
; linux multiboot kernel startup
;

name linux

.386

include linux.inc
include mb_info.inc
include mb_header.inc
include mb_etc.inc

extrn call_rm         :near
extrn cmain           :near
extrn exe_end         :near
extrn bss_end         :near
extrn l               :dword
extrn m               :dword

public linux_text_len
public linux_data_tmp_addr
public linux_data_real_addr

_TEXT16  segment dword public 'CODE'  use16
_TEXT16  ends
_TEXT    segment dword public 'CODE'  use32
_TEXT    ends
_DATA    segment dword public 'DATA'  use32
_DATA    ends
CONST    segment dword public 'DATA'  use32
CONST    ends
CONST2   segment dword public 'DATA'  use32
CONST2   ends
_end1    segment dword public 'DATA'  use32
_end1    ends
_end2    segment dword public 'DATA'  use32
_end2    ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends
_end3    segment dword public 'BSS'   use32
_end3    ends
_end4    segment dword public 'BSS'   use32
_end4    ends
_STACK   segment dword public 'STACK' use32
_STACK   ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_end1,_end2,_BSS,_end3,_end4,_STACK

_STACK   segment dword public 'STACK' use32
STACK_SIZE equ 4000h
public stack_top
public stack_bottom
stack_bottom     db STACK_SIZE dup (?)
stack_top        label byte
_STACK   ends

_TEXT16  segment dword public 'CODE' use16
begin:

        org 0h
start:

start_linux:
        ; final setup for linux boot
        cli
        mov     ss, bx
        mov     sp, LINUX_SETUP_STACK

        mov     ds, bx
        mov     es, bx
        mov     fs, bx
        mov     gs, bx

                ; jump to start
                ; ljmp
                db      0eah
                dw      0
linux_setup_seg dw      0

        ;
        ;  This next part is sort of evil.  It takes advantage of the
        ;  byte ordering on the x86 to work in either 16-bit or 32-bit
        ;  mode, so think about it before changing it.
        ;

hard_stop:
        cli
        hlt

stop_flop:
        xor     dl, dl
        int     13h
        retf

pad1size        equ STARTUP_SIZE - ($ - start)
pad1            db  pad1size dup (0)

_TEXT16  ends


_TEXT    segment dword public 'CODE'  use32

BASE1           equ     KERN_BASE - 0x10000
VIDEO_BUF       equ     0xb8000

        org     BASE1
start1:

        org     BASE1 + _TEXT16_SIZE

start2:

align 4

; Multiboot structure
_magic          equ       0x1badb002
_flags          equ       0x00010001
mbhdr           multiboot_header  <_magic,_flags,-_magic-_flags,mbhdr,start1,exe_end,bss_end,entry,0,0,0,0>

;pad2size        equ  0x80 - ($ - start2)
;pad2            db   pad2size dup (0)

        org     start2 + 0x80
entry:
        cmp   eax, MULTIBOOT_VALID
        jne   stop

        ; save multiboot structure address
        mov   m, ebx

        ; save lip2 pointer from ECX
        mov   l, ecx        

        call  cmain

        ; We should not return here
        cli
        hlt
        jmp   $

stop:
        cld
        lea   esi, errmsg
        mov   edi, VIDEO_BUF
        mov   ecx, msglen
        mov   ah, 02h  ; attribute
loop1:
        lodsb          ; symbol
        stosw
        loop  loop1    ; copy a string to video buffer

        cli
        hlt
        jmp   $


;
;  linux_boot()
;
;  Does some funky things (including on the stack!), then jumps to the
;  entry point of the Linux setup code.
;
linux_boot:
        ; don't worry about saving anything, we're committed at this point
        cld     ; forward copying

        ; copy kernel
        mov     ecx, linux_text_len
        add     ecx, 3
        shr     ecx, 2
        mov     esi, LINUX_BZIMAGE_ADDR
        mov     edi, LINUX_ZIMAGE_ADDR

        rep     movsd

big_linux_boot:
        mov     ebx, linux_data_real_addr

        ; copy the real mode part
        mov     esi, linux_data_tmp_addr
        mov     edi, ebx
        mov     ecx, LINUX_SETUP_MOVE_SIZE
        cld
        rep     movsb

        ; change ebx to the segment address
        shr     ebx, 4
        mov     eax, ebx
        add     eax, 20h
        mov     dword ptr linux_setup_seg, eax

        ; XXX new stack pointer in safe area for calling functions
        mov     esp, 4000h
        call    stop_floppy

        ; final setup for linux boot

        mov     eax, KERN_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:start_linux
        push    eax
        call    call_rm
        add     esp, 4

        ; we should not return here
        cli
        hlt

;
;  stop_floppy()
;
;  Stops the floppy drive from spinning, so that other software is
;  jumped to with a known state.
;
stop_floppy:
        pusha
        mov     eax, KERN_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:stop_flop
        push    eax
        call    call_rm
        add     esp, 4
        popa
        ret


linux_text_len        dd  0
linux_data_tmp_addr   dd  0
linux_data_real_addr  dd  0

errmsg  db  "No multiboot magic in EAX, panic!",13,10,0
msglen  equ errmsg - $    

_TEXT    ends

         end entry

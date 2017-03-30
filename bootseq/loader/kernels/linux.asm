;
; linux.asm
; linux multiboot kernel startup
;

name linux

.386p

include fsd.inc
include linux.inc
include struc.inc
include mb_info.inc
include mb_header.inc
include mb_etc.inc

extrn call_rm         :near
extrn cmain           :near
extrn exe_end         :near
extrn bss_end         :near
extrn gdtsrc          :byte
extrn gdtdesc         :fword
extrn l               :dword
extrn m               :dword

public base

public linux_text_len
public linux_data_tmp_addr
public linux_data_real_addr
public big_linux_boot
public linux_boot
public start
public stop

public outstr_

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

        org     0h
start:
        jmp     real_start
        org     10h
base    dd      REAL_BASE
        org     20h
real_start:
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

;pad1size        equ STARTUP_SIZE - ($ - start)
;pad1            db  pad1size dup (0)

_TEXT16  ends


_TEXT    segment dword public 'CODE'  use32

BASE1           equ     KERN_BASE - 0x10000
VIDEO_BUF       equ     0xb8000

        org     BASE1
start1:

        org     BASE1 + _TEXT16_SIZE

start2:

align 4

; Multiboot header
_magic          equ       0x1badb002
_flags          equ       0x00010001
_checksum       equ       - _magic - _flags
_mbhdr          multiboot_header  <_magic,_flags,_checksum,_mbhdr,start1,exe_end,bss_end,entry,0,0,0,0>

        org     start2 + 0x80

entry:
        cmp   eax, MULTIBOOT_VALID
        jne   stop

        ; save multiboot structure address
        mov   m, ebx

        ; set up stack
        cli
        mov   esp, 8000h - 4
        mov   ebp, esp
        ;sti

        call  set_gdt

        ; copy realmode part of boot_linux at REAL_BASE
        cld
        mov     ecx, 0x100
        mov     esi, KERN_BASE
        mov     edi, REAL_BASE

        rep     movsd

        call    cmain

        ; We should not return here
        cli
        hlt
;        jmp     $

stop:
        lea   eax, errmsg
        call  outstr_

        cli
        hlt
        jmp   $

outstr_:
        mov   esi, eax
        cld
        mov   edi, screenpos
        ;mov   edi, VIDEO_BUF
        mov   ah, 02h  ; attribute
loop1:
        lodsb          ; symbol

        cmp  al, 0ah
        jne  write

        mov  eax, edi
        sub  eax, VIDEO_BUF
        mov  ebx, 80 * 2
        div  bx
        and  eax, 0ffffh      ; get remainder
        sub  edi, eax
        add  edi, ebx

        mov  ah, 02h
        lodsb
write:
        stosw
        test  al, al   ; copy a string to video buffer
        jnz   loop1

        mov   dword ptr ds:screenpos, edi

        ret

set_gdt:
        ; set 16-bit segment (_TEXT16) base
        ; in GDT for protected mode
        ;lea  eax, oldgdtdesc
        ;sgdt fword ptr [eax]

        ; copy old gdt
        ;movzx ecx, [eax].g_limit
        ;inc  ecx
        ;shr  ecx, 2
        ;mov  esi, [eax].g_base
        ;mov  edi, offset _TEXT:gdt
        ;mov  ebx, edi
        ;rep  movsd

        ; fix gdt descriptors base
        ;mov  ebx, GDT_ADDR
        mov  ebx, offset _TEXT:gdtsrc
        mov  eax, REAL_BASE
        mov  [ebx][8*8].ds_baselo, ax
        mov  [ebx][9*8].ds_baselo, ax
        ror  eax, 16
        mov  [ebx][8*8].ds_basehi1, al
        mov  [ebx][9*8].ds_basehi1, al
        ror  eax, 8
        mov  [ebx][8*8].ds_basehi2, al
        mov  [ebx][9*8].ds_basehi2, al

        ; fill GDT descriptor
        ;mov  eax, GDT_ADDR
        mov  eax, ebx
        mov  ebx, offset _TEXT:gdtdesc
        mov  [ebx].g_base, eax

        lgdt fword ptr [ebx]

        ret

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
        mov     edx, linux_data_real_addr

        ; copy the real mode part
        mov     esi, linux_data_tmp_addr
        mov     edi, edx
        mov     ecx, LINUX_SETUP_MOVE_SIZE
        cld
        rep     movsb

        ; change edx to the segment address
        shr     edx, 4
        mov     eax, edx
        add     eax, 20h

        mov     ebx, offset _TEXT16:linux_setup_seg + REAL_BASE
        mov     dword ptr [ebx] , eax
        mov     ebx, edx

        ; XXX new stack pointer in safe area for calling functions
        mov     esp, 4000h
        call    stop_floppy

        ; final setup for linux boot

        mov     eax, REAL_BASE
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
        mov     eax, REAL_BASE
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

_TEXT    ends

_DATA    segment dword public 'DATA'  use32

screenpos  dd   VIDEO_BUF
errmsg     db   "This is not a multiboot loader or no LIP module!",0
oldgdtdesc gdtr <>

_DATA    ends

         end entry

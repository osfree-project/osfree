;
; linux.asm
; linux multiboot kernel startup
;

name linux

.386p

include linux.inc
include struc.inc
include mb_info.inc
include mb_header.inc
include mb_etc.inc

extrn call_rm         :near
extrn cmain           :near
extrn exe_end         :near
extrn bss_end         :near
extrn gdt             :byte
extrn gdtdesc         :fword
extrn l               :dword
extrn m               :dword

public stop

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

        call  set_gdt

        ; set stack
        ;mov   esp, stack_top
        ; at the moment, we use loader stack

        ; copy realmode part of boot_linux at REAL_BASE
        cld
        mov     ecx, 0x80
        mov     esi, KERN_BASE
        mov     edi, REAL_BASE
 
        rep     movsd

        call    cmain

        ; We should not return here
        cli
        hlt
        jmp     $

stop:
        cld
        lea   esi, errmsg
        mov   edi, VIDEO_BUF
        mov   ah, 02h  ; attribute
loop1:
        lodsb          ; symbol
        stosw
        test  al, al   ; copy a string to video buffer
        jnz   loop1

        cli
        hlt
        jmp   $

set_gdt:
        ; set 16-bit segment (_TEXT16) base
        ; in GDT for protected mode
        lea  eax, oldgdtdesc
        sgdt fword ptr [eax]

        ; copy old gdt
        movzx ecx, [eax].g_limit
        inc  ecx
        shr  ecx, 2
        mov  esi, [eax].g_base
        mov  edi, offset _TEXT:gdt
        mov  ebx, edi
        rep  movsd

        ; fix gdt descriptors base
        mov  eax, REAL_BASE
        mov  [ebx][7*8].ds_baselo, ax
        mov  [ebx][8*8].ds_baselo, ax
        ror  eax, 16
        mov  [ebx][7*8].ds_basehi1, al
        mov  [ebx][8*8].ds_basehi1, al
        ror  eax, 8
        mov  [ebx][7*8].ds_basehi2, al
        mov  [ebx][8*8].ds_basehi2, al

        mov  eax, offset _TEXT:gdtdesc
        lgdt fword ptr [eax]

        ret

qwe:


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

errmsg  db  "This is not a multiboot loader or no LIP module!",0

oldgdtdesc gdtr <>

_TEXT    ends

         end entry

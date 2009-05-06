;
; krnla.asm
; template for multiboot kernel startup
;

name krnla

.386p

include fsd.inc
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

public stop
public base

_16BIT_SIZE equ 40960                                                ; 16-bit part size

;
; segments ordering
;

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
STACK_SIZE         equ 4000h
public stack_top
public stack_bottom
stack_top          db STACK_SIZE dup (?)
stack_bottom       label byte
_STACK   ends

BASE1              equ     KERN_BASE - 0x10000
VIDEO_BUF          equ     0xb8000

_TEXT16  segment dword public 'CODE' use16
                   org     0h
start:
                   ;
                   ; .mdl file header
                   ;

                   ; emulate 32-bit call instruction
                   ; to 32-bit entry point
                   db   0e8h
                   dd   offset _TEXT:entry - 5

                   org     start + 10h

base               dd      REAL_BASE

                   org     start + 20h

                   ;
                   ; end of header
                   ;

hard_stop:
        sti
        hlt

stop_flop:
                   xor     dl, dl
                   int     13h
                   retf

pad1size           equ STARTUP_SIZE - ($ - start)
pad1               db  pad1size dup (0)

                   org     start + 80h
start1:
;
; Multiboot header
;
_magic             equ 0x1badb002                                    ; multiboot header magic
_flags             equ 0x00010001                                    ; flags
_checksum          equ - _magic - _flags                             ; header checksum


__mbhdr    equ KERN_BASE + 80h                                       ; header address in memory
__start    equ KERN_BASE                                             ; executable address in memory
__exe_end  equ offset _TEXT:exe_end                                  ; executable end address
__bss_end  equ offset _TEXT:bss_end                                  ; bss end address
__entry    equ offset _TEXT:entry                                    ; entry point

;
; This is multiboot header itself
;
_mbhdr          multiboot_header  <_magic,_flags,_checksum,__mbhdr,__start,__exe_end,__bss_end,__entry,0,0,0,0>

                   org     start + 100h
real_start:
                   ;
                   ;  Place for realmode part
                   ;

_TEXT16  ends


_TEXT    segment dword public 'CODE'  use32
                   org     BASE1
entry0:
                   org     BASE1 + _16BIT_SIZE + 100h
                   ;
                   ; 32-bit entry point. Invokes by multiboot
                   ; loader from multiboot header
                   ;
entry:
                   cmp   eax, MULTIBOOT_VALID                        ; check if multiboot magic (0x2badb002)
                   jne   stop                                        ; is present in eax

                   mov   m, ebx                                      ; save multiboot structure address

                   call  set_gdt                                     ; make our own GDT

                   ; set stack
                   ;mov   esp, stack_top
                   ; at the moment, we use loader stack

                   ; copy realmode part of boot_linux at REAL_BASE
                   cld                                               ; move forward
                   mov     ecx, _16BIT_SIZE + 100h                   ; 16-bit part size
                   add     ecx, 3                                    ; divide by 4
                   shr     ecx, 2                                    ; to find size in dwords
                   mov     esi, KERN_BASE                            ; copy from
                   mov     edi, REAL_BASE                            ; copy to

                   rep     movsd                                     ; copy

                   call    cmain                                     ; call 32-bit C entry point

                   ; We should not return here                       ;
                   sti                                               ; hang
                   hlt                                               ; machine
                   jmp     $                                         ;

stop:
                   cld
                   lea     esi, errmsg
                   mov     edi, VIDEO_BUF
                   mov     ah, 02h  ; attribute
loop1:
                   lodsb          ; symbol
                   stosw
                   test    al, al   ; copy a string to video buffer
                   jnz     loop1

                   sti                                               ;
                   hlt                                               ; hang machine
                   jmp     $                                         ;

set_gdt:
                   ; set 16-bit segment (_TEXT16) base
                   ; in GDT for protected mode

                   ; fix gdt descriptors base
                   mov     ebx, GDT_ADDR
                   mov     eax, REAL_BASE
                   mov     [ebx][8*8].ds_baselo, ax
                   mov     [ebx][9*8].ds_baselo, ax
                   ror     eax, 16
                   mov     [ebx][8*8].ds_basehi1, al
                   mov     [ebx][9*8].ds_basehi1, al
                   ror     eax, 8
                   mov     [ebx][8*8].ds_basehi2, al
                   mov     [ebx][9*8].ds_basehi2, al

                   ; fill GDT descriptor
                   mov     ebx, offset _TEXT:gdtdesc
                   mov     eax, GDT_ADDR
                   mov     [ebx].g_base, eax

                   lgdt    fword ptr [ebx]

                   ret


                   ; we should not return here
                   sti
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

errmsg             db "This is not a multiboot loader or no LIP module!",0

oldgdtdesc         gdtr <>

_TEXT    ends

         end entry

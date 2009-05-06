;
; startup.asm
; bt_os2 startup
;

name startup

.386p

include fsd.inc
include struc.inc
include mb_info.inc
include mb_header.inc
include mb_etc.inc

extrn exe_end         :near
extrn bss_end         :near

public stop
public base

public init_
public cmain_

_16BIT_SIZE equ 100h                                                 ; 16-bit part size

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
                   db      'кукуй666'                                ; just to include BSS into executable file
_end4    ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_end1,_end2,_BSS,_end3,_end4

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
                   ;db   0e8h
                   ;dd   offset _TEXT:entry - 5
                   jmp     real_start

                   org     start + 10h

base               dd      REAL_BASE

                   org     start + 20h

; sizes of bundle files
sz struc
startup_size       dd      ?
microfsd_size      dd      ?
minifsd_size       dd      ?
os2ldr_size        dd      ?
sz ends

sizes              sz      <>
                   ;
                   ; end of header
                   ;
                   org     start + 80h
start1:
;
; Multiboot header
;
_magic             equ     0x1badb002                                ; multiboot header magic
_flags             equ     0x00010001                                ; flags
_checksum          equ     - _magic - _flags                         ; header checksum


__mbhdr            equ     KERN_BASE + 80h                           ; header address in memory
__start            equ     KERN_BASE                                 ; executable address in memory
__exe_end          equ     offset _TEXT:exe_end                      ; executable end address
__bss_end          equ     offset _TEXT:bss_end                      ; bss end address
__entry            equ     offset _TEXT:entry                        ; entry point

;
; This is multiboot header itself
;
_mbhdr          multiboot_header  <_magic,_flags,_checksum,__mbhdr,__start,__exe_end,__bss_end,__entry,0,0,0,0>

                   org     start + 100h
real_start:
                   ; set up initial stack
                   mov     ax, 4000h

                   cli
                   mov     ss, ax
                   mov     sp, 0ffffh
                   sti

                   push    ds
                   push    es
                   push    si
                   push    di
                   push    dx

                   mov     ax, 1000h
                   mov     cs, ax
                   mov     ds, ax

                   ; We're started at the address 0x10000 like standard os2ldr
                   ; copy startup at 0x90000
                   push    ds
                   push    es

                   cld

                   mov     ax, 1000h
                   mov     ds, ax

                   mov     ax, 9000h
                   mov     es, ax

                   xor     si, si
                   xor     di, di

                   lea     bx, sizes
                   mov     cx, [bx].startup_size

                   rep     movsb

                   pop     es
                   pop     ds

                   ; jump to the address of relocation
                   push    ax
                   push    reloc
                   retf
reloc:
                   mov     ax, cs
                   mov     ds, ax
                   mov     es, ax

                   ; copy os2ldr at 1000h
                   cld

                   mov     ax, 1000h
                   mov     es, ax

                   lea     bx, sizes
                   shl     eax, 4
                   add     eax, [bx].startup_size
                   add     eax, [bx].microfsd_size
                   add     eax, [bx].minifsd_size
                   mov     ecx, [bx].os2ldr_size
                   mov     edx, eax
                   and     edx, 0fh
                   shr     eax, 4

                   mov     ds, ax
                   mov     si, dx
                   xor     di, di

                   add     ecx, 3
                   shr     ecx, 2

                   rep     movsd

                   pop     dx
                   pop     di
                   pop     si
                   pop     es
                   pop     ds

                   ; pass control to os2ldr
                   push    1000h
                   push    0
                   retf
_TEXT16  ends

_TEXT    segment dword public 'CODE'  use32
                   ; beginning of the executable
                   org     BASE1
entry0:
                   ; start of 32-bit part
                   org     BASE1 + _16BIT_SIZE + 100h
                   ;
                   ; 32-bit entry point. Invokes by multiboot
                   ; loader from multiboot header
                   ;
entry:
                   cmp   eax, MULTIBOOT_VALID                        ; check if multiboot magic (0x2badb002)
                   jne   stop                                        ; is present in eax

                   ;mov   m, ebx                                      ; save multiboot structure address

                   ; set stack
                   ;mov   esp, stack_top
                   ; at the moment, we use loader stack

                   ; copy bt_os2 parts at REAL_BASE (below 1 Mb)
                   cld                                               ; move forward
                   mov     esi, KERN_BASE                            ; begin of startup
                   mov     edx, offset _TEXT16:sizes                 ;
                   add     edx, esi
                   add     esi, [edx]                                ; got copied part start address
                   mov     ecx, [edx].microfsd_size                  ; 16-bit part size
                   add     ecx, [edx].minifsd_size                   ;
                   add     ecx, [edx].os2ldr_size                    ; copied part size in bytes
                   add     ecx, 3                                    ; divide by 4
                   shr     ecx, 2                                    ; to find size in dwords
                   mov     edi, REAL_BASE                            ; copy to

                   rep     movsd                                     ; copy

                   ; start microfsd emulator
                   push    REAL_BASE
                   ret

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


errmsg             db "This is not a multiboot loader or no LIP module!",0

oldgdtdesc         gdtr <>

init_              label near
cmain_             label near

_TEXT    ends

         end entry

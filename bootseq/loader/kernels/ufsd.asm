;
; ufsd.asm
; microfsd emulator
;


name microfsd

public realmode_init

extrn  _m          :dword
extrn  init_       :near
extrn  cmain_      :near

.386p

include fsd.inc
include struc.inc
include mb_info.inc

BASE1              equ     REL_BASE - 0x10000
_16BIT_SIZE        equ     300
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
                   org     start + 80h
start1:

                   org     start + 100h
real_start:

                   org     start + 200h
realmode_init:
                   mov     al, 'q'
                   mov     bx, 1
                   mov     ah, 0Eh
                   int     10h                                       ; display a byte

                   sti
                   hlt
                   jmp     $
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

                   mov   _m, ebx                                      ; save multiboot structure address

		   call  cmain_

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

errmsg             db   "This is not a multiboot loader or no LIP module!",0

oldgdtdesc         gdtr <>

_TEXT    ends
_DATA    segment dword public 'DATA'  use32
_DATA    ends
CONST    segment dword public 'DATA'  use32
CONST    ends
CONST2   segment dword public 'DATA'  use32
CONST2   ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends
_STACK   segment dword public 'STACK' use32
_STACK   ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_DATA,_BSS,_STACK

         end entry

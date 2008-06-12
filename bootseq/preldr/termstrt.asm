;
; termstrt.asm:
; terminal blackbox (uT) segments
; alignment and ordering
;

name termstart

public  set_gdt
public  base

extrn   base32        :near

extrn   gdt           :byte
extrn   gdtdesc       :fword

include fsd.inc
include struc.inc

.386p

; Start of the executable
BASE1      equ EXT_BUF_BASE - 10000h
; Start of 32-bit code. 380h is
; the limit of 16-bit part size
BASE2      equ BASE1 + 380h

_TEXT16  segment dword public 'CODE'  use16
           org 0h
HEADER_SIZE     equ     20h
header_begin:
           ; here we simulate 32-bit call instruction
           ; WASM can't generate 32-bit instruction
           ; inside 16-bit segment, so this workaround.
           ; This instruction must route a call to the
           ; beginning of executable to init() function
           ; at the beginning of 32-bit part (header_begin + 380h).
           db   0e8h
           dd   offset _TEXT:start2 - EXT_BUF_BASE - 5
           ; return
           db   0c3h
           org 8h 
           ; a sort of header signature
           db   '$header$'
           org 10h
           ; base of realmode part segment
base       dd  TERMLO_BASE
           ; padding with NOP's
           ;db   HEADER_SIZE  - ($ - header_begin)       dup (90h)
           org 20h
header_end:
_TEXT16  ends
_TEXT    segment dword public 'CODE'  use32
           org BASE1
start1:
           org BASE2
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
        mov  eax, dword ptr base32
        mov  [ebx][5*8].ds_baselo, ax
        mov  [ebx][6*8].ds_baselo, ax
        ror  eax, 16
        mov  [ebx][5*8].ds_basehi1, al
        mov  [ebx][6*8].ds_basehi1, al
        ror  eax, 8
        mov  [ebx][5*8].ds_basehi2, al
        mov  [ebx][6*8].ds_basehi2, al

        mov  eax, offset _TEXT:gdtdesc
        lgdt fword ptr [eax]

        ret

oldgdtdesc gdtr <>

start2:
           ; here must start init() function
           ; in terminit.c file

_TEXT    ends
_DATA    segment dword public 'DATA'  use32
_DATA    ends
CONST    segment dword public 'DATA'  use32
CONST    ends
CONST2   segment dword public 'DATA'  use32
CONST2   ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_BSS

         end

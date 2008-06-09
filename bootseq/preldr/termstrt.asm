;
; termstrt.asm:
; terminal blackbox (uT) segments
; alignment and ordering
;

name termstart

include fsd.inc

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
           ; a sort of header signature
           db   '$header$'
           ; padding with NOP's
           db   HEADER_SIZE  - ($ - header_begin)       dup (90h)
header_end:
_TEXT16  ends
_TEXT    segment dword public 'CODE'  use32
           org BASE1
start1:
           org BASE2
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

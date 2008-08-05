;
; segord.asm
; Segments ordering and definition
;

.386p

extrn    _text16_begin :word
extrn    _text16_end   :word

include  fsd.inc

assume   ds:_TEXT16,cs:_TEXT16,ss:_TEXT16

_TEXT16  segment dword public 'CODE'  use16
org 0h
_TEXT16  ends

BASE1      equ STAGE0_BASE - 10000h
TEXT16_END equ (offset cs:_text16_end)
TEXT16_BEG equ (offset cs:_text16_begin)
TEXT16_LEN equ (TEXT16_END - TEXT16_BEG)

; hardcoded for now
; ! todo: fix
ifndef STAGE1_5
BASE       equ (BASE1 + 680h)    ; 600h; TEXT16_LEN
else
BASE       equ (BASE1 + 260h)    ; 233h
endif

_TEXT    segment dword public 'CODE'  use32
org BASE
DGROUP_begin:
_TEXT    ends
_DATA    segment dword public 'DATA'  use32
_DATA    ends
CONST    segment dword public 'DATA'  use32
CONST    ends
CONST2   segment dword public 'DATA'  use32
CONST2   ends
_end1    segment dword public 'DATA'  use32
align 4
_end1    ends
_end2    segment dword public 'DATA'  use32
_end2    ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends
_end3    segment dword public 'BSS'   use32
align 4
_end3    ends
_end4    segment dword public 'BSS'   use32
_end4    ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_end1,_end2,_BSS,_end3,_end4

         end

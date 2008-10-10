;
; start.asm:
; uFSD startup code
;

name start

extern  init:NEAR
public  fsd_init

include fsd.inc
include fsname.inc

.386

_TEXT    segment dword public 'CODE'  use32
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

;lala     equ 'ext2fs'

_TEXT    segment dword public 'CODE'  use32
           org EXT_BUF_BASE
;
;           jmp  fsd_init
;           nop
;           nop
;           nop
;           ; return
;           db   0c3h
;           org EXT_BUF_BASE + 8h
;           ;
;modname    db mdl
;           org EXT_BUF_BASE + 20h
;           ; base of realmode part segment
;base       dd    0
;
;           org EXT_BUF_BASE + 40h
fsd_init:
_TEXT    ends

         end fsd_init

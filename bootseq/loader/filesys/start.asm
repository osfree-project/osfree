;
; start.asm:
; uFSD startup code
;

name start

extern  init:NEAR
public  fsd_init

public   exe_end
public   bss_end
public   bss_start
public   bss_len

include fsd.inc
;include fsname.inc

.386

_TEXT    segment dword public 'CODE'  use32
_TEXT    ends
_DATA    segment dword public 'DATA'  use32
_DATA    ends
CONST    segment dword public 'DATA'  use32
CONST    ends
CONST2   segment dword public 'DATA'  use32
CONST2   ends
_end1    segment dword public 'DATA'  use32
align 4
exe_end:
_end1    ends
_end2    segment dword public 'BSS'   use32
_end2    ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends
_end3    segment dword public 'BSS'   use32
align 4
bss_end:
_end3    ends
_end4    segment dword public 'BSS'   use32
_end4    ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_BSS,_end1,_end2,_end3,_end4

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
         jmp  real_start
__bss_start     dd (exe_end - EXT_BUF_BASE)
__bss_end       dd (bss_end - EXT_BUF_BASE)
real_start:
_TEXT    ends

         end fsd_init

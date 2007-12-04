;
; ldrstart.asm
;

name ldrstart

extrn   init           :near

public l
public ft
public bpb
public boot_drive

.386

include fsd.inc

_TEXT    segment byte public 'CODE'  use32
_TEXT    ends
_DATA    segment byte public 'DATA'  use32
_DATA    ends
CONST    segment byte public 'DATA'  use32
CONST    ends
CONST2   segment byte public 'DATA'  use32
CONST2   ends
_BSS     segment byte public 'BSS'   use32
_BSS     ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_BSS

_TEXT segment byte public 'CODE' use32

      org LDR_BASE

start proc near
      cmp  eax, BOOT_MAGIC
      je   ok
      jmp  stop
ok:
      assume ds:_TEXT

      mov  l,   ebx  ; save lip pointer
      mov  ft,  edi  ; save filetable pointer
      mov  bpb, esi  ; save BPB pointer
      mov  boot_drive, dl ; boot drive number

      assume cs:_TEXT

      call init
;
; We should not return here
;
stop:
      cli
      hlt

; lip address
l            dd   0
; filetable address
ft           dd   0
; BPB address
bpb          dd   0
; boot drive
boot_drive   db   0

start endp

_TEXT ends

      end start

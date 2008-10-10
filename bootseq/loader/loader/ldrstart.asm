;
; ldrstart.asm
;

name ldrstart

extrn  cmain           :near
extrn  m               :dword
extrn  entry_addr      :dword

public l
public ft
public bpb
public boot_drive
public multi_boot
public oldstack
public loader_stack_top

.386

include fsd.inc
include mb_etc.inc

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
_STACK   segment dword public 'STACK' use32
_STACK   ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_BSS,_STACK

_STACK   segment dword public 'STACK' use32
LDR_STACK_SIZE equ 8000h
public loader_stack_top
public loader_stack_bottom
;                        db 3 dup (?)
loader_stack_bottom     db LDR_STACK_SIZE dup (?)
loader_stack_top        label byte
_STACK   ends

_TEXT segment dword public 'CODE' use32

      org LDR_BASE

VIDEO_BUF  equ 0b8000h

start proc near
      cmp  eax, BOOT_MAGIC
      je   ok
      jmp  stop
ok:
      assume ds:_TEXT

      mov  l,   ebx  ; save lip pointer
      mov  ft,  edi  ; save filetable pointer
      mov  bpb, esi  ; save BPB pointer
      mov  byte ptr boot_drive, dl ; boot drive number

      ; set loader stack
      mov  oldstack, esp
      mov  esp, offset DGROUP:loader_stack_top
      mov  ebp, esp

      assume cs:_TEXT

      call cmain

      cli
      hlt
;      jmp     $

multi_boot:
      ;
      ; Now booting the kernel
      ;

      ; no need to save anything
      ;call    stop_floppy

      ; magic
      mov     eax, 2badb002h

      ; mbi structure
      mov     ebx, offset _TEXT:m
      mov     ebx, [ebx]

      ; boot kernel here (absolute address call)
      mov     ebp, offset _TEXT:entry_addr

      call    dword ptr [ebp]

      cli
      hlt
;      jmp     $
;
; We should not return here
;
stop:
      cld
      lea     esi, errmsg
      mov     edi, VIDEO_BUF
      mov     ah, 02h  ; attribute
loop1:
      lodsb            ; symbol
      stosw
      test    al, al
      jnz     loop1    ; copy a string to video buffer

      cli
      hlt
;      jmp     $

; lip2 address
l             dd   0
; filetable address
ft            dd   0
; BPB address
bpb           dd   0
; boot drive
boot_drive    dd   0

errmsg        db   "No boot magic in EAX, panic...",13,10,0
oldstack      dd   0

start endp

_TEXT ends

      end start

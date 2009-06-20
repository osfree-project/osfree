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
public multi_boot_
public return_to_preldr_
public oldstack
public loader_stack_top

public   exe_end
public   bss_end

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
_STACK   segment dword public 'STACK' use32
_STACK   ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_BSS,_end1,_end2,_end3,_end4,_STACK

_STACK   segment dword public 'STACK' use32
LDR_STACK_SIZE equ 8000h
public loader_stack_top
public loader_stack_bottom
;                        db 3 dup (?)
loader_stack_bottom     db LDR_STACK_SIZE dup (?)
loader_stack_top        label byte
_STACK   ends

_TEXT segment dword public 'CODE' use32

      org  LDR_BASE

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
      mov  oldframe, ebp
      mov  esp, offset DGROUP:loader_stack_top
      mov  ebp, esp

      ; clear bss
      ;cld
      ;xor  eax, eax
      ;lea  edi, exe_end
      ;lea  ecx, bss_end
      ;sub  ecx, edi
      ;shr  ecx, 2
      ;inc  ecx

      ;rep  stosd

      assume cs:_TEXT

      call cmain

      cli
      hlt

      int 3
;      jmp     $

return_to_preldr_:
      mov  esp, oldstack
      mov  ebp, oldframe

      ret

multi_boot_:
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

      mov     dl, byte ptr boot_drive
      mov     edi, ft
      mov     esi, bpb

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
oldframe      dd   0

start endp

_TEXT ends

      end start

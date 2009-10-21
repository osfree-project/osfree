;
; ufsd.asm
; microfsd emulator
;


name microfsd

public __sp
public m
public base
public realmode_init

public boot_flags
public boot_drive
public ft

public stack_bottom
public force_lba

public get_disk_type_
public com_outchar
public port
public _debug

public _small_code_

extrn  callback     :dword
extrn  idt_initted  :byte
extrn  kprintf_     :near
extrn  init_        :near
extrn  cmain_       :near
extrn  call_rm      :near
extrn  idt_init     :near
extrn  gdtdesc      :fword

extrn  preldr_ds    :word
extrn  preldr_ss_sp :dword
extrn  preldr_es    :word

.386p

include fsd.inc
include struc.inc
include mb_info.inc
include loader.inc
include bpb.inc

BASE1              equ     REL1_BASE - 0x10000
_16BIT_SIZE        equ     0x600
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
                   dd   (entry - entry0 - 5)

                   org     start + 10h

base               dd      REL1_BASE

                   org     start + 20h

mfs_len            dd      ?
port               dw      0
_debug             db      0
force_lba          db      0

                   ;
                   ; end of header
                   ;
                   org     start + 80h
start1:

                   org     start + 100h
real_start:

                   org     start + 200h
realmode_init:
                   ; save pre-loader segment registers and stack
                   mov  ax, ds
                   mov  word ptr cs:preldr_ds, ax
                   mov  ax, es
                   mov  word ptr cs:preldr_es, ax
                   mov  ax, sp
                   mov  word ptr cs:preldr_ss_sp, ax
                   mov  ax, ss
                   mov  word ptr cs:preldr_ss_sp + 2, ax

                   ;push ebx

                   ;mov  al, '|'
                   ;call com_outchar

                   ;xor  ax, ax
                   ;mov  gs, ax
                   ;mov  bx, 1f41h
                   ;mov  al, byte ptr gs:[bx]
                   ;call com_outchar

                   ;mov  al, ';'
                   ;call com_outchar

                   ;push cx

                   ;mov  ebx, dword ptr cs:preldr_ss_sp
                   ;mov  cx, 8
;lp1:
                   ;mov  al, bl
                   ;and  al, 0fh
                   ;add  al, 30h
                   ;call com_outchar
                   ;ror  ebx, 4

                   ;loop lp1

                   ;pop  cx

                   ;mov  al, '|'
                   ;call com_outchar

                   ;pop  ebx

                   ;
                   ; pass structures to os2ldr
                   ;
                   xor  eax, eax
                   mov  eax, offset _TEXT:boot_flags - REL1_BASE
                   ; set bootflags
                   mov  dx, [eax]

                   ; set bootdrive
                   mov  eax, offset _TEXT:boot_drive - REL1_BASE
                   mov  dl, [eax]

                   mov  edi, offset _TEXT:ft - REL1_BASE

                   ; set BPB
                   mov  eax, REL1_BASE - 200h
                   shr  eax, 4
                   mov  ds,  ax
                   mov  si,  0bh           ; 3 + 8 = 11 -- BPB offset from the beginning of boot sector

                   and  ebp, 0ffffh

                   ; return to os2ldr
                   push OS2LDR_SEG
                   push 0

                   pusha

                   mov    al, '*'
                   xor    bx, bx
                   mov    ah, 0eh ; ah=0eh -- function

                   int    10h

                   popa

                   retf

                   ; output a char to commport
                   ; Input:
                   ; al == char
com_outchar:
                   cmp  _debug, 0
                   jz   exitXX

                   pusha
                   ; wait while comport is ready
                   mov  bl, al
loo1:
                   mov  dx, port
                   add  dx, 5
                   in   al, dx
                   out  80h, al
                   test al, 20h
                   jz   loo1

                   ; output a char
                   mov  dx, port
                   mov  al, bl
                   out  dx, al
                   out  80h, al

                   popa
exitXX:
                   ret

get_disk_type_rm:
                   push ecx

                   mov  dx, bx
                   mov  ax, 15ffh
                   mov  cx, 0ffffh

                   int  13h

                   xor  ebx, ebx
                   xor  edx, edx
                   mov  bl, ah
                   jc   err1
                   mov  dl, 0
                   jmp  ret1
err1:
                   mov  dl, 1

ret1:
                   pop  ecx

                   retf

_small_code_  dd 0

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
                   cmp     eax, MULTIBOOT_VALID                        ; check if multiboot magic (0x2badb002)
                   jne     stop                                        ; is present in eax

                   ; setup stack
                   mov     esp, PM_STACK_INIT

                   mov     ds:m, ebx                                   ; save multiboot structure address

                   mov     ebx, offset _TEXT:callback
                   mov     [ebx], ecx

                   ; setup GDT
                   call    set_gdt
                   ; setup IDT
                   call    idt_init

                   ; 32-bit uFSD init
                   call    cmain_

                   ; enter real mode
                   mov     eax, REL1_BASE
                   shl     eax, 12
                   mov     ax,  offset _TEXT16:realmode_init
                   push    eax
                   xor     eax, eax
                   call    call_rm
                   add     esp, 4

                   ; We should not return here                       ;
                   cli                                               ; hang
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

                   cli                                               ;
                   hlt                                               ; hang machine
                   jmp     $                                         ;                          s

set_gdt:
                   ; fix gdt descriptors base
                   mov  ebx, GDT_ADDR
                   mov  eax, REL1_BASE
                   ; FLAT DS and CS
                   mov  [ebx][1*8].ds_limit, 0xffff
                   mov  [ebx][2*8].ds_limit, 0xffff

                   mov  [ebx][1*8].ds_baselo, 0
                   mov  [ebx][2*8].ds_baselo, 0

                   mov  [ebx][1*8].ds_basehi1, 0
                   mov  [ebx][2*8].ds_basehi1, 0

                   mov  [ebx][1*8].ds_basehi2, 0
                   mov  [ebx][2*8].ds_basehi2, 0

                   mov  [ebx][1*8].ds_acclo, 0x9a
                   mov  [ebx][2*8].ds_acclo, 0x92

                   mov  [ebx][1*8].ds_acchi, 0xcf
                   mov  [ebx][2*8].ds_acchi, 0xcf

                   ; pseudo RM DS and CS
                   mov  [ebx][8*8].ds_limit, 0xffff
                   mov  [ebx][9*8].ds_limit, 0xffff

                   mov  [ebx][8*8].ds_baselo, ax
                   mov  [ebx][9*8].ds_baselo, ax
                   ror  eax, 16
                   mov  [ebx][8*8].ds_basehi1, al
                   mov  [ebx][9*8].ds_basehi1, al
                   ror  eax, 8
                   mov  [ebx][8*8].ds_basehi2, al
                   mov  [ebx][9*8].ds_basehi2, al

                   mov  [ebx][8*8].ds_acclo, 0x9e
                   mov  [ebx][9*8].ds_acclo, 0x92

                   mov  [ebx][8*8].ds_acchi, 0
                   mov  [ebx][9*8].ds_acchi, 0

                   ; fill GDT descriptor
                   mov  ebx, offset _TEXT:gdtdesc
                   mov  eax, GDT_ADDR
                   mov  [ebx].g_base, eax
                   mov  [ebx].g_limit, 10*8 - 1

                   lgdt fword ptr [ebx]

                   ret

                   ;
                   ; int get_disk_type(int driveno, int *status);
                   ;
                   ; (check if the BIOS drive number is valid)
                   ;
get_disk_type_:
                   push ebx
                   push ecx
                   push edx

                   mov  ebx, eax
                   mov  ecx, edx

                   mov     eax, REL1_BASE
                   shl     eax, 12
                   mov     ax,  offset _TEXT16:get_disk_type_rm
                   push    eax
                   call    call_rm
                   add     esp, 4

                   mov  eax, edx    ; return code
                   mov  [ecx], ebx  ; status/error code

                   pop  edx
                   pop  ecx
                   pop  ebx

                   ret

errmsg             db   "This is not a multiboot loader or no LIP module!",0

oldgdtdesc         gdtr <>

boot_flags         dw 0         ; <-- DX
boot_drive         dd 0         ; <-- DL
ft                 FileTable <>

_TEXT    ends

_DATA    segment dword public 'DATA'  use32

m        dd  ?
__sp     dw  0

_DATA    ends

_TEXT16  segment dword public 'CODE'  use16
_TEXT16  ends
__DATA  segment dword public 'DATA'  use16
__DATA  ends
_CONST  segment dword public 'DATA'  use16
_CONST  ends
_CONST2  segment dword public 'DATA'  use16
_CONST2  ends
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
_end4    ends

; protected mode stack size
PM_STACK_SIZE    equ   2000h
PM_STACK_INIT    equ   REL1_BASE - 200h
; real mode stack size
STACK_SIZE       equ   1000h

_STACK   segment dword public 'STACK' use32
stack_top:
         db STACK_SIZE dup (?)
stack_bottom:
_STACK   ends

DGROUP16 group _TEXT16,__DATA,_CONST,_CONST2
DGROUP   group _TEXT,_DATA,CONST,CONST2,_end1,_end2,_BSS,_end3,_end4,_STACK

         end entry

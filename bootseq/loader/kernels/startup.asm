;
; startup.asm
; bt_os2 startup
;

name startup

.386p

include fsd.inc
include struc.inc
include mb_info.inc
include mb_header.inc
include mb_etc.inc

extrn exec_cmd        :dword
extrn callback        :near
extrn cmain_          :near
extrn exe_end         :near
extrn bss_end         :near
extrn kprintf_        :near
extrn call_rm         :near
extrn mfsd_size       :dword
extrn gdtsrc          :byte
extrn gdtdesc         :fword

public stop
public base

public m

public getlowmem_

_16BIT_SIZE equ 600h                                                 ; 16-bit part size

;
; segments ordering
;

_TEXT16  segment dword public 'CODE'  use16
_TEXT16  ends
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
                   ;db      'кукуй666'                                ; just to include BSS into executable file
_end4    ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_end1,_end2,_BSS,_end3,_end4

BASE1              equ     KERN_BASE - 0x10000
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

; sizes of bundle files
sz struc
startup_size       dd      ?
microfsd_size      dd      ?
minifsd_size       dd      ?
os2ldr_size        dd      ?
sz ends

sizes              sz      <>
                   ;
                   ; end of header
                   ;
                   org     start + 80h
start1:
;
; Multiboot header
;
_magic             equ     0x1badb002                                ; multiboot header magic
_flags             equ     0x00010001                                ; flags
_checksum          equ     - _magic - _flags                         ; header checksum


__mbhdr            equ     KERN_BASE + 80h                           ; header address in memory
__start            equ     KERN_BASE                                 ; executable address in memory
__exe_end          equ     offset _TEXT:exe_end                      ; executable end address
__bss_end          equ     offset _TEXT:bss_end                      ; bss end address
__entry            equ     offset _TEXT:entry                        ; entry point

;
; This is multiboot header itself
;
_mbhdr          multiboot_header  <_magic,_flags,_checksum,__mbhdr,__start,__exe_end,__bss_end,__entry,0,0,0,0>

_flags_ext         equ     0x00000001
_checksum_ext      equ     - _magic - _flags_ext

_mbhdr_ext      multiboot_header_ext <_magic,_flags_ext,_checksum_ext,offset _TEXT:exec_cmd,0,0,0,0,0,0,0,0>

                   org     start + 100h
real_start:
getlowmem_rm:
                   int  12h
                   mov  ebx, eax

                   retf

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
.386p
entry:
                   cmp   eax, MULTIBOOT_VALID                        ; check if multiboot magic (0x2badb002)
                   jne   stop                                        ; is present in eax

                   mov   ds:m, ebx                                   ; save multiboot structure address

                   ; set stack
                   mov   esp, 9e000h

                   push    eax

                   call    set_gdt

                   call    cmain_

                   ; pass mfsd_size to uFSD emulator
                   mov     esi, eax                                  ; uFSD copying address
                   add     esi, 0x20                                 ; mfs_len address in uFSD emulator
                   mov     edx, mfsd_size
                   mov     [esi], edx

                   push    eax

                   ; show progress
                   mov     ebx, eax
                   push    ebx
                   push    edx
                   lea     eax, mid_msg
                   push    eax
                   call    kprintf_
                   add     esp, 12

                   pop     eax

                   mov     edx, eax

                   push    edx
                   lea     eax, pass_msg
                   push    eax
                   call    kprintf_
                   add     esp, 8

                   pop     eax

                   ; pass config.sys callback address
                   lea     ecx, callback

                   ; set up uFSD stack
                   mov     ebx, edx
                   sub     ebx, 200h
                   mov     esp, ebx

                   mov     ebx, ds:m

                   ; start microfsd emulator
                   push    edx

                   ret

                   ; We should not return here                       ;
                   cli                                               ; hang
                   hlt                                               ; machine
                   jmp     $                                         ;

                   ;
                   ; get lower memory
                   ;
getlowmem_:
                   mov  eax, REL1_BASE
                   shr  eax, 4
                   push ax
                   mov  eax, offset _TEXT:getlowmem_rm
                   push ax
                   call call_rm
                   add  esp, 4
                   mov  eax, ebx

                   ret

set_gdt:
                   ; fix gdt descriptors base
                   ;mov  ebx, GDT_ADDR
                   mov  ebx, offset _TEXT:gdtsrc
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
                   mov  [ebx][2*8].ds_acclo, 0x93

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
                   mov  [ebx][9*8].ds_acclo, 0x93

                   mov  [ebx][8*8].ds_acchi, 0
                   mov  [ebx][9*8].ds_acchi, 0

                   ; fill GDT descriptor
                   ;mov  eax, GDT_ADDR
                   mov  eax, ebx
                   mov  ebx, offset _TEXT:gdtdesc
                   mov  [ebx].g_base, eax
                   mov  [ebx].g_limit, 10*8 - 1

                   lgdt fword ptr [ebx]

                   ret

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
                   jmp     $                                         ;

errmsg             db   "This is not a multiboot loader or no LIP module!",0

oldgdtdesc         gdtr <>

_TEXT    ends

_DATA    segment dword public 'DATA'  use32

m                  dd   ?
mid_msg            db   "cmain() finished.",10
                   db   "mFSD size: %lu, uFSD base: 0x%lx",10,0
pass_msg           db   "passing control to uFSD @ 0x%x",10,10,0
_DATA    ends

         end entry

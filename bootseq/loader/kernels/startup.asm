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

extrn cmain_          :near
extrn exe_end         :near
extrn bss_end         :near

public rel_start
public ufsd_start
public ufsd_size

public stop
public base

public m

_16BIT_SIZE equ 100h                                                 ; 16-bit part size

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
                   db      'кукуй666'                                ; just to include BSS into executable file
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

                   org     start + 100h
real_start:

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


ufsd_start:
; here uFSD image begins
include ufsd.inc
; here it ends
; its size
ufsd_size          dd $ - ufsd_start

mfsd_start:
; here mFSD image begins
include mfsd.inc
; here it ends
; its size
mfsd_size          dd $ - mfsd_start

rel_start:
; here mFSD reloc. info begins
include urel.inc
; here it ends
; its size
rel_size           dd $ - rel_start

entry:
                   cmp   eax, MULTIBOOT_VALID                        ; check if multiboot magic (0x2badb002)
                   jne   stop                                        ; is present in eax

                   mov   ds:m, ebx                                   ; save multiboot structure address

                   ; set stack
                   ;mov   esp, stack_top
                   ; at the moment, we use loader stack

                   ; copy bootos2 parts at REL_BASE (below 1 Mb)
                   ;cld                                               ; move forward
                   ;mov     esi, ufsd_start                           ; begin of startup
                   ;mov     ecx, ufsd_size
                   ;shr     ecx, 2
                   ;inc     ecx
                   ;mov     edi, REL1_BASE                            ; copy to

                   ;rep     movsd                                     ; copy

                   ; copy mFSD at 0x7c0
                   mov     esi, mfsd_start
                   mov     ecx, mfsd_size
                   shr     ecx, 2
                   inc     ecx
                   mov     edi, 0x7c0

                   rep     movsd

                   push    eax

                   call    cmain_
.386p
                   ; pass mfsd_size to uFSD emulator
                   mov     esi, eax                                  ; uFSD copying address
                   add     esi, 0x20                                 ; mfs_len address in uFSD emulator
                   mov     edx, mfsd_size
                   mov     [esi], edx

                   mov     edx, eax

                   pop     eax

                   ; start microfsd emulator
                   push    edx
                   ret

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
                   jmp     $                                         ;

errmsg             db   "This is not a multiboot loader or no LIP module!",0

oldgdtdesc         gdtr <>

_TEXT    ends

_DATA    segment dword public 'DATA'  use32

m        dd  ?

_DATA    ends

         end entry

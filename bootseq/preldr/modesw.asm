;
; modesw.asm:
; real <-> protected
; mode switching
;

name modesw

.386p

public  gdt
public  gdtdesc
public  call_rm

ifndef  NO_PROT
public  call_pm
endif

public  __CHK
public  __I8LS
public  __U8RS

include fsd.inc
include struc.inc

include mb_etc.inc


ifdef NO_PROT

ifdef BLACKBOX
  BASE    equ TERMLO_BASE
else
  BASE    equ REAL_BASE
endif

else

  BASE    equ STAGE0_BASE

endif

_TEXT16  segment dword public 'CODE'  use16

start1   label byte

ifndef NO_PROT

;
; void __cdecl call_pm(unsigned long func);
;
; (Call 32-bit protected mode function
; with entry point address func)
;

call_pm proc near
        ; Save stack frame
        push bp
        mov  bp, sp
        ; Disable interrupts
        cli
        ; Load GDTR
        mov  eax, offset _TEXT:gdtdesc - BASE
        lgdt fword ptr [eax]
        ; Enable protected mode
        mov  eax, cr0
        or   eax, 1
        mov  cr0, eax
        ; Do a far jump to 16-bit segment
        ; to switch to protected mode
        mov  ax, PSEUDO_RM_CSEG
        push ax
        push protmode
        retf
protmode:
        ; set selectors
        mov  ax, PSEUDO_RM_DSEG
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax
        ; do a far call to a 32-bit segment
        push esi
        mov  esi, offset _TEXT:address - BASE
        mov  ax,  PROT_MODE_CSEG
        mov  word ptr  [esi + 4], ax
        mov  eax, offset _TEXT:pmode
        mov  dword ptr [esi], eax
        mov  ebp,  dword ptr [bp + 4]
        mov  eax, esi
        pop  esi

        call fword ptr [eax]

        ; Clear PE bit in CR0 register
        mov  eax, cr0
        and  al,  0feh
        mov  cr0, eax
        ; long jump to 16-bits entry point
        mov  eax, BASE
        shr  eax, 4
        push ax
        push realmode
        retf
realmode:
        ; Set up segments
        mov  ds,  ax
        mov  es,  ax
        mov  fs,  ax
        mov  gs,  ax
        mov  ss,  ax
        ; Restore interrupts
        sti
        ; Restore stack frame
        pop  bp
        ; Return
        ret
call_pm endp

endif

;
; This function gets called from
; 32-bit segment and it switches
; machine to real mode and calls
; a real mode function with address
; specified in ebp register.
;

rmode_switch proc far
        ; switch to real mode
        mov  eax, cr0
        and  al,  0feh
        mov  cr0, eax
        ; set segment registers
        mov  eax, BASE
        shr  eax, 4
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax
        ; do a far jump to reload a
        ; real mode CS
        push ds
        push rmode1
        retf
rmode1:
        ; Now we are in a real mode
        ; call a function with address in ebp
        mov  eax, ebp
        shr  eax, 16
        push ax
        push bp
        mov  bp, sp
        call dword ptr [bp]
        add  sp, 4
        ; Switcch back to protected mode
        mov  eax, cr0
        or   al, 1
        mov  cr0, eax
        ; load segment registers
        mov  ax, PSEUDO_RM_DSEG
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax
        ; do a far jump to load a
        ; protected mode 16-bit CS
        mov  ax, PSEUDO_RM_CSEG
        push ax
        push pmode1
        retf
pmode1:
        ; return to 32-bit segment
        ; use jmp instead of retf
        ; because retf uses 16-bit offset,
        ; not 32-bit one
        ;retf
        mov bp, sp
        jmp fword ptr ss:[bp]
rmode_switch endp

ifdef NO_PROT

MODESW_SZ   equ  0x100
padsize     equ  MODESW_SZ - ($ - start1)
pad         db   padsize dup (0)

endif

_TEXT16 ends


_TEXT   segment dword public 'CODE' use32

ifndef NO_PROT

;
; pmode:
; A wrapper for 32-bit function call,
; gets called from 16-bit protmode
; function, sets selectors and calls
; function; takes function entry point
; flat address in ebp.
;

pmode   proc far
        ; Load segment registers
        mov  ax, PROT_MODE_DSEG
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax
        ; Get protected mode stack
        mov  eax, protstack
        mov  rmstack, esp
        mov  esp, eax
        ; Call protected mode func
        push  cs
        push  ebp
        call  fword ptr ss:[esp]
        add   esp, 6

        ; Save protected mode stack
        mov  eax, esp
        mov  protstack, eax
        mov  eax, rmstack
        mov  esp, eax
        ; Set up selectors
        mov  ax, PSEUDO_RM_DSEG
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax

        xor  eax, eax

        retf
pmode   endp

endif

;
; void __cdecl call_rm(fp_t func);
;
; Call real-mode function with address func
; from protected mode
;

call_rm proc near
        push ebp
        mov  ebp, esp

        mov  ebp, dword ptr [ebp + 8]

        ; set segment registers
        ; and switch stack to 16-bit
        mov  eax, esp
        mov  esp, rmstack
        mov  protstack, eax

        mov  ax, PSEUDO_RM_DSEG
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax
        ; call 16-bit function
        mov  ax, PSEUDO_RM_CSEG
        push ax
        mov  eax, offset _TEXT16:rmode_switch
        push eax
        call fword ptr ss:[esp]
        add  esp, 14      ; 6 bytes are the called function address and 8
                          ; bytes are the return address.
        mov  ax, PROT_MODE_DSEG
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax

        mov  eax, esp
        mov  esp, protstack
        mov  rmstack, eax

        pop  ebp

        ret
call_rm endp

__CHK:
        ret  4
__I8LS:
        ret

__U8RS:
        ret

_TEXT   ends

_DATA   segment dword public 'DATA' use32

CR0_PE_ON       equ 01h
CR0_PE_OFF      equ 0fffffffeh
PROT_MODE_CSEG  equ 08h
PROT_MODE_DSEG  equ 10h

ifndef NO_PROT
PSEUDO_RM_CSEG  equ 18h
PSEUDO_RM_DSEG  equ 20h
else
PSEUDO_RM_CSEG  equ 28h
PSEUDO_RM_DSEG  equ 30h
endif

;STACKOFF        equ (2000h - 10h)
PROTSTACKINIT   equ STACK_SP - 10h ; SCRATCHADDR - 10h

;align 4

protstack         dd   PROTSTACKINIT
rmstack           dd   REALSTACKINIT
boot_drive        dd   0

;align 4

;/*
; * This is the Global Descriptor Table
; *
; *  An entry, a "Segment Descriptor", looks like this:
; *
; * 31          24         19   16                 7           0
; * ------------------------------------------------------------
; * |             | |B| |A|       | |   |1|0|E|W|A|            |
; * | BASE 31..24 |G|/|0|V| LIMIT |P|DPL|  TYPE   | BASE 23:16 |
; * |             | |D| |L| 19..16| |   |1|1|C|R|A|            |
; * ------------------------------------------------------------
; * |                             |                            |
; * |        BASE 15..0           |       LIMIT 15..0          |
; * |                             |                            |
; * ------------------------------------------------------------
; *
; *  Note the ordering of the data items is reversed from the above
; *  description.
; */

gdt     desc  <0,0,0,0,0,0>                  ;
        desc  <0FFFFh,0,0,09Ah,0CFh,0>       ; flat DS
        desc  <0FFFFh,0,0,092h,0CFh,0>       ; flat CS
        desc  <0FFFFh,?,?,09Eh,0h,?>         ; 16-bit real mode CS
        desc  <0FFFFh,?,?,092h,0h,?>         ; 16-bit real mode DS
ifdef NO_PROT
        desc  <0FFFFh,?,?,09Eh,0h,?>         ; 16-bit real mode CS
        desc  <0FFFFh,?,?,092h,0h,?>         ; 16-bit real mode DS
endif

gdtsize equ   ($ - gdt)                      ; GDT size

gdtdesc gdtr  <gdtsize - 1, gdt>

address dd    ?
        dw    ?

_DATA   ends

        end

;
; init.asm
; stage0 startup
;

name init

public  __CHK
public  __I8LS

public  real_test
public  call_rm

public  pmode
public  stage0_init

public  _text16_begin
extrn   _text16_end  :dword
extrn   init         :near
extrn   bss_end      :dword
extrn   preldr0_end  :dword

include fsd.inc

.386p

_TEXT16 segment byte public 'CODE' use16

_text16_begin:

;
; stage0 startup
;
stage0_init:
        jmp  short real_start
;
; Configuration variables
;

; this variable is filled with uFSD size,
; if stage0 is loaded concatenated with
; uFSD and needs to be relocated to proper
; addresses, otherwise it must be zero.
uFSD_size     dw    0

real_start:
        ; Set segment registers
        ; to CS value, set stack
        ; to the end of this segment
        mov  ax, cs
        mov  ds, ax

        cli

        mov  ss, ax
        mov  sp, 0ffffh
        mov  bp, sp

        sti

        mov  cx, uFSD_size
        jcxz skip_reloc  ; if uFSD_size == 0, it means that
                         ; uFSD and stage0 are already loaded
                         ; to proper places by the bootsector.
        ; relocate uFSD to UFSD_BASE
        cld


        shr  cx, 1
        inc  cx

        ; preldr0 size = uFSD begin
        mov  esi, offset _TEXT:preldr0_end - STAGE0_BASE

        push si

        mov  eax, UFSD_BASE
        shr  eax, 4
        mov  es,  ax
        xor  di,  di

        rep  movsw

        ; relocate itself to STAGE0_BASE
        pop  cx          ; stage0 length

        push cx

        shr  cx, 1
        inc  cx

        xor  si, si

        mov  eax, STAGE0_BASE
        shr  eax, 4
        mov  es, ax
        xor  di, di

        rep  movsw
skip_reloc:
        ; clear BSS
        mov  ecx, offset _TEXT:bss_end
        mov  eax, offset _TEXT:preldr0_end
        sub  ecx, eax    ; BSS length
        mov  ebx, STAGE0_BASE
        sub  eax, ebx
        shr  ebx, 4
        mov  es, bx
        mov  edi, eax
        xor  ax, ax

        rep  stosb

        ; jump to the new location
        push es
        push reloc
        retf
reloc:
        ; Set segment registers
        mov  ax, cs
        mov  ds, ax
        mov  es, ax

        cli

        mov  ss, ax
        mov  sp, STAGE0_LEN

        sti

        ; call 32-bit protected mode init
        mov  eax, offset _TEXT:init
        push eax
        call call_pm
        add  sp, 4
;
;        ; pass structures to os2ldr
;        mov  dh, flags
;        mov  dl, boot_device
;
;        lds  si, bpb_ptr
;        les  di, ft_ptr
;
;        mov  ebx, offset _TEXT:lip
;        mov  eax, BOOT_MAGIC
;
;        ; return to os2ldr
;        push LDR_SEG
;        push 0
;        retf

loop1:
        hlt
        jmp loop1
;
; void __cdecl real_test(void);
;

real_test:
        retf

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
        mov  eax, offset _TEXT:gdtdesc - STAGE0_BASE
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
        mov  esi, offset _TEXT:address - STAGE0_BASE
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
        mov  eax, STAGE0_BASE
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
        mov  eax, STAGE0_BASE
        shr  eax, 4
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax
        ; do a far jump to load a
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
        ; protected mode CS
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

_TEXT16 ends


_TEXT   segment byte public 'CODE' use32

;
; pmode:
; takes function entry point as ULONG
; in ebp.
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
        add  esp, 14      ; 6 bytes are arguments and 8 bytes --
                          ; return address from rmode_switch
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

test_func    proc near
        ret
test_func    endp

__CHK:
__I8LS:
        ret  4

_TEXT   ends

_DATA   segment dword public 'DATA' use32

FSYS_BUFLEN     equ 8000h
FSYS_BUF        equ 68000h

CR0_PE_ON       equ 01h
CR0_PE_OFF      equ 0fffffffeh
PROT_MODE_CSEG  equ 08h
PROT_MODE_DSEG  equ 10h
PSEUDO_RM_CSEG  equ 18h
PSEUDO_RM_DSEG  equ 20h
STACKOFF        equ (2000h - 10h)
PROTSTACKINIT   equ 50000h        ; (FSYS_BUF - 10h)

align 4

protstack         dd   PROTSTACKINIT
rmstack           dd   0
boot_drive        dd   0

align 4

;
; segment descriptor structure
;
desc struc
ds_limit          dw   ?
ds_baselo         dw   ?
ds_basehi1        db   ?
ds_acclo          db   ?
ds_acchi          db   ?
ds_basehi2        db   ?
desc ends

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
        desc  <0FFFFh,2000h,03h,09Eh,0h,0>   ; 16-bit real mode CS   ; 9Ah,40h
        desc  <0FFFFh,2000h,03h,092h,0h,0>   ; 16-bit real mode DS   ; 92h,40h

gdtsize equ   ($ - gdt)                           ; GDT size

gdtr struc
g_limit dw    ?                              ; limit
g_base  dd    ?                              ; address
gdtr ends

gdtdesc gdtr  <gdtsize - 1, gdt>

address dd    ?
        dw    ?

_DATA   ends

        end stage0_init

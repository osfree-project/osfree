;
; init.asm
; stage0 real-mode startup
;

name init

public  stage0_init
public  force_lba
public  real_test

public  _text16_begin
extrn   _text16_end  :dword
extrn   bss_end      :dword
extrn   preldr0_end  :dword

extrn   gdt          :byte
extrn   init         :near
extrn   call_pm      :near
extrn   message      :far

include fsd.inc
include struc.inc

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
stage0_size   dw    0
force_lba     db    0

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
        mov  si, stage0_size

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

        ; set 16-bit segment (_TEXT16) base
        ; in GDT for protected mode
        mov  eax, STAGE0_BASE
        mov  esi, offset _TEXT:gdt - STAGE0_BASE
        mov  [si][3*8].ds_baselo, ax
        mov  [si][4*8].ds_baselo, ax
        ror  eax, 16
        mov  [si][3*8].ds_basehi1, al
        mov  [si][4*8].ds_basehi1, al
        ror  eax, 8
        mov  [si][3*8].ds_basehi2, al
        mov  [si][4*8].ds_basehi2, al

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
        mov     esi, offset _TEXT:msg - STAGE0_BASE
        callf   message
        retf

_TEXT16 ends

_TEXT   segment byte public 'CODE' use32
_TEXT   ends

_DATA   segment byte public 'DATA' use32

msg     db    "Hello from protected mode!",0

_DATA   ends

        end stage0_init

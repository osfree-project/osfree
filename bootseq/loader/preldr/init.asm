;
; init.asm
; stage0 real-mode startup
;

name init

public  base

public  _preldr

public  boot_flags
public  boot_drive
public  install_partition
public  install_filesys

public  stage0_init
public  force_lba

public  filetab_ptr
public  filetab_ptr16

public  _text16_begin
extrn   _text16_end  :dword
extrn   bss_end      :dword
extrn   exe_end  :dword

extrn   gdtaddr      :dword
extrn   gdtsrc       :byte
extrn   gdtdesc      :fword
extrn   init         :near
extrn   call_pm      :near
extrn   lip1         :dword

extrn   mem_lower    :dword

extrn   preldr_ss_sp :dword
extrn   preldr_ds    :word
extrn   preldr_es    :word

ifndef STAGE1_5

public  ft
public  jmp_reloc

extrn   mu_Open      :far
extrn   mu_Read      :far
extrn   mu_Close     :far
extrn   mu_Terminate :far

extrn   printhex8    :far
extrn   printhex4    :far
extrn   printhex2    :far

endif

include fsd.inc
include struc.inc
include loader.inc
include bpb.inc

.386p

_TEXT16 segment dword public 'CODE' use16

_text16_begin:

;
; stage0 startup
;
stage0_init:
        jmp  short real_start
;
; Configuration variables
;

; variables block size
CONF_VARS_SIZE     equ  40h

; this variable is filled with uFSD size,
; if stage0 is loaded concatenated with
; uFSD and needs to be relocated to proper
; addresses, otherwise it must be zero.
uFSD_size          dw    0
stage0_size        dw    0
force_lba          db    0

                   org   8h

base               dd    STAGE0_BASE
install_part       dd    0ffffffh
                   db    '/boot/loader/preldr0.mdl'
                   db    4 dup (0)
install_fsys       db    "iso9660"
                   db    9 dup (0)

;padsize  equ  CONF_VARS_SIZE - ($ - stage0_init - 2)
;pad           db    padsize dup (0)
                   org   40h
real_start:
        ; Set segment registers
        ; to CS value, set stack
        ; to the end of this segment
        xor  eax, eax
        xor  ebx, ebx
        mov  ax,  cs
        shl  eax, 4
        call getaddr
        ; now rel0 absolute address is in EBX
rel0:
        add  eax, ebx
        add  eax, rel1 - rel0
        ; now phys addr of rel1 is in EAX
        mov  ebx, rel1 - stage0_init
        sub  eax, ebx
        shr  eax, 4
        push ax
        push bx
        ; now ax:bx is the rel1 address, and ax:0 is the
        ; beginning of the executable
        retf
rel1:
        pusha
        mov     al, '['
        mov     bx, 1
        mov     ah, 0Eh
        int     10h
        popa

        mov  bx, ds
        mov  cx, es

        mov  ds, ax
        mov  es, ax

        cli

        mov  ss, ax
        mov  sp, 0ffffh
        mov  bp, sp

        sti

        ; Save physical boot drive
        mov  eax, offset _TEXT:boot_drive - STAGE0_BASE
        mov  byte ptr [eax], dl

        ; save boot flags in dx
        mov  eax, offset _TEXT:boot_flags - STAGE0_BASE
        mov  word ptr [eax], dx

        mov  eax, offset _TEXT:bpb_ptr - STAGE0_BASE
        mov  word ptr [eax + 2], bx
        mov  word ptr [eax], si

        cmp  dh, 0
        jz   non_16bit_ufsd

        ; are minifsd and microfsd bits set?
        and  dh, 14h
        cmp  dh, 14h
        jne  non_16bit_ufsd

        ; we're started from old-type 16-bit uFSD

        ; copy to 32-bit variable
        mov  eax, offset _TEXT:filetab_ptr - STAGE0_BASE
        mov  word ptr [eax + 2], cx
        mov  word ptr [eax], di

        ; copy to 16-bit variable
        mov  eax, offset _TEXT16:filetab_ptr16
        mov  word ptr [eax + 2], cx
        mov  word ptr [eax], di
non_16bit_ufsd:
        ; copy install_part to 32-bit segment
        mov  eax, offset _TEXT:install_partition - STAGE0_BASE
        mov  ebx, dword ptr install_part
        mov  dword ptr [eax], ebx


        ; copy install filesystem name to 32-bit segment
        cld
        lea  esi, install_part
        mov  edi, offset _TEXT:install_partition - STAGE0_BASE
        mov  cx, 16 + 32
        rep  movsb

        mov  cx, uFSD_size
        jcxz skip_reloc_ufsd  ; if uFSD_size == 0, it means that
                              ; uFSD and stage0 are already loaded
                              ; to proper places by the bootsector.

        ; relocate uFSD to EXT_BUF_BASE
        shr  cx, 1
        inc  cx

        ; preldr0 size = uFSD begin
        mov  si, stage0_size
        ;push si

        mov  eax, EXT_BUF_BASE
        shr  eax, 4
        mov  es,  ax
        xor  di,  di

        rep  movsw

        ;pop  ds
skip_reloc_ufsd:
        ; relocate itself to STAGE0_BASE
        ;pop  cx          ; stage0 length
        mov  cx, stage0_size
        jcxz skip_reloc_stage0

        ;push cx

        shr  cx, 1
        inc  cx

        xor  si, si

        mov  eax, STAGE0_BASE
        shr  eax, 4
        mov  es, ax
        xor  di, di

        rep  movsw

skip_reloc_stage0:
        ; clear BSS
        ;mov  ecx, offset _TEXT:bss_end
        ;mov  eax, offset _TEXT:exe_end
        ;sub  ecx, eax    ; BSS length
        ;mov  ebx, STAGE0_BASE
        ;sub  eax, ebx
        ;shr  ebx, 4
        ;mov  es, bx
        ;mov  edi, eax
        ;xor  ax, ax

        ;rep  stosb

        ;push es

        ; clear BSS of uFSD
        ;mov  bx, 7c00h
        ;mov  es, bx
        ;xor  bx, bx
        ;mov  edi, dword ptr es:[bx + 2] ; bss start
        ; now es:di->bss
        ;mov  ecx, dword ptr es:[bx + 6] ; bss end
        ;sub  ecx, edi
        ; now ecx contains bss length
        ;xor  eax, eax

        ;rep  stosb

        ;pop  es

        ; relocate boot sector to safe place
        push ds
        push es

        mov  cx, 100h
        xor  ax, ax
        mov  ds, ax
        mov  si, 7C00h
        mov  di, ax
        mov  eax, BOOTSEC_BASE
        shr  eax, 4
        mov  es, ax

        rep  movsw

        pop  es
        pop  ds

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

        ; init stack position variables
        push ds

        xor  ax, ax
        mov  ds, ax
        mov  eax, PROTSTACK
        mov  dword ptr [eax], PROTSTACKINIT

        pop  ds

        ; copy GDT
        push es

        xor  ax, ax
        mov  es, ax

        cld
        mov  cx, 0x180
        mov  esi, offset _TEXT:gdtsrc  - STAGE0_BASE
        mov  edi, GDT_ADDR

        push di
        rep  movsb
        pop  di

        ; set 16-bit segment (_TEXT16) base
        ; in GDT for protected mode
        mov  eax, STAGE0_BASE

        mov  es:[di][3*8].ds_baselo, ax
        mov  es:[di][4*8].ds_baselo, ax
        mov  es:[di][5*8].ds_baselo, ax
        ror  eax, 16
        mov  es:[di][3*8].ds_basehi1, al
        mov  es:[di][4*8].ds_basehi1, al
        mov  es:[di][5*8].ds_basehi1, al
        ror  eax, 8
        mov  es:[di][3*8].ds_basehi2, al
        mov  es:[di][4*8].ds_basehi2, al
        mov  es:[di][5*8].ds_basehi2, al

        pop  es

        ; fill GDT descriptor
        mov  ebx, offset _TEXT:gdtdesc - STAGE0_BASE
        mov  eax, GDT_ADDR
        mov  [bx].g_base, eax

;ifndef STAGE1_5
        ; get available memory
        call getmem
;endif
        ; enable A20 address line
        ;call EnableA20Line

        pusha
        mov     al, '*'
        mov     bx, 1
        mov     ah, 0Eh
        int     10h
        popa

        ; call 32-bit protected mode init
        mov  eax, offset _TEXT:init
        push eax
        call call_pm
        add  sp, 4

        pusha
        mov     al, ']'
        mov     bx, 1
        mov     ah, 0Eh
        int     10h
        popa

ifndef STAGE1_5
        ; save pre-loader segment registers and stack
        mov  ax, ds
        mov  word ptr preldr_ds, ax
        mov  ax, es
        mov  word ptr preldr_es, ax
        mov  ax, sp
        mov  word ptr preldr_ss_sp, ax
        mov  ax, ss
        mov  word ptr preldr_ss_sp + 2, ax
endif
        ;
        ; pass structures to os2ldr
        ;
        xor  eax, eax
        mov  eax, offset _TEXT:boot_flags - STAGE0_BASE
        ; set bootflags
        mov  dx, [eax]

        ; set bootdrive
        mov  eax, offset _TEXT:boot_drive - STAGE0_BASE
        mov  dl, [eax]

        mov  edi, offset _TEXT:ft - STAGE0_BASE

        ; set BPB
        mov  eax, BOOTSEC_BASE
        shr  eax, 4
        mov  ds,  ax
        mov  si,  0bh           ; 3 + 8 = 11 -- BPB offset from the beginning of boot sector

        ; set LIP pointer and magic
        mov  ebx, offset _TEXT:lip1
        mov  eax, BOOT_MAGIC

        ; return to os2ldr
        push OS2LDR_SEG
        push 0

        retf

;else

; Control transferring from lite version to full one

;endif

;ifndef STAGE1_5
getmem:
        xor  eax, eax
        int  12h
        mov  ebx, offset _TEXT:mem_lower - STAGE0_BASE
        mov  [ebx], eax
        ret
;endif

;EnableA20Line:
;    ; Enable A20 address line:
;    push ax
;    in   al, 0x92
;    or   al, 2
;    out  0x92, al
;    pop  ax
;
;    ret

getaddr:
    mov  bp, sp
    mov  bx, [bp]
    ret

;fname   db "/boot/bootblock",0
;fsize   dd 0

filetab_ptr16      dd 0

_TEXT16 ends

_TEXT   segment dword public 'CODE' use32

; these we get on entry to a pre-loader
bpb_ptr            dd 0         ; pointer to the BPB
filetab_ptr        dd 0         ; pointer to the FileTable. If <> 0 then we got called from 16-bit uFSD
boot_flags         dw 0         ; <-- DX
boot_drive         dd 0         ; <-- DL
; copied from pre-loader header
_preldr            label byte
install_partition  dd 0
                   db 28 dup (?)
install_filesys    db 16 dup (?)
ft                 FileTable <>

ifndef STAGE1_5
;
; void jmp_reloc(unsigned long addr);
;
jmp_reloc:
        add   [esp], eax
        ret

endif

_TEXT   ends

        end stage0_init

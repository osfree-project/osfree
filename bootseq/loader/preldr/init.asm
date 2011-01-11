;
; init.asm
; stage0 real-mode startup
;

name    init

public  base

public  _preldr

public  boot_drive

public  stage0_init
public  force_lba

public  relshift16
public  rs

public  install_partition

public  _text16_begin
extrn   _text16_end  :dword
extrn   bss_end      :near
extrn   exe_end      :near

extrn   gdtaddr      :dword
extrn   gdtsrc       :byte
extrn   gdtdesc      :fword
extrn   init         :near
extrn   call_pm      :near
extrn   call_rm      :near
extrn   lip1         :dword

ifndef STAGE1_5

public  bpb_ptr
public  filetab_ptr
public  filetab_ptr16

public  install_filesys
public  boot_flags

;extrn   mem_lower    :dword

extrn   preldr_ss_sp :dword
extrn   preldr_ds    :word
extrn   preldr_es    :word

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
include mb_info.inc
include mb_header.inc

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
bundle             db    0

                   org   8h

base               dd    STAGE0_BASE
                   db    '/boot/loader/preldr0.mdl'
                   db    4 dup (0)
install_part       dd    0ffffffh
install_fsys       db    "iso9660"
                   db    9 dup (0)

                   org   38h

rs                 dd    0
relshift16         dd    0

;padsize  equ  CONF_VARS_SIZE - ($ - stage0_init - 2)
;pad           db    padsize dup (0)
                   org   40h

ifndef STAGE1_5

; Multiboot header
__magic          equ     0x1badb002
__flags          equ     0x00010001
__checksum       equ     - __magic - __flags
__start          equ     KERN_BASE                                           ; executable address in memory
__exe_end        equ     (offset _TEXT:exe_end + KERN_BASE - STAGE0_BASE)        ; executable end address
__bss_end        equ     (offset _TEXT:bss_end + KERN_BASE - STAGE0_BASE)        ; bss end address
__entry          equ     (offset _TEXT:kernel_entry + KERN_BASE - STAGE0_BASE)             ; entry point

__mbhdr          multiboot_header  <__magic,__flags,__checksum,__start+__mbhdr,__start,__exe_end+70000h,__bss_end+70000h,__entry,0,0,0,0>

endif

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
        ; EAX contains our segment address
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
        mov  bx, ds
        mov  cx, es

        mov  ds, ax
        mov  es, ax

        cli

        mov  ss, ax
        mov  sp, 0ffffh
        mov  bp, sp

        sti

        mov  al, '['
        call charout

;;;;;;;;;;;;;;
        pusha

        ;mov  cx, uFSD_size
        ;jcxz skip_reloc_ufsd  ; if uFSD_size == 0, it means that
        ;                      ; uFSD and stage0 are already loaded
        ;                      ; to proper places by the bootsector.

        cmp  bundle, 0
        jz   skip_reloc_ufsd

        xor  eax, eax
        mov  ax,  cs
        mov  ds,  ax

        mov  ebx, eax
        shl  ebx, 4

        mov  eax, STAGE0_BASE
        mov  esi,  offset _TEXT:exe_end
        sub  esi,  eax

        mov  cx, EXT_LEN

        ; relocate uFSD to EXT_BUF_BASE
        shr  cx, 1
        inc  cx

        ; preldr0 size = uFSD begin
        ;mov  si, stage0_size
        ;;push si

        mov  eax, EXT_BUF_BASE
        shr  eax, 4
        mov  es,  ax
        xor  di,  di

        push esi
        rep  movsw
        pop  esi

        mov  ecx,  esi

        ;pop  ds
skip_reloc_ufsd:
        ; relocate itself to STAGE0_BASE

        ;mov  cx, stage0_size
        ;jcxz skip_reloc_stage0

        mov  eax, STAGE0_BASE

        cmp  eax, ebx
        jz   skip_reloc_stage0

        shr  cx, 1
        inc  cx

        xor  si, si

        ;mov  eax, STAGE0_BASE
        shr  eax, 4
        mov  es, ax
        xor  di, di

        rep  movsw

skip_reloc_stage0:
;;;;;;;;;
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

        popa

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

        sub  ax, 1060h       ; 0x1000 + 0x20
        mov  ss, ax
        mov  sp, 0ffffh

        sti

        push  es
        pusha

        ; clear BSS
        mov  ecx, offset _TEXT:bss_end
        mov  eax, offset _TEXT:exe_end
        sub  ecx, eax    ; BSS length
        mov  ebx, STAGE0_BASE
        sub  eax, ebx
        shr  ebx, 4
        mov  es, bx
        mov  edi, eax
        xor  eax, eax

        rep  stosb

        ;push es
        ;push ebx

        ; clear BSS of uFSD
        mov  bx, 73e0h
        mov  es, bx
        xor  bx, bx
        mov  edi, dword ptr es:[bx + 5] ; bss start
        ; now es:di->bss
        mov  ecx, dword ptr es:[bx + 9] ; bss end
        sub  ecx, edi
        ; now ecx contains bss length
        xor  eax, eax

        rep  stosb

        ;pop  ebx
        ;pop  es

        popa
        pop   es

        ; Save physical boot drive
        mov  eax, offset _TEXT:boot_drive - STAGE0_BASE
        mov  dword ptr [eax], 0
        mov  byte  ptr [eax], dl

ifndef STAGE1_5

        ; save boot flags in dx
        mov  eax, offset _TEXT:boot_flags - STAGE0_BASE
        mov  word ptr [eax], dx

        mov  eax, offset _TEXT:bpb_ptr - STAGE0_BASE
        mov  word ptr [eax + 2], bx
        mov  word ptr [eax], si
endif
        mov  rs, 0

        cmp  dh, 0
        jz   non_16bit_ufsd

        ; are minifsd and microfsd bits set?
        and  dh, 14h
        cmp  dh, 14h
        jne  non_16bit_ufsd

        ; we're started from old-type 16-bit uFSD

ifndef STAGE1_5

        ; copy to 32-bit variable
        mov  eax, offset _TEXT:filetab_ptr - STAGE0_BASE
        mov  word ptr [eax + 2], cx
        mov  word ptr [eax], di

        ; copy to 16-bit variable
        mov  eax, offset _TEXT16:filetab_ptr16
        mov  word ptr [eax + 2], cx
        mov  word ptr [eax], di

endif

non_16bit_ufsd:
;        ; copy install_part to 32-bit segment
;        mov  eax, offset _TEXT:install_partition - STAGE0_BASE
;        mov  ebx, dword ptr install_part
;        mov  dword ptr [eax], ebx
;
;
;        ; copy install filesystem name to 32-bit segment
;        cld
;        lea  esi, install_part
;        mov  edi, offset _TEXT:install_partition - STAGE0_BASE
;        mov  cx, 16 + 32
;        rep  movsb

ifndef STAGE1_5

        cld
        lea  esi, install_part
        mov  edi, offset _TEXT:install_partition - STAGE0_BASE
        mov  cx, 20
        rep  movsb

endif
        call set_gdt
ifndef STAGE1_5
        ; get available memory
        ;call getmem
endif
        ; enable A20 address line
        ;call EnableA20Line

        mov     al, '*'
        call    charout

        ;xor edi, edi
        ; call 32-bit protected mode init
        mov  eax, offset _TEXT:init
        push eax
        call call_pm
        add  sp, 4

        mov  al, ']'
        call charout

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

        mov  edi, offset _TEXT:ft - STAGE0_BASE

        ;
        ; pass structures to os2ldr
        ;
        xor  eax, eax
        mov  eax, offset _TEXT:boot_flags - STAGE0_BASE
        ; set bootflags
        mov  dx, [eax]

endif

        ; set bootdrive
        mov  eax, offset _TEXT:boot_drive - STAGE0_BASE
        mov  dl, [eax]

        ; set BPB
        ;mov  eax, BOOTSEC_BASE
        ;shr  eax, 4
        mov  ax,  ds
        sub  ax,  20h
        mov  ds,  ax
        mov  si,  0bh           ; 3 + 8 = 11 -- BPB offset from the beginning of boot sector

ifndef STAGE1_5
        ; set LIP pointer and magic
        mov  ebx, offset _TEXT:lip1
        mov  eax, BOOT_MAGIC
endif

        ; return to os2ldr
        push OS2LDR_SEG
        push 0

        retf

;else

; Control transferring from lite version to full one

;endif

;ifndef STAGE1_5
;getmem:
;        xor  eax, eax
;        int  12h
;        mov  ebx, offset _TEXT:mem_lower - STAGE0_BASE
;        ;lea  ebx, _TEXT:mem_lower
;        ;sub  ebx, STAGE0_BASE
;        mov  [ebx], eax
;        ret
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

;
; get pre-loader offset in EBX
; we assume that it got control
; via a RETF instruction, so its
; address remains on stack
;
getaddr:
        mov  bp, sp
        mov  bx, [bp]
        ret

charout:
        pusha
        mov     bx, 1
        mov     ah, 0Eh
        int     10h
        popa
        ret

;fname   db "/boot/bootblock",0
;fsize   dd 0

set_gdt:
        ; init stack position variables
        push ds

        mov  ax, ss

        xor  bx, bx
        mov  ds, bx
        mov  ebx, PROTSTACK
        mov  dword ptr [ebx], PROTSTACKINIT
        mov  word ptr ds:[RMSTACK + 2], ax

        pop  ds

        ; copy GDT
        push es

        ;xor  bx, bx
        ;mov  es, bx
        ;
        ;cld
        ;mov  cx, 0x180
        ;mov  esi, offset _TEXT:gdtsrc  - STAGE0_BASE
        ;mov  edi, GDT_ADDR
        ;
        ;push di
        ;rep  movsb
        ;pop  di

        ; set 16-bit segment (_TEXT16) base
        ; in GDT for protected mode
        mov  ebx, STAGE0_BASE
        shl  eax, 4

        ;mov  esi, ebx
        ;shr  esi, 4
        ;mov  es, si
        mov  edi, offset _TEXT:gdtsrc - STAGE0_BASE

        mov  es:[di][3*8].ds_baselo, bx
        mov  es:[di][4*8].ds_baselo, bx
        mov  es:[di][5*8].ds_baselo, ax
        ror  eax, 16
        ror  ebx, 16
        mov  es:[di][3*8].ds_basehi1, bl
        mov  es:[di][4*8].ds_basehi1, bl
        mov  es:[di][5*8].ds_basehi1, al
        ror  eax, 8
        ror  ebx, 8
        mov  es:[di][3*8].ds_basehi2, bl
        mov  es:[di][4*8].ds_basehi2, bl
        mov  es:[di][5*8].ds_basehi2, al

        pop  es

        ; fill GDT descriptor
        mov  ebx, offset _TEXT:gdtdesc - STAGE0_BASE
        ;mov  eax, GDT_ADDR
        mov  eax, offset _TEXT:gdtsrc
        mov  [bx].g_base, eax

        lgdt fword ptr [ebx]

        ret

ifndef STAGE1_5

filetab_ptr16      dd 0

endif

_TEXT16 ends

_TEXT   segment dword public 'CODE' use32

; these we get on entry to a pre-loader
boot_drive         dd 0         ; <-- DL
; copied from pre-loader header
_preldr            label byte
                   db    '/boot/loader/preldr0.mdl'
                   db    4 dup (0)

ifndef STAGE1_5

install_partition  dd    0
install_filesys    db    "iso9660"
                   db    9 dup (0)

bpb_ptr            dd 0         ; pointer to the BPB
filetab_ptr        dd 0         ; pointer to the FileTable. If <> 0 then we got called from 16-bit uFSD
boot_flags         dw 0         ; <-- DX
ft                 FileTable <>

errmsg             db   "This is not a multiboot loader or no LIP module!",0
VIDEO_BUF          equ  0b8000h

;
; void jmp_reloc(unsigned long addr);
;
jmp_reloc:
        add   [esp], eax
        ret

; Multiboot kernel entry
kernel_entry:
        cmp   eax, MULTIBOOT_VALID
        jne   stop

        cli

        mov   esp, 10000h

        push  edx

        ; copy myself to the intended address
        mov   edi, STAGE0_BASE
        mov   ecx, offset _TEXT:exe_end
        sub   ecx, STAGE0_BASE
        mov   esi, KERN_BASE
        rep   movsb

        mov   edx, [ebx].mods_addr       ; 1st module address

        ; copy uFSD (1st module)
        mov   edi, EXT_BUF_BASE
        mov   ecx, [edx].mod_end
        mov   esi, [edx].mod_start
        sub   ecx, esi
        rep   movsb

        call  set_gdt2

        pop   edx

        mov  eax, STAGE0_BASE - 600h
        mov  esp, eax

        mov   eax, STAGE0_BASE
        shr   eax, 4
        push  ax
        xor   ax, ax
        push  ax
        call  call_rm
        add   esp, 4

        cli
        hlt

stop:
        cld
        lea   esi, errmsg
        mov   edi, VIDEO_BUF
        mov   ah, 02h  ; attribute
loop1:
        lodsb          ; symbol
        stosw
        test  al, al   ; copy a string to video buffer
        jnz   loop1

        cli
        hlt

set_gdt2:
        ; fix gdt descriptors base
        mov  edi, offset _TEXT:gdtsrc

        mov  ebx, STAGE0_BASE
        mov  eax, ebx
        sub  eax, 10600h       ; 0x10000 + 0x200

        mov  [edi][3*8].ds_baselo, bx
        mov  [edi][4*8].ds_baselo, bx
        mov  [edi][5*8].ds_baselo, ax
        ror  eax, 16
        ror  ebx, 16
        mov  [edi][3*8].ds_basehi1, bl
        mov  [edi][4*8].ds_basehi1, bl
        mov  [edi][5*8].ds_basehi1, al
        ror  eax, 8
        ror  ebx, 8
        mov  [edi][3*8].ds_basehi2, bl
        mov  [edi][4*8].ds_basehi2, bl
        mov  [edi][5*8].ds_basehi2, al

        ; fill GDT descriptor
        mov  eax, edi
        mov  ebx, offset _TEXT:gdtdesc
        mov  [ebx].g_base, eax

        lgdt fword ptr [ebx]

        ret

endif

_TEXT   ends

        end stage0_init

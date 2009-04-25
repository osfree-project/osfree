;
; linux.asm
; linux multiboot kernel startup
;

name linux

.386p

include fsd.inc
include linux.inc
include struc.inc
include mb_info.inc
include mb_header.inc
include mb_etc.inc

include bpb.inc

extrn ldr_drive       :dword
extrn boot_drive      :dword
extrn call_rm         :near
extrn cmain           :near
extrn exe_end         :near
extrn bss_end         :near
extrn gdtsrc          :byte
extrn gdtdesc         :fword
extrn l               :dword
extrn m               :dword
extrn lip_module_present :byte
;extrn grub_printf_    :near

public base
public force_lba
public stop
public start_kernel
;public printk

;
; bpb.inc: data structures
;

disk_addr_packet struc
pkt_size         db          ?
reserved         db          ?
num_blocks       dw          ?
buffer           dd          ?
starting_block   dq          ?
disk_addr_packet ends

ext_params struc
;
; bp register points here
;
force_chs        db               ?
drive            db               ?
part             db               3 dup (?)
BootPart         db               ?
BootDev          db               ?
disk_addr_pkt    disk_addr_packet <>

;
; bootsector boundary
; (at address 0x0:0x7c00)
;
jump             dw               ?
force_lba1       db               ?
oemid            db               8 dup (?)
bpb              bios_parameters_block <>
pad              db               (512 - 2 - 40h - 0 - (size bios_parameters_block) - 8 - 3) dup (?)
parttable        db               40h      dup (?)
bootsig          dw               0aa55h
ext_params ends

EXT_PARAMS_OFFS  equ (2 + 3 + 2 + size (disk_addr_packet))

                ;
                ; A macro to check if a partition is extended
                ;

check_ext macro lbl                                        ;
                mov  bh, byte ptr [di + 4]                 ; Type of partition
                cmp  bh, 05h                               ; si --> partition descriptor in MBR or EBR.
                je   lbl                                   ;
                cmp  bh, 0fh                               ; if part. is extended, then goto lbl
                je   lbl                                   ;
          endm                                             ;

K_RDWR          equ     0x60    ; keyboard data & cmds (read/write)
K_STATUS        equ     0x64    ; keyboard status
K_CMD           equ     0x64    ; keybd ctlr command (write-only)

K_OBUF_FUL      equ     0x01    ; output buffer full
K_IBUF_FUL      equ     0x02    ; input buffer full

KC_CMD_WIN      equ     0xd0    ; read  output port
KC_CMD_WOUT     equ     0xd1    ; write output port
KB_OUTPUT_MASK  equ     0xdd    ; enable output buffer full interrupt
                                ;   enable data line
                                ;   enable clock line
KB_A20_ENABLE   equ     0x02

_TEXT16  segment para public 'CODE'  use16
_TEXT16  ends
_TEXT    segment para public 'CODE'  use32
_TEXT    ends
_DATA    segment para public 'DATA'  use32
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
_STACK   segment dword public 'STACK' use32
_STACK   ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_end1,_end2,_BSS,_end3,_end4,_STACK

_STACK   segment dword public 'STACK' use32
STACK_SIZE equ 4000h
public stack_top
public stack_bottom
stack_bottom     db STACK_SIZE dup (?)
stack_top        label byte
_STACK   ends

_TEXT16  segment para public 'CODE' use16
begin:

        org   0h
start:
        jmp   real_start
        org   10h
base    dd    REAL_BASE
        org   20h
real_start:

.386
        mov  ax, 9000h                             ; Set stack
        mov  bp, 0x8000 - EXT_PARAMS_OFFS

        cli                                        ; Disable interrupts
        mov  ss, ax                                ;
        mov  sp, bp                            ; to above the ext_params
        sti                                        ; Enable interrupts

        mov  ds, ax
        mov  es, ax

        mov   edx, ebx 
        shr   edx, 24                              ; extract drive number

        mov   eax, ebx
        and   eax, 0ffffffh                      ; extract partition number

        cmp   eax, 0x00ffffffh
        je    no_hiddensecs

        shr   eax, 16

        cmp   eax, 0ffh
        jne   skip1
        xor   eax, eax
        jmp   short detect_lba_chs
skip1:
        inc   eax
        mov   esi, eax
detect_lba_chs:
        ;
        ; Test if LBA
        ; is supported
        ;

        mov  ah, 41h                               ; try 41h of int 13h
        mov  bx, 55aah                             ; (probe if LBA is available)
        int  13h

        jc   short use_chs
        cmp  bx, 0aa55h
        jne  short use_chs
        cmp  ah, 21h                               ; is EDD 1.1 supported?
        jb   short use_chs                         ;
        and  cx, 1                                 ; check bit 0 of cx register,
        jz   short use_chs                         ; if set then int13 ext disk read
                                                           ; functions are supported
use_lba:
.386
        mov  bl, 0
        jmp  short switchBootDrv
use_chs:
        mov  bl, 1
switchBootDrv:

        mov  [bp].force_chs, bl

        ; dl      --> drive we booted from (set by BIOS when control is given to MBR code)
        ; BootDev --> drive to continue booting from (set in MBR sector field)

        lea  di, [bp].parttable                         ; di --> parttable at 0x0:0x7c00 + 0x1be, si = 0x1be

        ;
        ; From here, ds:si address partition table entry
        ; for current partition
        ;

        ; Choose hard disk to continue booting from
        ; and load its MBR into scratch sector

        mov  [bp].drive, dl
        mov  eax, esi
        mov  byte ptr [bp].BootPart, al

        ;
        ; descriptor of the MBR of hard disk to continue load from
        ; in the format of Partition table partition descriptor,
        ; pointed by ds:si:
        ;

        mov  byte ptr [di + 1], 0                  ; Head 0
        mov  word ptr [di + 2], 1h                 ; Cyl  0, Sec 1 (like ch = 00, cl = 01 in int 13h)
        mov dword ptr [di + 8], 0                  ; LBA

        call ReadSec                               ; Load the MBR of the HDD we continue booting from

        lea  di, [bp].parttable

searchPartition:
        ;
        ; Now let's search partition descriptor in MBR or EBR
        ;

        mov  cx, 4                                 ; number of entries in parttable

        mov  al, byte ptr [bp].BootPart                 ; now ax = BootPart
        cbw                                        ; (partition to continue booting from)

selectPart:
        ; Find the selected partition
        ; (with number in BootPart)

        cmp  ax, 4
        jbe  primaryPart

        sub  ax, 5                                 ; BootPart is now the number of logical part. beginning from 0
logicalPart:

findExtended:   ; 1st find extended partition in PT

        check_ext extendedFound                    ; If partition is extended, goto extendedFound
        add  di, 10h
        loop findExtended

        jmp  part_not_found                        ; Ext. part. not found, panic
extendedFound:                                             ; Ext part. found
        mov  cx, ax                                ; Set loop counter to logical part. number
        mov  eax, [di + 8]

        call ReadSec                               ; Read 1st EBR into scratch sector

searchEBR:                                         ; find the EBR of needed partition
        lea  di, [bp].parttable                    ; si --> partition table of EBR

        jcxz ebrFound

        dec  cx
        
        add  di, 10h                               ; ds:si --> reference to the next EBR

newEbrFound:                                               ; found a reference to the next EBR
        add  dword ptr [di + 8], eax               ; now
                                                           ; dword ptr [di + 8] --> beginning of the next EBR

        test cx, cx                                ;
        jnz  skip                                  ; Save the beginning of
        mov  eax, [di + 8]
skip:
        call ReadSec                               ; Read the next EBR
        jmp  searchEBR
ebrFound:                                                  ; It is a EBR of our partition, let's take part. descriptor
        ;add  dword ptr [di + 8], eax                 ; dx:ax --> beginning of our partition
        add  eax, dword ptr [di + 8]

        mov  bx, 1                                 ; Logical partition condition is TRUE

        ;
        ; now ds:si --> descriptor, pointing to the beginning of the needed logical partition
        ;

        jmp  bootFound                             ;
primaryPart:
        ; Find primary partition MBR PT entry

        dec  ax
        mov  bh, 10h
        mul  bh
        add  di, ax

        mov  bx, 0                                 ; Logical partition condition is FALSE
bootFound:
        ; Boot partition found                     ; ds:si --> Part. descriptor of boot partition
        ;call ReadSec                               ; Read bootsector into 0x7c0:0x0

        test bx, bx
        jz   nofix_hiddensecs

        ;
        ; Fix hiddensectors value if booting from logical partition
        ;

        xor  bx, bx
        mov  ds, bx       
        mov  si, 0x7c00 - EXT_PARAMS_OFFS
 
        mov  dword ptr [si].bpb.hidden_secs, eax

nofix_hiddensecs:
        ;
        ; Fix diskNum and logDrive values
        ;

        mov  dl, [bp].drive
        mov  byte ptr [si].bpb.disk_num, dl

        mov  bl, [bp].part
        inc  bl
        inc  bl
        or   bl, 80h
        mov  [si].bpb.log_drive, bl

        ;mov  eax, dword ptr [bp].drive
        ;mov  [bp].drivepart, eax

no_hiddensecs:
        push 0                                     ; zero ds
        push 7c00h                                 ;

        ;mov  al, 'J'
        ;call err
                
        retf                                       ; "return" to bootsector

err_read:
        mov  al, 'R'
        jmp  short Err
part_not_found:
        mov  al, 'P'
Err:
        mov  ah, 0eh
        xor  bx, bx
        int  10h

        jmp  short $

;
; ReadSec:
;
;               Reads a sector
;               at the beginning of the partition,
;               corresponding to the partition table entry at ds:[si]
;               to scratch sector  at 0x7c0:0x0.
;
;               es:di --> current partition table entry
;

ReadSec proc near
        mov  dl, [bp].drive                        ; Drive Number

        cmp  byte ptr [bp].force_chs, 0            ; LBA or CHS?
        jnz  chs
lba:
        pushad
        call ReadSecLBA                            ; Read by LBA
        popad

        jc   short chs                             ; if LBA fails, fallback to CHS

        ret
chs:
        pushad
        call ReadSecCHS                            ; Read by CHS
        popad

        jc   short err_read
return_lb:
        ret
ReadSec endp


;
; ReadSecLBA:
;
;               Reads a sector
;               using LBA,
;               to 0x7c0:0x0
;               at the beginning of the partition,
;               described by part table entry at ds:[si]
;
;               es:di --> partition table entry
;

ReadSecLBA proc near
        ;
        ; int 13h 42h function
        ; Input:
        ;
        ;            ah = 42h
        ;            dl = drive number
        ;            ds:si = pointer to disk address packet
        ;
        ; Returns:
        ;
        ;            al = 0 if success,
        ;            error code otherwise

        
        lea  si, [bp].disk_addr_pkt                ; disk address packet

        mov  word ptr [si].pkt_size, 10h           ; size of packet absolute starting address

        mov  eax, [di + 8]                         ;
        mov  dword ptr [si].starting_block, eax    ; LBA of the 1st
        mov  dword ptr [si].starting_block + 4, 0  ;

        mov  word ptr [si].num_blocks, 1           ; number of blocks to transfer
        mov  word ptr [si].buffer, 8000h           ; offset of disk read buffer = 0x7c00
        mov  word ptr [si].buffer + 2, 9000h       ; segment of disk read buffer at 0x0

        mov  ax, 4200h

        int  13h
lb2:
        ret

ReadSecLBA endp


;
; ReadSecCHS:
;
;               Reads a sector
;               using CHS,
;               to 0x7c0:0x0
;               at the beginning of the partition,
;               described by part table entry at ds:[si]
;
;               ds:si --> partition descriptor in PT
;

ReadSecCHS proc near
        ;
        ; phys disk no. in dl
        mov  cx, word ptr [di + 2]                 ; cyl. and sector
        mov  dh, byte ptr [di + 1]                 ; head no.

        mov  bx, 7c00h

        mov  ax, 0201h                             ; ah = 2 -- function and al = 1 -- count of sectors
        int  13h                                   ; read one sector at 0x7c0:0x0

        ret
ReadSecCHS endp

up1:
        mov     bx, 1
        mov     ah, 0Eh
        int     10h             ; display a byte

message:
        lodsb
        or      al, al
        jne     up1              ; if not end of string, jmp to display
        ret

;
; printhex[248]: Write a hex number in (AL, AX, EAX) to the console
;

printhex2:
        pusha
        rol     eax, 24
        mov     cx, 2
        jmp     pp1
printhex4:
        pusha
        rol     eax, 16
        mov     cx, 4
        jmp     pp1
printhex8:
        pusha
        mov     cx, 8
pp1:
        rol     eax, 4
        push    eax
        and     al, 0Fh
        cmp     al, 10
        jae     high1
low1:
        add     al, '0'
        jmp     pp2
high1:
        add     al, 'A' - 10
pp2:
        mov     bx, 0001h
        mov     ah, 0Eh
        int     10h              ; display a char
        pop     eax
        loop    pp1
        popa

        ret


gateA20_rm:
        mov     ax, 2400h
        test    dx, dx
        jz      ft1
        inc     ax
ft1:    stc
        int     15h
        jnc     ft2

        ; set non-zero if failed
        mov     ah, 1

        ; save the status
ft2:    mov     dl, ah

        retf

hard_stop:
        cli
        hlt

stop_flop:
        xor     dl, dl
        int     13h
        retf

force_lba       db  0
;pad1size        equ STARTUP_SIZE - ($ - start)
;pad1size        equ _TEXT16_SIZE - ($ - start)
;pad1            db  pad1size dup (0)
;pad1size        equ  100h
;pad1            db   pad1size dup (0)
_TEXT16  ends


_TEXT    segment para public 'CODE'  use32

BASE1           equ     KERN_BASE - 0x10000
VIDEO_BUF       equ     0xb8000

        org     BASE1
start1:

        org     BASE1 + _TEXT16_SIZE

start2:

align 4

; Multiboot header
_magic          equ       0x1badb002
_flags          equ       0x00010001
_checksum       equ       - _magic - _flags
_mbhdr          multiboot_header  <_magic,_flags,_checksum,_mbhdr,start1,exe_end,bss_end,entry,0,0,0,0>

        org     start2 + 0x80

        align   0x10
entry:
        cmp   eax, MULTIBOOT_VALID
        jne   stop

        ; save multiboot structure address
        mov   m, ebx

        mov   byte ptr boot_drive, dl

        call  set_gdt

        ; set stack
        ;mov   esp, stack_top
        ; at the moment, we use loader stack

        ; copy realmode part of boot_chain at REAL_BASE
        cld
        mov     ecx, 0x100
        mov     esi, KERN_BASE
        mov     edi, REAL_BASE

        rep     movsd

        call    cmain

        ; We should not return here
        cli
        hlt
;        jmp     $

;printk:
;        cmp     lip_module_present, 0
;        je      nopr
;        call    grub_printf_
;nopr:
;        ret

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
;        jmp   $

.386p
set_gdt:
        ; set 16-bit segment (_TEXT16) base
        ; in GDT for protected mode
        ;lea  eax, oldgdtdesc
        ;sgdt fword ptr [eax]

        ; copy old gdt
        ;movzx ecx, [eax].g_limit
        ;inc  ecx
        ;shr  ecx, 2
        ;mov  esi, [eax].g_base
        ;mov  edi, offset _TEXT:gdt
        ;mov  ebx, edi
        ;rep  movsd

        ; fix gdt descriptors base
        mov  ebx, GDT_ADDR
        mov  eax, REAL_BASE
        mov  [ebx][8*8].ds_baselo, ax
        mov  [ebx][9*8].ds_baselo, ax
        ror  eax, 16
        mov  [ebx][8*8].ds_basehi1, al
        mov  [ebx][9*8].ds_basehi1, al
        ror  eax, 8
        mov  [ebx][8*8].ds_basehi2, al
        mov  [ebx][9*8].ds_basehi2, al

        ; fill GDT descriptor
        mov  ebx, offset _TEXT:gdtdesc
        mov  eax, GDT_ADDR
        mov  [ebx].g_base, eax

        lgdt fword ptr [ebx]

        ret

;
; void start_kernel(void);
;
start_kernel:
        xor  eax, eax
        push eax
        call gateA20
        add  eax, 4

        mov  ebx, ldr_drive
        mov  eax, REAL_BASE
        shl  eax, 12
        mov  ax, offset _TEXT16:start
        push eax
        call call_rm
        add  esp, 4

        ;ret

        ; we should not return here
        cli
        hlt

;
; gateA20(int linear)
;
; Gate address-line 20 for high memory.
;
; This routine is probably overconservative in what it does, but so what?
;
; It also eats any keystrokes in the keyboard buffer.  :-(
;

gateA20:
        ; first, try a BIOS call
        push    ebp
        mov     edx, [esp + 8]

        mov     eax, REAL_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:gateA20_rm
        push    eax
        call    call_rm
        add     esp, 4

        pop     ebp
        test    dl, dl
        jnz     ft3

        ret

ft3:    ; use keyboard controller
        push    eax

        call    gloop1

        mov     al, KC_CMD_WOUT
        out     K_CMD, al

gloopint1:
        in      al, K_STATUS
        and     al, K_IBUF_FUL
        jnz     gloopint1

        mov     al, KB_OUTPUT_MASK
        cmp     byte ptr [esp + 8], 0
        jz      gdoit

        or      al, KB_A20_ENABLE
gdoit:
        out     K_RDWR, al

        call    gloop1

        ; output a dummy command (USB keyboard hack)
        mov     al, 0ffh
        out     K_CMD, al
        call    gloop1

        pop     eax

        ret

gloop1:
        in      al, K_STATUS
        and     al, K_IBUF_FUL
        jnz     gloop1

gloop2:
        in      al, K_STATUS
        and     al, K_OBUF_FUL
        jz      gloop2ret
        in      al, K_RDWR
        jmp     gloop2

gloop2ret:
        ret

;
;  stop_floppy()
;
;  Stops the floppy drive from spinning, so that other software is
;  jumped to with a known state.
;
stop_floppy:
        pusha
        mov     eax, REAL_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:stop_flop
        push    eax
        call    call_rm
        add     esp, 4
        popa
        ret

errmsg  db  "This is not a multiboot loader or no LIP module!",0

oldgdtdesc gdtr <>

_TEXT    ends

         end entry

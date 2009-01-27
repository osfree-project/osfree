;
; An MBR, which loads a bootsector from
; active or selected primary/logical partition
; of arbitrary hard disk.
; (c) osFree project
; valerius, 2006/10/08
;

.386

.model tiny

public start

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

include bpb.inc

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
disk_addr_pkt    disk_addr_packet <>
;
; bootsector boundary
; (at address 0x0:0x7c00)
;
jump             dw               ?
nop1             db               ?
oemid            db               8 dup (?)
bpb              bios_parameters_block <>
; here the bootsector code starts. if bit 7 of [bp].bpb.log_drive is set
; then the following filelds are valid (they overwrite the bootsector code):
;drivepart        dd               ?
ext_params ends

EXT_PARAMS_SIZE  equ (size ext_params - size bios_parameters_block - 11)

DGROUP group MbrSeg

MbrSeg segment para public 'CODE' USE16
       assume cs:MbrSeg, ds:MbrSeg, es:MbrSeg ,ss:MbrSeg

                org 7c00h
start proc far
                xor  ax, ax                                ; Set stack
                mov  bp, 7c00h - EXT_PARAMS_SIZE           ; now bp points at the ext_params

                cli                                        ; Disable interrupts
                mov  ss, ax                                ;
                mov  sp, bp                                ; to above the ext_params
                sti                                        ; Enable interrupts

                mov  ds, ax
                mov  es, ax

                cld
                ; scan strings forward
                mov  si, 7c00h                             ;
                mov  di, 600h                              ;
                mov  cx, 100h                              ; Relocate itself
                rep  movsw                                 ;

                push ax
                push 600h + (boot - start)                 ;

                retf                                       ; "return" to the new location at 0x0:0x600 + boot
boot:
.386
                mov  ax, 3000h                             ; InfoSeg
                mov  fs, ax                                ; with LBA flag
.286
                ;
                ; cs:ip --> 0x0:0x600 + boot
                ; ss:sp --> stack with ss = 0x0, bottom = 0x7bff
                ;

probeLba:
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
                mov  eax, 'X31I'
                mov  bl, 0
                jmp  short switchBootDrv
use_chs:
                xor  eax, eax
                mov  bl, 1
switchBootDrv:
                mov  fs:[0], eax

                mov  [bp].force_chs, bl

                ; dl      --> drive we booted from (set by BIOS when control is given to MBR code)
                ; BootDev --> drive to continue booting from (set in MBR sector field)

                lea  di, parttable                         ; di --> parttable at 0x0:0x7c00 + 0x1be, si = 0x1be

                ;
                ; From here, ds:si address partition table entry
                ; for current partition
                ;

                ; Choose hard disk to continue booting from
                ; and load its MBR into scratch sector

                cmp  byte ptr BootDev, dl                  ; compare disk to continue booting from with disk MBR loaded from

                mov  [bp].drive, dl

                mov  al, BootPart
                mov  byte ptr [bp].part, al
                mov  word ptr [bp].part + 1, 0ffffh

                jz   searchPartition                       ; if they're equal, then no need to change them.
                mov  dl, BootDev                           ; else set drive to BootDev

                ;
                ; descriptor of the MBR of hard disk to continue load from
                ; in the format of Partition table partition descriptor,
                ; pointed by ds:si:
                ;

                mov  byte ptr [di + 1], 0                  ; Head 0
                mov  word ptr [di + 2], 1h                 ; Cyl  0, Sec 1 (like ch = 00, cl = 01 in int 13h)
                mov dword ptr [di + 8], 0                  ; LBA

                call ReadSec                               ; Load the MBR of the HDD we continue booting from
searchPartition:
                ;
                ; Now let's search partition descriptor in MBR or EBR
                ;

                mov  cx, 4                                 ; number of entries in parttable

                mov  al, byte ptr BootPart                 ; now ax = BootPart
                cbw                                        ; (partition to continue booting from)

                ;pop  ds                                    ; ds --> ScrSeg = 0x7c0 (points to the new MBR)

                cmp  ax, 0                                 ;
                jz   findActive                            ; if BootPart = 0, then boot from active partition
                jmp  selectPart                            ; else select partition with number in BootPart byte.

                ; Find the active
                ; partition
findActive:
                ;
                ; now es:di --> PT of the new MBR
                ;
lp3:
                mov  bh, byte ptr [di]                     ; partition status
                test bh, 80h                               ; if active bit set
                jnz  lpEnd

                add  di, 10h                               ; go to the next partition
                loop lp3

                jmp  Err$PartNotFound                      ; active partition not found, panic
lpEnd:                                                     ; active partition found
                mov  bx, 0                                 ; Logical partition condition is FALSE

                jmp  bootFound
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

                jmp  Err$PartNotFound                      ; Ext. part. not found, panic
extendedFound:                                             ; Ext part. found
                mov  cx, ax                                ; Set loop counter to logical part. number

                ;mov  ax, word ptr [di + 10]                ; save LBA address of the beginning
                ;shl  eax, 10h
                ;mov  ax, word ptr [di + 8]                 ; of extended partition in dx:ax
                mov  eax, [di + 8]

                call ReadSec                               ; Read 1st EBR into scratch sector

searchEBR:                                                 ; find the EBR of needed partition
                lea  di, parttable                         ; si --> partition table of EBR

                jcxz ebrFound

                dec  cx

                add  di, 10h                               ; ds:si --> reference to the next EBR

newEbrFound:                                               ; found a reference to the next EBR
                add  dword ptr [di + 8], eax               ; now
                                                           ; dword ptr [di + 8] --> beginning of the next EBR

                test cx, cx                                ;
                jnz  skip                                  ; Save the beginning of
                ;mov  ax, word ptr [di + 8]                 ; our EBR in dx:ax
                ;shl  eax, 10h
                ;mov  ax, word ptr [di + 10]                ;
                mov  eax, [di + 8]
skip:
                call ReadSec                               ; Read the next EBR
                jmp  searchEBR
ebrFound:                                                  ; It is a EBR of our partition, let's take part. descriptor
                add  dword ptr [di + 8], eax                 ; dx:ax --> beginning of our partition

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

if 0

                mov  cx, 10h                               ; partition descriptor size

                push di
checkZero:                                                 ; check if part. desc. is zero
                cmp  byte ptr [di], 0
                jnz  endCheck
                inc  di
                loop checkZero

                jmp  Err$PartNotFound                       ; partition doesn't exist, its desc = 0
endCheck:
                pop  di

endif

                mov  bx, 0                                 ; Logical partition condition is FALSE
bootFound:
                ; Boot partition found                     ; ds:si --> Part. descriptor of boot partition
                call ReadSec                               ; Read bootsector into 0x7c0:0x0

                test bx, bx
                jz   nofix_hiddensecs

                ;
                ; Fix hiddensectors value if booting from logical partition
                ;

                add  dword ptr [bp].bpb.hidden_secs, eax

nofix_hiddensecs:
                ;
                ; Fix diskNum and logDrive values
                ;

                mov  dl, [bp].drive
                mov  byte ptr [bp].bpb.disk_num, dl

                mov  bl, [bp].part
                inc  bl
                inc  bl
                or   bl, 80h
                mov  [bp].bpb.log_drive, bl

                ;mov  eax, dword ptr [bp].drive
                ;mov  [bp].drivepart, eax

                push ds                                    ; zero ds
                push 7c00h                                 ;

                retf                                       ; "return" to bootsector
start endp


Err$Read:
                mov  al, 'R'
                jmp  short Err
Err$PartNotFound:
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

                jc   short Err$Read
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
                mov  word ptr [si].buffer, 7c00h           ; offset of disk read buffer = 0x7c00
                mov  word ptr [si].buffer + 2, 0           ; segment of disk read buffer at 0x0
                mov  ah, 42h

                int  13h
lb2:
                ret

ReadSecLBA endp


if 0

;
; printhex[248]: Write a hex number in (AL, AX, EAX) to the console
;

;printhex2:
;        pusha
;        rol     eax, 24
;        mov     cx, 2
;        jmp     short pp1
printhex4:
        pusha
        rol     eax, 16
        mov     cx, 4
;        jmp     pp1
;printhex8:
;        pusha
;        mov     cx, 8
pp1:
        rol     eax, 4
        push    eax
        and     al, 0Fh
        cmp     al, 10
        jae     short high1
low1:
        add     al, '0'
        jmp     short pp2
high1:
        add     al, 'A' - 10
pp2:
        mov     bx, 0001h
        mov     ah, 0Eh
        int     10h              ; display a char
        pop     eax
        loop    short pp1
        popa

        ret

endif


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

padSize        equ         512 - ($ - start + signature - vars + 2)        ; padding size

if padSize gt 0
   codeEnd         db          padSize  dup (0)            ; pad by zeroes
endif

if padSize lt 0
   error    "MBR does not fit into 512 bytes!"
endif

;
; Some variables
;

vars            label       byte
                                                           ; Boot partition number,
                                                           ; if = 0, then boot from active partition.
BootPart        db          0                              ; if 1 to 4, then it is a number of
                                                           ; primary partition. If > 4, then it is a
                                                           ; number of logical partition

BootDev         db          80h                            ; Boot device in BIOS int 13h format

;
; Partition table
;

parttable       db          40h      dup (?)               ; Partition table
signature       db          055h,0aah                      ; MBR signature

MbrSeg ends

       end start

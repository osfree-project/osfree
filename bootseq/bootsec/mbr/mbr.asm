;
; An MBR, which loads a bootsector from
; active or selected primary/logical partition
; of arbitrary hard disk.
; (c) osFree project
; valerius, 2006/10/08
;

.286

.model tiny

public start

SecSize        equ         200h                            ; Sector size

ScrSeg         equ         7c0h                            ; MBR load segment (aka scratch sector)
mbrAddr        equ         ScrSeg * 10h                    ; MBR load address
newSeg         equ         60h                             ; MBR relocation segment
NewAddr        equ         newSeg * 10h                    ; MBR relocation address

                ;
                ; A macro to check if a partition is extended
                ;

check_ext macro lbl                                        ;
                mov  bh, byte ptr [si + 4]                 ; Type of partition
                cmp  bh, 05h                               ; si --> partition descriptor in MBR or EBR.
                je   lbl                                   ;
                cmp  bh, 0fh                               ; if part. is extended, then goto lbl
                je   lbl                                   ;
          endm                                             ;

DGROUP group MbrSeg

MbrSeg segment para public 'CODE' USE16
       assume cs:MbrSeg, ds:MbrSeg, es:PktSeg ,ss:nothing

start proc far

                cli                                        ; Disable interrupts
                                                           ;
                xor  ax, ax                                ; Set stack
                mov  ss, ax                                ;
                mov  sp, mbrAddr - 1                       ; to above the bootsector
                                                           ;
                sti                                        ; Enable interrupts

		;mov  ds, ax
		;mov  es, ax

                cld                                        ; scan strings forward
                mov  si, mbrAddr + boot                    ;
                mov  di, NewAddr + boot                    ;
                mov  cx, SecSize                           ; Relocate itself
                sub  cx, boot                              ; to address 0x60:0x0
                push newSeg                                ;
                push boot                                  ;
                rep  movsb                                 ;

                retf                                       ; "return" to the new location at 0x60:0x0 + boot
boot:
                ;
                ; cs:ip --> 0x60:0x0 + boot
                ; ss:sp --> stack with ss = 0x0, bottom = 0x7bff
                ;

                push NewSeg
                pop  ds
                push PktSeg
                pop  es

                push ScrSeg

                ;
                ; ds --> 0x60
                ; es --> 0x7e0
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
                jnz  short use_chs
                cmp  ah, 21h                               ; is EDD 1.1 supported?
                jb   short use_chs                         ;
                and  cx, 1                                 ; check bit 0 of cx register,
                jz   short use_chs                         ; if set then int13 ext disk read
                                                           ; functions are supported
use_lba:
.386
                mov  eax, 58333149h                        ; i13X
                mov  bl, 0
                jmp  short switchBootDrv
use_chs:
                xor  eax, eax
                mov  bl, 1
switchBootDrv:
                push 03000h
                pop  fs
                mov  fs:[0], eax
.286
                mov  byte ptr es:UseCHS, bl
                ; dl      --> drive we booted from (set by BIOS when control is given to MBR code)
                ; BootDev --> drive to continue booting from (set in MBR sector field)

                lea  si, parttable                         ; si --> parttable at 0x60:0x1be, si = 0x1be

                ;
                ; From here, ds:si address partition table entry
                ; for current partition
                ;

                ; Choose hard disk to continue booting from
                ; and load its MBR into scratch sector

                cmp  byte ptr BootDev, dl                  ; compare disk to continue booting from with disk MBR loaded from
                jz   searchPartition                       ; if they're equal, then no need to change them.
                mov  dl, BootDev                           ; else set drive to BootDev

                ;
                ; descriptor of the MBR of hard disk to continue load from
                ; in the format of Partition table partition descriptor,
                ; pointed by ds:si:
                ;

                mov  byte ptr [si + 1], 0                  ; Head 0
                mov  word ptr [si + 2], 1h                 ; Cyl  0, Sec 1 (like ch = 00, cl = 01 in int 13h)
                mov dword ptr [si + 8], 0                  ; LBA

                call ReadSec                               ; Load the MBR of the HDD we continue booting from
searchPartition:
                ;
                ; Now let's search partition descriptor in MBR or EBR
                ;

                mov  cx, 4                                 ; number of entries in parttable

                mov  al, byte ptr BootPart                 ; now ax = BootPart
                cbw                                        ; (partition to continue booting from)

                pop  ds                                    ; ds --> ScrSeg = 0x7c0 (points to the new MBR)

                cmp  ax, 0                                 ;
                jz   findActive                            ; if BootPart = 0, then boot from active partition
                jmp  selectPart                            ; else select partition with number in BootPart byte.

                ; Find the active
                ; partition
findActive:
                ;
                ; now ds:si --> PT of the new MBR
                ;
lp3:
                mov  bh, byte ptr [si]                     ; partition status
                test bh, 80h                               ; if active bit set
                jnz  lpEnd

                add  si, 10h                               ; go to the next partition
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
                add  si, 10h
                loop findExtended
                jmp  Err$PartNotFound                      ; Ext. part. not found, panic
extendedFound:                                             ; Ext part. found
                mov  cx, ax                                ; Set loop counter to logical part. number

                mov  dx, word ptr [si + 10]                ; save LBA address of the beginning
                mov  ax, word ptr [si + 8]                 ; of extended partition in dx:ax

                call ReadSec                               ; Read 1st EBR into scratch sector

searchEBR:                                                 ; find the EBR of needed partition
                lea  si, parttable                         ; si --> partition table of EBR

                jcxz ebrFound

                dec  cx

                add  si, 10h                               ; ds:si --> reference to the next EBR

newEbrFound:                                               ; found a reference to the next EBR
                add  word ptr [si + 8], ax                 ; now
                adc  word ptr [si + 10], dx                ; dword ptr [si + 8] --> beginning of the next EBR

                test cx, cx                                ;
                jnz  skip                                  ; Save the beginning of
                mov  ax, word ptr [si + 8]                 ; our EBR in dx:ax
                mov  dx, word ptr [si + 10]                ;
skip:
                call ReadSec                               ; Read the next EBR
                jmp  searchEBR
ebrFound:                                                  ; It is a EBR of our partition, let's take part. descriptor
                add  word ptr [si + 8], ax                 ; dx:ax --> beginning of our partition
                adc  word ptr [si + 10], dx                ;

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
                add  si, ax

if 0

                mov  cx, 10h                               ; partition descriptor size

                push si
checkZero:                                                 ; check if part. desc. is zero
                cmp  byte ptr [si], 0
                jnz  endCheck
                inc  si
                loop checkZero
                jmp Err$PartNotFound                       ; partition doesn't exist, its desc = 0
endCheck:
                pop  si

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

                add  word ptr hiddenSecs, ax
                adc  word ptr hiddenSecs + 2, dx

nofix_hiddensecs:
                ;
                ; Fix diskNum and logDrive values
                ;

                mov  dl, byte ptr cs:BootDev
                mov  byte ptr diskNum, dl

                mov  bl, byte ptr cs:BootPart
                inc  bl
                inc  bl
                or   bl, 80h
                mov  byte ptr logDrive, bl

                push ds                                    ; ScrSeg = 0x7c0
                push 0                                     ; address

                retf                                       ; "return" to bootsector
start endp

if 0

Err proc near
                assume ds:NewSeg
Err$Read:
                mov  al, 'R'
                lea  si, Err_DskReadError
                jmp  short entry
Err$PartNotFound:
                mov  al, 'P'
                lea  si, Err_PartNotFound
entry:
                push NewSeg
                pop  ds
beginloop:
                cld
Disp1:
                lodsb
                test al, al
                jz Disp2
                mov ah, 0eh                                ; int 10h/ah=0eh: write char on screen in teletype mode
                xor bx, bx
                push si
                int 10h
int10_exit:
                pop si
                jmp Disp1
Disp2:
                hlt
Err endp

else

Err proc near
Err$Read:       mov  al, 'R'
                jmp  short entry
Err$PartNotFound:
                mov  al, 'P'
entry:
                mov  ah, 0eh
                xor  bx, bx
                int  10h
int10_exit:
                hlt
Err endp

endif

;
; ReadSec:
;
;               Reads a sector
;               at the beginning of the partition,
;               corresponding to the partition table entry at ds:[si]
;               to scratch sector  at 0x7c0:0x0.
;
;               ds:si --> current partition table entry
;               es    --> PktSeg

ReadSec proc near
                push dx
                mov  dl, byte ptr cs:BootDev               ; Drive Number

                cmp  byte ptr cs:ForceLBA, 0               ; force LBA?
                jnz  lba

                cmp  byte ptr es:UseCHS, 0                 ; LBA or CHS?
                jz   lba
chs:
                push es
                pusha
                call ReadSecCHS                            ; Read by CHS
                popa
                pop  es
                jmp  return_lb
lba:
                push es
                pusha
                call ReadSecLBA                            ; Read by LBA
                popa
                pop  es
                jnc  return_lb                             ; if LBA fails, fallback to CHS
                jmp  chs
return_lb:
                pop  dx

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
;               ds:si --> partition table entry
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
                push ds
                pop  es

                assume ds:PktSeg

                push PktSeg
                pop  ds

                mov  di, si                                ; save part. desc. in di

                lea  si, disk_addr_pkt                     ; disk address packet
                mov  word  ptr [si], 10h                   ; size of packet absolute starting address

                mov  ax, word ptr es:[di + 8]              ;
                mov  word ptr [si + 8], ax                 ; LBA of the 1st
                mov  ax, word ptr es:[di + 10]             ; sector of partition (bootsector)
                mov  word ptr [si + 10], ax                ;

                mov  word ptr [si + 2], 1                  ; number of blocks to transfer
                mov  word ptr [si + 6], ScrSeg             ; segment of disk read buffer at 0x7c0
                mov  word ptr [si + 4], 0                  ; offset of disk read buffer = 0x0
                mov  ah, 42h
                int  13h
lb2:
                mov  si, di                                ; restore part. descriptor in si

                push es
                pop  ds

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
                                                           ; phys disk no. in dl
                mov  cx, word ptr [si + 2]                 ; cyl. and sector
                mov  dh, byte ptr [si + 1]                 ; head no.
                push ScrSeg                                ;
                pop  es                                    ; buffer address
                xor  bx, bx                                ; in es:bx

                mov  ax, 0201h                             ; ah = 2 -- function and al = 1 -- count of sectors
                int  13h                                   ; read one sector at 0x7c0:0x0

                jnc  noerr1
                jmp  Err$Read
noerr1:
                cmp  ah, 0
                jz   noerr2
                jmp  Err$Read
noerr2:
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

;Err_DskReadError db         "DRE", 0                       ; Disk read error
;Err_PartNotFound db         "PNF", 0                       ; Partition not found
BootDev         db          80h                            ; Boot device in BIOS int 13h format
forceLBA        db          0                              ; force LBA mode flag

;
; Partition table
;

parttable       db          40h      dup (?)               ; Partition table
signature       db          055h,0aah                      ; MBR signature

MbrSeg ends

;
; Additional data. Not included in MBR.
;

PktSeg segment at 7e0h
disk_addr_pkt   db          10h     dup (0)                ; disk address packet structure for LBA mode
UseCHS          db          0                              ; whether to use CHS
PktSeg ends

;
; Bootsector
;

BootSeg segment at 7c0h
                db          1ch     dup (?)
hiddenSecs      dd          ?
                dd          ?
diskNum         db          ?
logDrive        db          ?
BootSeg ends

;
; The segment of MBR relocation
;

NewSeg segment at 60h
NewSeg ends

;InfoSeg segment at 3000h
;I13XFlag        dd  ?
;InfoSeg ends

       end start

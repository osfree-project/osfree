;
; +------------+
; |bootsect.asm|
; +------------+
; A universal bootsector.
; ~~~~~~~~~~~~~~~~~~~~~~
; There are three versions of this bootsector:
; 1) version 1:
; Loads a program as a contiguous sequence of sectors from
; disk at arbitrary address, aligned to paragraph boundary,
; (default is 0x800:0x0), and gives control to it (via far jump).
; A file is loaded by logical sector number (32-bit LSN offset
; from the beginning of logical disk) and a length value in
; sectors, which are written by sysinstx utility to a boot
; sector.
; 2) version 2:
; Loads a program by allocation map, the program may be frag-
; mented; the address of each program's sector is written by
; sysinstx utility to a special allocation map sector, which
; can reside in any disk region, like bootblock or ordinary
; file, which is 512 bytes in size. The load address is by
; default 0x820:0x0, and allocation map is loaded at 0x800:0x0.
; The entry point of a file can also have an arbitrary value
; (default is 0x0). The file must be <= 64Kb in size.
; 3) version 3:
; Loads a program by allocation map. Differs from previous
; version in that the size of a file is not limited to 64 Kb.
; This version specifies in its map not only sector number, but
; beginning sectors and a length of extent. The map file is
; not restricted to one sector, but can be of any length.
; Each map file sector is linked to the next map file sector.
; Primary use of this bootsector is to use it in OS/3 boot
; sequence to load a blackbox (aka microFSD) code from arbitrary
; file or other location like partition boot block.
; The sector is file system independent.
;
; (c) osFree project
; valerius, 2006/10/03,
; support of fragmented uFSD file
; added 2006/11/12
;
; Note:
; N-th version of a bootsector can be made from this source
; by specifying "-dCODE_N" preprocessor define, N = 1..3.
;

.286

.model tiny

public boot

DGROUP group BootSeg

BootSeg segment para public 'CODE' USE16
        assume cs:BootSeg,ds:BootSeg,es:nothing,ss:nothing

boot proc far

                jmp     short bootStrap                    ; Jump to bootstrap procedure
                nop                                        ;
                                                           ;
SysId           db     '[[OS/3]]'                          ; System Identifier

; Standard BPB                                             ;

BPB             label       byte                           ; Begin of Boot parameters block
sectSize        dw          200h                           ; Sector size in bytes
clusSize        db          1                              ; Cluster size in sectors
resSectors      dw          1                              ; Reserved sectors
nFats           db          2                              ; The number of FAT copies
rootDirSize     dw          224                            ; Max. no of root directory entries
nSect           dw          2*18*80                        ; No. of sectors of disk, if its size
                                                           ; is less then 32M
mediaDesc       db          0f0h                           ; Media descriptor byte
fatSize         dw          9                              ; FAT size in sectors
trackSize       dw          18                             ; Number of sectors in track
headsCnt        dw          2                              ; Number of surfaces
hiddenSecs      dd          0                              ; Number of hidden sectors
nSectExt        dd          0                              ; Number of sectors on disk, if its
                                                           ; size exceeds 32M
; Extended BPB

diskNum         db          0                              ; 00 -- floppy; 80 -- hard drive
logDrive        db          0                              ; if (logDrive & 80h) = true, (logDrive & 7Fh) is
                                                           ; the number of logical disk (c: is 00)
marker          db          29h                            ;
volSerNo        dd          ?                              ; Volume serial number
volLabel        db          11   dup (?)                   ; Volume label
fsName          db          8    dup (?)                   ; File system name

ifndef CODE_3
ifdef IBM_FAT_COMPAT
;
; Additional fields (not included in [Ext]BPB)
; (for compatibility with IBM's FAT blackbox;
; without them, there will be errors SYS2025
; and SYS2027 from the blackbox)
;

dataAreaOfs     dw          0                              ; beginning of the file area
rootDirSeg      dw          1000h                          ; root directory segment
rootDirOfs      dw          0                              ; the number of 1st sector of root directory
rootDirSecs     dw          0                              ; size of root dir in sectors

else

reserved        db          28 dup (?)                     ; reserved space for FAT32 and NTFS

endif
endif


;
; Additional variables to pass to blackbox
;

; ... (none for now) ...

bootStrap:
                cli                                        ; Disable interrupts

                mov  ax, 0                                 ;
                mov  ss, ax                                ; Set stack
                mov  sp, btAddr - 1                        ; to above the bootsector

                sti                                        ; Enable interrupts

                mov  ax, btSeg
                mov  ds, ax
                ;push btSeg
                ;pop  ds

                push ds                                    ; This is to properly set cs register
                push offset start                          ; to the segment of a bootsector
                retf
start:
;                push ds
;
;ifdef CODE_1
;                push muFSDLoadSeg
;else
;                push mapLoadSeg
;endif

                ;mov  dl, diskNum
                test dl, 80h
                jnz  short no_floppy
                call floppy_Init                           ; Set floppy parameters, if booting from floppy
                jmp  short use_chs                         ; floppy drive -- use CHS
no_floppy:
                mov  ah, 41h                               ; try 41h of int 13h
                mov  bx, 55aah                             ; (probe if LBA is available)
                int  13h

                jc   short use_chs
                cmp  bx, 0aa55h
                jnz  short use_chs

                and  cx, 1                                 ; check bit 0 of cx register,
                jz   short use_chs                         ; if set then int13 ext disk read
                                                           ; functions are supported
use_lba:
                mov  bl, 0
                jmp  short return
use_chs:
                mov  bl, 1
return:
                assume ds:BootSeg

                mov  byte ptr [ds:UseCHSAddr], bl
readRoot:

ifdef CODE_1

                mov  bx, dx

ifdef IBM_FAT_COMPAT
                ; Fill in the variables at the
                ; end of the BPB.
                ; (specific for FAT, has no sense when
                ; booting from other FS; here we calculate
                ; parameters for FAT root directory)

                mov  cx, rootDirSize                       ; rootDirSize * 32 / 512
                mov  dx, cx                                ;
                shr  cx, 4                                 ; = size of root dir in sectors
                and  dx, 0fh
                jz   short no_add                          ;
                inc  cx                                    ; If partial sector, then add 1
no_add:
                mov  rootDirSecs, cx
                mov  ax, fatSize
                mul  nFats
                add  ax, resSectors
                mov  rootDirOfs, ax                        ; dx:ax = LSN of starting sector

                add  ax, rootDirSecs
                mov  dataAreaOfs, ax
endif

read_muFSD:
                ; Read muFSD
                ; at 0x800:0x0
                push cs
                pop  ds

                mov  ax, muFSDLoadSeg
                mov  es, ax                                ; muFSDLoadSeg
                xor  di, di                                ; zero
                                                           ; load muFSD load address
                mov  ax, word ptr muFSDAddr                ; muFSD address, low word
                mov  dx, word ptr muFSDAddr + 2            ; high word
                mov  cl, muFSDLen                          ; Number of sectors to read

                call read_run                               ; read sectors

                mov  si, muFSDEntry                        ;
                ;sub  si, 200h                              ;

                ;mov  ax, es                                ;
                ;add  ax, 20h                               ;
                ;mov  es, ax
                ;push ax                                    ; loader segment
                push es
                push si                                    ; offset

ifdef IBM_FAT_COMPAT
                lea  si, BPB                               ; pass BPB address in ds:si
endif

                mov  dx, bx

                retf                                       ; "return" to micro FSD code

                ;
                ; On exit:
                ;
                ;            ds:si --> BPB at 0x7c0:0xb
                ;            dl    --  int 13h boot drive number
                ;            cs:ip --> muFSD at address 0x800:0x0
else
ifdef CODE_2

read_allocmap:
                ; Read muFSD allocation map
                ; sector at 0x800:0x0
                ;push cs
                ;pop  ds

                mov  ax, mapLoadSeg
                mov  es, ax                                ; mapLoadSeg
                xor  di, di                                ; zero

                mov  bx, dx
                                                           ; load muFSD allocation map address
                mov  ax, word ptr mapAddr                  ; muFSD alloc. map address, low word
                mov  dx, word ptr mapAddr + 2              ; high word
                mov  cx, 1                                 ; Number of sectors to read

                call read_run                              ; read alloc map sector

                push cs                                    ; save ds == 0x7c0

                push es                                    ;
                pop  ds                                    ; ds --> muFSD alloc map

                mov  ax, es                                ; muFSDLoadSeg = mapLoadSeg + 0x20
                add  ax, 20h                               ; es --> 0x820 -- will be a
                mov  es, ax                                ; segment of muFSD load, right
                                                           ; after alloc map segment at 0x800
                xor  si, si
read_loop:
                mov  ax, word ptr [si]                     ; ds:si --> current muFSD sector address
                mov  dx, word ptr [si + 2]                 ; dword ptr [si] ==> dx:ax

                mov  cx, ds                                ;
                pop  ds                                    ; ds == 0x7c0
                push cx                                    ; save old ds == 0x800

                mov  cx, 1                                 ; Sector count
                call read_run                              ; Read next muFSD sector

                mov  cx, ds                                ;
                pop  ds                                    ; Exchange stack top <--> ds
                push cx                                    ;

                add  si, 4                                 ; Next
                add  di, 200h                              ; sector

                cmp  si, 200h                              ; If a sector number
                jae  end_read                              ; is greater than 128?
.386
                cmp  dword ptr [si], 0
.286
                jnz  read_loop
end_read:
                push cs
                pop  ds                                    ; restore ds

call_muFSD:
                mov  si, muFSDEntry                        ;
                ;sub  si, 200h                              ;

                mov  ax, es                                ;
                ;add  ax, 20h                               ;
                ;mov  es, ax
                push ax                                    ; loader segment
                mov  ds, ax
                push si

                mov  dx, bx

ifdef IBM_FAT_COMPAT
                lea  si, BPB                               ; address in ds:si
endif

                retf                                       ; "return" to micro FSD code

                ;
                ; On exit:
                ;
                ;            ds:si --> BPB at 0x7c0:0xb
                ;            dl    --  int 13h boot drive number
                ;            cs:ip --> muFSD at address 0x820:0x0
else
ifdef CODE_3

read_allocmap:
                ; Read muFSD allocation map
                ; sector at 0x800:0x0
                mov  ax, mapLoadSeg
                mov  es, ax                                ; mapLoadSeg

                ;pop  es                                    ; mapLoadSeg

                push dx                                    ; phys disk number
                push ds                                    ; save ds == 0x7c0

                call read_alloc_sector

                push es                                    ;
                pop  ds                                    ; ds --> current muFSD alloc sector

                call adjust_es

                push es
begin_read:
                xor  si, si                                ;

read_loop:
                mov  ax, word ptr [4*si]                   ; ds:si --> current muFSD sector address
                mov  dx, word ptr [4*si + 2]               ; dword ptr [4*si] ==> dx:ax

                mov  cl, byte ptr [bx + si]                ; Sector count
                ;
                ; Read extent
                ;

                push bx

                mov  bx, ds                                ;
                pop  ds                                    ; Exchange stack top <--> ds
                push bx                                    ;

                call read_run                               ; Read next muFSD extent

                mov  bx, ds                                ;
                pop  ds                                    ; Exchange stack top <--> ds
                push bx                                    ;

                pop  bx

                inc  si                                    ; Next sector / extent

                call adjust_es                             ; Adjust es by number of paragraphs read

                cmp  si, 80h                               ; If a sector number
                jae  end_read                              ; is greater than 128?
.386
                cmp  dword ptr [si], 0
.286
                jnz  read_loop
end_read:

.386
                cmp dword ptr mapAddr, 0
.286
                jz   the_end
                call read_alloc_sector
                jmp  begin_read
the_end:
                pop  es                                    ; restore es --> 0x820
                pop  ds                                    ; restore ds --> 0x7c0
                pop  dx                                    ; Physical disk number in dl
call_muFSD:

ifdef IBM_FAT_COMPAT
                lea  si, BPB                               ; address in ds:si
endif

                push es                                    ; muFSD segment
                push muFSDEntry                            ; offset

                retf                                       ; "return" to micro FSD code

                ;
                ; On exit:
                ;
                ;            ds:si --> BPB at 0x7c0:0xb
                ;            dl    --  int 13h boot drive number
                ;            cs:ip --> muFSD at address 0x820:0x0
endif
endif
endif

boot endp

ifdef CODE_3

;
; Loads next map sector into
; the scratch buffer and loads
; lengths array offset into bx
;

read_alloc_sector proc near
                xor  di, di                                ; zero
                                                           ; load muFSD allocation map address
                mov  ax, word ptr mapAddr                  ; muFSD alloc. map address, low word
                mov  dx, word ptr mapAddr + 2              ; high word
                ;mov  cx, 1                                 ; Number of sectors to read

                ;call read_run                              ; read alloc map sector
                call readsec

                mov  bx, 404

                ret
read_alloc_sector endp

;
; Increment es by number of paragraphs read in cx
;

adjust_es proc near
                mov  ax, es
                shl  cx, 5                                 ; cx == number of paragraphs read
                add  ax, cx
                mov  es, ax

                ret
adjust_es endp

endif

.386p

floppy_Init proc near
                push ds

                int  12h                                   ; determine the amount of low memory
                shl  ax, 6                                 ; Set es to last 64k segment
                mov  es, ax                                ; at the end of low memory

                push  BIOSDataSeg                          ;
                pop   ds                                   ; ds --> BIOS data area

                assume ds:BIOSDataSeg

                dec  word ptr ds:memSize                   ; borrow 1 Kb of memory for FDD params table

                assume ds:IntVecSeg

                lds  si, FddParmTblVec                     ; ds:si --> current FDD params table
                xor  di, di                                ; es:di --> new FDD params table
                push di

                cld                                        ; Copy fdd params table
                mov  cx, 11                                ; to the new location
                rep  movsb                                 ;

                assume ds:BootSeg

                pop  di

;                mov  al, byte ptr sectSize                 ;
;                mov  byte ptr es:[di + 3], al              ; Set new values for sector size
                mov  al, byte ptr trackSize                ; and sectors per track in new FDD
                mov  byte ptr es:[di + 4], al              ; params table

                mov  word ptr FddParmTblVec, di            ; Load new FDD params table address
                mov  word ptr FddParmTblVec + 2, es        ; to 1eh interrupt vector

                mov  ax, 0                                 ; Reset
                int  13h                                   ; FDD controller

                jnc  on10
                jmp Err$Read
on10:
                pop  ds

                ret

floppy_Init endp


;
; ReadRunF:
; a wrapper for ReadRun to call from
; other segments (like blackbox code).
;


read_run_f proc far
                call read_run
                retf
read_run_f endp

;
; must I do the function with
; the following interface?:
;
; int read_run(unsigned long lba,
;              unsigned short count,
;              char *readto);
;
;
;


;
; ReadRun:
; Reads a contiguous run of sectors.
;
;          Input:
;          dx:ax  -- logical sector number to read from
;          cx     -- sector count to read 1 <= cx <= 65535
;          es:di  -- address to read to
;          bl     -- drive number
;          ds     == 0x7c0 -- a bootsector segment

read_run proc near
                add  ax, word ptr hiddenSecs               ;
                adc  dx, word ptr hiddenSecs + 2           ; Add hidden sectors value

                push es
                push ds
                push si
                push di
beginRead1:
                call readsec

                add  ax, 1
                adc  dx, 0

                mov  si, es
                add  si, 20h
                mov  es, si

                loop  beginRead1
endRead1:
                pop  di
                pop  si
                pop  ds
                pop  es

                ret
read_run endp


;
; ReadSec:
;
;               Reads a sector
;               from dx:ax lba address on disk
;               to es:di in memory from
;               BIOS device number at bl
;
;               ds == 0x7c0
;

readsec proc near
                cmp  byte ptr ds:ForceLBA, 0               ; force LBA?
                jnz  lba

                cmp  byte ptr [ds:UseCHSAddr], 0           ; LBA or CHS?
                jz   lba
chs:
                push es
                pusha
                call readsec_chs                           ; Read by CHS
                popa
                pop  es
                jmp  return_lb
lba:
                push es
                pusha
                call readsec_lba                           ; Read by LBA
                popa
                pop  es
                jnc  return_lb                             ; if LBA fails, fallback to CHS
                jmp  chs
return_lb:
                ret

readsec endp

;
; ReadSecLBA:
;
;               Reads a sector
;               using LBA,
;               to es:di
;               from LBA address dx:ax
;               from disk device at bl
;

readsec_lba proc near
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

                push PktSeg
                pop  ds

                assume ds:PktSeg

                lea  si, disk_addr_pkt                     ; disk address packet
                mov  byte  ptr [si], 10h                   ; size of packet absolute starting address

                mov  word ptr [si + 8],  ax                ; LBA of the 1st
                mov  word ptr [si + 10], dx                ; sector of partition (bootsector)
                mov  dword ptr [si + 12], 0                ;

                mov  word ptr [si + 2], 1                  ; number of blocks to transfer
                mov  word ptr [si + 6], es                 ; segment
                mov  word ptr [si + 4], di                 ; offset of read buffer
                mov  dl, bl
                mov  ah, 42h
                int  13h
lb2:
                pop  ds

                ret

readsec_lba endp

;
; ReadSecCHS:
;
;               Reads a sector
;               using CHS,
;               to es:di
;               from LBA address in dx:ax
;               from disk device in bl
;

readsec_chs proc near
                push ax
                mov  ax, headsCnt                          ; headsCnt * trackSize
                mul  byte ptr trackSize                    ; --> ax (cyl size)
                mov  si, ax                                ; si = cylinder size
                pop  ax
                                                           ; divide LSN in dx:ax by cyl size in si
                div  si                                    ; now dx holds remainder
                                                           ; and ax holds quotient
                mov  cx, ax                                ; cx = Cylinder no
                mov  ax, dx                                ;
                div  byte ptr trackSize                    ; now al holds Head no
                                                           ; and ah holds sector no
                inc  ah                                    ; sectors in CHS are numbered from 1, not 0 !!!

                mov  dh, al                                ; head no

                xchg cl, ch                                ;

                or   cl, ah                                ;
                mov  ax, 0201h                             ; ah = 2 -- function and al = 1 -- count of sectors
                mov  dl, bl
                mov  bx, di
                int  13h

                jnc  noerr1
                jmp  Err$Read
noerr1:
                cmp  ah, 0
                jz   noerr2
                jmp  Err$Read
noerr2:
                ret
readsec_chs endp


;
; Err:
;
;          Displays an error message
;


Err   proc near
Err$Read:
                lea si, Err_DskReadError
                call outstr
loop3:
                hlt
                jmp  loop3
Err   endp


;outstr_f proc far
;                call outstr
;                retf
;outstr_f endp


outstr proc near
                cld
Disp1:
                lodsb
                test al, al
                jz Disp2
                mov ah, 0eh  ; int 10h/ah=0eh: write char on screen in teletype mode
                xor bx, bx
                push si
                int 10h
int10_exit:
                pop si
                jmp Disp1
Disp2:
                ret
outstr endp

;
; Defines
;

btSeg           equ         07c0h                                          ; bootsector segment
btAddr          equ         btSeg * 16                                     ;
UseCHSAddr      equ         210h                                           ; Address of UseCHS variable

padSize         equ         512 - ($ - boot + signature - vars + 2)        ; padding size

if padSize gt 0
codeEnd         db          padSize  dup (0)                               ; pad by zeroes
endif

if padSize lt 0
  error    "Bootsector does not fit into 512 bytes!"
endif

vars             label       byte

;OutstrFAddr      dw          outstr_f
ReadRunFAddr     dw          read_run_f                                     ; ReadRunF function address
Err_DskReadError db          "R",0                                          ; Disk read error message

;
; Some variables set by sysinstx:
;

forceLBA         db          0                                              ; force LBA mode flag

ifdef CODE_1

muFSDLoadSeg     dw          8C20h                                          ; muFSD load Address (paragraph number, 0..FFFFh)
muFSDEntry       dw          0                                              ; muFSD entry point, 0..65535 bytes
muFSDLen         db          0                                              ; mFSD length in sectors, 0..128
muFSDAddr        dd          0                                              ; mFSD 1st sector number (LSN from log. disk beginning)

else

mapLoadSeg       dw          8C00h                                          ; muFSD allocation map load segment
muFSDEntry       dw          0                                              ; muFSD entry point, 0..65535 bytes
lenOffset        db          ?                                              ; Lengths array offset into the map sector
mapAddr          dd          1                                              ; muFSD allocation map sector number

endif

signature       db          055h,0aah                                       ; Boot sector signature

BootSeg ends

;
; Additional variables (not in bootsector)
;

PktSeg      segment at 7e0h
disk_addr_pkt   db          10h     dup (0)                                 ; disk address packet structure for LBA mode
UseCHS          db          0
PktSeg       ends

;
; BIOS data area
;

BIOSDataSeg segment at 40h
                db          19h     dup (?)
memSize         dw          ?
BIOSDataSeg ends

;
; Interrupt vector table
;

IntVecSeg   segment at 0h
                dd          1eh     dup (?)
FddParmTblVec   dd          ?
IntVecSeg   ends

          end boot

;
; +------------+
; |bootsect.asm|
; +------------+
; An exFAT bootsector.
; ~~~~~~~~~~~~~~~~~~~~~~
; Loads a program as a contiguous sequence of sectors from
; disk at arbitrary address, aligned to paragraph boundary,
; (default is 0x800:0x0), and gives control to it (via far jump).
; A file is loaded by logical sector number (32-bit LSN offset
; from the beginning of logical disk) and a length value in
; sectors, which are written by sysinstx utility to a boot
; sector.
;
; (c) osFree project
; valerius, 2017/09/16,
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
SysId           db     'EXFAT   '                          ; System Identifier

; Standard BPB                                             ;

BPB             db          53 dup (?)                     ; BPB (empty)

partOffset         dq          0
volumeLen          dq          0
fatOffset          dd          0
fatLen             dd          0
clusterHeapOff     dd          0
clusterCount       dd          0
rootDirStrtClus    dd          0
volSerial          dd          997d0015h
fsRev              dw          1
volFlags           dw          0
bytesPerSecShift   db          0
secPerClusShift    db          0
numFats            db          1
drive              db          80h
percentInUse       db          0
reserved           db          7  dup(0)

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
                assume ds:BootSeg

readRoot:
                mov  bx, dx
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

                mov  dx, bx

                retf                                       ; "return" to micro FSD code

                ;
                ; On exit:
                ;
                ;            ds:si --> BPB at 0x7c0:0xb
                ;            dl    --  int 13h boot drive number
                ;            cs:ip --> muFSD at address 0x800:0x0
boot endp

.386p

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
                add  ax, word ptr partOffset               ;
                adc  dx, word ptr partOffset + 2           ; Add hidden sectors value

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
                push es
                pusha
                call readsec_lba                           ; Read by LBA
                popa
                pop  es

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

;teststr         db           'test string',0

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

muFSDLoadSeg     dw          8C20h                                          ; muFSD load Address (paragraph number, 0..FFFFh)
muFSDEntry       dw          0                                              ; muFSD entry point, 0..65535 bytes
muFSDLen         db          0                                              ; mFSD length in sectors, 0..128
muFSDAddr        dd          0                                              ; mFSD 1st sector number (LSN from log. disk beginning)

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

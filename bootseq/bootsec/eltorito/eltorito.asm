;
; eltorito.asm
; startup boot sector
; for el torito non-emulation boot
;
; Ported to wasm from GRUB
; by valerius
; osFree project, 2007, Oct 16
;

;
;   GRUB  --  GRand Unified Bootloader
;   Copyright (C) 1994-2002  H. Peter Anvin
;   Copyright (C) 1999,2000,2001,2004   Free Software Foundation, Inc.
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
;

name eltorito

.386p

_TEXT    segment para public 'CODE' use16

SECTOR_SIZE         equ 200h
BOOTSEC_LISTSIZE    equ 8
ISO_SECTOR_SIZE     equ 2048
ISO_SECTOR_BITS     equ 11
LOAD_ADDR           equ 7e00h
LOAD_SEG            equ 7e0h             ; bootsector load seg
BOOT_SEG            equ 800h             ; bootblock load seg

MSG      macro x
         lea  si, x
         call message
         endm

         org 7c00h
start:
         xor  ax, ax
         push ax
         push offset _TEXT:real_start
         retf

pad1size equ (8 - ($ - start))
         db  pad1size dup (0)
;
; Boot info table. Gets filled by mkisofs
; with -boot-info-table option
;
bi_pvd       dd  0DEADBEEFh        ; Primary Volume Descriptor LBA
bi_file      dd  0DEADBEEFh        ; LBA of boot file beginning
bi_length    dd  0DEADBEEFh        ; boot file length
bi_csum      dd  0DEADBEEFh        ; boot file checksum
bi_reserved  db  (10*4) dup (0)

real_start:
         mov  ds, ax
         mov  es, ax
         mov  fs, ax
         mov  gs, ax

         cli
         mov  ss, ax
         mov  sp, offset _TEXT:start
         sti

         cld

         mov  byte ptr BootDrive, dl

         mov  al, dl
         call printhex2

         MSG  <notification_string>

load_image:
         mov  eax, bi_length
         add  eax, ISO_SECTOR_SIZE - 1
         shr  eax, ISO_SECTOR_BITS            ; dwords -> sectors
         mov  bp,  ax                         ; boot file sectors
         mov  bx,  LOAD_SEG
         mov  es,  bx
         xor  bx,  bx
         mov  eax, bi_file
         call getlinsec
         mov  ax,  ds
         mov  es,  ax

         MSG  <notification_done>
bootit:
        ; save the sector number of the second sector in ebp
        mov     si, firstlist - BOOTSEC_LISTSIZE
        mov     ebp, [si]
        mov     dl, byte ptr [BootDrive]       ; this makes sure dl is our "boot" drive
        push    BOOT_SEG
        push    0                              ; jump to boot block
        retf
stop:
        jmp     $

;
; Get linear sectors - EBIOS LBA addressing, 2048-byte sectors.
;
; Note that we can't always do this as a single request, because at least
; Phoenix BIOSes has a 127-sector limit.  To be on the safe side, stick
; to 16 sectors (32K) per request.
;
; Input:
;       EAX     - Linear sector number
;       ES:BX   - Target buffer
;       BP      - Sector count
;

getlinsec:
        lea     si, dapa                    ; Load up the DAPA
        mov     [si + 4], bx
        mov     bx, es
        mov     [si + 6], bx
        mov     [si + 8], eax
l1:
        push    bp
        push    si

        movzx   ax, byte ptr [MaxTransfer]
        cmp     bp, ax
        jbe     l2
        mov     bp, ax
l2:
        mov     [si + 2], bp
        mov     dl, byte ptr [BootDrive]
        mov     ah, 42h                     ; Extended Read

        call    xint13

        pop     si
        pop     bp

        movzx   eax, word ptr [si + 2]      ; Sectors we read
        add     [si + 8], eax               ; Advance sector pointer
        sub     bp, ax                      ; Sectors left
        shl     ax, ISO_SECTOR_BITS - 4     ; 2048-byte sectors -> segment
        add     [si + 6], ax                ; Advance buffer pointer

        pushad
        MSG     <notification_step>
        popad

        cmp     bp, 0
        ja      l1
        mov     eax, [si + 8]               ; Return next sector
        ret


;
; INT 13h with retry
;

xint13:
        mov     byte ptr [RetryCount], 6
try:
        pushad

        int     13h
        jc      ll1
        add     sp, 8*4                     ;  Clean up stack
        ret
ll1:
        mov     dl, ah                      ; Save error code
        dec     byte ptr [RetryCount]
        jz      real_error
        mov     al, byte ptr [RetryCount]
        mov     ah, byte ptr [dapa + 2]     ; Sector transfer count
        cmp     al, 2                       ; Only 2 attempts left
        ja      ll2
        mov     ah, 1                       ; Drop transfer size to 1
        jmp     setmaxtr
ll2:
        cmp     al, 3
        ja      ll3                         ; First time, just try again
        shr     ah, 1                       ; Otherwise, try to reduce
        adc     ah, 0                       ; the max transfer size, but not
setmaxtr:
        mov     byte ptr [MaxTransfer], ah
        mov     byte ptr [dapa + 2], ah
ll3:
        popad

        jmp     try

real_error:
        MSG     <read_error_string>
        mov     al, dl
        call    printhex2

        popad

        jmp     stop


;
; message: write the string pointed to by %si
;
;   WARNING: trashes si, ax, and bx
;

;
;         Use BIOS "int 10H Function 0Eh" to write character in teletype mode
;              ah = 0xe       al = character
;              bh = page      bl = foreground color (graphics modes)
;

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


pad2size equ SECTOR_SIZE - ($ - start) - (end_list - lst)
pad2     db  pad2size dup (0)

lst      label byte

notification_string:
         db   10,13,"Loading boot file",0
notification_step:
         db   ".",0
notification_done:
         db   10,13,0
read_error_string:
         db   "Read error 0x",0

;
; EBIOS disk address packet
;

;align 8
dapa:
         db   16          ; Packet size
         db   0           ; reserved
         dw   0           ; +2 block count
         dw   0           ; +4 offset of buffer
         dw   0           ; +6 segment of buffer
         dd   0           ; +8 LBA (LSW)
         dd   0           ; +C LBA (MSW)

         db   0
BootDrive:
         db   0ffh
MaxTransfer:              ; Max sectors per transfer (32K)
         db   16
RetryCount:
         db   0

         dw   0
         dw   0

blocklist_default_start:
         dd   0
         dw   0
blocklist_default_seg:
         dw   0

firstlist:            ; this label has to be after the list data!!!

bootsig  dw   0xaa55  ; boot sector signature
end_list label byte

_TEXT    ends

         end  start

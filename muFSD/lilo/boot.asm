;
; $Header: /cur/cvsroot/boot/muFSD/lilo/boot.asm,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
;

; 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
; access your Linux ext2fs partitions as normal drive letters.
; Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

;
; This is a dummy 512 bytes boot sector to put at the head of the boot image
; A Linux image seems to consist of :
;     - a 512 byte boot sector (8086 real mode code)
;     - the setup code         (8086 real mode code)
;     - the Linux kernel       (80386 protected mode code)
;
; Our ext2-os2 image will be :
;     - this dummy 512 byte boot sector (8086 real mode code)
;     - the microfsd           (8086 real mode code)
;
; The microfsd will load OS2LDR and the minifsd, and then call
; OS2LDR. OS2LDR will then load OS2KRNL using the microfsd.
;

        .286
        include bsedev.inc

code segment para public 'CODE'
assume cs:code, ds:code, ss:code
        org 0
start:
        jmp bootstrap                ; jump to bootstrap code


        db "EXT2-OS2"                ; OEM info
        bpb BIOSPARAMETERBLOCK <?>   ; BIOS parameter block
bootstrap:
        hlt
        jmp bootstrap

        org 497
        dw  0h                       ; number of sectors of boot image

        org 510
        dw 0AA55h                    ; DOS boot sector signatur
code ends
        end start



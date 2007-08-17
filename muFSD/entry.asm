;
; blackbox assembler startup
;

;
; $Header: /cur/cvsroot/boot/muFSD/entry.asm,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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

name blackbox

;****************************************************************************
;*** Segment definitions                                                  ***
;****************************************************************************
        .386p

_TEXT   SEGMENT  WORD USE16 PUBLIC 'CODE'
_TEXT   ENDS
_DATA   SEGMENT  WORD USE16 PUBLIC 'DATA'
_DATA   ENDS
CONST   SEGMENT  WORD USE16 PUBLIC 'CONST'
CONST   ENDS
_BSS    SEGMENT  WORD USE16 PUBLIC 'BSS'
_BSS    ENDS
mfs_stack       SEGMENT  WORD USE16 PUBLIC 'STACK'
mfs_stack       ENDS
DGROUP  GROUP   _TEXT, CONST, _BSS, _DATA, mfs_stack
        ASSUME CS:DGROUP, DS: DGROUP, SS: DGROUP

;****************************************************************************
;*** External variables                                                   ***
;****************************************************************************
        .286
;****************************************************************************
;*** Stack segment                                                        ***
;****************************************************************************

mfs_stack SEGMENT
        public _microfsd_stack_bottom
        public _microfsd_stack_top
        _microfsd_stack_bottom db 4096 dup (?)
        _microfsd_stack_top label byte
mfs_stack ENDS


_DATA segment
        public _disk_table

        checkpoint_1 db "MICROFSD - checkpoint 1"
        checkpoint_1_len equ $ - offset DGROUP:checkpoint_1

        checkpoint_2 db "MICROFSD - checkpoint 2"
        checkpoint_2_len equ $ - offset DGROUP:checkpoint_2

        checkpoint_3 db "MICROFSD - checkpoint 3"
        checkpoint_3_len equ $ - offset DGROUP:checkpoint_3

        _disk_table dw 11 dup (0)
_DATA ends

;****************************************************************************
;** Code segment                                                          ***
;****************************************************************************

_TEXT   SEGMENT

        public microfsd_entry

        EXTRN BIOS_READ_HD      : near ; in diskread.asm

        EXTRN stub_mu_Open      : near ; in stubs.asm
        EXTRN stub_mu_Read      : near ; in stubs.asm
        EXTRN stub_mu_Close     : near ; in stubs.asm
        EXTRN stub_mu_Terminate : near ; in stubs.asm

        EXTRN mu_Start     : near      ; in start.c

        EXTRN _parm_cs      : word      ; in start.c
        EXTRN _parm_ds      : word      ; in start.c
        EXTRN _parm_es      : word      ; in start.c
        EXTRN _parm_ss      : word      ; in start.c

        EXTRN output_com   : near      ; in vsprintf.c

        EXTRN VIDEO_INIT    : near      ; in video.asm
        EXTRN VIDEO_OUTPUT  : near      ; in video.asm
        EXTRN VIDEO_CRLF    : near      ; in video.asm

        BOOTSEC_SEG       equ 007C0h    ; bootsector segment (if we were called from
                                        ; bootsector, not LILO) (valerius, 2006/11/04)
        LILO_DATA_SEGMENT equ 09000h    ; segment of LILO data passed to microfsd.fsd
        RELOC_LILO_DATA   equ 08C00h    ; LILO data will be relocated here
        RELOC_BOOTSEC     equ 08C00h    ; bootsector with BPB will be relocated here
        RELOC_SEG         equ 08C20h    ; microfsd.fsd will be relocated here

        DEBUG_PORT        equ 00000h    ; COM port for debug output (0 : com1, 1 : com2)

        ;
        ; >>>>> LILO will call us here normally <<<<<
        ;
        org 0h                  ; 0 based code

microfsd_entry proc far
        ;
        ; Saves the DS ES and SS registers as passed to us
        ;
        mov ax, cs
        mov bx, ds
        mov cx, es
        mov dx, ss

        ;
        ; Updates the data segment registers
        ;
        mov ds, ax
        mov es, ax

        ;
        ; Updates the stack pointer
        ;
        cli
        mov ss, ax
        mov sp, offset DGROUP:_microfsd_stack_top
        sti

        ASSUME DS:DGROUP

        ;
        ; Saves the DS ES and SS passed to us into parm_ds and parm_es
        ;
        mov _parm_cs, ax
        mov _parm_ds, bx
        mov _parm_es, cx
        mov _parm_ss, dx

        ;
        ; Initialize video (and clear screen)
        ;
        call VIDEO_INIT

        ;
        ; Checkpoint
        ;
        push offset DGROUP:checkpoint_1
        push checkpoint_1_len
        call VIDEO_OUTPUT
        call VIDEO_CRLF

ifndef __NO_LILO__

        ;
        ; Copies the LILO command line parameter area to RELOC_LILO_DATA:0000
        ; (512 bytes)
        ;
        push ds
        push es

        mov ax, LILO_DATA_SEGMENT
        mov ds, ax

        mov ax, RELOC_LILO_DATA
        mov es, ax

        xor si, si
        xor di, di
        mov cx, 256
        cld
        rep movsw

        pop es
        pop ds

        ;
        ; Relocales ourself at 2000:0000
        ;
        mov cx, offset DGROUP:_microfsd_stack_top
        shr cx, 1
        inc cx
        mov ax, 02000h
        mov es, ax
        mov ax, 0
        mov si, ax
        mov di, ax
        cld
        rep movsw

        push 02000h
        push offset DGROUP:reloc_1
        retf
reloc_1:

        ;
        ; Updates the stack segment
        ;
        mov ax, cs
        mov ds, ax
        mov es, ax
        cli
        mov ss, ax
        mov sp, offset DGROUP:_microfsd_stack_top
        sti

else

        ;
        ; Copies the the bootsector to RELOC_BOOTSEC:0000
        ; (512 bytes)
        ;
        push ds
        push es

        mov ax, BOOTSEC_SEG
        mov ds, ax

        mov ax, RELOC_BOOTSEC
        mov es, ax

        xor si, si
        xor di, di
        mov cx, 256
        cld
        rep movsw

        pop es
        pop ds

endif

        ;
        ; Checkpoint
        ;
        push offset DGROUP:checkpoint_2
        push checkpoint_2_len
        call VIDEO_OUTPUT
        call VIDEO_CRLF


ifndef __NO_LILO__
        ;
        ; Relocales ourself at RELOC_SEG:0000
        ;
        mov cx, offset DGROUP:_microfsd_stack_top
        shr cx, 1
        inc cx
        mov ax, RELOC_SEG
        mov es, ax
        mov ax, 0
        mov si, ax
        mov di, ax
        cld
        rep movsw

        push RELOC_SEG
        push offset DGROUP:reloc_2
        retf

reloc_2:

        ;
        ; Updates the stack segment
        ;
        mov ax, cs
        mov ds, ax
        mov es, ax
        cli
        mov ss, ax
        mov sp, offset DGROUP:_microfsd_stack_top
        sti

endif


        ;
        ; Checkpoint 3
        ;
        push offset DGROUP:checkpoint_3
        push checkpoint_3_len
        call VIDEO_OUTPUT
        call VIDEO_CRLF

ifndef __NO_LILO__

        ;
        ; Initialize the BIOS floppy parameter table
        ;
        push ds
        push es
        pusha

        xor ax, ax
        mov ds, ax
        mov si, 078h                                    ; DS:SI -> 0000:0078
        lea di, _disk_table
        mov cx, 11
        cld
        rep movsb

        mov ax, es
        mov ds:[si], di
        mov ds:[si + 2], ax

        mov byte ptr [di + 3], 3        ; 512 bytes per sector
        mov byte ptr [di + 4], 18       ; 18 sectors per track

        popa
        pop es
        pop ds



        ;
        ; Initialize disk controller
        ;
        pusha
        push ds
        push es
        xor ax, ax
        xor dx, dx
        int 13h
        pop es
        pop ds
        popa

endif

        ;
        ; Calls the micro FSD main entry point - We'll normally never return from it,
        ; except in test mode.
        ;
        call mu_Start

endless_loop:
        ;
        ; Normally we should NEVER reach this point ; otherwise we halt the system.
        ;
        hlt
        jmp endless_loop

microfsd_entry endp

;
; Linker requires this function to be defined
; for C library.
;

public __do_exit_with_msg__

__do_exit_with_msg__ proc near
           ret
__do_exit_with_msg__ endp

_TEXT   ENDS

_DATA segment
        public _current_seg
        public _image_length

        _current_seg  dw RELOC_SEG
        _image_length dw offset DGROUP:_microfsd_stack_top

_DATA ends

;****************************************************************************
;*** End                                                                  ***
;****************************************************************************
        end microfsd_entry

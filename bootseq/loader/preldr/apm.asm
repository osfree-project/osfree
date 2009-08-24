;
; apm.asm:
; APM routines
;

.386

include fsd.inc

extrn   call_rm       :near

ifndef STAGE1_5

public get_apm_info
public apm_bios_info

_TEXT16 segment dword public 'CODE' use16


get_apm_info_rm:
	; APM BIOS installation check
	mov	ax, 5300h
	xor	bx, bx
	int	15h
	; error -> no APM BIOS
	jc	done_apm_bios

	; check for "PM" signature
	cmp	bx, 504dh
	; no signature -> no APM BIOS
	jne	done_apm_bios

	; Is 32 bit supported?
	and	cx, 0002h
	; no ...
	je	done_apm_bios

	; Disconnect first just in case
	mov	ax, 5304h
	xor	bx, bx
	; ignore return code
	int	15h

	; 32 bit connect 
	mov	ax, 5303h
	xor	ebx, ebx
	; paranoia
	xor	cx, cx
	xor	dx, dx
	xor	esi, esi
	xor	di, di
	int	15h
	; error
	jc	no_32_apm_bios

	; BIOS code segment
	mov	word ptr apm_bios_info + 2, ax                     
	; BIOS entry point offset
	mov	dword ptr apm_bios_info + 4, ebx
	; BIOS 16 bit code segment
	mov	word ptr apm_bios_info + 8, cx
	; BIOS data segment
	mov	word ptr apm_bios_info + 10, dx
	; BIOS code segment length
	mov	dword ptr apm_bios_info + 14, esi
	; BIOS data segment length
	mov	word ptr apm_bios_info + 18, di

	;
	; Redo the installation check as the 32 bit connect
	; modifies the flags returned on some BIOSs
	;

	; APM BIOS installation check
	mov	ax, 5300h
	xor	bx, bx
	; paranoia
	xor	cx, cx
	int     15h
	; error -> should not happen, tidy up
	jc	done_apm_bios

	; check for "PM" signature
	cmp	bx, 504dh
	; no signature -> should not happen, tidy up
	jne	done_apm_bios

	; record the APM BIOS version
	mov	word ptr apm_bios_info, ax
	; and flags
	mov	word ptr apm_bios_info + 12, cx
	jmp	done_apm_bios

no_32_apm_bios:
	; remove 32 bit support bit
	and     word ptr apm_bios_info + 12, 0fffdh

done_apm_bios:
	; Some paranoia here: Always Disconnect from APM
	mov	ax, 5304h
	xor	bx, bx
	; ignore return code
	int     15h

        retf

apm_bios_info:
	dw	0	; version
	dw	0	; cseg 
	dd	0	; offset
	dw	0	; cseg_16
	dw	0	; dseg_16
	dw	0	; cseg_len
	dw	0	; cseg_16_len
	dw	0	; dseg_16_len

        dd      0       ; additional 4 bytes for safety

_TEXT16 ends 

;
;  GRUB  --  GRand Unified Bootloader
;  Copyright (C) 2000, 2001 Free Software Foundation, Inc.
;
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published by
;  the Free Software Foundation; either version 2 of the License, or
;  (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; if not, write to the Free Software
;  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;

;/* This is stolen from arch/i386/boot/setup.S in Linux 2.2.17 */
;/*
;!       setup.S         Copyright (C) 1991, 1992 Linus Torvalds
;*/

_TEXT   segment dword public 'CODE' use32

get_apm_info:
	push	ebp
	push	ebx
	push	edi
	push	esi

        ; enter real mode
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:get_apm_info_rm
        push    eax
        call    call_rm
        add     esp, 4

	pop	esi
	pop	edi
	pop	ebx
	pop	ebp
	ret

_TEXT   ends 

endif

        end

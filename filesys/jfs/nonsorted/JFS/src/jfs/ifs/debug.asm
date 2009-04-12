; $Id: debug.asm,v 1.1.1.1 2003/05/21 13:36:06 pasha Exp $ 

SCCSID equ "@(#)1.4  10/6/98 14:23:10 src/jfs/ifs/debug.asm, sysjfs, w45.fs32, 990417.1"
;
;
;   Copyright (c) International Business Machines  Corp., 2000
;
;   This program is free software;  you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or 
;   (at your option) any later version.
; 
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY;  without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
;   the GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program;  if not, write to the Free Software 
;   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
;
;***	PUTC.ASM - com2 output of a single byte

;**	Ripped off from hpfs, who ripped it off from the kernel debugger

include thunk.inc

COM1_PORT	equ	03f8H
COM2_PORT	equ	02f8H

COM_DAT		equ	00H
COM_IEN		equ	01H		; interrupt enable
COM_IER		equ	02H		; interrupt ID
COM_LCR		equ	03H		; line control registers
COM_MCR		equ	04H		; modem control register
COM_LSR		equ	05H		; line status register
COM_MSR		equ	06H		; modem status regiser
COM_DLL		equ	00H		; divisor latch least sig
COM_DLM		equ	01H		; divisor latch most sig


;;-----------------------------------------------------------------------------
;;-----------------------------------------------------------------------------
DefData	EXPORT,DATA32

PUBLIC PortAdr

PortAdr DW COM2_PORT		; actual adr of which port to
				; use: com2=02f8H, com1=03f8H
EndData
;;-----------------------------------------------------------------------------
;;-----------------------------------------------------------------------------
DefCode	EXPORT,CODE32

aflatCS
aflatContext

;;-----------------------------------------------------------------------------
IODelay Macro
    local a
    jmp a
a:
endm
;;-----------------------------------------------------------------------------
Procedure	_INT3@0,near
	EnterProc

	int 3

	LeaveProc
	RETC
EndProc		_INT3@0
;;-----------------------------------------------------------------------------
Procedure	_brkpoint@0,near
	EnterProc

	int 3

	LeaveProc
	RETC
EndProc		_brkpoint@0
;;-----------------------------------------------------------------------------
Procedure	PollC,near

	EnterProc

	mov	dx, PortAdr
	add	dx, COM_LSR

	in	al,dx			; get input status
	IODelay

	and	al,1			; is there a char in RECV buffer?
	jz	plc1			; no, go return empty

	mov	dx, PortAdr
	add	dx, COM_DAT

	in	al,dx			; suck char out of buffer
	IODelay

	and	al,07fh 		; strip off stupid parity crap
plc1:	LeaveProc
	RETC
EndProc		PollC
;;-----------------------------------------------------------------------------


;**	PUTC - output a single char to COM2 handling ^S


Procedure	_PUTC@4,near
ArgVar	chData,DWORD

	EnterProc

;	See if ^S

	call	PollC		    ; is there a char at input
	jz	pc2		    ; no, go output our char
	cmp	al,'S' - 'A' + 1    ; is it ^S?
	jnz	pc2		    ; no, go output our char

;	Saw ^S.  Wait for and eat next char.

pc1:	call	PollC		    ; look for next char
	jz	pc1		    ; no char, go look again
	cmp	al,'S' - 'A' + 1    ; is it ^S again?
	jz	pc1		    ; yes, go look for something else

pc2:	mov	dx, PortAdr
	add	dx, COM_LSR
	in	al,dx
	IODelay
	test	al,020h
	jz	pc2

;	ready.	crank it out!

	mov	dx, PortAdr
	add	dx, COM_DAT
	mov	eax,chData
	out	dx,al
	IODelay
	LeaveProc
	RETC

EndProc	_PUTC@4
;;-----------------------------------------------------------------------------

;	ComInit - looks at a global to determine which port to use
;		  and sets up a variable to use later that contains
;		  the address of desired port

Procedure	_ComInit@4,near
ArgVar	Port,DWORD

	EnterProc

	cmp	Port, 1			; test the global	
	jz	cini1			; jump if com1
	mov	PortAdr, COM2_PORT	; desired port is com2
	jmp	cini2
cini1:	mov	PortAdr, COM1_PORT	; desired port is com1
cini2:	LeaveProc
	RETC
EndProc	_ComInit@4
;;-----------------------------------------------------------------------------
EndCode

end

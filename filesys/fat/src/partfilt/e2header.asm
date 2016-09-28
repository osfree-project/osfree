;/*
; * $Header$
; */

;/************************************************************************/
;/*                       Linux partition filter.                        */
;/*          (C) Copyright Deon van der Westhuysen, July 1995.           */
;/*                                                                      */
;/*  Dedicated to Jesus Christ, my Lord and Saviour.                     */
;/*                                                                      */
;/* This program is free software; you can redistribute it and/or modify */
;/* it under the terms of the GNU General Public License as published by */
;/* the Free Software Foundation; either version 2, or (at your option)  */
;/* any later version.                                                   */
;/*                                                                      */
;/* This program is distributed in the hope that it will be useful,      */
;/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
;/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
;/* GNU General Public License for more details.                         */
;/*                                                                      */
;/* You should have received a copy of the GNU General Public License    */
;/* along with this program; if not, write to the Free Software          */
;/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            */
;/*                                                                      */
;/*  This code is still under development; expect some rough edges.      */
;/*                                                                      */
;/************************************************************************/
	Title	E2Header- Device driver header and segment ordering.

ifndef __MASM__
locals				; I really don't know the MASM equivalent ! (M.Willm 1995-11-14)
endif

.386P

include reqpkt.inc
include devcmd.inc
DRIVER_INIT= 1

ifdef __MASM__
DGROUP	group DDHeader, _DATA, _BSS, CONST
else
DGROUP	group DDHeader, _DATA, _BSS
endif

CodeGroup  group StratCode, _TEXT, InitCode


DDHeader	segment word public use16 'DATA'
	public DevHeader
DevHeader	dd -1			; Pointer to next driver
		dw 8180h		; Device attributes
		dw offset E2Strategy	; Strategy routine entry point
		dw 0			; IDC routine entry point
		db 'PARTFLT$'		; Device name
		db 8 dup (0)		; Reserved
		dd 8			; Level 3 device drive capabilities
					; 8= Adapter device driver
	public InitFlags
InitFlags	db 0

DDHeader	ends

_DATA           segment word public use16 'DATA'
_DATA           ends

ifdef __MASM__
CONST	segment word public use16 'CONST'
CONST	ends
endif

_BSS	segment word public use16 'BSS'
_BSS	ends

StratCode	segment byte public use16 'CODE'

	assume cs:CodeGroup, ds:DGROUP
;	assume cs:_TEXT, ds:DGROUP
ifdef __LARGE__
	extrn _E2Init: far
else
	extrn _E2Init: near
endif

	public E2Strategy
E2Strategy	proc far
	cmp byte ptr es:[bx+ReqPacket.ReqCommand],CMDInitBase
	jne short @@BadCommand
	test InitFlags,DRIVER_INIT
	jne short @@BadCommand
	or InitFlags,DRIVER_INIT
	push es
	push bx
	call _E2Init
	add sp,4
	jmp short @@Exit
@@BadCommand:
	mov word ptr es:[bx+ReqPacket.ReqStatus],8103h	; Done, error
@@Exit:
	retf
E2Strategy	endp

StratCode	ends

_TEXT	segment byte public use16 'CODE'
_TEXT   ends

InitCode  segment byte public use16 'CODE'
InitCode  ends

	end

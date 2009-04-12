; $Id: flat2sel.asm,v 1.1.1.1 2003/05/21 13:36:06 pasha Exp $ 

SCCSID equ "@(#)1.4  7/30/98 14:09:32 src/jfs/ifs/flat2sel.asm, sysjfs, w45.fs32, 990417.1"
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
	.386
	.387
;   KSO: This library isn't actually used.
;	includelib cppos35.lib
;	includelib cpprsi36.lib ;this is the VAC 3.6.5 name
	includelib os2386.lib
	assume	cs:FLAT, ds:FLAT, ss:FLAT, es:FLAT
	extrn	_FlatToSel@4:proc
CODE32	segment	dword public use32 'code'

	align 020h

	public _DosFlatToSel
_DosFlatToSel	proc
	push	ebx
	push	ecx
	push	edx
	push	ebp
	mov	ebp,esp
	sub	esp,4

	push	eax
	call	_FlatToSel@4

	mov	esp,ebp
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	ret
_DosFlatToSel	endp

CODE32	ends
end

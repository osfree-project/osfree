; FreeDOS APPEND
; Copyright (c) 2004 Eduardo Casino <casino_e@terra.es>
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	USA
;
; ENVIRON.ASM - Get/Set append path from/to environment
;
; 04-06-01  casino_e@terra.es	First version
; 04-06-14  casino_e@terra.es	Update comments on FreeCOM's MUX-AE
;


environ		dw	0	; Segment of parent environment
envsize		dw	0

; Command name is padded with blanks in set_lin as required by erlier
; versions of FreeCOM
;
set_nam		db	3, "SET"
SET_NAM_LEN	equ	$ - set_nam
set_lin		db	14, "SET    APPEND=", 13
SET_LIN_LEN	equ	$ - set_lin



; ---------------------------------------------------------------------------
; Function: get_environ - Get PARENT environment
;
; Args:	    BX		- PSP segment
;
; Returns:  [environ]	- Segment of parent environment
;	    [envsize]	- Size of parent environment
;
get_environ:	push	bx
		push	cx
		push	es

		mov	es, bx
		mov	bx, [es:22]	; get the segment for our parent's PSP
		mov	es, bx
		mov	bx, [es:44]	; get the segment for its environment
		mov	[cs:environ], bx
		dec	bx		; Point to MCB
		mov	es, bx
		mov	bx, [es:0x0003] ; get size of environment
		mov	cl, 4
		shl	bx, cl		; paragraphs -> bytes
		mov	[cs:envsize], bx

		pop	es
		pop	cx
		pop	bx
		ret


; ---------------------------------------------------------------------------
; Function: copy_environ - Copy APPEND var into append_path from environment
;
; Args:	    [environ]	 - Parent environment
;	    [envsize]
;
; Returns:  Modifies [append_path]
;
copy_environ:	mov	byte [cs:append_path], 0	; Empty
		push	ds
		push	es
		push	si
		push	di
		push	cx
		push	ax

		mov	cx, cs			; Searches the environment for
		mov	es, cx			; the APPEND= string
		mov	cx, [cs:envsize]
		mov	ds, [cs:environ]
		xor	si, si

srchloop:	cmp	byte [ds:si], 0		; End of environment?
		jz	ce_done

		push	cx
		mov	di, append_prefix	; "APPEND="
		mov	cx, 7
		cld
		repe	cmpsb			; Found?
		pop	cx
		jne	notappend

		sub	cx, 7			; It is APPEND. Update length
		mov	di, append_path		; and move the rest of the
moveappend:	lodsb				; string to append_path
		stosb
		or	al, al
		jz	ce_done
		loop	moveappend

notappend:	mov	al, [ds:si]		; Not APPEND.
		inc	si			; Search end of string
		or	al, al
		jz	srchloop
		loop	notappend

ce_done:	pop	ax
		pop	cx
		pop	di
		pop	si
		pop	es
		pop	ds
		ret

; ---------------------------------------------------------------------------
; Function: setenv_append - Updates APPEND var in environment with append_path
;
; Args:	    [append_path] - Current APPEND
;
; Returns:  Updates command name and line buffers of the installable command
;	    call (function AE01 of int 2F), so a SET APPEND is executed
;	    by the command interpreter on return.
;
setenv_append:	pushf
		push	cx
		push	bx
		push	di
		push	si
		push	es
		push	ds

		push	cs
		pop	ds

		; Updates command name and line buffers with "SET APPEND=..."
		; this will be executed by the command interpreter on return.
		; Command name is padded with chars. Required by erlier
		; versions of FreeCOM
		;
		mov	si, set_nam		; Update Command Name with
		mov	di, [cs:cmdnambuf]	; "SET"
		mov	es, [cs:cmdnambuf+2]
		mov	cx, SET_NAM_LEN
		cld
		rep	movsb

		mov	si, set_lin		; Update Command Line with
		mov	di, [cs:cmdlinbuf]	; "SET	  APPEND="
		mov	es, [cs:cmdlinbuf+2]
		inc	di
		mov	bx, di			; Pointer to length
		mov	cx, SET_LIN_LEN
		cld
		rep	movsb

		mov	si, append_path		; Adds path, if any, to
		cmp	byte [cs:si], 0		; Command Line
		jz	endset
		dec	di			; overwrites final 0x0D
		xor	cl, cl
		cld
setappendloop:	lodsb
		stosb
		or	al, al
		je	endappend
		inc	cl
		jmp	setappendloop

endappend:	dec	di
		mov	byte [es:di], 0x0D	; Adds final 0X0D
		mov	di, bx
		add	byte [es:di], cl	; Updates length

endset:		pop	ds
		pop	es
		pop	si
		pop	di
		pop	bx
		pop	cx
		popf
		ret

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
; CMDLINE.ASM - Command line processing
;
; 04-06-01  casino_e@terra.es	First version
; 04-06-03  casino_e@terra.es	Fix a hang when command switches appear before
;				paths in command line. Change behaviour from MS
;				APPEND: Now it accepts paths in any point in
;				the command line.
;				No paths that can appear on the command line
;				when /E is used. Abort with error if it happens.
; 04-06-13  casino_e@terra.es	Fix a bug in command parsing.
;	    Eric Auer		Make older versions of nasm happy.
; 04-06-17  casino_e@terra.es	Move cmdline to last 128 bytes of PSP and first
;				128 bytes of program.
;

MAXARGS		equ	20
setenv		db	0		; Set Environment indicator
currpar		dw	0		; Pointer to current argument
cmdline		dw	0x80, 0		; Copy of command line

p_xon		db	"X:ON", 0
p_xoff		db	"X:OFF", 0
p_pon		db	"PATH:ON", 0
p_poff		db	"PATH:OFF", 0

p_flags		db	00000000b
;			|  |||||
;			|  ||||+-- switch found
;			|  |||+--- /e
;			|  ||+---- /x(on:off)
;			|  |+----- /path
;			|  +------ append found
;			|
;			+--------- "we are resident" flag
S_FOUND		equ	00000001b
E_FOUND		equ	00000010b
X_FOUND		equ	00000100b
P_FOUND		equ	00001000b
A_FOUND		equ	00010000b
RESIDENT	equ	10000000b



; ---------------------------------------------------------------------------
; Function: copy_cmdline - Copies command line to our own buffer
;
; Args:	    ES:SI	 - Pointer to command line
;
; Returns:  Modifies [cmdline]
;
copy_cmdline:	push	di
		push	si
		push	ds
		push	es

		push	es
		pop	ds

		les	di, [cs:cmdline]

ce_copyloop:	lodsb
		stosb
		cmp	al, 0xD
		je	ce_endcopy
		jmp	ce_copyloop
ce_endcopy:	pop	es
		pop	ds
		pop	si
		pop	di
		ret



; ---------------------------------------------------------------------------
; Function: print_append - Print append path
;
; Args:	    None, prints the contents of append_path with "APPEND=" prepended
;
; Returns:  Nothing
;
print_append:	push	ax
		push	dx
		push	ds
		test	word [cs:append_state], APPEND_ENVIRON	; /E?
		jz	print_own		; No, read our own append

		call	copy_environ		; Read environ

print_own:	cmp	byte [cs:append_path], 0	; Empty?
		jnz	prt_append

noappend:	push	cs
		pop	ds
		mov	dx, NoAppend
		mov	ah, 0x09
		int	0x21
		jmp	pra_done

prt_append:	push	si
		mov	si, append_prefix
		push	cs
		pop	ds
		call	print
		pop	si

pra_done:	pop	ds
		pop	dx
		pop	ax
		ret


; ---------------------------------------------------------------------------
; Function: skipblanks - Points DS:SI to next non-blank character and
;			 decrements CX accordingly
; Args:	    DS:SI      - Pointer to string
;	    CX	       - Length of string
;
; Returns:  DS:SI      - Points to first non-blank char,
;	    AL	       - The char
;
skipblanks:	cld
sb_loop:	lodsb
		cmp	al, 0xD		; End of string
		je	sb_return
		cmp	al, 9		; TAB
		je	sb_next
		cmp	al, ' '
		jne	sb_return
sb_next:	jmp	sb_loop
sb_return:	dec	si
		ret


; ---------------------------------------------------------------------------
; Function: parse_cmds - Parse command line arguments
;
; Args:	    ES:SI      - Pointer to command line
;	    CX	       - Length of command line
;
; Returns:  Set carry on error
;	    Modifies command line string
;
; NOTE:	    This function is called from the resident and non-resident parts
;	    of append
;
parse_cmds:
		push	ds
		push	es
		push	di
		push	si
		push	ax
		push	bx
		push	cx
		push	dx

		call	copy_cmdline

		push	cs
		pop	ds
		les	si, [cs:cmdline]


		mov	byte [cs:setenv], 0
		clc
		call	skipblanks
		cmp	al, 0xD			; End of string?
		jne	pa_nextarg

		test	byte [cs:p_flags], RESIDENT
		jz	pa_done			; Not resident, just install
		call	print_append		; Resident, print append
		clc
		jmp	pa_done

pa_nextarg:	call	skipblanks
		cmp	al, 0xD			; End of string?
		je	pa_done

		mov	[cs:currpar], si	; Points to next parameter

		lodsb

		cmp	al, '/'			; Is it a switch?
		jz	do_switch

		dec	si			; It's a path, then.
		call	get_path		; Get it.
		jc	pa_return		; Error, we're done
		jmp	pa_nextarg

do_switch:	call	process_switch
		jc	pa_return
		jmp	pa_nextarg

pa_done:	test	byte [cs:setenv], 0xFF	; Do we have to setenv?
		jz	pa_return
		call	setenv_append		; Yep, set environment

pa_return:	pop	dx
		pop	cx
		pop	bx
		pop	ax
		pop	si
		pop	di
		pop	es
		pop	ds
		ret

; ---------------------------------------------------------------------------
; Function: print - ASCIZ string + CR + NL
;
; Args:	    DS:SI      - Null-terminated string
;
; Returns:  Nothing
;
print:		push	si
		push	ax
		push	dx
prt_loop:	lodsb
		or	al, al
		jz	prt_end
		mov	dl, al
		mov	ah, 02
		int	0x21
		jmp	prt_loop
prt_end		mov	dl, 13
		mov	ah, 02
		int	0x21
		mov	dl, 10
		mov	ah, 02
		int	0x21
		pop	dx
		pop	ax
		pop	si
		ret

; ---------------------------------------------------------------------------
; Function: is_separator  - Check if a character is a separator
;
; Args:	    AL		  - Character
;
; Returns:  Zero if it's a separator
;
is_separator:	cmp	al, ' '
		je	iss_done
		cmp	al, '/'
		je	iss_done
		cmp	al, 0xD		; CR (End of string)
		je	iss_done
		cmp	al, 9		; TAB
		je	iss_done
		or	al, al
iss_done:	ret


; ---------------------------------------------------------------------------
; Function: find_separator  - Moves DS:SI to the next separator or EOS
;
; Args:	    DS:SI	    - Current position in command line
;	    CX		    - Remaining length of command line
;
; Returns:  DS:SI points to separator
;	    CX	  contains updated length
;	    Modifies AX
;
find_separator: lodsb
		call	is_separator
		je	fs_done
		jmp	find_separator
fs_done:	dec	si
		ret


; ---------------------------------------------------------------------------
; Function: print_arg  - Prints current arg
;
; Args:	    None, uses [cs:currpar] to set beginning of arg
;	    CX	       - Remaining length of command line
;
; Returns:  Nothing, modifies SI, CX and the contents of command line
;
print_arg:	call	find_separator
		mov	byte [ds:si], 0
		mov	si, [cs:currpar]
		call	print
		ret


; ---------------------------------------------------------------------------
; Function: get_path   - Gets append path from cmdline
;
; Args:	    DS:SI      - Pointer to path in command line
;	    CX	       - Remaining length of command line
;
; Returns:  Set carry on error
;	    DS:SI points to next arg
;	    Updates CX
;	    Modifies AX, DX
;
get_path:	mov	ah, [cs:p_flags]
		test	ah, A_FOUND		; Did any append appeared?
		jnz	gp_toomany		; That's too many
		test	ah, E_FOUND		; Using /E?
		jnz	gp_toomany		; No paths allowed with /E

gp_setapp:	call	set_append
		clc
		jmp	gp_done

gp_toomany:	push	ds
		push	cs
		pop	ds
		mov	dx, TooMany
		mov	ah, 0x09
		int	0x21
		pop	ds
		call	print_arg
		stc

gp_done:	ret


; ---------------------------------------------------------------------------
; Function: set_append - Sets append_path from command line
;
; Args:	    DS:SI      - Pointer to path in command line
;	    CX	       - Remaining length of command line
;
; Returns:  Set carry
;	    DS:SI points to next arg
;	    Updates CX
;	    Modifies AX
;
set_append:	or	byte [cs:p_flags], A_FOUND
		cmp	byte [ds:si], ';'	; Special: Clear path
		jne	sa_copypath
		call	find_separator
		mov	byte [cs:append_path], 0
		jmp	sa_done

sa_copypath:	push	di
		mov	di, append_path

sa_copyloop:	lodsb
		toupper al
		stosb
		call	is_separator
		je	sa_endcopy
		jmp	sa_copyloop

sa_endcopy:	mov	byte [es:di-1], 0
		dec	si
		pop	di

sa_done:	test	word [cs:append_state], APPEND_ENVIRON	; /E?
		jz	sa_return		; No, done, next parameter

		test	byte [cs:p_flags], RESIDENT
		jz	sa_return		; We're not resident, no setenv

		mov	byte [cs:setenv], 0xFF	; Mark for setting environment

sa_return:	ret


; ---------------------------------------------------------------------------
; Macro: is_help   - Checks if switch is "/?"
;
; Returns: Zero if matches
;	   Modifies AL
;
%macro		is_help 0
		mov	al, '?'
		call	is_al
%endmacro

; ---------------------------------------------------------------------------
; Macro: is_x	- Checks if switch is "/X"
;
; Returns: Zero if matches
;	   Modifies AL
;
%macro		is_x	0
		mov	al, 'X'
		call	is_al
%endmacro

; ---------------------------------------------------------------------------
; Macro: is_e	- Checks if switch is "/E"
;
; Returns: Zero if matches
;	   Modifies AL
;
%macro		is_e	0
		mov	al, 'E'
		call	is_al
%endmacro

; ---------------------------------------------------------------------------
; Function: is_al - Checks if switch is a single char
;
; Args:	 AL	  - Char
;	 DS:SI	  - Current position in command line
;
; Returns: Zero if matches
;	   Modifies AL
;
is_al:		mov	bl, byte [ds:si]
		toupper bl
		cmp	al, bl
		jne	ia_return
		inc	si
		mov	al, [ds:si]
		call	is_separator
		je	ia_return
		mov	si, [cs:currpar]
		inc	si			; Skip '/'
ia_return:	ret


; ---------------------------------------------------------------------------
; Macro: is_pathon - Checks if arg is PATH:ON
;
; Returns: Zero if matches
;	   Modifies CX, DI
;
%macro		is_pathon	0
		mov	cx, 7
		mov	di, p_pon
		call	strncasecmp
%endmacro


; ---------------------------------------------------------------------------
; Macro: is_pathoff - Checks if arg is PATH:OFF
;
; Returns: Zero if matches
;	   Modifies CX, DI
;
%macro		is_pathoff	0
		mov	cx, 8
		mov	di, p_poff
		call	strncasecmp
%endmacro


; ---------------------------------------------------------------------------
; Macro: is_xon - Checks if arg is X:ON
;
; Returns: Zero if matches
;	   Modifies CX, DI
;
%macro		is_xon	0
		mov	cx, 4
		mov	di, p_xon
		call	strncasecmp
%endmacro


; ---------------------------------------------------------------------------
; Macro: is_xoff - Checks if arg is X:OFF
;
; Returns: Zero if matches
;	   Modifies CX, DI
;
%macro		is_xoff 0
		mov	cx, 5
		mov	di, p_xoff
		call	strncasecmp
%endmacro


; ---------------------------------------------------------------------------
; Function: strncasecmp - Compares at most N bytes of two strings
;			  (case insensitive)
;
; Args:	 CX	  - N bytes to compare
;	 DS:SI	  - string1
;	 ES:DI	  - string2
;
; Returns: Zero if matches
;	   Modifies AL, BL, CX, DI, SI
;
strncasecmp:	mov	al, [ds:si]
		toupper al
		mov	bl, [es:di]
		cmp	al, bl
		jne	snc_nomatch
		inc	si
		inc	di
		loop	strncasecmp
		or	cx, cx
		jnz	snc_nomatch
		mov	al, [ds:si]
		call	is_separator
		je	snc_return
snc_nomatch:	pushf
		mov	si, [cs:currpar]
		inc	si			; Skip '/'
		popf
snc_return:	ret


; ---------------------------------------------------------------------------
; Function: proces_switch - Parses and processes command line switches
;
; Args:	 DS:SI	  - Current position in command line
;
; Returns: Carry if error
;	   Modifies AL, BL, CX, DI, SI
;
process_switch: is_help
		jne	test_x
		call	prt_help		; Also sets carry
		jmp	ps_done
test_x:		is_x
		jne	test_e
		call	set_x_flag
		jmp	ps_done
test_e:		is_e
		jne	test_xon
		call	set_e_flag
		jmp	ps_done
test_xon:	is_xon
		jne	test_xoff
		call	set_xon_flag
		jmp	ps_done
test_xoff:	is_xoff
		jne	test_pathon
		call	set_xoff_flag
		jmp	ps_done
test_pathon:	is_pathon
		jne	test_pathoff
		call	set_pon_flag
		jmp	ps_done
test_pathoff:	is_pathoff
		jne	ps_invalid
		call	set_poff_flag
		jmp	ps_done
ps_invalid:	call	invalid_switch		; Also sets carry
ps_done:	ret


; ---------------------------------------------------------------------------
; Function: set_e_flag - Sets environment flag
;
; Args:	 None
;
; Returns: Carry if error
;	   Modifies AH, [p_flags], [append_state]
;
set_e_flag:	mov	ah, [cs:p_flags]
		test	ah, RESIDENT
		jz	se_teste
		call	invalid_switch
		ret
se_teste:	test	ah, E_FOUND
		jz	se_testp
		call	invalid_switch
		ret
se_testp:	test	ah, A_FOUND		; Did any append appeared?
		jz	se_set
		call	invalid_switch		; /E not allowed with any path
		ret
se_set:		or	byte [cs:p_flags], E_FOUND|S_FOUND
		or	word [cs:append_state], APPEND_ENVIRON
		clc
		ret

; ---------------------------------------------------------------------------
; Function: set_x_flag - Sets extended flag
;	    NOTE: There is a different function for X:ON in order to
;		  reproduce MS APPEND behaviour.
;
; Args:	 None
;
; Returns: Carry if error
;	   Modifies [p_flags], [append_state]
;
set_x_flag:	test	byte [cs:p_flags], X_FOUND
		jz	sx_set
		call	invalid_switch
		ret
sx_set:		or	byte [cs:p_flags], S_FOUND
		or	word [cs:append_state], APPEND_EXTENDD
		clc
		ret

; ---------------------------------------------------------------------------
; Function: set_xon_flag - Sets extended flag
;	    NOTE: There is a different function for /X in order to
;		  reproduce MS APPEND behaviour.
;
; Args:	 None
;
; Returns: Carry if error
;	   Modifies [p_flags], [append_state]
;
set_xon_flag:	call set_x_flag
		pushf
		or	byte [cs:p_flags], X_FOUND
		popf
		ret


; ---------------------------------------------------------------------------
; Function: set_xoff_flag - Unsets extended flag
;
; Args:	 None
;
; Returns: Carry if error
;	   Modifies [p_flags], [append_state]
;
set_xoff_flag:	test	byte [cs:p_flags], X_FOUND
		jz	sxoff_set
		call	invalid_switch
		ret
sxoff_set:	or	byte [cs:p_flags], X_FOUND|S_FOUND
		and	word [cs:append_state], APPEND_EXTENDD^0xFFFF
		clc
		ret


; ---------------------------------------------------------------------------
; Function: set_pon_flag - Sets search path flag
;
; Args:	 None
;
; Returns: Carry if error
;	   Modifies [p_flags], [append_state]
;
set_pon_flag:	test	byte [cs:p_flags], P_FOUND
		jz	spon_set
		call	invalid_switch
		ret
spon_set:	or	byte [cs:p_flags], P_FOUND|S_FOUND
		or	word [cs:append_state], APPEND_SRCHPTH|APPEND_SRCHDRV
		clc
		ret

; ---------------------------------------------------------------------------
; Function: set_poff_flag - Unsets search path flag
;
; Args:	 None
;
; Returns: Carry if error
;	   Modifies [p_flags], [append_state]
;
set_poff_flag:	test	byte [cs:p_flags], P_FOUND
		jz	spoff_flag
		call	invalid_switch
		ret
spoff_flag:	or	byte [cs:p_flags], P_FOUND|S_FOUND
		and	word [cs:append_state], (APPEND_SRCHPTH|APPEND_SRCHDRV)^0xFFFF
		clc
		ret


; ---------------------------------------------------------------------------
; Function: prt_help - Prints help string and sets carry
;
; Args:	    None
;
; Returns:  Set carry
;	    Modifies AX, DX
;
prt_help:	push	ds
		push	cs
		pop	ds
		mov	dx, Help
		mov	ah, 0x09
		int	0x21
		pop	ds
		stc
		ret



; ---------------------------------------------------------------------------
; Function: invalid_switch - Prints invalid switch string, followed by
;			     the offending arg, and sets carry
;
; Args:	    None
;
; Returns:  Set carry
;	    Modifies AX, DX, SI, CX
;
invalid_switch:
		push	ds
		push	cs
		pop	ds
		mov	dx, Invalid
		mov	ah, 0x09
		int	0x21
		pop	ds
		call	find_separator
		mov	byte [ds:si], 0
		mov	si, [cs:currpar]
		call	print
		stc
		ret

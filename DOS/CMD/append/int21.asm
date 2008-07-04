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
; INT21.ASM - Int21h hook
;
; 04-06-01  casino_e@terra.es	First version
; 04-06-13  Eric Auer		Make older versions of nasm happy
; 04-06-15  casino_e@terra.es	Save some bytes by re-using buffers. (Suggested
;				by Eric)
; 04-06-17  casino_e@terra.es	Move tempDTA to PSP
;

old_int21	dd	0		; Original INT 21 handler
usr_int21	dd	0		; User INT 21 handler

handler		db	0		; Flag: chain to user int21 handler
USERHNDL	equ	1

%define dirname tempDTA
defdrive	db	0		; 0x00 == A, 0x01 == B, etc.
%define FCB_off filename_off
filename_off	dw	0
%define FCB_seg filename_seg
filename_seg	dw	0
basename_off	dw	0
%define new_filepath cmdline

flags		dw	0		; Saved registers before an int call
r_ax		dw	0		;
r_bx		dw	0		;
r_cx		dw	0		;
r_dx		dw	0		;
r_ds		dw	0		;
r_es		dw	0		;
r_di		dw	0		;
r_si		dw	0		;

s_ax		dw	0		; Saved registers after an int call
s_cx		dw	0		;

rfnstate	dd	0		; Pointer to Real Filename state flag

DTA		dd	0
tempDTA		dd	0

; ===========================================================================
;  INT 21 HOOK
; ===========================================================================
;

; INT 21 normal entry point
;
int21:		pushf
		test	byte [cs:handler], USERHNDL
		jz	int21cnt
		jmp	i21_jumporig

; INT 21 entry point when chaining to a user INT 21 handler
;
int21usr:	pushf
int21cnt:	test	word [cs:append_state], APPEND_ENABLED
		jz	i21_jump

		cmp	ah, 0x0F	; Open file using FCB
		je	i2123
		cmp	ah, 0x23	; Get file size for FCB
		je	i2123
		cmp	ah, 0x3D	; Open existing file
		jne	i21_ck6c
		jmp	i213d
i21_ck6c:	cmp	ax, 0x6C00	; Extended open/create
		jne	i21_ck_x
		jmp	i216c

i21_ck_x:	test	word [cs:append_state], APPEND_EXTENDD
		jz	i21_jump

		cmp	ah, 0x11	; Find first matching file using FCB
		je	i2111
		cmp	ax, 0x4B00	; Exec - Load and execute program
		jne	i21_ck_4b03
		jmp	i214b00
i21_ck_4b03:	cmp	ax, 0x4B03	; Exec - Load overlay
		jne	i21_ck_4e
		jmp	i214b03
i21_ck_4e:	cmp	ah, 0x4E	; FindFirst - Find first matching file
		jne	i21_jump
		jmp	i214e

i21_jump:	test	byte [cs:handler], USERHNDL
		jz	i21_jumporig
		popf
		jmp	far [cs:usr_int21]
i21_jumporig:	popf
		jmp	far [cs:old_int21]


; ---------------------------------------------------------------------------
;  2111 : FIND FIRST MATCHING FILE USING FCB
; ---------------------------------------------------------------------------
;
		; Do not waste time searching for volume labels
i2111:		push	bx
		mov	dx, bx
		cmp	byte [ds:bx], 0xFF	; Extended FCB?
		jne	i2111_2			; No, then continue
		test	byte [ds:bx+6], 0x08	; Volume label?
		pop	bx
		jnz	i21_jump		; Yes, just chain
i2111_2:	pop	bx

; ---------------------------------------------------------------------------
;  210F : OPEN FILE USING FCB
;  2123 : GET FILE SIZE FOR FCB
; ---------------------------------------------------------------------------
;
i210f:
i2123:		call	get_rfnstate
		popf
		mov	[cs:FCB_off], dx	; FCB offset
		mov	[cs:FCB_seg], ds	; FCB segment
		mov	[cs:r_di], di
		mov	[cs:r_si], si
		mov	[cs:r_ax], ax

		call	call_int21		; Execute normal call first
		savef	[cs:flags]
		mov	[cs:s_ax], ax		; Saved result
		or	al, al
		jz	i2123_check
		jmp	i2123_ret		   ; File found, return
i2123_check:	call	check_extended	; checks extended error
		jc	i2123_ret	; Error other than file or
					; path not found

		test	word [cs:append_state], APPEND_ENVIRON	; /E?
		jz	i2123_noenv
		call	copy_environ


i2123_noenv:	call	save_drive		; Save drive letter from
						; FCB in [cs:defdrive]
		mov	si, append_path
		lds	di, [cs:new_filepath]
		call	find_first	; Get first path from APPEND

i2123_srch:	cmp	byte [cs:di], 0 ; No more paths
		je	i2123_done

		call	set_newpath	; Set new drive letter and default
					; path from new_filepath
		jc	i2123_again	; Invalid drive, try next one

		mov	di, [cs:r_di]	; Restore regs and call again
		mov	si, [cs:r_si]
		mov	ax, [cs:r_ax]
		mov	ds, [cs:FCB_seg]
		mov	dx, [cs:FCB_off]
		call	call_int21
		mov	[cs:s_ax], ax		; Saved result
		savef	[cs:flags]

		call	restor_path	; Restore default path for drive
					; letter

		or	al, al
		jz	i2123_done	; File found
		call	check_extended	; checks extended error
		jc	i2123_done	; Error other than file or
					; path not found

i2123_again:	lds	di, [cs:new_filepath]
		call	find_next
		jmp	i2123_srch

i2123_done:	call	restor_drive	; Restore drive letter in FCB

		mov	di, [cs:r_di]
		mov	si, [cs:r_si]
		mov	ds, [cs:FCB_seg]
		mov	dx, [cs:FCB_off]
		mov	ax, [cs:s_ax]

i2123_ret:	call	clear_rfnstate	; Clear return found name state
		updatef [cs:flags]
		iret

; ---------------------------------------------------------------------------
;  216C : EXTENDED OPEN/CREATE
; ---------------------------------------------------------------------------
;
i216c:		test	dl, 0x10		; Create if file doesn't exist?
		jz	i216c_tail		; then get and store...
		jmp	i21_jump		; else do not execute

i216c_tail:	call	get_rfnstate		; Get and store rfn state flag
		popf
		mov	[cs:filename_off], si
		jmp	i213d_2

; ---------------------------------------------------------------------------
;  214E	 : FINDFIRST - FIND FIRST MATCHING FILE
; ---------------------------------------------------------------------------
;
i214e:		test	cl, 0x08		; Is it a volume label?
		jz	i214b03			; No, go on
		jmp	i21_jump		; Yes, just chain

; ---------------------------------------------------------------------------
;  213D	 : OPEN	     - OPEN EXISTING FILE
;  214B03: EXEC	     - LOAD OVERLAY
; ---------------------------------------------------------------------------
;
i214b03:
i213d:		call	get_rfnstate		; Get and store rfn state flag
		popf
		mov	[cs:filename_off], dx
i213d_2:	mov	[cs:filename_seg], ds
		mov	[cs:r_ax], ax		; Access mode
		mov	[cs:r_bx], bx		; Open mode (For function 6C)
		mov	[cs:r_cx], cx		; Attrib mask
		mov	[cs:r_dx], dx	; Action if file do/doesn't exist (6C)
		mov	[cs:r_es], es
		mov	[cs:r_ds], ds
		mov	[cs:r_si], si
		mov	[cs:r_di], di

		call	call_int21
		savef	[cs:flags]
		mov	[cs:s_ax], ax		; Saved result
		mov	[cs:s_cx], cx		; Saved result (6C)
		jc	i213d_ckerr
		jmp	i213d_found		; File found, return
i213d_ckerr:	call	check_error
		jnc	i213d_srch
		jmp	i213d_found		; File found, other error

i213d_srch:	test	word [cs:append_state], APPEND_ENVIRON	; /E?
		jz	i213d_noenv
		call	copy_environ	; Copy env APPEND to append_path

		; Check if the file contains a drive letter. If so, continue
		; only if APPEND_SRCHDRV is set.
		; PATH:[ON|OFF] also toggles this flag
i213d_noenv:	mov	ds, [cs:filename_seg]
		mov	si, [cs:filename_off]
		cmp	byte [ds:si+1], ':'
		jne	i213d_basename
		test	word [cs:append_state], APPEND_SRCHDRV
		jnz	i213d_basename
		jmp	i213d_done

		; Get the file basename. If it is not the same as the
		; complete path, continue only if /PATH:ON
i213d_basename: call	basename
		cmp	[cs:filename_off], si
		je	i213d_srch2	; Okay, filename had no path
		test	word [cs:append_state], APPEND_SRCHPTH ;  /PATH:ON
		jnz	i213d_srch2
		jmp	i213d_done

i213d_srch2:	mov	[cs:basename_off], si	; Store offset to basename

		mov	ax, cs
		mov	es, ax
		mov	ds, ax

		mov	si, append_path
		lds	di, [cs:new_filepath]
		call	find_first		; Get first path from APPEND

i213d_srchloop: cmp	byte [cs:di], 0		; No more paths
		jz	i213d_done

		mov	di, ax		; Appends filename to new_filepath
		mov	si, [cs:basename_off]
		push	ds
		mov	ds, [cs:filename_seg]
		call	strcpy		; Now new_filepath has the complete path
		pop	ds
					; Function 3D:
		mov	ax, [cs:r_ax]
		mov	bx, [cs:r_bx]
		mov	cx, [cs:r_cx]
		lds	dx, [cs:new_filepath]; F 3D uses ds:dx for filename
		cmp	ah, 0x6C
		jne	i213d_callint
					; Function 6C:
		lds	si, [cs:new_filepath]; 6C uses ds:si for filename
		mov	dx, [cs:r_dx]	; Action if file does/doesn't exist

i213d_callint:	call	call_int21
		savef	[cs:flags]
		mov	[cs:s_ax], ax		; Saved result
		mov	[cs:s_cx], cx		; Saved result (6C)
		jnc	i213d_found		; Success
		call	check_error
		jc	i213d_found		; File found, other error

i213d_again:	lds	di, [cs:new_filepath]
		call	find_next
		jmp	i213d_srchloop

i213d_found:	call	set_rfn ; Set real filename if rfnstate is set
				; Warning: Trashes ds, si, dx, es

i213d_done:	mov	ax, [cs:s_ax]	; Restore saved result
		mov	bx, [cs:r_bx]
		mov	cx, [cs:s_cx]	; Restore saved result (6C)
		mov	dx, [cs:r_dx]
		mov	es, [cs:r_es]
		mov	ds, [cs:r_ds]
		mov	si, [cs:r_si]
		mov	di, [cs:r_di]

		call	clear_rfnstate	; Clear return found name state
		updatef [cs:flags]
		iret


; ---------------------------------------------------------------------------
;  214B00 : EXEC - LOAD AND EXECUTE PROGRAM
; ---------------------------------------------------------------------------
;
i214b00:	popf
		mov	[cs:filename_off], dx
		mov	[cs:filename_seg], ds
		mov	[cs:r_ax], ax
		mov	[cs:r_bx], bx
		mov	[cs:r_cx], cx
		mov	[cs:r_es], es
		mov	[cs:r_si], si
		mov	[cs:r_di], di

		call	call_int21
		savef	[cs:flags]
		jc	i214b00_ckerr
		jmp	i214b00_done		; File found, return
i214b00_ckerr:	call	check_error
		jnc	i214b00_srch
		jmp	i214b00_done		; File found, other error

i214b00_srch:	test	word [cs:append_state], APPEND_ENVIRON	; /E?
		jz	i214b00_noenv
		call	copy_environ	; Copy env APPEND to append_path

i214b00_noenv:	mov	ds, [cs:filename_seg]
		mov	si, [cs:filename_off]
		call	basename
		mov	[cs:basename_off], si	; Store offset to basename

		mov	ah, 0x2F		; Saves caller's DTA
		call	call_int21
		mov	[cs:DTA], bx
		mov	[cs:DTA+2], es

		mov	ax, cs
		mov	es, ax
		mov	ds, ax

		mov	ah, 0x1A		; Sets temporary DTA
		lds	dx, [cs:tempDTA]
		call	call_int21

		mov	si, append_path
		lds	di, [cs:new_filepath]
		call	find_first		; Get first path from APPEND

i214b00_schlp:	cmp	byte [cs:di], 0		; No more paths
		jz	i214b00_nfound

		mov	di, ax		; Appends filename to new_filepath
		mov	si, [cs:basename_off]
		push	ds
		mov	ds, [cs:filename_seg]
		call	strcpy		; Now new_filepath has the complete path
		pop	ds

		mov	ax, 0x4E00		; FindFirst
		mov	cx, 0x0037		; Do not find volume labels
		lds	dx, [cs:new_filepath]
		call	call_int21

		jnc	i214b00_found		; Success
		call	check_error
		jc	i214b00_found		; File found, other error

		lds	di, [cs:new_filepath]
		call	find_next
		jmp	i214b00_schlp

i214b00_found:	lds	dx, [cs:new_filepath]	; Exec with found path
		jmp	i214b00_exec

i214b00_nfound: mov	dx, [cs:filename_off]	; Exec with orig path
		mov	ds, [cs:filename_seg]

i214b00_exec:	push	dx			; Restores caller's DTA
		push	ds
		mov	ah, 0x1A
		mov	dx, [cs:DTA]
		mov	ds, [cs:DTA+2]
		call	call_int21
		pop	ds
		pop	dx

		mov	ax, [cs:r_ax]
		mov	bx, [cs:r_bx]
		mov	cx, [cs:r_cx]
		mov	es, [cs:r_es]
		mov	si, [cs:r_si]
		mov	di, [cs:r_di]
		call	call_int21
		savef	[cs:flags]
		mov	ds, [cs:filename_seg]	; Restore caller's ds
		mov	dx, [cs:filename_off]	; and dx

i214b00_done:	updatef [cs:flags]
		iret

; ---------------------------------------------------------------------------
; Function: call_int21	- Simulate an int21 call.
;
; Args:	   [cs:handler] - Call original or user provided handler
;
call_int21:	pushf
		test	byte [cs:handler], USERHNDL
		jnz	cint21_usr
		call	far [cs:old_int21]
		ret
cint21_usr:	call	far [cs:usr_int21]
		ret


; ---------------------------------------------------------------------------
; Function: set_rfn  - Set real filename if rfnstate is set
;
; Args:
;
; Returns:  Modifies bx, ds, si, dx, es, carry
;	    Updates file name buffer
;
; NOTES - Contrary to what is stated in RBIL, MS APPEND does not return the
;	  fully qualified name. It returns the search path prepended to the
;	  real file name (eg , if APPEND=\soMEdir and we search for
;	  file.TxT, it returns \soMEdir\file.TxT). In case of a FindFirst
;	  call with AL==00, it returns the search path prepended to the real
;	  (with wildcards expanded) filename in the DTA.
;
set_rfn:	push	ax

		les	bx, [cs:rfnstate]
		test	byte [es:bx], 0x01
		jz	sr_done

		mov	ax, [cs:r_ax]
		mov	bx, cs
		mov	ds, bx

		cmp	ax, 0x4E00	; AL nonzero has special meaning for
		jne	sr_copy		; setrfn in function FindFirst.

		mov	ah, 0x2F	; Get current Disk Transfer Area
		call	call_int21	; (returned in ES:BX)
		add	bx, 0x1E	; Offset of uppercased found filename

		lds	si, [cs:new_filepath]
		call	basename	; DS:SI points to basename
		mov	di, si
		mov	si, bx
		push	ds
		push	es
		pop	ds
		pop	es
		call	strcpy		; new_filepath updated with
					; uppercased found filename

sr_copy:	lds	si, [cs:new_filepath]
		mov	es, [cs:filename_seg]
		mov	di, [cs:filename_off]
		call	strcpy			; Copy last file path

sr_done:	pop	ax
		ret

; ---------------------------------------------------------------------------
; Function: clear_rfnstate - Clear return real file name state in caller's PSP
;
clear_rfnstate: push	bx
		push	es
		les	bx, [cs:rfnstate]
		and	byte [es:bx], 0 ; Clear state in caller's PSP
		pop	es
		pop	bx
		ret

; ---------------------------------------------------------------------------
; Function: get_rfnstate - Get and store pointer to real filename state flag
;
; Returns:  [rfnstate] - Pointer to RFN state flag
;
get_rfnstate:	push	ax
		push	bx
		mov	ah, 0x51	; Get caller's PSP pointer
		call	call_int21
		mov	word [cs:rfnstate], 0x003D
		mov	[cs:rfnstate+2], bx
		pop	bx
		pop	ax
		ret

; ---------------------------------------------------------------------------
; Function: check_error - Check if error other than file or path not found
;
; Returns:  Carry set if error other than file or path not found or no more
;	    files
;
check_error:	cmp	al, 0x02	; File not found
		jnc	ce_ret
		cmp	al, 0x03	; Path not found
		jnc	ce_ret
		cmp	al, 0x12	; No more files
ce_ret:		ret


; ---------------------------------------------------------------------------
; Function: check_extended  - Check true error for FCB functions
;
; Returns:  Carry set if extended error other than file or path not found or
;	    no more files
;
check_extended: push	ax
		push	bx
		push	cx
		push	dx
		push	es
		push	ds
		push	si
		push	di
		push	bp

		mov	ah, 0x59
		xor	bx, bx
		call	call_int21

		call	check_error

		pop	bp
		pop	di
		pop	si
		pop	ds
		pop	es
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret


;-----------------------------------------------------------------------------
; FCB manipulation
;-----------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Function: save_drive - Gets and stores drive number from file FCB
;
; Args:	    [FCB_seg]
;	    [FCB_off]  - Pointer to unopened FCB
;
; Returns:  [defdrive] - Drive number (0 == default, 1 == 'A', 2 == 'B', ...)
;
save_drive:	push	ds
		push	si
		push	ax

		mov	ds, [cs:FCB_seg]
		mov	si, [cs:FCB_off]

		mov	al, [ds:si]
		cmp	al, 0xFF		; Extended FCB
		jne	drive_found

		mov	al, [ds:si+7]		; Drive from extended FCB

drive_found:	mov	[cs:defdrive], al
		pop	ax
		pop	si
		pop	ds
		ret

; ---------------------------------------------------------------------------
; Function: set_drive  - Sets drive in FCB
;
; Args:	    AL	       - Drive number
;	    [FCB_seg]
;	    [FCB_off]  - Pointer to unopened FCB
;
; Returns:  Updates FCB
;
set_drive:	push	ds
		push	si
		push	ax

		mov	ds, [cs:FCB_seg]
		mov	si, [cs:FCB_off]

		mov	ah, [ds:si]
		cmp	ah, 0xFF		; Extended FCB
		jne	sd_doit
		add	si, 7

sd_doit:	mov	[ds:si], al
		pop	ax
		pop	si
		pop	ds
		ret


; ---------------------------------------------------------------------------
; Function: restor_drive - Restore drive in FCB from saved value
;
; Args:	    [FCB_seg]
;	    [FCB_off]  - Pointer to unopened FCB
;	    [defdrive] - Drive number (0 == default, 1 == 'A', 2 == 'B', ...)
;
restor_drive:	push	ax
		mov	al, [cs:defdrive]
		call	set_drive
		pop	ax
		ret

; ---------------------------------------------------------------------------
; Function: restor_path - Restores default path for drive letter
;
; Args:	    dirname    - Buffer with saved path, including drive letter
;
; Returns:  Carry set if invalid drive (ignore, there's nothing we can do)
;
restor_path:	push	ax
		push	dx
		push	ds

		lds	dx, [cs:dirname]
		mov	ah, 0x3B	; Set current directory
		call	call_int21

		pop	ds
		pop	dx
		pop	ax
		ret


; ---------------------------------------------------------------------------
; Function: set_newpath	 - Set new drive letter and default path from
;			   new_filepath
;
; Args:	    new_filepath - Buffer with new path, including drive letter
;
; Returns:  Fill [dirname] with old path
;	    Carry set if drive is not valid
;
set_newpath:	clc
		push	ax
		push	dx
		push	di
		push	si
		push	ds
		push	es

		lds	si, [cs:dirname]

		les	di, [cs:new_filepath]
		cmp	byte [es:di+1], ':'		; Check if there is a
							; drive letter
		je	sn_getdrive

		xor	al, al				; Default drive
		jmp	sn_setdrive

sn_getdrive:	mov	al, [es:di]			; Stores drive letter
		mov	byte [ds:si], al		; in buffer
		inc	si
		mov	byte [ds:si], ':'
		inc	si
		toupper al
		sub	al, 0x40	; 'A' == 1, ... ('A' == 0x41 ASCII)

sn_setdrive:	call	set_drive	; Set new drive in FCB

		mov	byte [ds:si], '\'	; Add leading backslash
		inc	si

		push	cs
		pop	ds
		mov	dl, al
		mov	ah, 0x47
		call	call_int21		; Get current path for drive
		jc	sn_return

		lds	dx, [cs:new_filepath]
		mov	ah, 0x3B	; Set default directory
		call	call_int21

sn_return:	pop	es
		pop	ds
		pop	si
		pop	di
		pop	dx
		pop	ax
		ret


;-----------------------------------------------------------------------------
; File path manipulation
;-----------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Function: get_drive - Gets drive letter from a path
;
; Args:	    CS:SI      - Null-terminated path
;
; Returns:  AL	       - Drive ('A', 'B', ... 0xFF == none)
;			 WARNING  - AL can contain an invalid drive letter
;				    invalid data. Must be checked.
;
get_drive:	mov	al, 0xFF
		cmp	byte [cs:si+1], ':'
		jne	gd_nodrive
		mov	al, [cs:si]
gd_nodrive:	ret

; ---------------------------------------------------------------------------
; Function: basename - Gets the file name part of a path
;
; Args:	    DS:SI    - Null terminated path
;
; Returns:  DS:SI    - Null terminated file name (basename)
;
basename:	push	ax
		mov	ah, 0
bn_loop:	mov	al, [ds:si]
		or	al, al
		jz	bn_searchbs
		inc	ah
		inc	si
		jmp	bn_loop
bn_searchbs:	or	ah, ah
		jz	bn_return
		mov	al, [ds:si-1]
		cmp	al, '\'
		je	bn_return
		cmp	al, ':'
		je	bn_return
		dec	si
		dec	ah
		jmp	bn_searchbs
bn_return:	pop	ax
		ret

;-----------------------------------------------------------------------------
; String manipulation
;-----------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Function: strcpy - Copy a null terminated string
;
; Args:	    DS:SI  - Null terminated origin
;	    ES:DI  - Destination
;
; Returns:  [ES:DI] buffer updated
;
strcpy:		push	di
		push	si
		cld
strcpy_loop:	movsb
		cmp	byte [ds:si-1], 0
		jnz	strcpy_loop
strcpy_end:	pop	si
		pop	di
		ret


findp		dw	0		; Pointer to next element
; ---------------------------------------------------------------------------
; Function: find_first - Gets first element from a semicolon separated list
;
; Args:	    CS:SI      - Semicolon separated, null terminated string
;	    DS:DI      - Buffer to hold the null terminated element,
;			 without semicolon
;
; Returns:  AX	       - points to the end of the CS:DI string
;	    [findp]    - Points to next element in list
;
find_first:	push	si
		push	di
		push	cx
		xor	cx, cx
ff_loop:	mov	al, [cs:si]
		mov	[ds:di], al
		cmp	al, ';'
		je	ff_found
		or	al, al
		jz	ff_eostr
		inc	di
		inc	si
		inc	cx
		jmp	ff_loop
ff_found:	inc	si
ff_eostr:	or	cx, cx
		jz	ff_empty
		mov	byte [ds:di], 0x5C	; Add '\' to the end
		inc	di
		mov	byte [ds:di], 0		; Terminate string
ff_empty:	mov	[cs:findp], si		; Store pointer to next element
		mov	ax, di
		pop	cx
		pop	di
		pop	si
		ret

; ---------------------------------------------------------------------------
; Function: find_next - Gets next element from a semicolon separated list
;			Must be called after a find_first invocation
;
; Args:	    CS:DI     - Buffer to hold the null terminated element,
;			 without semicolon
;
; Returns:  AX	      - points to the end of the CS:DI string
;	    [findp]   - Points to next element in list
;
find_next:	push	si
		push	di
		push	cx
		xor	cx,cx
		mov	si, [cs:findp]		; Points to next element
		jmp	ff_loop



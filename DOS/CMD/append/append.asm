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
; 04-06-01  casino_e@terra.es	First version
; 04-06-03  casino_e@terra.es	Add note to help stating that when /E is used,
;				no paths that can appear on the command line.
; 04-06-13  Eric Auer		Make older versions of nasm happy
; 04-06-16  Eric Auer /
;	    casino_e@terra.es	Convert to COM
; 04-06-17  casino_e@terra.es	Put tempDTA and part of cmdline in PSP
; 04-06-18  casino_e@terra.es	Remove unnecessary cs -> ds setting
; 04-06-27  casino_e@terra.es	Back again to EXE (as suggested by Arkady),
;				but just adding a minimalistic EXE header
;				Bug: Now it _really_ frees the environment
; 06-01-23  casino_e@terra.es   Add version string
;
VERSION equ "5.0.0.6"

.model small
.8086

;include "exebin.mac"
; -*- nasm -*-
; NASM macro file to allow the `bin' output format to generate
; simple .EXE files by constructing the EXE header by hand.
; Adapted from a contribution by Yann Guidon <whygee_corp@hol.fr>
; Modified by Eduardo Casino <casino_e@terra.es> with info by
;   Arkady V. Belousov

;EXE_headersize equ offset header_end  - offset header_start
;EXE_acodesize  equ offset EXE_endcode - offset EXE_startcode
;EXE_datasize   equ offset EXE_enddata - offset EXE_startdata
;EXE_absssize   equ offset EXE_endbss  - offset EXE_startbss
;EXE_allocsize  equ EXE_acodesize + EXE_datasize + EXE_headersize
; 
;EXE_stacksize equ 800h	; default if nothing else was used

public start
public append_state
public append_path
public append_prefix
public cmd_id
public NoAppend
public TooMany
public Help
public Invalid

extern p_flags :byte
extern cmdline :word
extern tempDTA :dword
extern old_int2f :dword
extern old_int21 :dword
extern get_environ :near
extern parse_cmds  :near
extern int2f       :near
extern int21       :near

; ===========================================================================
; RESIDENT PART
; ===========================================================================

; PSP
;
; tempDTA	times 0x80 db 0
; cmdline	times 0x100 db 0	; First 128 bytes of cmdline in PSP
;
;EXE_begin
          ;.code
	  ;section .text
          ;.code

if 0
_TEXT segment word public 'CODE' use16

	  org 0E0h
header_start:
	  db 4Dh,5Ah		; EXE file signature
	  dw EXE_allocsize mod 512
	  dw (EXE_allocsize + 511) / 512
	  dw 0			; relocation information: none
	  dw 2			; header size in paragraphs
	  ;dw (EXE_absssize + EXE_realstacksize) / 16 ; min extra mem
	  dw (EXE_absssize + EXE_stacksize) / 16 ; min extra mem
	  ;dw (EXE_absssize + EXE_realstacksize) / 16 ; max extra mem
	  dw (EXE_absssize + EXE_stacksize) / 16 ; max extra mem
	  dw -10h		; Initial SS (before fixup)
	  ;dw offset EXE_endbss + EXE_realstacksize ; Initial SP (1K DPMI+1K STACK)
	  dw (EXE_allocsize + EXE_stacksize) ; Initial SP (1K DPMI+1K STACK)
	  dw 0			; (no) Checksum
	  dw start		; Initial IP
	  dw -10h		; Initial CS (before fixup)
	  dw 0	 		; file offset to relocation table
	  dw 0			; (no overlay)
	  ;align 16
          dw 0
          dw 0
header_end:

_TEXT ends
endif

_TEXT segment

;SECTION .text
                ;.code
		;times 0x80 db 0		; Last 128 bytes of cmdline
                ;;;;db 80h dup(0)		; Last 128 bytes of cmdline
                ;org 0E0h

include useful.inc
include cmdline.inc
include append.inc

;include	environ.asm
;include	cmdline.asm
;include	int2f.asm
;include	int21.asm

cmd_id:
append_prefix	db	"APPEND="
;append_path	times 0x100 db 0
append_path	db 100h dup(0)

append_state	dw	0011000000000001b
;			||||\_________/|
;			||||	 |     +- 0	set if APPEND enabled
;			||||	 +------- 1-11	reserved
;			|||+------------- 12	(DOS 5.0) set if APPEND
;			|||			   applies directory search
;			|||			   even if a drive has been
;			|||			   specified
;			||+-------------- 13	set if /PATH flag active
;			|+--------------- 14	set if /E flag active
;			|			   (environment var APPEND
;			|			    exists)
;			+---------------- 15	set if /X flag active
;
;APPEND_ENABLED	equ	0000000000000001b
;APPEND_SRCHDRV	equ	0001000000000000b
;APPEND_SRCHPTH	equ	0010000000000000b
;APPEND_ENVIRON	equ	0100000000000000b
;APPEND_EXTENDD	equ	1000000000000000b

NoAppend	db	13, "There are no appended directories.", 13, 10, '$'
Invalid		db	13, "Invalid switch  - ", '$'
TooMany		db	13, "Too many parameters -  ", '$'
NotAllw		db	13, "Parameter value not allowed -  ", '$'

if 0
Help	db	13, "FreeDOS APPEND. Enables programs to open data files in "
	db		"specified directories as", 13, 10
	db		"		 if the files were in the current "
	db		"directory.", 13, 10
	db	13, "Ver. ", VERSION, " (C) 2004-2006 Eduardo Casino, under "
	db	"the GNU GPL, Version 2", 13, 10, 10
	db	"Syntax:", 13, 10, 10
	db	"  APPEND [[drive:]path[", 59, "...]] [/X[:ON|:OFF]] "
	db		"[/PATH:ON|/PATH:OFF] [/E]", 13, 10
	db	"  APPEND ", 59, 13, 10, 10
	db	"    [drive:]path Drive and directory to append."
	db		13, 10
	db	"    /X[:ON]	  Extend APPEND to "
	db		"searches and command execution.", 13, 10
	db	"    /X:OFF	  Applies APPEND only to "
	db		"requests to open files.", 13, 10
	db	"		  Defaults to /X:OFF", 13, 10
	db	"    /PATH:ON	  Search appended directories for file "
	db		"requests that already", 13, 10
	db	"		  include a path.  This is the default "
	db		"setting.", 13, 10
	db	"    /PATH:OFF	  Switches off /PATH:ON.", 13, 10
	db	"    /E		  Stores the appended directory "
	db		"list in the environment.", 13, 10
	db	"		  /E may be used only in the first invocation "
	db	"of APPEND. You", 13, 10
	db	"		  can not include any paths on the same "
	db	"command line as /E.", 13, 10, 10
	db	"  APPEND ", 59, " clears the list of appended "
	db		"directories.", 13, 10
	db	"  APPEND without parameters displays the list of appended "
	db		"directories.", 13, 10, '$'
endif

Help	db	13, "Use the APPEND command to set a search path for data files", 13, 10
    	db	    "that are outside the current directory.", 13, 10
    	db	    "Syntax:", 13, 10
    	db	    "  APPEND [drive][path];[drive][path][/PATH:ON or /PATH:OFF][/E]", 13, 10
    	db	    "where:", 13, 10
    	db	    "  drive\path   Specifies the drive and directory to append.  You can", 13, 10
    	db	    "               specify more than one directory by using a semicolon.", 13, 10
    	db	    "  /PATH:ON     Adds appended directories to file requests that", 13, 10
    	db	    "               that currently specify a path.  /PATH:ON is the default.", 13, 10
    	db	    "  /PATH:OFF    Turns off the effect of PATH:ON.", 13, 10
    	db	    "  /E           Files a copy of the appended directory list in an", 13, 10
    	db	    "               environment variable named APPEND.  /E can be used", 13, 10
    	db	    "               only the first time you APPEND after starting your", 13, 10
    	db	    "               session.", 13, 10
    	db	    "", 13, 10
    	db	    "To cancel the APPEND command, type: APPEND ;", 13, 10
    	db	    "To display the appended directory list, type: APPEND", 13, 10, '$'

end_resident:
; ================== END OF RESIDENT CODE ================================

WrnInstalled	db	13, "APPEND is already installed.", 13, 10, '$'
WrongAppend	db	13, "You are attempting to use an incorrect version of APPEND.", 13, 10, '$'

start:		mov	ax, 0B710h	; Check if we're already installed
		mov	dx, 00000h
		int	2Fh

		cmp	dx, 0000h	; Not installed
		je	install

		cmp	dl, 5		; Check installed version
		jne	wrong
		cmp	dh, 0
		je	installed

wrong:		mov	dx, offset WrongAppend
		mov	ah, 09h
		int	21h
		jmp	quit

installed:	mov	dx, offset WrnInstalled
		mov	ah, 09h
		int	21h

quit:		mov	ax, 4C01h	; Exit, errorlevel 1
		int	21h

install:	mov	bx, cs

		; points tempDTA to first 128 bytes of PSP and cmdline to last
		; 128 bytes of PSP plus first 128 bytes of program. cmdline
		; already contains 0x80
		;
		mov	word ptr cs:[tempDTA+2], bx
		mov	cs:[cmdline+2], bx

		call	get_environ	; Get PARENT environment

		; Parse command line parameters.
		;
		mov	es, bx		; ES:SI to command line
		mov	si, 80h
		xor	cx, cx
		mov	cl, es:[si]	; Length of command line
		inc	si
		call	parse_cmds
		jc	quit

		; Free some bytes, release environment
		;
		mov	bx, es:[2Ch]	 ; Segment of environment
		mov	es, bx
		mov	ah, 49h		 ; Free memory
		int	21h

		; Get vect to original int2f handler
		;
		mov	ax, 352Fh
		int	21h		; get vector to ES:BX
		mov	ax, es
		mov	word ptr [old_int2f], bx
		mov	word ptr [old_int2f+2], ax

		; Now, install new int2f handler
		;
		mov	ax, 252Fh
		mov	dx, offset int2f
		int	21h		; DS:DX -> new interrupt handler

		; Get vect to original int21 handler
		;
		mov	ax, 3521h
		int	21h		; get vector to ES:BX
		mov	ax, es
		mov	word ptr [old_int21], bx
		mov	word ptr [old_int21+2], ax

		; Now, install new int21 handler
		;
		mov	ax, 2521h
		mov	dx, offset int21
		int	21h		; DS:DX -> new interrupt handler

		mov	byte ptr [p_flags], RESIDENT     ; Set resident flag
						     ; and clean the rest

		; Terminate and stay resident
		;
		mov	dx, offset end_resident+15
		mov	cl, 4
		shr	dx, cl		; Convert to paragraphs

		mov	ax, 3100h	; Errorlevel 0
		int	21h

;EXE_end
	  ;section .text
          ;.code
;EXE_endcode:

_TEXT ends

;_DATA segment

	  ;section .data
          ;.data
;EXE_enddata:

;_DATA ends

;_BSS  segment

	  ;section .bss
	  ;alignb 2
;EXE_endbss:

;_BSS  ends

;STACK segment

          ;.stack
;EXE_endstack:

;STACK ends

DGROUP group _TEXT ;, _DATA, _BSS, STACK

       end

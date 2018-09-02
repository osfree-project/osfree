;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosSetVerify DOS wrapper
;
;   (c) osFree Project 2018, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;--------D-212E--DL00-------------------------
;INT 21 - DOS 1+ - SET VERIFY FLAG
;        AH = 2Eh
;        DL = 00h (DOS 1.x/2.x only)
;        AL = new state of verify flag
;            00h off
;            01h on
;Notes:  default state at system boot is OFF
;        when ON, all disk writes are verified provided the device driver
;          supports read-after-write verification
;SeeAlso: AH=54h
;
;
;*/

.8086

		; Helpers
		INCLUDE	helpers.inc

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		@PROLOG	DOSSETVERIFY
VERIFYSETTING	DW	?
		@START	DOSSETVERIFY
		XOR	DL, DL
		MOV	AX, [DS:BP].VERIFYSETTING
		MOV	AH, 2EH
		INT	21H
		XOR	AX, AX
		@EPILOG	DOSSETVERIFY

_TEXT		ENDS

		END

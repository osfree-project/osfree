;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosGetVersion DOS wrapper
;
;   (c) osFree Project 2018, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;
;
;
;--------D-2130-------------------------------
;INT 21 - DOS 2+ - GET DOS VERSION
;        AH = 30h
;---DOS 5+ ---
;        AL = what to return in BH
;            00h OEM number (see #01394)
;            01h version flag
;Return: AL = major version number (00h if DOS 1.x)
;        AH = minor version number
;        BL:CX = 24-bit user serial number (most versions do not use this)
;---if DOS <5 or AL=00h---
;        BH = MS-DOS OEM number (see #01394)
;---if DOS 5+ and AL=01h---
;        BH = version flag
;            bit 3: DOS is in ROM
;            other: reserved (0)
;
;*/

.8086

		; Helpers
		INCLUDE	helpers.inc

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		@PROLOG	DOSGETVERSION
VERSIONWORD	DD	?
		@START	DOSGETVERSION
		MOV	AH, 30H
		INT	21H
		XCHG	AH, AL
		LDS	BP, [DS:BP].VERSIONWORD
		MOV	[DS:BP], AX
		XOR	AX, AX
		@EPILOG	DOSGETVERSION

_TEXT		ENDS

		END

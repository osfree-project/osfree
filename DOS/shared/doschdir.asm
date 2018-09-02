;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosChDir DOS wrapper
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
;--------D-213B-------------------------------
;INT 21 - DOS 2+ - "CHDIR" - SET CURRENT DIRECTORY
;        AH = 3Bh
;        DS:DX -> ASCIZ pathname to become current directory (max 64 bytes)
;Return: CF clear if successful
;            AX destroyed
;        CF set on error
;            AX = error code (03h) (see #01680 at AH=59h/BX=0000h)
;Notes:  if new directory name includes a drive letter, the default drive is
;          not changed, only the current directory on that drive
;        changing the current directory also changes the directory in which
;          FCB file calls operate
;        under the FlashTek X-32 DOS extender, the pointer is in DS:EDX
;SeeAlso: AH=47h,AX=713Bh,INT 2F/AX=1105h
;
;
;*/

.8086

		; Helpers
		INCLUDE	helpers.inc

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		@PROLOG	DOSCHDIR
DIRNAME		DD	?
RESERVED	DD	?
		@START	DOSCHDIR
		LDS	DX, [DS:BP].DIRNAME
		MOV	AH, 3BH
		INT	21H
		JC	EXIT
		XOR	AX, AX
EXIT:
		@EPILOG	DOSCHDIR

_TEXT		ENDS

		END


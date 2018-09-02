;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosMkDir DOS wrapper
;
;   (c) osFree Project 2018, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;--------D-2139-------------------------------
;INT 21 - DOS 2+ - "MKDIR" - CREATE SUBDIRECTORY
;        AH = 39h
;        DS:DX -> ASCIZ pathname
;Return: CF clear if successful
;            AX destroyed
;        CF set on error
;            AX = error code (03h,05h) (see #01680 at AH=59h/BX=0000h)
;Notes:  all directories in the given path except the last must exist
;        fails if the parent directory is the root and is full
;        DOS 2.x-3.3 allow the creation of a directory sufficiently deep that
;          it is not possible to make that directory the current directory
;          because the path would exceed 64 characters
;        under the FlashTek X-32 DOS extender, the pointer is in DS:EDX
;SeeAlso: AH=3Ah,AH=3Bh,AH=6Dh,AX=7139h,AH=E2h/SF=0Ah,AX=43FFh/BP=5053h
;SeeAlso: INT 2F/AX=1103h,INT 60/DI=0511h
;
;
;*/

.8086

		; Helpers
		INCLUDE	helpers.inc

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		@PROLOG	DOSMKDIR
DIRNAME		DD	?
RESERVED	DD	?
		@START	DOSMKDIR
		LDS	DX, [DS:BP].DIRNAME
		MOV	AH, 39H
		INT	21H
		JC	EXIT
		XOR	AX, AX
EXIT:
		@EPILOG	DOSMKDIR

_TEXT		ENDS

		END




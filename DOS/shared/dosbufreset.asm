;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosBufReset DOS wrapper
;
;   (c) osFree Project 2018, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
; --------D-2168-------------------------------
;INT 21 - DOS 3.3+ - "FFLUSH" - COMMIT FILE
;        AH = 68h
;        BX = file handle
;Return: CF clear if successful
;            all data still in DOS disk buffers is written to disk immediately,
;              and the file's directory entry is updated
;        CF set on error
;            AX = error code (see #01680 at AH=59h/BX=0000h)
;SeeAlso: AX=5D01h,AH=6Ah,INT 2F/AX=1107h
;
;
;*/

.8086

		; Helpers
		INCLUDE	helpers.inc

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		@PROLOG	DOSBUFRESET
FILEHANDLE	DW	?
		@START	DOSBUFRESET
		MOV	BX, [BP].FILEHANDLE
		MOV	AH, 68H
		INT	21H
		JC	EXIT
		XOR	AX, AX
EXIT:
		@EPILOG	DOSBUFRESET

_TEXT		ENDS

		END

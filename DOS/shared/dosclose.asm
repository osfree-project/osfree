;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosClose DOS wrapper
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
;--------D-213E-------------------------------
;INT 21 - DOS 2+ - "CLOSE" - CLOSE FILE
;        AH = 3Eh
;        BX = file handle
;Return: CF clear if successful
;            AX destroyed
;        CF set on error
;            AX = error code (06h) (see #01680 at AH=59h/BX=0000h)
;Notes:  if the file was written to, any pending disk writes are performed, the
;          time and date stamps are set to the current time, and the directory
;          entry is updated
;        recent versions of DOS preserve AH because some versions of Multiplan
;          had a bug which depended on AH being preserved
;SeeAlso: AH=10h,AH=3Ch,AH=3Dh,INT 2F/AX=1106h,INT 2F/AX=1227h
;
;
;*/

.8086

		; Helpers
		INCLUDE	helpers.inc

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		@PROLOG	DOSCLOSE
FILEHANDLE	DW	?
		@START	DOSCLOSE
		MOV	BX, [BP].FILEHANDLE
		MOV	AH, 3EH
		INT	21H
		JC	EXIT
		XOR	AX, AX
EXIT:
		@EPILOG	DOSCLOSE

_TEXT		ENDS

		END

;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosQCurDir DOS wrapper
;
;   (c) osFree Project 2018, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;*/

.8086

		; Helpers
		EXTRN	@INIT : NEAR
		EXTRN	@DONE : NEAR

_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		; API export
		PUBLIC	DOSQCURDIR

DriveNumber	EQU	[BP+14]
DirPath		EQU	[BP+10]
DirPathLen	EQU	[BP+6]

DOSQCURDIR	PROC	FAR
		CALL	@INIT
		CALL	@DONE
		RET	10
DOSQCURDIR	ENDP

_TEXT		ENDS

		END

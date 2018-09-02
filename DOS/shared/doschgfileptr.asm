;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosChgFilePtr DOS wrapper
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
		PUBLIC	DOSCHGFILEPTR

FileHandle	EQU	[BP+16]
Distance	EQU	[BP+12]
MoveType	EQU	[BP+10]
NewPointer	EQU	[BP+6]

DOSCHGFILEPTR	PROC	FAR
		CALL	@INIT
		CALL	@DONE
		RET	12
DOSCHGFILEPTR	ENDP

_TEXT		ENDS

		END

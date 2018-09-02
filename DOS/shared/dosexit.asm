;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosExit DOS wrapper
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
		PUBLIC	DOSEXIT

Action		EQU	[BP+8]
Result		EQU	[BP+6]

DOSEXIT		PROC	FAR
		CALL	@INIT
		CALL	@DONE
		RET	4
DOSEXIT		ENDP

_TEXT		ENDS

		END

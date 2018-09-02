;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosQFileMode DOS wrapper
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

_TEXT			SEGMENT DWORD PUBLIC 'CODE' USE16

			; API export
			PUBLIC	DOSQFILEMODE

FileName		EQU	[BP+14]
CurrentAttribute	EQU	[BP+10]
Reserved		EQU	[BP+6]

DOSQFILEMODE		PROC	FAR
			CALL	@INIT
			CALL	@DONE
			RET	12
DOSQFILEMODE		ENDP

_TEXT		ENDS

		END

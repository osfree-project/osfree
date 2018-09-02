;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosQFSInfo DOS wrapper
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
		PUBLIC	DOSQFSINFO

DriveNumber	EQU	[BP+14]
FSInfoLevel	EQU	[BP+12]
FSInfoBuf	EQU	[BP+8]
FSInfoBufSize	EQU	[BP+6]

DOSQFSINFO	PROC	FAR
		CALL	@INIT
		CALL	@DONE
		RET	10
DOSQFSINFO	ENDP

_TEXT		ENDS

		END

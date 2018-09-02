;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosQFileInfo DOS wrapper
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
		PUBLIC	DOSQFILEINFO

FileHandle	EQU	[BP+14]
FileInfoLevel	EQU	[BP+12]
FileInfoBuf	EQU	[BP+8]
FileInfoBufSize	EQU	[BP+6]

DOSQFILEINFO	PROC	FAR
		CALL	@INIT
		CALL	@DONE
		RET	10
DOSQFILEINFO	ENDP

_TEXT		ENDS

		END

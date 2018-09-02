;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosOpen DOS wrapper
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
		PUBLIC	DOSOPEN

FileName	EQU	[BP+28]
FileHandle	EQU	[BP+24]
ActionTaken	EQU	[BP+20]
FileSize	EQU	[BP+16]
FileAttribute	EQU	[BP+14]
OpenFlag        EQU	[BP+12]
OpenMode	EQU	[BP+10]
Reserved	EQU	[BP+6]

DOSOPEN		PROC	FAR
		CALL	@INIT
		CALL	@DONE
		RET	26
DOSOPEN		ENDP

_TEXT		ENDS

		END

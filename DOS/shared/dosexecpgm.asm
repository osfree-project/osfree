;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief DosExecPgm DOS wrapper
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
		EXTERN	@INIT : NEAR
		EXTERN	@DONE : NEAR


_TEXT		SEGMENT DWORD PUBLIC 'CODE' USE16

		; API export
		PUBLIC	DOSEXECPGM

ObjNameBuf	EQU	[BP+26]
ObjNameBuf	EQU	[BP+24]
ExecFlags	EQU	[BP+22]
ArgPointer	EQU	[BP+18]
EnvPointer	EQU	[BP+14]
ReturnCodes	EQU	[BP+10]
PgmPointer	EQU	[BP+6]

DOSEXECPGM	PROC	FAR
		CALL	@INIT
		CALL	@DONE
		RET	24
DOSEXECPGM	ENDP

_TEXT		ENDS

		END

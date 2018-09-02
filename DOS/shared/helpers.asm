;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief Family API implementation helpers
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
_TEXT   	SEGMENT DWORD PUBLIC 'CODE' USE16

		; Internal helper functions
	        PUBLIC	@INIT
	        PUBLIC	@INIT_STACK
		PUBLIC	@DONE

@INIT		PROC	NEAR
		XOR	AX, AX        
@INIT		ENDP

@INIT_STACK	PROC	NEAR
	        POP	_TEXT:[@RETADDR]
		PUSH	BP
		MOV	BP, SP
		SUB	SP, AX
		PUSH	SI
		PUSH	DI
		PUSH	DS
		PUSH	ES
		PUSH	BX
		PUSH	CX
		PUSH	DX
		JMP	_TEXT:[@RETADDR]
@INIT_STACK	ENDP

@DONE		PROC	NEAR
	        POP	_TEXT:[@RETADDR]
		POP	DX
		POP	CX
		POP	BX
		POP	ES
		POP	DS
		POP	DI
		POP	SI
		MOV	SP, BP
		POP	BP
		JMP	_TEXT:[@RETADDR]
@DONE		ENDP

@RETADDR	DW	0

_TEXT   ENDS

	END

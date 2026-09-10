;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief Family API implementation
;
;   (c) osFree Project 2018-2022, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;*/

@DEFINE		MACRO	CALLNAME
		IFNDEF	CALLNAME
			EXTERN	CALLNAME: FAR
		ENDIF
		ENDM

@PUSHW		MACRO	ARG
		MOV		AX,ARG
		PUSH	AX
		ENDM

@PUSHS		MACRO	ARG
		.ERRB	<ARG>
		a=.type ARG
		IF (A EQ 62h)
			MOV	AX, WORD PTR ARG
			PUSH	AX
			MOV	AX, WORD PTR ARG+2
			PUSH	AX
		else
			MOV		AX,SEG ARG
			PUSH	AX
			LEA		AX,ARG
			PUSH	AX
		endif
		ENDM

@PUSHD		MACRO	ARG
		.ERRB	<ARG>
		IF(OPATTR(ARG) EQ 35h)	; Immediate and defined
			MOV	AX, LOWWORD ARG
			PUSH	AX
			MOV	AX, HIGHWORD ARG
			PUSH	AX
		ELSE
			PUSH	DS		;SAVE DATA SEGMENT
			PUSH	BX		;SAVE CURRENT BX VALUE
			MOV	AX, SEG ARG
			MOV	DS, AX
			MOV	BX, OFFSET ARG	;LOAD BX WITH OFFSET VALUE
			PUSH	WORD PTR [BX]	;PUSH LOW-ORDER OF 32 BIT VAL STACK
			MOV	AX,[BX+2]	;PUT HIGH ORDER 32-BIT VALUE IN AX
			PUSH	BP		;SAVE REGISTER
			MOV	BP,SP
			XCHG	[BP+6],AX	;
			POP	BP		;RESTORE
			MOV	DS,AX		;RESTORE DS TO SAVED VALUE.
			POP	AX		;POP OFF LOW ORDER 32 BIT VAL
			POP	BX		;RESTORE BX TO SAVED VALUE.
			PUSH	AX		;PUT LOW ORDER 32 BIT VALUE ON STACK
		ENDIF
		ENDM

@LOADARG	MACRO	REG, ADR
		IF (.TYPE (ADR)) AND 00010000b		; REGISTER
			MOV	REG, ADR
		ELSEIF TYPE (ADR) EQ 1			; BYTES (1 BYTE)
			MOV	REG, OFFSET ADR
		ELSEIF TYPE (ADR) EQ 2			; NEAR POINTER (2 BYTE)
			MOV	REG, ADR
		ELSEIF TYPE (ADR) EQ 4			; FAR POINTER (4 BYTE)
			LDS	REG, ADR
		ELSE
			ECHO Bad argument
			.ERR
		ENDIF
		ENDM

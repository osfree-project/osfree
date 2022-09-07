;/*!
;   @file
;
;   @brief Mouse Router
;
;   (c) osFree Project 2018-2022, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS and OS/2
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;*/

.8086

		; OS/2
		INCLUDE	bsedos.inc
		INCL_MOU	EQU	1
		INCLUDE	bsesub.inc
		INCLUDE	bseerr.inc

		;
		INCLUDE	GlobalVars.inc

; Screen group controls block
SGCB	STRUC
	AMSPROC			DD	?	; AMSMAIN far address
	AMSHANDLE		DW	?	; AMSHANDLE module handle
	MOUFUNCTIONMASK		DD	0	; MOU FUNCTIONS REDIRECTION MASK
SGCB	ENDS

;------------------------------------------------------------MOU
@MOUPROLOG	MACRO	NAME


		PUBLIC	NAME

DELTA		=	0

@CATSTR(NAME,MOUARGS)	STRUC
		ENDM

;----------------------------------------------------------------------

@MOUSTART	MACRO	NAME
@CATSTR(NAME,MOUARGS)		ENDS
argsize=SIZE @CATSTR(NAME,MOUARGS)
@CATSTR(NAME,MOU_ARG_SIZE)	EQU	argsize

NAME		PROC	FAR

		ENDM

;------------------------------------------------------------

@MOUEPILOG	MACRO	NAME
		RETF	@CATSTR(NAME,MOU_ARG_SIZE)
NAME		ENDP
		ENDM

;------------------------------------------------------------

@MOUROUTE	MACRO	FUNC, OFF
		MOV	AX, @CATSTR(FC_, FUNC)
		PUSH	AX
		MOV	AX, _DATA
		MOV	ES, AX
		MOV	AX, WORD PTR ES:SG.MOUFUNCTIONMASK+OFF
		IF	OFF EQ 0
		AND	AX, LOWWORD @CATSTR(MR_, FUNC)
		CMP	AX, LOWWORD @CATSTR(MR_, FUNC)
		ELSE
		AND	AX, HIGHWORD @CATSTR(MR_, FUNC)
		CMP	AX, HIGHWORD @CATSTR(MR_, FUNC)
		ENDIF
		CALL    MOUROUTE
		ENDM

;------------------------------------------------------------

@MOUPROC	MACRO	NAME, OFF, ARGS: VARARG

		@MOUPROLOG	NAME
		FOR ARG, <ARGS>
			ARG
		ENDM
		@MOUSTART	NAME
		CALL	@CATSTR(Pre, NAME)
		CALL	NEAR PTR INIT
		@MOUROUTE	NAME, OFF
		CALL	@CATSTR(Post, NAME)
		@MOUEPILOG	NAME

		ENDM

; Global Data
_DATA		SEGMENT BYTE PUBLIC 'DATA' USE16

;-- move to SG structure (name it control block?) --
SG	SGCB <?, ?, 0>
;-- move to SG structure (name it control block?) --

MOUSEFLAG	DW	0		; Is mouse device driver presented
MOUSEDD		DB	'MOUSE$', 0	; Mouse device driver name
BMSCALLS	DB	'BMSCALLS', 0	; Base Mouse Subsystem filename
BMSMAIN		DB	'BMSMAIN', 0	; Base Mouse Subsystem BMSMAIN name
BMSPROC		DD	?		; Base Mouse Subsystem BMSMAIN address
BMSHANDLE	DW	?		; Base Mouse Subsystem handle

SHELL_PID	DW	?		; PID of session manager/shell. Used by MouFree to prevent non-sesmgs call.

;-- move to function local stack --
MOUSEH		DW	?		; Mouse device driver handle
MOUSEA		DW	?		; ActionTaken on mouse device driver open
GBL		DW	?		; Segment of GInfoSeg
LCL		DW	?		; Segment of LInfoSeg
;-- move to function local stack --

_DATA		ENDS

_TEXT		SEGMENT BYTE PUBLIC 'CODE' USE16

EXTERN	PreMOUCLOSE: PROC
EXTERN	PreMOUDRAWPTR: PROC
EXTERN	PreMOUFLUSHQUE: PROC
EXTERN	PreMOUGETDEVSTATUS: PROC
EXTERN	PreMOUGETEVENTMASK: PROC
EXTERN	PreMOUGETHOTKEY: PROC
EXTERN	PreMOUGETNUMBUTTONS: PROC
EXTERN	PreMOUGETNUMMICKEYS: PROC
EXTERN	PreMOUGETNUMQUEEL: PROC
EXTERN	PreMOUGETPTRPOS: PROC
EXTERN	PreMOUGETPTRSHAPE: PROC
EXTERN	PreMOUGETSCALEFACT: PROC
EXTERN	PreMOUINITREAL: PROC
EXTERN	PreMOUOPEN: PROC
EXTERN	PreMOUREADEVENTQUE: PROC
EXTERN	PreMOUREMOVEPTR: PROC
EXTERN	PreMOUROUTE: PROC
EXTERN	PreMOUSETDEVSTATUS: PROC
EXTERN	PreMOUSETEVENTMASK: PROC
EXTERN	PreMOUSETHOTKEY: PROC
EXTERN	PreMOUSETPTRPOS: PROC
EXTERN	PreMOUSETPTRSHAPE: PROC
EXTERN	PreMOUSETSCALEFACT: PROC
EXTERN	PreMOUSYNCH: PROC
EXTERN	PostMOUCLOSE: PROC
EXTERN	PostMOUDRAWPTR: PROC
EXTERN	PostMOUFLUSHQUE: PROC
EXTERN	PostMOUGETDEVSTATUS: PROC
EXTERN	PostMOUGETEVENTMASK: PROC
EXTERN	PostMOUGETHOTKEY: PROC
EXTERN	PostMOUGETNUMBUTTONS: PROC
EXTERN	PostMOUGETNUMMICKEYS: PROC
EXTERN	PostMOUGETNUMQUEEL: PROC
EXTERN	PostMOUGETPTRPOS: PROC
EXTERN	PostMOUGETPTRSHAPE: PROC
EXTERN	PostMOUGETSCALEFACT: PROC
EXTERN	PostMOUINITREAL: PROC
EXTERN	PostMOUOPEN: PROC
EXTERN	PostMOUREADEVENTQUE: PROC
EXTERN	PostMOUREMOVEPTR: PROC
EXTERN	PostMOUROUTE: PROC
EXTERN	PostMOUSETDEVSTATUS: PROC
EXTERN	PostMOUSETEVENTMASK: PROC
EXTERN	PostMOUSETHOTKEY: PROC
EXTERN	PostMOUSETPTRPOS: PROC
EXTERN	PostMOUSETPTRSHAPE: PROC
EXTERN	PostMOUSETSCALEFACT: PROC
EXTERN	PostMOUSYNCH: PROC
EXTERN	PreMOUSHELLINIT: PROC
EXTERN	PostMOUSHELLINIT: PROC
EXTERN	PreMOUFREE: PROC
EXTERN	PostMOUFREE: PROC

;
; This function called by Shell/Session manager during initialization
;
; 1. Check is mouse device driver exists
; 2. Remember Shell/Session Manager PID
; 3. Allocate memory for Screen Group variables
; 4. Load base mouse subsystem
;
; * NO_ERROR
; * ERROR_MOUSE_NO_DEVICE
;
		PUBLIC	MOUSHELLINIT
MOUSHELLINIT	PROC FAR
		CALL		PreMOUSHELLINIT
		CALL		NEAR PTR INIT
		CALL		PostMOUSHELLINIT
		RETF
MOUSHELLINIT	ENDP

trc1		db	'1',0dh,0ah
trc1s		equ	$-trc1
trc2		db	'2',0dh,0ah
trc2s		equ	$-trc2
trc3		db	'3',0dh,0ah
trc3s		equ	$-trc3
trc4		db	'4',0dh,0ah
trc4s		equ	$-trc4
trc5		db	'5',0dh,0ah
trc5s		equ	$-trc5

INIT		PROC NEAR
		PUSH		ES
		PUSH		DS

		MOV		AX, SEG _DATA
		MOV		ES, AX
		@DosOpen	ES:MOUSEDD, ES:MOUSEH, ES:MOUSEA, 0, 0, 1, 42h, 0
		CMP		AX, 0
		MOV		AX, ERROR_MOUSE_NO_DEVICE
		JNE		BAD
		@DosClose	[ES:MOUSEH]
		@DosGetInfoSeg	ES:GBL, ES:LCL
		PUSH		ES
		MOV		ES, ES:LCL
		MOV		AX, [ES:0]	; pidCurrent
		POP		ES
		MOV		ES:SHELL_PID, AX
		@DosLoadModule	0,0, ES:BMSCALLS,ES:BMSHANDLE
		CMP		AX, 0
		MOV		AX, ERROR_MOUSE_NO_DEVICE
		JNE		BAD
		@DosGetProcAddr	[ES:BMSHANDLE], ES:BMSMAIN, ES:BMSPROC
		CMP		AX, 0
		MOV		AX, ERROR_MOUSE_NO_DEVICE
		JNE		BAD
OK:
		MOV		ES:MOUSEFLAG, 1
		XOR		AX, AX
BAD:
		POP		DS
		POP		ES
		RETN
INIT		ENDP

;
; This function called during Shell/Sesson manager shutdown
;
; 1. Check is caller PID same as Shell/Session Manager PID
; 2. Check is mouse device driver presented
; 3. Uninitialize Screen Group variables
;
; * NO_ERROR
; * ERROR_MOUSE_NO_DEVICE
; * ERROR_MOUSE_SMG_ONLY
;
		PUBLIC	MOUFREE
MouFree		PROC FAR
		CALL		PreMOUFREE
		PUSH		ES
		PUSH		DS
		MOV		AX, _DATA
		MOV		ES, AX
		@DosGetInfoSeg	GBL, LCL
		MOV		DS, ES:LCL
		MOV		AX, lis_pidCurrent
		CMP		ES:SHELL_PID, AX
		MOV		AX, ERROR_MOUSE_SMG_ONLY
		JNE		EXIT
		@DosFreeModule	[ES:BMSHANDLE]
		XOR		AX, AX
EXIT:
		POP		DS
		POP		ES
		CALL		PostMOUFREE
		RETF
MouFree		ENDP


		PUBLIC	MOUSYNCH
MOUSYNCH	PROC FAR
ARGS	STRUC
FLWAIT	DW	?
ARGS	ENDS
		CALL	PreMouSynch
; code here
		CALL	PostMouSynch
		XOR		AX, AX
		RETF	SIZE ARGS
MOUSYNCH	ENDP

@MOUPROC	MOUOPEN, 0, MOUHANDLE DD ?, DRIVERNAME DD ?
@MOUPROC	MOUCLOSE, 0, MOUHANDLE DW ?
@MOUPROC	MOUDRAWPTR, 0, MOUHANDLE DW ?
@MOUPROC	MOUFLUSHQUE, 2, MOUHANDLE DW ?
@MOUPROC	MOUGETDEVSTATUS, 0, MOUHANDLE DW ?, DEVICSTATUS DD ?
@MOUPROC	MOUGETEVENTMASK, 0, MOUHANDLE DW ?, EVENTMASK DD ?
@MOUPROC	MOUGETHOTKEY, 0, MOUHANDLE DW ?, BUTTONMASK DD ?
@MOUPROC	MOUGETNUMBUTTONS, 0, MOUHANDLE DW ?, NUMBEROFBUTTONS DD ?
@MOUPROC	MOUGETNUMMICKEYS, 0, MOUHANDLE DW ?, NUMBEROFMICKEYS DD ?
@MOUPROC	MOUGETNUMQUEEL, 0, MOUHANDLE DW ?, QUEDATARECORD DD ?
@MOUPROC	MOUGETPTRPOS, 2, MOUHANDLE DW ?, PTRPOS DD ?
@MOUPROC	MOUGETPTRSHAPE, 0, MOUHANDLE DW ?, PTRDEFREC DD ?, PTRBUFFER DD ?
@MOUPROC	MOUGETSCALEFACT, 0, MOUHANDLE DW ?, SCALESTRUCT DD ?
@MOUPROC	MOUINITREAL, 2, DRIVERNAME DD ?
@MOUPROC	MOUREADEVENTQUE, 0, MOUHANDLE DW ?, READTYPE DD ?, BUFFER DD ?
@MOUPROC	MOUREMOVEPTR, 2, MOUHANDLE DW ?, PTRAREA DD ?
@MOUPROC	MOUSETDEVSTATUS, 2, HANDLE DW ?, DEVSTATUS DD ?
@MOUPROC	MOUSETEVENTMASK, 0, MOUHANDLE DW ?, EVENTMASK DD ?
@MOUPROC	MOUSETHOTKEY, 0, MOUHANDLE DW ?, BUTTONMASK DD ?
@MOUPROC	MOUSETPTRPOS, 2, MOUHANDLE DW ?, PTRPOS DD ?
@MOUPROC	MOUSETPTRSHAPE, 0, MOUHANDLE DW ?, PTRDEFREC DD ?, PTRBUFFER DD ?
@MOUPROC	MOUSETSCALEFACT, 0, MOUHANDLE DW ?, SCALESTRUCT DD ?

MOUROUTE	PROC	NEAR

		CALL		PreMOUROUTE

		JNZ	BMS		; Skip if AMS not registered
;Call alternate mouse subsystem if function routed
		PUSH	DS		; caller data segment
;		XOR	AX,AX
		MOV	AX, SEG _DATA
		MOV	ES, AX
		CALL	DWORD PTR [ES:SG.AMSPROC]
		POP	DS
; Return code = 0 
;           No error.  Do not invoke the corresponding Base Mouse Subsystem 
;           routine.  Return to caller with Return code = 0. 
; Return code = -1 
;           No error.  Invoke the corresponding Base Mouse Subsystem 
;           routine. Return to caller with Return code = return code from 
;           Base Mouse Subsystem. 
; Return code = error (not 0 or -1) 
;           Do not invoke the corresponding Base Mouse Subsystem routine. 
;           Return to caller with Return code = error. 
		CMP	AX, 0
		JZ	EXIT
		CMP	AX, -1
		JNZ	EXIT
BMS:
		PUSH	DS		; caller data segment
		MOV	AX, SEG _DATA
		MOV	ES, AX
;		MOV	AX,AX
		; Need to set AX to FC_*
		; Need to set stack frame correctly
		CALL	DWORD PTR ES:BMSPROC
		POP	DS

EXIT:
		CALL		PostMOUROUTE

		RETN	2			; POP function code
MOUROUTE	ENDP

_TEXT		ENDS
		END

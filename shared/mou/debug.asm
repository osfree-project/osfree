.8086

		include helpers.inc
		INCL_VIO	EQU	1
		include bsesub.inc

_DATA		SEGMENT BYTE PUBLIC 'DATA' USE16

@tracemsg2	MouClose
@tracemsg2	MouDrawPtr
@tracemsg2	MouFlushQue
@tracemsg2	MouFree
@tracemsg2	MouGetDevStatus
@tracemsg2	MouGetEventMask
@tracemsg2	MouGetHotKey
@tracemsg2	MouGetNumButtons
@tracemsg2	MouGetNumMickeys
@tracemsg2	MouGetNumQueEl
@tracemsg2	MouGetPtrPos
@tracemsg2	MouGetPtrShape
@tracemsg2	MouGetScaleFact
@tracemsg2	MouInitReal
@tracemsg2	MouOpen
@tracemsg2	MouReadEventQue
@tracemsg2	MouRemovePtr
@tracemsg2	MouRoute
@tracemsg2	MouSetDevStatus
@tracemsg2	MouSetEventMask
@tracemsg2	MouSetHotKey
@tracemsg2	MouSetPtrPos
@tracemsg2	MouSetPtrShape
@tracemsg2	MouSetScaleFact
@tracemsg2	MouShellInit
@tracemsg2	MouSynch

_DATA		ENDS

_TEXT		SEGMENT BYTE PUBLIC 'CODE' USE16


if 0
@LOCALW		GLOBALSEG
@LOCALW		LOCALSEG
		@START	DOSGETPID
		PUSH	SS
		LEA	AX, GLOBALSEG
		PUSH	AX
		PUSH	SS
		LEA	AX, LOCALSEG
		PUSH	AX
		CALL	DOSGETINFOSEG
		MOV	AX, LOCALSEG
endif

@tracecall2	MOUCLOSE
@tracecall2	MOUDRAWPTR
@tracecall2	MOUFLUSHQUE
@tracecall2	MOUFREE
@tracecall2	MOUGETDEVSTATUS
@tracecall2	MOUGETEVENTMASK
@tracecall2	MOUGETHOTKEY
@tracecall2	MOUGETNUMBUTTONS
@tracecall2	MOUGETNUMMICKEYS
@tracecall2	MOUGETNUMQUEEL
@tracecall2	MOUGETPTRPOS
@tracecall2	MOUGETPTRSHAPE
@tracecall2	MOUGETSCALEFACT
@tracecall2	MOUINITREAL
@tracecall2	MOUOPEN
@tracecall2	MOUREADEVENTQUE
@tracecall2	MOUREMOVEPTR
@tracecall2	MOUROUTE
@tracecall2	MOUSETDEVSTATUS
@tracecall2	MOUSETEVENTMASK
@tracecall2	MOUSETHOTKEY
@tracecall2	MOUSETPTRPOS
@tracecall2	MOUSETPTRSHAPE
@tracecall2	MOUSETSCALEFACT
@tracecall2	MOUSHELLINIT
@tracecall2	MOUSYNCH

_TEXT		ENDS

			END
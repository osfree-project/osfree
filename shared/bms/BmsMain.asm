;/*!
;   @file
;
;   @brief BmsMain entry point wrapper
;
;   (c) osFree Project 2021, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;*/

.8086

		; OS/2
		INCLUDE bseerr.inc
INCL_MOU	EQU	1
		INCLUDE bsesub.inc
		INCLUDE bsedos.inc

_TEXT		SEGMENT BYTE PUBLIC 'CODE' USE16

; BMSMAIN expects in AX function code
;
BMSMAIN		PROC FAR
		PUSH	BP
		MOV	BP, SP

		PUSH	AX		; PUSHA
		PUSH	CX
		PUSH	DX
		PUSH	BX
		PUSH	BP
		PUSH	SI
		PUSH	DI
		PUSH	DS

		MOV	BX, AX
		CMP	BX, FC_MOUOPEN
		JE	SWITCH
		CMP	BX, FC_MOUCLOSE
		JE	SWITCH
		CMP	BX, FC_MOUINITREAL
		JE	SWITCH
		@MouSynch 1
SWITCH:
		SHL	BX, 1
		JMP	WORD PTR CS:bmstable[BX]

BmsGetNumButtons:
		@DosDevIOCtl	[DS:BP-12], , MOU_GETBUTTONCOUNT, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT

BmsGetNumMickeys:
		@DosDevIOCtl	[DS:BP-12], , MOU_GETMICKEYCOUNT, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT

BmsGetDevStatus:
		@DosDevIOCtl	[DS:BP-12], , MOU_GETMOUSTATUS, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsGetNumQueEl:
		@DosDevIOCtl	[DS:BP-12], , MOU_GETQUESTATUS, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsReadEventQue:
		@DosDevIOCtl	[DS:BP-12], , MOU_READEVENTQUE, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsGetScaleFact:
		@DosDevIOCtl	[DS:BP-12], , MOU_GETSCALEFACTORS, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsGetEventMask:
		@DosDevIOCtl	[DS:BP-12], , MOU_GETEVENTMASK, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsSetScaleFact:
		@DosDevIOCtl	, [DS:BP-8], MOU_SETSCALEFACTORS, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsSetEventMask:
		@DosDevIOCtl	, [DS:BP-8], MOU_SETEVENTMASK, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsGetHotKey:
		@DosDevIOCtl	[DS:BP-12], , MOU_GETHOTKEYBUTTON, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsSetHotKey:
		@DosDevIOCtl	, [DS:BP-8], MOU_SETHOTKEYBUTTON, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsOpen:
		JMP	EXIT


BmsClose:
		JMP	EXIT


BmsGetPtrShape:
		@DosDevIOCtl	[DS:BP-12], [DS:BP-8], MOU_GETPTRSHAPE, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsSetPtrShape:
		@DosDevIOCtl	[DS:BP-12], [DS:BP-8], MOU_SETPTRSHAPE, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsDrawPtr:
		@DosDevIOCtl	, , MOU_DRAWPTR, IOCTL_POINTINGDEVICE,  [DS:BP-4]
		JMP	EXIT


BmsRemovePtr:
		@DosDevIOCtl	, [DS:BP-8], MOU_MARKCOLLISIONAREA, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsGetPtrPos:
		@DosDevIOCtl	[DS:BP-12], , MOU_GETPTRPOS, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsSetPtrPos:
		@DosDevIOCtl	, [DS:BP-8], MOU_MARKCOLLISIONAREA, IOCTL_POINTINGDEVICE, [DS:BP-4]
		JMP	EXIT


BmsInitReal:
		JMP	EXIT


BmsFlushQue:
		JMP	EXIT


BmsSetDevStatus:
		@DosDevIOCtl	, [DS:BP-8], MOU_SETMOUSTATUS, IOCTL_POINTINGDEVICE, [DS:BP-4]

EXIT:
		POP	DS		; POPA
		POP	DI
		POP	SI
		POP	BP
		POP	BX
		POP	DX
		POP	CX
		POP	AX

		MOV	SP,BP
		POP	BP
		RETF
bmstable:
	DW	BmsGetNumButtons	;00H
	DW	BmsGetNumMickeys	;01H
	DW	BmsGetDevStatus		;02H
	DW	BmsGetNumQueEl		;03H
	DW	BmsReadEventQue		;04H
	DW	BmsGetScaleFact		;05H
	DW	BmsGetEventMask		;06H
	DW	BmsSetScaleFact		;07H
	DW	BmsSetEventMask		;08H
	DW	BmsGetHotKey		;09H
	DW	BmsSetHotKey		;0AH
	DW	BmsOpen			;0BH
	DW	BmsClose		;0CH
	DW	BmsGetPtrShape		;0DH
	DW	BmsSetPtrShape		;0EH
	DW	BmsDrawPtr		;0FH
	DW	BmsRemovePtr		;10H
	DW	BmsGetPtrPos		;11H
	DW	BmsSetPtrPos		;12H
	DW	BmsInitReal		;13H
	DW	BmsFlushQue		;14H
	DW	BmsSetDevStatus		;15H
BMSMAIN		ENDP


_TEXT		ENDS
		END

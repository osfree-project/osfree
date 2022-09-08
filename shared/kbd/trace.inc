;-------------------------------------------------------------
@PUSHA	MACRO
		PUSH	AX
		PUSH	CX
		PUSH	DX
		PUSH	BX
		PUSH	BP
		PUSH	SI
		PUSH	DI
		PUSH	DS
		ENDM

@POPA	MACRO
		POP	DS
		POP	DI
		POP	SI
		POP	BP
		POP	BX
		POP	DX
		POP	CX
		POP	AX
		ENDM
;----------------------------------------------------------------


@TRACE	MACRO Name
		PUSHF
		@PUSHA
		@VioWrtTTY	@CATSTR(NAME, Msg), @CATSTR(NAME, MsgSize), 0
		@POPA
		POPF
		ENDM

;----------------------------------------------------------------

@tracemsg	macro	event
@CATSTR(event, Msg)			DB	@CATSTR( <!'>, event, <!'>), 0dh, 0ah
eventmsg=$-@CATSTR(event, Msg)
@CATSTR(event, MsgSize)		EQU	eventmsg
			endm

;----------------------------------------------------------------

@tracemsg2	macro	event
@tracemsg	@CATSTR(Pre, event)
@tracemsg	@CATSTR(Post, event)
			endm

;----------------------------------------------------------------

@tracecall	macro	event
event		proc
			@TRACE	event
			ret
event		endp
		endm

;----------------------------------------------------------------

@tracecall2	macro	event
@tracecall	@CATSTR(Pre, event)
@tracecall	@CATSTR(Post, event)
			endm

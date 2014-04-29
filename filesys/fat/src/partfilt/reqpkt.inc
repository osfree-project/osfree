;/*
; * $Header$
; */

; Definition of the request packet header.

ReqPacket	struc
ReqLenght	db ?
ReqUnit		db ?
ReqCommand	db ?
ReqStatus	dw ?
ReqFlags	db ?
		db 3 dup (?)	; Reserved field
ReqLink		dd ?
IFDEF __MASM__
ReqPacket	ends		; MASM seems to need it ... (M.Willm 1995-11-14)
ELSE
		ends
ENDIF

;
; THUNK32$ support
;
RpInitOut       struc
rph		db size ReqPacket dup (?)
unit		db ?
codeEnd		dw ?
dataEnd		dw ?
BPBArray	dd ?
Status		dw ?
RpInitOut	ends

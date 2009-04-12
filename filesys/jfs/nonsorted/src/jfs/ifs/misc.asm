; $Id: misc.asm,v 1.1 2000/04/21 10:58:19 ktk Exp $ 

SCCSID equ "@(#)1.21  7/22/99 13:11:48 src/jfs/ifs/misc.asm, sysjfs, w45.fs32, fixbld"
;
;
;   Copyright (c) International Business Machines  Corp., 2000
;
;   This program is free software;  you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or 
;   (at your option) any later version.
; 
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY;  without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
;   the GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program;  if not, write to the Free Software 
;   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
;
;; Change History :
;;
;;  7/22/99 DJK F226941 Implement Directory Limits
;;

include thunk.inc

extrn32	memcpy,near
extrn32	dhrouter,dword
extrn32	_jfsIODone16@4,near
extrn32 _pageIOdone2@4,near

extrn16 FSH_CRITERROR,far

?DefExtrn TCBUser_ID,ABS					;; F226941
extrn32 pTCBCur,dword						;; F226941

DefData	EXPORT,DATA32

ALIGN	4

EndData

DefData	EXPORT,DATA16

;;	Page IO routines need some data in a 16-bit segment

ALIGN	4

PUBLIC	dummyAction
dummyAction	DW	?

PUBLIC	dummyFlag
dummyFlag	DW	?

PUBLIC	dummyCds
dummyCds	DD	?	; Currently not using the whole structure

	PUBLIC pgreq
pgreq	db	500 dup (0)

	PUBLIC p16_pgreq
p16_pgreq	dw	offset DATA16:pgreq
		dw	seg DATA16

	PUBLIC p16_IODone16
p16_IODone16	dw	offset CODE16:IODone16
		dw	seg CODE16

	PUBLIC p16_pageIOdone16
p16_pageIOdone16	dw	offset CODE16:pageIOdone16
			dw	seg CODE16

IFDEF _JFS_OS2
;; Error Log Packet
LogDataPacket dw 1           ;;Packet version number(1 is for 32 bit)
  NumPackets  dw 1           ;;Number of log packets
  PacketLen   dw offset EndLogDataPacket - offset LogDataPacket
  PacketID    dw ?           ;;Packet id -  sna gen alert
  Status      dd 0           ;;Status (No Process Name)
  Qualifier   db 'JFS,'      ;;Qualifier name
  Reserved    dd 0           ;;Reserved
  Time        dd 0           ;;Time field
  Date        dd 0           ;;Date field
  Originator  db 'OS/2    '  ;;Originator name
  FormatDLL   db 12 dup(0)   ;;Formatting DLL name(none)
EndLogDataPacket dd 0
ENDIF

ALIGN	2

;;	Message & arguments must be in 16-bit memory for FSH call
;;
;; LT16Meg_Str  db	'JFS: Mount failed on drive %1:.'
LT16Meg_Str  db	'JFS: Mount failed on drive '
		PUBLIC LT16DriveLtr
LT16DriveLtr db 'D:.'
	     db 0dh, 0ah	; CR/LF
	     db 'Adapter does not support real memory above 16 Meg.'
	     db 0dh, 0ah	; CR/LF
LT16Meg_End  db	0

Args	db	64 dup(0)

ALIGN	2

CritMsgs	dw	offset LT16Meg_Str

CritMsgLen	dw	offset LT16Meg_End - offset LT16Meg_Str

EndData

DefCode	EXPORT,CODE32

aflatCS
aflatContext
;; -----------------------------------------------------------------------

Procedure _Strategy2@8

ArgVar	StratRoutine,DWORD
ArgVar	Packet,DWORD

EnterProc

SaveReg <ebx,esi,edi,ds,es>

lea	edi,StratRoutine

CCall	KernThunkStackTo16

les	bx,Packet
PCALL16	<dword ptr [edi]>

;; Restore ES to flat
mov	cx,ds
mov	es,cx

CCall	KernThunkStackTo32

RestoreReg <es,ds,edi,esi,ebx>
LeaveProc
RETC
EndProc _Strategy2@8
;; -----------------------------------------------------------------------

Procedure _SysLogError@0

EnterProc
SaveReg <ebx,esi,edi,ds,es>

CCall	KernThunkStackTo16

mov	bx, SEG LogDataPacket
mov	es, bx
mov	bx, OFFSET LogDataPacket
mov	cx, 090h               ; 32-bit error logging call
mov	dl, DevHlp_LogEntry
DEVHELP16

jc	le_fail


le_fail	label	near

CCall	KernThunkStackTo32

RestoreReg <es,ds,edi,esi,ebx>
LeaveProc
RETC
EndProc _SysLogError@0
;; -----------------------------------------------------------------------
;;			BEGIN F226941

Procedure _ProcessToGlobal@8

ArgVar	ProcAddress,DWORD
ArgVar	SegmentSize,DWORD

EnterProc
SaveReg <ebx,esi,edi,ds,es>

CCall	KernThunkStackTo16

mov	ebx, ProcAddress
mov	eax, 1
mov	ecx, SegmentSize	
mov	dl, DevHlp_VMProcessToGlobal
DEVHELP16

jnc	ptg_out

xor	eax, eax	;; Error!

ptg_out	label	near

mov	ebx, eax

CCall	KernThunkStackTo32	;; EAX = global address

mov	eax, ebx

RestoreReg <es,ds,edi,esi,ebx>
LeaveProc
RETC
EndProc _ProcessToGlobal@8

;;			END F226941
;; -----------------------------------------------------------------------
assume es: nothing

Procedure _FSH_CRITERROR@12

ArgVar	msgNum,DWORD
ArgVar	nSubs,DWORD
ArgVar	pSubs,DWORD

EnterProc

SaveReg <ebx,esi,edi,ds,es>

CCall KernThunkStackTo16

;; Copy arguments

	mov	ax, DATA16
	mov	es, ax

	mov	eax, nSubs
	cmp	eax, 0
	jz	noArgs

	mov	esi, pSubs
	mov	di, offset Args
again	label	near
	mov	bl, byte ptr [esi]
	mov	byte ptr es:[di], bl
	inc	esi
	inc	di
	cmp	bl, 0
	jnz	again
	dec	eax
	jnz	again
noArgs	label	near

;; call FSH_CRITERROR

mov	eax, msgNum
mov	bx, DATA16
shl	ebx, 16
mov	bx, CritMsgs[eax*2]	; Far ptr to CritMsgs[msgNum]
mov	ax, CritMsgLen[eax*2]	; length: CritMsgLen[msgNum]
mov	ecx, nSubs
;;mov	edx, 010h		; CE_ALLACK
mov	edx, 003h		; CE_ALLFAIL | CE_ALLABORT
PCALL16 FSH_CRITERROR <ax, ebx, cx, seg Args, offset Args, dx>

CCall KernThunkStackTo32

RestoreReg <es, ds, edi, esi, ebx>
LeaveProc
RETC
EndProc _FSH_CRITERROR@12
;; -----------------------------------------------------------------------
;;			BEGIN F226941
;;
;; percent - compute what percent eax is of edx: (eax*100)/edx
;;
;; Visual Age compiler will generate floating point code if intermediate
;; result exceeds 32 bits.
;;
;; eax - on input, fractional amount
;;       on output, percent
;; edx - on input, total amount
;;
assume es: nothing

Procedure percent

push edx	; divisor
mov ecx, 100
mul ecx		; (edx:eax) = eax * ecx
pop ecx
div ecx		; eax = (edx:eax) / ecx
ret

EndProc percent
;; -----------------------------------------------------------------------
;;
;; User_ID - Return TCBUser_ID field from curTCB
;;
;; eax - on output, User ID
;;
assume es: nothing

Procedure _User_ID@0

mov	eax, pTCBCur
mov	eax, [eax]
movzx	eax, word ptr [eax].TCBUser_ID

ret

EndProc _User_ID@0
;;			END F226941
;; -----------------------------------------------------------------------


EndCode

;;----------------------------------------------------------------------------
DefCode EXPORT,CODE16
astubCS
;;----------------------------------------------------------------------------
Procedure	IODone16,far

	push	ebp
	movzx	ebp,sp
	and	sp, -4	;; dword align stack
	SaveReg <ds,es>

	mov	dx,es
	shl	edx,16
	mov	dx,bx

	flatContext
	
	CCALL32	_jfsIODone16@4 <edx>

	RestoreReg <es,ds>
	mov	sp,bp
	pop	ebp
	RETC
EndProc		IODone16

Procedure	pageIOdone16,far

	push	ebp
	movzx	ebp,sp
	and	sp, -4	;; dword align stack
	SaveReg <ds,es>

	mov	dx,es
	shl	edx,16
	mov	dx,bx

	flatContext
	
	CCALL32	_pageIOdone2@4 <edx>

	RestoreReg <es,ds>
	mov	sp,bp
	pop	ebp
	RETC
EndProc		pageIOdone16

EndCode

END

; $Id: cswap.asm 1195 2006-06-11 02:47:06Z blairdude $
; File:
;                          cswap.asm
; Description:
;                       command.com swapping
;
;                    Copyright (c) 2001 tom ehlert
;                       All Rights Reserved
;
; This file is part of DOS-C.
;
; DOS-C is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version
; 2, or (at your option) any later version.
;
; DOS-C is distributed in the hope that it will be useful, but
; WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
; the GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public
; License along with DOS-C; see the file COPYING.  If not,
; write to the Free Software Foundation, 675 Mass Ave,
; Cambridge, MA 02139, USA.
;
;

;include "segs.inc"

TEXT segment word public 'CODE' use16

        public _dosFCB1,_dosFCB2
_dosFCB1 db 37 dup (0)
_dosFCB2 db 37 dup (0)

	public _dosCMDTAIL, _dosCMDNAME		; use command line from within PSP
	public _dosCMDNAME
_dosCMDTAIL db 128 dup (0)
_dosCMDNAME db 128 dup (0)
 	    db 256 dup (0)
        public localStack
localStack:


	public _dosParamDosExec
_dosParamDosExec db 22 dup (0)


	public _XMSdriverAdress
_XMSdriverAdress dd 0
callXMS	 EQU call far ptr [_XMSdriverAdress]

 	public _SwapResidentSize
_SwapResidentSize   dw 0

 	public _SwapTransientSize
_SwapTransientSize  dw 0

	public _XMSsave ; , _XMSrestore
_XMSsave            db 8 dup (0)
currentSegmOfFreeCOM  EQU	_XMSsave+8
xms_handle	      EQU       _XMSsave+10

	public _termAddr
_termAddr:
terminationAddressOffs	DW 0
terminationAddressSegm	DW 0
	public _myPID, _residentCS
_myPID	    DW 0
_residentCS DW 0
	public _origPPID
_origPPID   DW 0
	public _canexit
_canexit    DB 0		; 1 -> can exit; _else_ --> cannot exit

        public _mySS, _mySP
_mySS       DW 0
_mySP       DW 0

execSS      dw 0
execSP      dw 0

execRetval  dw 0

first_time  db 1

public SWAPXMSdirection

;;TODO make XMSsave two structures in order to drop this subroutine
SWAPXMSdirection:
	push word ptr [_XMSsave+4]
	push word ptr [_XMSsave+6]
	push word ptr [_XMSsave+8]

	push word ptr [_XMSsave+10]
	push word ptr [_XMSsave+12]
	push word ptr [_XMSsave+14]


	pop  word ptr [_XMSsave+8]
	pop  word ptr [_XMSsave+6]
	pop  word ptr [_XMSsave+4]

	pop  word ptr [_XMSsave+14]
	pop  word ptr [_XMSsave+12]
	pop  word ptr [_XMSsave+10]

	ret


;;TODO: DS ought to be equal to SS, DS could be reconstructed from
;;	SS at the end of the XMSexec function
;	global real_XMSexec
real_XMSexec:
						; save ALL registers needed later
		push si
		push di
		push bp
		push ds

		mov cx, cs
		mov ds, cx

        mov [_mySS],ss  ; 2E
        mov [_mySP],sp  ; 2E
		mov [execSS],ss
		mov [execSP],sp

		mov ss, cx		; this stack is definitely large enough AND present
        mov  sp,localStack
						; save everything to XMS
		mov ah,0bh
		mov si, word ptr _XMSsave
		callXMS

;;TODO: test of result

		mov es, word ptr [currentSegmOfFreeCOM]
						; first time: shrink current psp
		cmp byte ptr [first_time],1
		jne second_time

;;TODO: first_time either 04ah or 049h --> no jumps
		mov ah,04ah						; resize memory block
		mov bx,[_SwapResidentSize]
		int 21h

		mov byte ptr [first_time],0
		jmp save_done

second_time:							; this memory was allocated
										; and may be freed

		mov ah,049h						; free memory block
		int 21h

save_done:


		; do exec


		mov ax, cs
                   		; ds:dx = ASCIZ program name
		mov ds, ax
		mov  dx, word ptr _dosCMDNAME
		       
                        ; es:bx = parameter block
		mov es, cx
		mov bx, word ptr _dosParamDosExec


						; our temporary stack
        mov  ss, ax
        mov  sp,localStack


		mov ax,04b00h		; load & execute
		int 21h

		jc  exec_error
		xor ax,ax


exec_error:
		cld					; don't rely on that

	; reload into memory

        mov  cx, cs
        mov  ss, cx
        mov  sp, localStack

		; restore:

        mov ds, cx
		mov [execRetval],ax
							; we need some memory

		;; First ensure that FreeCOM is reloaded in low memory
		;; so that LOADHIGH would come into problems
;;ska		mov ax, 5800h		; Get current allocation strategy
;;ska		int 21h
;;ska		mov dx, ax

;;ska		mov ax, 5801h		; Set current allocation strategy
;;ska		mov bx, 0			; low memory / first fit
;;ska		int 21h
		; ignore any errors

		mov ah,48h
		mov bx,[_SwapTransientSize]
		int 21h

;;ska		pushf

		; Restore Alloc Strat
;;ska		mov ax, 5801h		; Set current allocation strategy
;;ska		mov bx, dx
;;ska		int 21h

;;ska		popf
		jc DOS_trouble_while_swapping_in

                                ; calculate relocation factor

		mov  bx, ax				; new execute address
		sub  bx, word ptr [currentSegmOfFreeCOM]		; new address - old address
		push bx					;
		mov  word ptr [currentSegmOfFreeCOM], ax	; new prog address

		call SWAPXMSdirection
								; restore everything to XMS
		mov ah,0bh
		mov si, word ptr _XMSsave
		callXMS

		call SWAPXMSdirection	; re-construct the XMSsave area
		pop bx                  ; get relocation factor back

		cmp ax,1
		jnz XMS_trouble_while_swapping_in

                                    ; relocate segment registers
		mov ax,[execSS]
		add ax,bx
		mov ss,ax
		mov sp,[execSP]

		mov bp,sp
		add [bp+0],bx				; ds
		add [bp+10],bx				; ret addr

		mov ax,[execRetval]

		pop ds
		pop bp
		pop di
		pop	si

		retf						; done

XMS_err 		db 'XMS copy error$'
DOS_err 		db 'Memory allocation error$'
common_error	db 0dh,0ah,0ah,'FreeCOM: XMSSwap-In: $'

XMS_trouble_while_swapping_in:
	mov bx, word ptr XMS_err
	jmp short trouble_while_swapping_in

DOS_trouble_while_swapping_in:
	mov bx, word ptr DOS_err

trouble_while_swapping_in:
;		push cs							; do some error message
;		pop  ds
		mov dx, word ptr common_error
		mov ah,09
		int 21h

		mov dx,bx
		mov ah,09
		int 21h

terminate_myself:
		mov ax,04cffh					; and die ...
		int 21h

		;; FALL THROUGH for elder FreeCOM kernels that simply ignore
		;; DOS-4C for shells

	;; central PSP:0xa hook <-> may be called in every circumstance
	public _terminateFreeCOMHook
_terminateFreeCOMHook:
	mov ax, cs				; setup run environment (in this module)
	mov ss, ax
	mov sp, localStack
	mov ds, ax

	; Next time we hit here it's != 1 --> no zero flag --> I_AM_DEAD status
	dec BYTE PTR [_canexit]
	jnz I_AM_DEAD

	mov ax, [_myPID]		; our own PSP [in case we arrived here
	mov es, ax				; in some strange ways]

	; Make sure the current PSP hasn't patched to nonsense already
	mov bx, ax
	mov ah, 50h				; Set PSP
	int 21h

	; Reset old termination address
	mov ax, [terminationAddressOffs]
	mov [es:0ah], ax
	mov ax, [terminationAddressSegm]
	mov [es:0ch], ax

	; Drop our "Shell" privileges
	mov ax, [_origPPID]		; original parent process ID
	mov [es:16h], ax

	; Kill the XMS memory block
	mov dx, word ptr [xms_handle]
	or dx, dx
	jz terminate_myself		; no block to deallocate
	mov ah, 0ah				; deallocate XMS memory block
	callXMS

	; Now, DOS-4C should proceed correctly
	jmp short terminate_myself


I_AM_DEAD:								; process 0 can't terminate ...
	mov dx, word ptr dead_loop_string
	mov ah, 9
	int 21h
I_AM_DEAD_loop:
	hlt
	jmp short I_AM_DEAD_loop

dead_loop_string	DB 13,10,7,'Cannot terminate permanent FreeCOM instance'
	DB 13,10,'System halted ... reboot or power off now$'

;
; as I don't know how to set the old interrupt handler
; I write my own
; or we use the TURBO_C _restorezero() and use the
; DOS default handler for that

if 0
MsgZerodivide db 'integer zero divide$'
	global _ZeroDivideInterrupt
_ZeroDivideInterrupt:

		push cs
		pop ds
		mov dx,MsgZerodivide
		mov ah,09
		int 21h

        mov ax,04c7fh       ; terminate with errorlevel 127
        int 21h
		jmp _ZeroDivideInterrupt
endif


;********************************************************************
; *************   END OF RESIDENT AREA ******************************
;********************************************************************
	public _SWAPresidentEnd
_SWAPresidentEnd:

if 0
;
; normal EXEC
;

		global _DosEXEC
_DosEXEC:
						; save ALL registers needed later
		push si
		push di
		push bp
		push ds

		push cs                   		; ds:dx = ASCIZ program name
		pop	 ds

		mov  dx,_dosCMDNAME

		mov [execSS],ss
		mov [execSP],sp

		push cs
		pop  es
		mov bx, _dosParamDosExec

		mov ax,04b00h
		int 21h

		jc      exec_error2  ; if there was an error, the error code is in AX

		xor ax,ax

exec_error2:

		cld					; don't rely on that

		mov ss,[cs:execSS]
		mov sp,[cs:execSP]

		pop ds
		pop bp
		pop di
		pop	si
		retf
endif

;; Added here to make it more easier for the C-part to make a XMS
;; request, because the code:
;;		(*XMSdriverAdress)();
;; is translated into something like:
;;		mov ax, _CODE		;; immediate value
;;		mov es, ax
;;		call DWORD PTR es:[_XMSdriverAdress]
;; detroying AX already holding the API function number

;; To be called with _far_!!
	public _XMSrequest
	;; Note: Because [CS:driverAdress] == [residentCS:driverAdress]
	;; we need not use a similiar approach as with XMSexec
_XMSrequest:
		jmp far ptr [cs:_XMSdriverAdress]

;; Added here to make it more easier for the C-part to call functions
;; located in the resident part, because:
;;		_XMSexec();
;; is translated into something like:
;;		nop
;;		push cs			<-> WRONG!!
;;		call _XMSexec

;; ALL To be called with _far_!!
		public	_XMSexec
_XMSexec:
		push WORD PTR [CS:_residentCS]
		push WORD PTR real_XMSexec
		retf

TEXT ends

      end

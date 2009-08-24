COMMENT ~
    ASSIGN: Replace drive letters
    Copyright (C) 1995-97 Steffen Kaiser

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
~
COMMENT `
$Id: ASGN_TSR.ASM 1.3 1997/01/27 08:09:37 ska Rel ska $

ASSIGN simply intercept any DOS (21h) request and patches the drive
specification, after the call is made, the original is re-patched.
The read/write absolute interrupts (25h/26h) are intercepted, too.

The target DOS versions are: MS-DOS 3.0 .. MS-DOS 6.22. DR-DOS need
not the TSR, because ASSIGN is handled via the CDS there. Other
DOS's are unknown to me. Nonetheless, no check is perfomed, if it's
MS-DOS, or not. If the DOS version is less than 3 a fatal error
occurs; greater than 6.22 a warning is displayed.

Because most calls preserve the original drive letter, the interrupt
is chained within ASSIGN; therefore I call the old INT21 handler with
the actual flags and patch the lower byte containing the Zero and Carry
flags into the original ones to preserve& avoid conflicts with the
caller's Interrupt and Trace flags.

The table "tableSubCode" contains the subindex into the table "tableAdr".
The way to handle each DOS function depends on how it stores the drive
specification and if it restores the old one (== POD). 
It's interresting, that except the IOCTL function (44h) there are no
conflicts between the DOS versions 3.0 .. 6.22.

The chkIn/chkOut macros cover the usage of the internal data area.
When compiled with CHECK defined, these macros contains a check for
recursive calling and go into an infinite loop displaying "error", „nd a
beep.

	Target assembler:
		TASM v2+, ASM v2.0

`
GetDrive equ 19h

_code	segment 'CODE'
	assume cs:_code, ds:_code, ss:_code
	
	org 100h

; a COM module is loaded at address 100h


start:
chkReply	db 0ffh	; already installed;; 1: not installed but not OK to inst
actDrv		db ?	; actual drive setted via setDrive
chkRecurs	db 0

;	org 103h
; here this table MUST reside, this is documented that way


transTbl:	; at ES:103h must ‘xist driver translation table
			; A==1, max.: 26 == 1a (Z:)
	db 1, 2, 3, 4, 5, 6, 7, 8, 9, 0ah, 0bh, 0ch, 0dh
	db 0eh, 0fh, 10h, 11h, 12h, 13h, 14h, 15h, 16h
	db 17h, 18h, 19h, 1ah

tableSubCode:		; INT 21h => Subcode mapper
	db offset tableSubCodeEnde - offset tableSubCode - 1
fct00	db 01h
fct01	db 00h
fct02	db 00h
fct03	db 00h
fct04	db 00h
fct05	db 00h
fct06	db 00h
fct07	db 00h
fct08	db 00h
fct09	db 00h
fct0a	db 00h
fct0b	db 00h
fct0c	db 00h
fct0d	db 01h
fct0e	db 0bh
fct0f	db 02h
fct10	db 02h
fct11	db 02h
fct12	db 02h
fct13	db 02h
fct14	db 02h
fct15	db 02h
fct16	db 02h
fct17	db 02h
fct18	db 01h
fct19	db 0ah
fct1a	db 01h
fct1b	db 01h
fct1c	db 0ch
fct1d	db 01h
fct1e	db 01h
fct1f	db 01h
fct20	db 01h
fct21	db 02h
fct22	db 02h
fct23	db 02h
fct24	db 02h
fct25	db 01h
fct26	db 01h
fct27	db 02h
fct28	db 02h
fct29	db 01h
fct2a	db 01h
fct2b	db 01h
fct2c	db 01h
fct2d	db 01h
fct2e	db 01h
fct2f	db 01h
fct30	db 01h
fct31	db 01h
fct32	db 05h
fct33	db 01h
fct34	db 01h
fct35	db 01h
fct36	db 0ch
fct37	db 01h
fct38	db 01h
fct39	db 03h
fct3a	db 03h
fct3b	db 03h
fct3c	db 03h
fct3d	db 03h
fct3e	db 01h
fct3f	db 01h
fct40	db 01h
fct41	db 03h
fct42	db 01h
fct43	db 03h
fct44	db 06h
fct45	db 01h
fct46	db 01h
fct47	db 05h
fct48	db 01h
fct49	db 01h
fct4a	db 01h
fct4b	db 08h
fct4c	db 01h
fct4d	db 01h
fct4e	db 03h
fct4f	db 01h
fct50	db 01h
fct51	db 01h
fct52	db 01h
fct53	db 01h
fct54	db 01h
fct55	db 01h
fct56	db 09h
fct57	db 01h
fct58	db 01h
fct59	db 01h
fct5a	db 03h
fct5b	db 03h
fct5c	db 01h
fct5d	db 01h
fct5e	db 01h
fct5f	db 01h
fct60	db 04h
fct61	db 01h
fct62	db 01h
fct63	db 01h
fct64	db 00h
fct65	db 00h
fct66	db 00h
fct67	db 00h
fct68	db 00h
fct69	db 07h
fct6a	db 00h
fct6b	db 00h
fct6c	db 04h
tableSubCodeEnde:
; no space allocating here
tableAdr:
	dw finish21		; NOP
	dw int2a21		; INT 2A-89-08
	dw fcb21		; DS:DX -> FCB ; preserve old drive (POD)
	dw dsdx21		; DS:DX -> UNIX path ; POD
	dw dssi21		; DS:SI -> UNIX path ; POD
	dw dl121		; DL == DRV(A: == 1) ; POD
	dw ioctl21		; IOCTL - subcode, BL == DRV(A: == 1); POD
	dw bl21			; BL == DRV(A: == 1) ; POD
	dw exec21		; EXEC - subcode; DS:DX -> UNIX path; POD except sub 0
	dw ren21		; Rename DS:DX & ES:DI -> UNIX path; POD
	dw get21		; getDisk; internal if any ASSIGN
	dw set21		; setDisk; store internal if successful
	dw dl1no21		; DL == DRV(A: == 1) ; no POD
tableAdrEnd:
					; POD := Preserve Old Drive spec

tableIOCTL:
	db offset tableIOCTLEnde - offset tableIOCTL 
io0	db 0
io1	db 0
io2	db 0
io3	db 0
io4	db 1
io5	db 1
io6	db 0
io7	db 0
io8	db 1
io9	db 1
ioa	db 0
iob	db 0
ioc	db 0
iod	db 1
ioe	db 1
iof	db 1
io10	db 0
io11	db 1
tableIOCTLEnde:


orgDrv		db ?	; original drive to map
adrDrv		dw ?	; Address to patch to orgDrv after call to INT21h
orgDrv1		db ?	; original drive #1 in rename
adrDrv1		dw ?	; address to patch to original drive #1 in rename
localRet	dw ?	; local return address of chain21
orgOfs		dw ?	; address of calling process
orgSeg		dw ?	; 
orgFlags	db ?	; HiByte of flags of calling process
origAX		dw ?	; original AX (of calling process)
;origBX		dw ?	; original BX (of calling process)


mapDrv0a proc near	; maps drive letter in al; A: == 0
	cmp al, 26
	jnc	skipDrv0
	push bx
	mov bx, offset transTbl
	xlat byte ptr cs:[bx]
	dec al
	pop bx
	clc
	ret
skipDrv0:
	stc
	ret
mapDrv0a endp

mapDrv0 macro
	call mapDrv0a
endm

mapDrv1a proc near	; maps drive letter in al; A: == 1
	;mov cs:[orgDrv], al	;#1  when CF is true, it's not used
	cmp al, 27
	jnc skipDrv
	or al, al
	jne drvSet
	mov al, cs:[actDrv]
	inc al
	;mov cs:[orgDrv], al	;#1
drvSet:
	mov cs:[orgDrv], al		;#1
	push bx
	mov bx, offset transTbl - 1
	xlat byte ptr cs:[bx]
	pop bx
	clc
	ret
skipDrv:
	stc
	ret
mapDrv1a endp
mapDrv1 macro
	call mapDrv1a
endm

tableMapper macro tab, def
	mov bx, offset tab + 1
	cmp al, cs:[bx-1]
	jnc def
	xlat byte ptr cs:[bx]
endm

ifdef CHECK
QQa	db 'error', 7
QQe:
chk	db 0
_enterChk proc near
	cmp cs:[chk], 0
	je isr21_chk
chk_cycl:
		mov ax, cs
		mov es, ax
		mov ds, ax
		mov ax, 1300h
		mov bx, 0fh
		mov cx, offset QQe - offset QQa
		mov dx, 101h
		mov bp, offset QQa
		int 10h
		jmp short chk_cycl
isr21_chk:
	mov cs:[chk], 1
	ret
_enterChk endp

enterChk macro
	call _enterChk
endm

else

enterChk macro
endm
endif

chkIna proc near	; calls 2A-89-08 and 2a-80-8
	push ax
	inc cs:chkRecurs
	mov ax, 8908h
	int 2ah
	mov ax, 8008h
	int 2ah
	pop ax
	ret
chkIna endp
chkIn macro
	enterChk
	call chkIna
endm


chkOut1 macro	; non-AX preserving chkOut
	mov ax, 8108h
	int 2ah
	dec cs:chkRecurs
ifdef CHECK
	mov cs:[chk], 0
endif
endm
chkOut macro
	push ax
	chkOut1
	pop ax
endm

unixMap macro
	call translate
endm

translate proc near	; check for UNIX compt path DS:[bx]
	assume cs:_code
	cmp byte ptr [bx+1], ':'
	jne retErr		; no drive specified
	mov al, [bx]
	cmp al, 'A'
	jc retErr
	cmp al, 'z'+1
	jnc retErr
	mov ah, 'A'
	cmp al, 'Z'+1
	jc ok
	cmp al, 'a'
	jc retErr
	mov ah, 'a'
ok:
	mov cs:[orgDrv], al
	mov cs:[adrDrv], bx
	sub al, ah		; make drv0
	mapDrv0
	jc retErr		; do not patch
	add al, ah
	mov [bx], al
	clc
	ret

retErr:
	stc
	ret
translate endp

mkCall macro
	call chain21
endm

chain21	proc near	; call INT21 chain
	pop cs:[localRet]
	pop cs:[origAX]
	pop bx
	pop cs:[orgSeg]
	pop cs:[orgOfs]
	pop ax			; Flag
	mov cs:[orgFlags], ah	; preserve old IF/TF
	pushf			; additional flags to prevent from I/T-flags while calling
	mov ax, cs:[origAX]		; no additional stack space used for calling
	call dword ptr cs:ofs21	; call int 21 chain
	mov cs:[origAX], ax
	lahf					; new CF, ZF
	mov al, cs:[orgFlags]	; old IF, TF
	xchg ah, al				; preserves new CF, ZF, as well as old IF, TF
	push ax       			; set Flags to be returned by iret
	push cs:[orgOfs]
	push cs:[orgSeg]
	push bx
	push cs:[origAX]
	jmp word ptr cs:[localRet]
chain21 endp

retOrg_: ; return to calling process
	chkOut1
	pop ax
	pop bx
	iret
retOrg macro
	jmp retOrg_
endm
retOrg1 macro
	chkOut
	iret
endm

instCheck:			; Check if ASSIGN is installed
	mov al, cs:[chkReply]	; enable stealth mode
	iret
isr2f:				; Multiplexer intr
	cmp ax, 600h	; ASSIGN installation check
	je instCheck
	cmp ax, 601h	; ASSIGN data segment retrieval
	jne jmpit2f
	push cs
	pop es			; ES := data seg
	iret
jmpit2f:
	db 0eah			; jump far: chain multiplex intr
ofs2f	dw 0
seg2f	dw 0

isr25:				; Absolute Disk Read
					; map drive in al(0); don't care about ret val
	mapDrv0
	db 0eah
ofs25	dw 0
seg25	dw 0

isr26:				; Absolute Disk Write
					; map drive in al(0); don't care about ret val
	mapDrv0
	db 0eah
ofs26	dw 0
seg26	dw 0

isr21:				; DOS intr
					; two-step determination:
					; AH -> sub-code (table)
					; sub-code -> fct-addr (table)
	cmp ah, offset tableSubCodeEnde - offset tableSubCode
	jnc jmpit21_
	test cs:chkRecurs, 0ffh
	jne jmpit21_
	push bx
	push ax
	xor al, al
	xchg al, ah
	mov bx, offset tableSubCode + 1
	xlat byte ptr cs:[bx]
	add al, al			
	add al, cs:[bx-1]	; SubCodetable
	add bx, ax			; ah is zero
	jmp word ptr cs:[bx]
jmpit21_:
	jmp jmpit21

dssi21:					; DS:SI -> UNIX path
	mov bx, si
	jmp short dsdx1
	
fcb21:				; DS:DX FCB
	mov bx, dx
	cmp byte ptr [bx], 0ffh
	jne callBXAL1
	add bx, 7		; skip Extended header
callBXAL1:			; call DOS function; BX Adr of drv; al drv; A: == 1; POD
					; drv == 0 is fullfilled
	mov al, [bx]
	chkIn
	mapDrv1
	jc errChkOut1
callRepl:			; call INT and repl after calling
	mov cs:[adrDrv], bx
	mov [bx], al	; write chg AL
	mkCall
	mov al, cs:[orgDrv]			; restore old Drive letter/# 
fcb3:
	mov bx, cs:[adrDrv]
	mov [bx], al
	retOrg

dsdx21:					; DS:DX -> UNIX path
	mov bx, dx
dsdx1:
	chkIn
dsdx2:
	unixMap
	jnc callRepl
errChkOut1:
	jmp short errChkOut

ren21:					; DS:DX, ES:DI -> UNIX path; POD
	push ds
	mov ax, es
	mov ds, ax
	mov bx, di
	chkIn
	unixMap				; check ES:DI
	pop ds
	mov bx, dx
	jc dsdx2			; ES:DI contains no drive letter => simple DS:DX

	mov al, cs:[orgDrv]
;	mov cs:[adrDrv1], di	; save the address of & the original
	mov cs:[orgDrv1], al	; drive letter corresponding to ES:

	unixMap				; check DS:DX
	jnc ren1			; drive letter
	mov cs:[orgDrv], 0	; no POD of DS:DX
ren1:
	mkCall				; perform the rename() call

	mov al, cs:[orgDrv1]	; This must be filled
	mov es:[di], al		; re-set drive letter

	mov al, cs:[orgDrv]
	or al, al
	jne fcb3			; there is also the DS:DX to re-set
	retOrg				; no DS:DX to re-set

exec21:						; 0=>no POD DS:DX map
	pop ax				; 3=>POD DS:DX map  Subfunction
	push ax
	cmp al, 3
	je dsdx21
	cmp al, 2
	jnc finish21	; sub-fct ==2 || >3  ==> no drive letter
exec21a:			; sub-fct ==0 || ==1 ==> no POD (DS gets destroyed)
	mov bx, dx
	chkIn
	unixMap
					; because no POD: the destroyed SS:SP & DS:DX
					; don't reflect into ASSIGN, we leave ASSIGN with
					; next statement
	; chkOut with no POD == errChkOut
	;chkOut
	;jmp finish21	; no POD

errChkOut:
	chkOut1
	jmp finish21

int2a21:			; call 2A-89-08 only
	mov ax, 8908h
	int 2ah
finish21:			; NOP
	pop ax
	pop bx
jmpit21:
	db 0eah			; jump far: chain DOS intr
ofs21	dw 0
seg21	dw 0

dl1no21:				; DL == DRV(A: == 1); no POD
	mov al, dl
	chkIn
	mapDrv1
	mov dl, al
	jmp errChkOut

dl121:					; DL == DRV(A: == 1); POD
	mov al, dl
	chkIn
	mapDrv1
	jc errChkOut
	mov dl, al
	mkCall
	mov dl, cs:[orgDrv]
	retOrg

ioctl21:					; IOCTL call
	pop ax					; Sub fct
	push ax
	tableMapper tableIOCTL, finish21
	or al, al
	je finish21
bl21:
	mov bx, sp
	mov al, byte ptr ss:[bx+2]		; drv	in bl (2. stack item)
	or al, al
	je finish21				; no mapping
	chkIn
	mapDrv1
	jc errChkOut
	mov byte ptr ss:[bx+2], al			; drv setzen (bl)
	mkCall
	pop ax
	pop bx
	mov bl, cs:[orgDrv]			; drv setzen (bl)
	retOrg1

set21:			; setDisk
	mov al, dl
	mapDrv0
	jc finish21
	chkIn
	mov cs:[orgDrv1], al
	mov cs:[orgDrv], dl
	mov dl, al
	mkCall
	mov ah, GetDrive
	pushf					; needs 10 additional bytes on stack (+6 for call)
	call dword ptr cs:ofs21	; verify, if setDisk OK
	cmp al, cs:[orgDrv1]
	mov al, cs:[orgDrv]
	jne setFailed		; failed, no update of internal actDrv
	mov cs:[actDrv], al
setFailed:
	mov dl, al			; DL must be preserved
	retOrg

get21:			; calls always DOS, then checks, if match
	chkIn
	mkCall		; get Disk from DOS
	chkOut1
	mov al, cs:[actDrv]
	mov ah, al
	mapDrv0		; ASSIGN-local current disk is equal?
	pop bx		; origAX, where mkCall the requested AL stored
	cmp al, bl
	jne	getDisk1	; not equal maybe some ASSIGN bypass settings?
	mov bl, ah	; set ASSIGN-local, because DOS internal is ASSIGN'ed
getDisk1:
	mov ax, bx
	pop bx
	iret


; Just a block for easy transfer (to avoid .MAP generation)
memBlock:
	dw offset io0
memBlockEnd:

;	This table informs the loader where to patch the interrupt addresses
;	The structure contains: 1 byte, INT number
;							2 byte, where to store old offset
;							2 byte, where to store old segment
;							2 byte, which is new offset

	db 21h
	dw offset ofs21
	dw offset seg21
	dw offset isr21
	db 25h
	dw offset ofs25
	dw offset seg25
	dw offset isr25
	db 26h
	dw offset ofs26
	dw offset seg26
	dw offset isr26
	db 2fh
	dw offset ofs2f
	dw offset seg2f
	dw offset isr2f
	db 4			; four interrupts
	dw offset memBlockEnd - offset memBlock	; number of additional bytes
	dw offset memBlock		; start of additional block
	db 1			; COM table
_code	ends

	end start

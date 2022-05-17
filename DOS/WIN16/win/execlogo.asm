;
; This is Windows loader
;
.8086

WINVER		EQU	101		; Windows 1.01
;WINVER		EQU	102		; Windows 1.02
;WINVER		EQU	103		; Windows 1.03
;WINVER		EQU	104		; Windows 1.04

code segment
	org 100h

main:

; Show LOGO
	call	ShowLogo

; check memory for KERNEL.EXE
	mov	ah, 48h			; Allocate memory
	mov	bx, 0ffffh		; Impossible value of memory
	int	21h
	jnc	panic			; Something wrong, not possible to have such many memory
	IF	WINVER eq 101
	cmp	bx, 3400h		; ??? kb in para
	ENDIF
	IF	WINVER eq 300
	cmp	bx, 6000h		; 384 kb in para
	ENDIF
	jb	NoMem

; search KERNEL.EXE
	mov	ah, 4eh			; Find first file entry
	lea	dx, szKernel		; Filename
	xor	cx, cx
	int	21h
	jc	NoKernel

; load and execute KERNEL.EXE
	push	ds
	pop	es
	lea	dx, szKernel
	lea	bx, exeparams
	mov	ax, 4b00h		; Execute program
	int	21h
	jc	ExecErr

; exit from windows kernel
	call	HideLogo
	int	20h			; die

; Call ShowLogo
ShowLogo:
	push	cs			; prepare return from ShowLogo
	lea	ax, LogoRet
	push	ax
; check for signature
	mov	ax, cs
	lea	bx, LogoStart
	mov	cl, 4
	shr	bx, cl
	add	ax, bx			; Number of segments before logo starts
	push	ax
	mov	ax, 4
	push	ax
	retf				; Simulate far jump to LogoStart:0004h
LogoRet:
; free unneeded memory (part of LOGO code/data)
	retn

; Call HideLogo
HideLogo:
	push	cs			; prepare return from ShowLogo
	lea	ax, HideLogoRet
	push	ax
	mov	ax, cs
	lea	bx, LogoStart
	mov	cl, 4
	shr	bx, cl
	add	ax, bx			; Number of segments before logo starts
	push	ax
	mov	ax, 7
	push	ax
	retf				; Simulate far jump to LogoStart:0007h
HideLogoRet:
	retn

exeparams label byte
	dw	0
	dw	80h
seg1s	dw	?
	dw	5Ch
seg2s	dw	?
	dw	6Ch
seg3s	dw	?

PanicMsg:
	db	'Panic! Unrecoverable error!$'
NoMemMsg:
	db	'Windows requires at least 256Kb of RAM$'
NoKernelMsg:
	db	'Windows kernel not found$'
ExecErrMsg:
	db	'Can''t execute Windows kernel$'
HelpMsg:
	db	'osFree Windows loader',0dh,0ah,0dh,0ah
	db	'WIN [/R] [/3] [/S] [/B] [/D:[F][S][V][X]]',0dh,0ah,0dh,0ah
	db	'   /?  Prints this instruction banner.',0dh,0ah
	db	'   /h  Synonym for the /? switch.',0dh,0ah
	db	'   /3  Starts Windows in 386 enhanced mode.',0dh,0ah
	db	'   /S  Starts Windows in standard mode.',0dh,0ah
	db	'   /2  Synonym for the /S switch.',0dh,0ah
	db	'   /R  Starts Windows in real mode.',0dh,0ah
	db	'   /B  Creates a file, BOOTLOG.TXT, that records system messages.',0dh,0ah
	db	'       generated during system startup (boot).',0dh,0ah
	db	'   /D  Used for troubleshooting when Windows does not start',0dh,0ah
	db	'       correctly.',0dh,0ah
	db	'   :F  Turns off 32-bit disk access. Equivalent to SYSTEM.INI [386enh]',0dh,0ah
	db	'       setting: 32BitDiskAccess=FALSE.',0dh,0ah
	db	'   :S  Specifies that Windows should not use ROM address space between',0dh,0ah
	db	'       F000:0000 and 1 MB for a break point. Equivalent to SYSTEM.INI',0dh,0ah
	db	'       [386enh] setting: SystemROMBreakPoint=FALSE.',0dh,0ah
	db	'   :V  Specifies that the ROM routine handles interrupts from the hard',0dh,0ah
	db	'       drive controller. Equivalent to SYSTEM.INI [386enh] setting:',0dh,0ah
	db	'       VirtualHDIRQ=FALSE.',0dh,0ah
	db	'   :X  Excludes all of the adapter area from the range of memory that',0dh,0ah
	db	'       Windows scans to find unused space. Equivalent to SYSTEM.INI',0dh,0ah
	db	'       [386enh] setting: EMMExclude=A000-FFFF.',0dh,0ah, '$'

szKernel:
if WINVER eq 101
	db	'WIN100.BIN', 0
endif
;	db	'SYSTEM\KERNEL.EXE', 0

Help:
	lea	dx, HelpMsg
	jp	Die
ExecErr:
	lea	dx, ExecErrMsg
	jp	Die
NoKernel:
	lea	dx, NoKernelMsg
	jp	Die
NoMem:	lea	dx, NoMemMsg
	jp	Die
Panic:
	lea	dx, PanicMsg
Die:	mov	ax, 0300h
	int	10h		; Switch to video mode 3
	mov	ah, 09h
	int	21h		; Print message
	call	HideLogo
	; add key wait here?
	int	20h		; Die

	align 16
LogoStart:
code	ends
	END main


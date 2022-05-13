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
	cmp	bx, 4000h		; 256 kb in para
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
szKernel:
if WINVER eq 101
	db	'WIN100.BIN', 0
endif
;	db	'SYSTEM\KERNEL.EXE', 0

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
	int	20h		; Die

	align 16
LogoStart:
code	ends
	END main


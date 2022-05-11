;
; This is function to execute windows logo file
;
.8086

code segment
	org 100h
	call 	ShowLogo
	int	20h

ShowLogo:
	push    cs			; prepare return from ShowLogo
	mov     ax, offset LogoRet
	push    ax
	mov	ax, cs
	mov	bx, LogoStart
	mov	cl, 4
	shr	bx, cl
	add	ax, bx		; Number of segments before logo starts
	push	ax
	mov	ax, 4
	push	ax
	retf				; Simulate far jump to LogoStart:0004h
LogoRet:
	retn

HideLogo:
	push    cs			; prepare return from ShowLogo
	mov     ax, offset LogoRet
	push    ax
	mov	ax, cs
	mov	bx, LogoStart
	mov	cl, 4
	shr	bx, cl
	add	ax, bx		; Number of segments before logo starts
	push	ax
	mov	ax, 7
	push	ax
	retf				; Simulate far jump to LogoStart:0007h

	align 16
LogoStart:
code	ends
	END


;
; vmode.inc:
; set video mode
;

name vmode

include fsd.inc

public  set_videomode_
public  graphics_get_font_
public  graphics_set_palette_

public  _cursorX
public  _cursorY
public  _cursorCount
public  _cursorBuf

extrn   call_rm    :near

_TEXT16 segment dword public 'CODE' use16

set_videomode_rm_:
	; sti		; it is not bad keeping interrupt off
	sti		; for hardware interrupt or watchdog

	xor	bx, bx
	mov	ah, 0fh
	int	10h			; Get Current Video mode
	mov	ch, al
	xor	ah, ah
	mov	al, cl
	int	10h			; Set Video mode

        retf

graphics_get_font_rm_:
	; sti		; it is not bad keeping interrupt off
	sti		; for hardware interrupt or watchdog

	mov	ax, 1130h
	mov	bh, 6		; font 8x16
	int	10h
	mov	dx, bp
	mov	cx, es

        retf

graphics_set_palette_rm_:
	; sti		; it is not bad keeping interrupt off
	sti		; for hardware interrupt or watchdog 

	mov	bh, bl
	mov	ax, 1000h
	int	10h

        retf

_TEXT16 ends

.386

_TEXT   segment dword public 'CODE' use32


;/*
; * int set_videomode(mode)
; * BIOS call "INT 10H Function 0h" to set video mode
; *	Call with	%ah = 0x0
; *			%al = video mode
; *      Returns old videomode.
; */

set_videomode_:
	push	ebp
	push	ebx
	push	ecx

	mov	cl, [esp + 10h]
	
        ; enter real mode
        mov     eax, EXT2LOBUF_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:set_videomode_rm_
        push    eax
        call    call_rm
        add     esp, 4

	xor	ah, ah
	mov	al, ch

	pop	ecx
	pop	ebx
	pop	ebp

	ret

;/*
; * unsigned char * graphics_get_font()
; * BIOS call "INT 10H Function 11h" to set font
; *      Call with       %ah = 0x11
; */

graphics_get_font_:
	push	ebp
	push	ebx
	push	ecx
	push	edx

        ; enter real mode
        mov     eax, EXT2LOBUF_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:graphics_get_font_rm_
        push    eax
        call    call_rm
        add     esp, 4

	xor	eax, eax
	mov	ax, cx
	shl	eax, 4
	mov	ax, dx

	pop	edx
	pop	ecx
	pop	ebx
	pop	ebp

	ret

;/*
; * graphics_set_palette(index, red, green, blue)
; * BIOS call "INT 10H Function 10h" to set individual dac register
; *	Call with	%ah = 0x10
; *			%bx = register number
; *			%ch = new value for green (0-63)
; *			%cl = new value for blue (0-63)
; *			%dh = new value for red (0-63)
; */
graphics_set_palette_:
	push	ebp
	push	eax
	push	ebx
	push	ecx
	push	edx

	mov	bx, 3c8h		; address write mode register

	; wait vertical retrace

	mov	dx, 3dah
l1b:	in	al, dx			; wait vertical active display
	test	al, 8
	jnz	l1b

l2b:	in	al, dx			; wait vertical retrace
	test	al, 8
	jnz	l2b

	mov	dx, bx
	mov	al, [esp + 18h]		; index
	out	dx, al
	inc	dx

	mov	al, [esp + 1ch]	; red
	out	dx, al

	mov	al, [esp + 20h]		; green
	out	dx, al

	mov	al, [esp + 24h]		; blue
	out	dx, al

	mov	bx, [esp + 18h]

        ; enter real mode
        mov     eax, EXT2LOBUF_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:graphics_set_palette_rm_
        push    eax
        call    call_rm
        add     esp, 4

	pop	edx
	pop	ecx
	pop	ebx
	pop	eax
	pop	ebp

	ret

_cursorX      dw	0
_cursorY      dw	0
_cursorCount  dw        0
_cursorBuf    db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

_TEXT   ends

        end

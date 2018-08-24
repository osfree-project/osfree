;
;
;from Ralf Brown Interrupt list:
;
;PORT 03D6-03D7 - CGA (Color Graphics Adapter) - MIRRORS OF 03D4/03D5
;
;03D6  -W  same as 03D4
;	(under OS/2, reads return 0 if full-screen DOS session,
;	  nonzero if windowed DOS session)
;
;
_TEXT   segment dword public 'CODE'  use16

.8086
	org	100h

start:

	mov     dx, 03D6h
	in	al, dx
	cmp     al, 0
	mov     ax, 4C01h
	jz      fs
	mov	al, 0
fs:	int     21h  ; exit with errorlevel

_TEXT   ends

end start

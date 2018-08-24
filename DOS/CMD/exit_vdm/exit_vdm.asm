_TEXT   segment dword public 'CODE'  use16

.8086
	org	100h

start:
; get errorlevel
mov ah,04Dh
int 21h

; call Exit VDM function with exit code in AX
hlt
db 02h
db not 02h

; terminate program
int 20h

_TEXT   ends

end start

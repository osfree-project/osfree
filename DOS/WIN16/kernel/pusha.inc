@push_a macro
if ?REAL
	push ax
	push cx
	push dx
	push bx
	push sp
	push bp
	push si
	push di
else
	pusha
endif
	endm

@pop_a macro
if ?REAL
	pop di
	pop si
	pop bp
	pop ax		; sp
	pop bx
	pop dx
	pop cx
	pop ax
else
	popa
endif
	endm

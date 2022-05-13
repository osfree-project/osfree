
@SetMode	macro	mode
		xor     ax,ax
		mov     al,mode
		int	10h
		endm

@GetMode	macro
		mov     ah,15
		int     10h
		endm
	
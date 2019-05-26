;	DPMICL32.ASM: NASM Sample for a 32-bit DPMI client.
;	To assemble, use:
;		nasm dpmicl32.asm -O 2 -f bin -o dpmicl32.com
;

LF	equ	10
CR	equ	13

	cpu	386

	use16

	org 100h

	pop ax			;get word saved on stack for COM files
    mov bx, sp
    shr bx, 4
    jnz stackok
    mov bx,1000h	;it was a full 64kB stack
stackok:    
	mov ah, 4Ah		;free unused memory
    int 21h
    mov ax, 1687h	;DPMI host installed?
    int 2Fh
    and ax, ax
    jnz nohost
    push es			;save DPMI entry address
    push di
    and si, si		;requires host client-specific DOS memory?
    jz nomemneeded
    mov bx, si
    mov ah, 48h		;alloc DOS memory
    int 21h
    jc nomem
    mov es, ax
nomemneeded:
	mov bp, sp
    mov ax, 0001	;start a 32-bit client
    call far [bp]	;initial switch to protected-mode
    jc initfailed

;--- now in protected-mode

	mov bx,cs
    mov cx,cs
    lar cx,cx
    shr cx,8
    or  ch,40h		;make a 32bit CS
    mov ax,9
    int 31h

;--- now in 32-bit protected-mode

	use32

    mov esi, szWelcome
    call printstring
    mov ax, 4C00h	;normal client exit
    int 21h

;--- print a string in protected-mode with simple
;--- DOS commands not using pointers.

printstring:    
    lodsb
    and al,al
    jz stringdone
    mov dl,al
    mov ah,2
    int 21h
    jmp printstring
stringdone:
	ret

	use16

nohost:
	mov dx, dErr1
    jmp error
nomem:
	mov dx, dErr2
    jmp error
initfailed:
	mov dx, dErr3
error:    
    mov ah, 9
    int 21h
    mov ax, 4C00h
    int 21h

dErr1 db "no DPMI host installed",CR,LF,'$'
dErr2 db "not enough DOS memory for client initialisation",CR,LF,'$'
dErr3 db "DPMI initialisation failed",CR,LF,'$'
szWelcome db "welcome in protected-mode",CR,LF,0


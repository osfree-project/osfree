;	DPMICL16.ASM: NASM Sample for a 16-bit DPMI client.
;	To assemble, use:
;		nasm dpmicl16.asm -O 2 -f bin -o dpmicl16.com
;

LF	equ	10
CR	equ	13

	org	100h

	cpu	286

	pop ax			;get word saved on stack
	mov bx, sp
    shr bx, 4
    jnz stackok
    mov bx,1000h	;it was a full 64kB stack
stackok:
	mov ah, 4Ah		;release DOS memory
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
    mov ax, 0000	;start a 16-bit client
    call far [bp]	;initial switch to protected-mode
    jc initfailed

;--- now in protected-mode
    
    mov si, szWelcome
    call printstring
    mov ax, 4C00h	;this exits protected-mode
    int 21h

nohost:
	mov dx, dErr1
    jmp error
nomem:
	mov dx, dErr2
    jmp error
initfailed:
	mov dx, dErr3
error:    
    mov ah,9
    int 21h
    mov ax,4C00h
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

dErr1 db "no DPMI host installed",CR,LF,'$'
dErr2 db "not enough DOS memory for client initialisation",CR,LF,'$'
dErr3 db "DPMI initialisation failed",CR,LF,'$'
szWelcome db "welcome in protected-mode",CR,LF,0


;
; console.asm:
; VGA/EGA console input/output
;

name console

.386p

include fsd.inc
include keyb.inc

public console_putchar
public console_getkey
public console_checkkey
public console_getxy
public console_gotoxy
public console_cls
public console_setcursor

extrn  call_rm                  :near
extrn  console_current_color    :dword

_TEXT16 segment dword public 'CODE' use16

;
;  translate_keycode translates the key code %dx to an ascii code.
;
translate_keycode:
	push	bx
	push	si
	
	lea	si, translation_table
	
l1:	lodsw
	; check if this is the end
	test	ax, ax
	jz	l2
	; load the ascii code into ax
	mov	bx, ax
	lodsw
	; check if this matches the key code
	cmp	dx, bx
	jne	l1
	; translate dx, if successful
	mov	dx, ax

l2:	pop	si
	pop	bx

	ret

; this table is used in translate_keycode below
translation_table:
	dw	KEY_LEFT, 2
	dw	KEY_RIGHT, 6
	dw	KEY_UP, 16
	dw	KEY_DOWN, 14
	dw	KEY_HOME, 1
	dw	KEY_END, 5
	dw	KEY_DC, 4
	dw	KEY_BACKSPACE, 8
	dw	KEY_PPAGE, 7
	dw	KEY_NPAGE, 3
	dw	0

align	4
ascii_key_map:
	db	((KEY_MAP_SIZE + 1) * 2)  dup (0)

;
;  remap_ascii_char remaps the ascii code %dl to another if the code is
;  contained in ASCII_KEY_MAP.
; 
remap_ascii_char:
	push	si
	
	lea	si, ascii_key_map
lb1:
	lodsw
	; check if this is the end
	test	ax, ax
	jz	lb2
	; check if this matches the ascii code
	cmp	dl, al
	jne	lb1
	; if so, perform the mapping
	mov	dl, ah
lb2:
	; restore si
	pop	si

	ret

console_putchar_rm:
	mov	al, dl
	xor	bh, bh

ifndef STAGE1_5
	; use teletype output if control character
	cmp	al, 07h
	je	ll1
	cmp	al, 08h
	je	ll1
	cmp	al, 0ah
	je	ll1
	cmp	al, 0dh
	je	ll1

	; save the character and the attribute on the stack
	push	ax
	push	bx
	
	; get the current position
	mov	ah, 03h
	int	10h

	; check the column with the width
	cmp	dl, 79
	jl	ll2
	
	; print CR and LF, if next write will exceed the width
	mov	ax, 0e0dh
	int	10h
	mov	al, 0ah
	int	10h
	
	; get the current position
	mov	ah, 03h
	int	10h

ll2:	
	; restore the character and the attribute
	pop	bx
	pop	ax
	
	; write the character with the attribute
	mov	ah, 09h
	mov	cx, 1
	int	10h

	; move the cursor forward
	inc	dl
	mov	ah, 02h
	int	10h

	jmp	ll3
endif   ; ! STAGE1_5
	
ll1:	mov	ah, 0eh
	int	10h
ll3:
	retf


console_getkey_rm:
	int	16h

	; real_to_prot uses eax
	mov	dx, ax
	call	translate_keycode
	call	remap_ascii_char

        retf


console_checkkey_rm:
	mov	AH, 01H
	int	16H

	jz	notpending
	
	mov	dx, ax
	call	translate_keycode
	call	remap_ascii_char
	jmp	pending

notpending:
	mov	edx, 0FFFFFFFFh

pending:
	retf


console_getxy_rm:
        xor	bh, bh                ; set page to 0
	mov	ah, 03h
	int	10h		      ; get cursor position

	retf

console_gotoxy_rm:
        xor	bh, bh                ; set page to 0
	mov	ah, 2
	int	10h		      ; set cursor position

	retf

console_cls_rm:
	; move the cursor to the beginning
	mov	ah, 02h
	xor	bh, bh
	xor	dx, dx
	int	10h

	; write spaces to the entire screen
	mov	ax, 0920h
	mov	bx, 07h
	mov	cx, 80 * 25
        int	10h

	; move back the cursor
	mov	ah, 02h
	int	10h

	retf

console_setcursor1_rm:
	mov	ah, 03h
	xor	bh, bh
	int	10h

	retf

console_setcursor2_rm:
	mov     ah, 1
	int     10h 

	retf


_TEXT16 ends

_TEXT    segment dword public 'CODE'  use32

; 
;   void console_putchar (int c)
;  
;   Put the character C on the console. Because GRUB wants to write a
;   character with an attribute, this implementation is a bit tricky.
;   If C is a control character (CR, LF, BEL, BS), use INT 10, AH = 0Eh
;   (TELETYPE OUTPUT). Otherwise, save the original position, put a space,
;   save the current position, restore the original position, write the
;   character and the attribute, and restore the current position.
;  
;   The reason why this is so complicated is that there is no easy way to
;   get the height of the screen, and the TELETYPE OUPUT BIOS call doesn't
;   support setting a background attribute.
; 
console_putchar:
	mov	edx, [esp + 4]
	pusha
ifdef STAGE1_5
	mov	bl, 07h
else
	mov	ebx, console_current_color
endif
        ; enter real mode
        mov     eax, TERM_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:console_putchar_rm
        push    eax
        call    call_rm
        add     esp, 4
	
	popa
	ret


ifndef STAGE1_5

;
;   int console_getkey (void)
;   BIOS call "INT 16H Function 00H" to read character from keyboard
;        Call with	%ah = 0x0
;   	 Return:	%ah = keyboard scan code
; 			%al = ASCII character
; 

console_getkey:
	push	ebp

        ; enter real mode
        mov     eax, TERM_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:console_getkey_rm
        push    eax
        call    call_rm
        add     esp, 4

	mov	ax, dx

	pop	ebp

	ret


;
;   int console_checkkey (void)
;  	if there is a character pending, return it; otherwise return -1
;   BIOS call "INT 16H Function 01H" to check whether a character is pending
; 	Call with	%ah = 0x1
; 	Return:
; 		If key waiting to be input:
; 			%ah = keyboard scan code
; 			%al = ASCII character
; 			Zero flag = clear
; 		else
; 			Zero flag = set
; 
console_checkkey:
	push	ebp
	xor	edx, edx

        ; enter real mode
        mov     eax, TERM_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:console_checkkey_rm
        push    eax
        call    call_rm
        add     esp, 4

	mov	eax, edx

	pop	ebp

	ret

	
;
;   int console_getxy (void)
;   BIOS call "INT 10H Function 03h" to get cursor position
; 	Call with	%ah = 0x03
;  			%bh = page
;        Returns         %ch = starting scan line
;                        %cl = ending scan line
;                        %dh = row (0 is top)
;                        %dl = column (0 is left)
; 
console_getxy:
	push	ebp
	push	ebx                    ; save EBX

        ; enter real mode
        mov     eax, TERM_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:console_getxy_rm
        push    eax
        call    call_rm
        add     esp, 4

	mov	ah, dl
	mov	al, dh

	pop	ebx
	pop	ebp

	ret


;
;   void console_gotoxy(int x, int y)
;   BIOS call "INT 10H Function 02h" to set cursor position
; 	Call with	%ah = 0x02
;  			%bh = page
;                        %dh = row (0 is top)
;                        %dl = column (0 is left)
; 
console_gotoxy:
	push	ebp
	push	ebx                    ; save EBX

	mov	dl, [esp + 0ch]        ; %dl = x
	mov	dh, [esp + 10h]        ; %dh = y

        ; enter real mode
        mov     eax, TERM_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:console_gotoxy_rm
        push    eax
        call    call_rm
        add     esp, 4

	pop	ebx
	pop	ebp

	ret

	
;
;   void console_cls (void)
;   BIOS call "INT 10H Function 09h" to write character and attribute
;  	Call with	%ah = 0x09
;                        %al = (character)
;                        %bh = (page number)
;                        %bl = (attribute)
;                        %cx = (number of times)
; 
console_cls:
	push	ebp
	push	ebx                    ; save EBX

        ; enter real mode
        mov     eax, TERM_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:console_cls_rm
        push    eax
        call    call_rm
        add     esp, 4

	pop	ebx
	pop	ebp

	ret

	
;
;   int console_setcursor (int on)
;   BIOS call "INT 10H Function 01h" to set cursor type
;        Call with       %ah = 0x01
;                        %ch = cursor starting scanline
;                        %cl = cursor ending scanline
; 
console_cursor_state	db	1
console_cursor_shape	dw	0
	
console_setcursor:
	push	ebp
	push	ebx

	; check if the standard cursor shape has already been saved
	mov	ax, console_cursor_shape
	test	ax, ax
	jne	lw1

        ; enter real mode
        mov     eax, TERM_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:console_setcursor1_rm
        push    eax
        call    call_rm
        add     esp, 4

	mov	console_cursor_shape, cx
lw1:
	; set %cx to the designated cursor shape
	mov	cx, 2000h
	mov	ebx, [esp + 0ch]
	test	ebx, ebx
	jz	lw2
	mov	cx, console_cursor_shape
lw2:	
        ; enter real mode
        mov     eax, TERM_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:console_setcursor2_rm
        push    eax
        call    call_rm
        add     esp, 4

	movzx	eax, console_cursor_state
	mov	console_cursor_state, bl
	
	pop	ebx
	pop	ebp

	ret

endif

_TEXT   ends

        end

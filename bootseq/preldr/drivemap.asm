;
; map.asm:
; int 13h handler and its installer
; (for drive mapping)
;

name map

.386p

include mb_etc.inc

_TEXT16  segment dword public 'CODE' use16

; The size of the drive map.
DRIVE_MAP_SIZE	equ	8

;
;  Map a drive to another drive.
; 

int13_handler:
	push	ax
	push	bp
	mov	bp, sp
	
	push	si

	; set si to the drive map
	mov	si, drive_map - int13_handler
	; find the drive number from the drive map
	cld
ll1:	
	cs 	lodsw
	; check if this is the end
	test	ax, ax
	jz	ll2
	; check if this matches the drive number
	cmp	dl, al
	jne	ll1
	; if so, perform the mapping
	mov	dl, ah
ll2:
	; restore si
	pop	si
	; save ax in the stack
	push	ax
	; simulate the interrupt call
	push	word ptr [bp + 8]
	; set ax and bp to the original values
	mov	ax, [bp + 2]
	mov	bp, [bp]
	; lcall
	        db	9ah
int13_offset	dw	0
int13_segment	dw	0
	; save flags
	pushf
	; restore bp
	mov	bp, sp
	; save ax
	push	ax
	; set the flags in the stack to the value returned by int13
	mov	ax, [bp]
	mov	[bp + 0ch], ax
	; check if should map the drive number
	mov	ax, [bp + 6]
	cmp	ax, 8h
	jne	ll3
	cmp	ax, 15h
	jne	ll3
	; check if the mapping was performed
	mov	ax, [bp + 2]
	test	ax, ax
	jz	ll3
	; perform the mapping
	mov	dl, al
ll3:
	pop	ax
	mov	bp, [bp + 4]
	add	sp, 8

	iret

align	4

drive_map	db	(DRIVE_MAP_SIZE + 1) * 2  dup (0)
int13_handler_end:

_TEXT16  ends

	
_TEXT    segment dword public 'CODE' use32

;
;  set_int13_handler(map)
; 
;  Copy MAP to the drive map and set up int13_handler.
; 
set_int13_handler:
	push	ebp
	mov	ebp, esp

	push	edi
	push	esi

	; copy MAP to the drive map
	mov	ecx, DRIVE_MAP_SIZE * 2
	mov	edi, offset _TEXT16:drive_map + KERN_BASE
	mov	esi, [ebp + 8]
	cld
	rep 	movsb

	; save the original int13 handler
	mov	edi, 4ch

	mov	ax, [edi]
	mov     ebx, offset _TEXT16:int13_offset + KERN_BASE
 	mov	[ebx], ax

	mov	ax, [edi + 2]
	mov     ebx, offset _TEXT16:int13_segment + KERN_BASE
	mov	[ebx], ax
	
	; decrease the lower memory size and set it to the BIOS memory
	mov	edi, 413h
	dec	word ptr [edi]
	xor	eax, eax
	mov	ax, [edi]
	
	; compute the segment
	shl	eax, 6

	; save the new int13 handler
	mov	edi, 4ch
	mov	[edi + 2], ax
	xor	cx, cx
	mov	[edi], cx

	; copy int13_handler to the reserved area
	shl	eax, 4
	mov	edi, eax
	mov	esi, offset _TEXT16:int13_handler + KERN_BASE
	mov	ecx, int13_handler_end - int13_handler
	rep	movsb

	pop	esi
	pop	edi
	pop	ebp

	ret

_TEXT   ends

        end

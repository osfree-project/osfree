;
; assembly support routines for FreeDOS MEM
;
        group   DGROUP _DATA

        segment	_DATA

        extern  _xms_drv:wrt DGROUP

        segment _TEXT class=CODE

; struct xms_dx call_xms_driver_dx_bl_al(unsigned char ah, ushort dx)
global _call_xms_driver_dx_bl_al
_call_xms_driver_dx_bl_al:
	mov	bx, sp
	mov	ah, [bx+2]
	mov	dx, [bx+4]
	call	far [_xms_drv]
	mov	ah, bl
	ret

; struct xms_bx call_xms_driver_bx(unsigned char ah, ushort dx)
global _call_xms_driver_bx_ax
_call_xms_driver_bx_ax:
	mov	bx, sp
	mov	ah, [bx+2]
	mov	dx, [bx+4]
	call	far [_xms_drv]
	mov	dx, bx
	ret

; ulong cdecl call_xms_driver_edx(unsigned char rah, ushort rdx);
global _call_xms_driver_edx
_call_xms_driver_edx:
	mov	bx, sp
	mov	ah, [bx+2]
	mov	dx, [bx+4]
	call	far [_xms_drv]
xms_ext_common: 
	test	bl, bl
	je	noerror
	xor	edx, edx
noerror:
	mov	ax, dx
	shr	edx, 0x10
	ret

; ulong cdecl call_xms_driver_eax(unsigned char rah, ushort rdx);
global _call_xms_driver_eax
_call_xms_driver_eax:
	mov	bx, sp
	mov	ah, [bx+2]
	mov	dx, [bx+4]
	call	far [_xms_drv]
	mov	edx, eax
        jmp	short xms_ext_common

; int cdecl is_386_(void)        
global _is_386_
_is_386_:
        pushf
        pushf
        pop     ax
        or      ax, 0x7000
	and	ax, 0x7fff
        push    ax
        popf
        pushf
        pop     ax
        popf
        ret

; ulong check_e820(struct e820map far *e820map, ulong counter)
global _check_e820
_check_e820:
        mov	bx, sp
	push	di
	push	si
	les	di, [bx+2]
	mov	ax, [bx+6]
	mov	bx, [bx+8]
	shl	ebx, 0x10
	mov	bx, ax
	mov	eax, 0xe820
	mov	edx, 0x534d4150 ; SMAP
	mov	ecx, 0x14
	int	0x15
	jc	no386
	cmp	eax, 0x534d4150 ; SMAP
	je	yes386
 no386:
	xor	ebx, ebx
 yes386:
	mov	ax, bx
	shr	ebx, 16
	mov	dx, bx
	pop	si
	pop	di
	ret

; ulong get_ext_mem_size(void)
global _get_ext_mem_size
_get_ext_mem_size:
	mov	ah, 0x88
;;; FIXME: need to save registers first?
	int	15h
	mov	dx, 0	; do not use xor here, it would modify flags
	jnc	success
	mov	dx, 1
 success:
	ret

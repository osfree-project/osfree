;
; asm32.asm:
; 32-bit assembler routines
;

.386

include fsd.inc

extrn   call_rm             :near
extrn   bss_end             :near

ifndef STAGE1_5

public stop
public stop_floppy

public get_memsize
public gateA20
public get_code_end
public get_mmap_entry
public get_rom_config_table
public get_eisamemsize

K_RDWR          equ	0x60	; keyboard data & cmds (read/write)
K_STATUS        equ	0x64	; keyboard status
K_CMD           equ	0x64	; keybd ctlr command (write-only) 

K_OBUF_FUL      equ 	0x01	; output buffer full
K_IBUF_FUL      equ 	0x02	; input buffer full

KC_CMD_WIN      equ	0xd0	; read  output port
KC_CMD_WOUT     equ 	0xd1	; write output port
KB_OUTPUT_MASK  equ     0xdd	; enable output buffer full interrupt
				;   enable data line
				;   enable clock line
KB_A20_ENABLE   equ     0x02

_TEXT16 segment dword public 'CODE' use16

        ;
        ;  This next part is sort of evil.  It takes advantage of the
        ;  byte ordering on the x86 to work in either 16-bit or 32-bit
        ;  mode, so think about it before changing it.
        ;

hard_stop:
        cli
        hlt

stop_flop:
        xor     dl, dl
        int     13h
        retf

get_memsize_rm:
	cmp	bl, 1
	je	xext

	int	12h
	jmp	xdone

xext:
	mov	ah, 88h
	int	15h

xdone:
	mov	ebx, eax
        retf


get_eisamemsize_rm:
	mov	ax, 0e801h
	int	15h

	shl	ebx ,16
	mov	bx, ax

        retf


gateA20_rm:
	mov	ax, 2400h
	test	dx, dx
	jz	ft1
	inc	ax
ft1:	stc
	int	15h
	jnc	ft2

	; set non-zero if failed 
	mov	ah, 1

	; save the status
ft2:	mov	dl, ah
      
        retf


get_mmap_entry_rm:
	mov	es, si
	mov	eax, 0e820h
	int	15h

	jc	xnosmap

	cmp	eax, 534d4150h
	jne	xnosmap

	cmp	ecx, 14h
	jl	xnosmap

	cmp	ecx, 400h
	jg	xnosmap

	jmp	xsmap

xnosmap:
	mov	ecx, 0

xsmap:
        retf



get_rom_config_table_rm:
	mov	ax, 0c0h
	int	15h

	jc	no_rom_table
	test	ah, ah
	jnz	no_rom_table
	
	mov	dx, es
	jmp	found_rom_table
	
no_rom_table:
	xor	dx, dx
	xor	bx, bx
	
found_rom_table:
        retf



_TEXT16 ends


_TEXT    segment dword public 'CODE'  use32

;
;  multi_boot(int start, int mb_info)
;
;  This starts a kernel in the manner expected of the multiboot standard.
;


;
;  This call is special...  it never returns...  in fact it should simply
;  hang at this point!
;
stop:
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:hard_stop
        push    eax
        call    call_rm
        add     esp, 4

        ; we should not return here
;
;  stop_floppy()
;
;  Stops the floppy drive from spinning, so that other software is
;  jumped to with a known state.
;
stop_floppy:
        pusha
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:stop_flop
        push    eax
        call    call_rm
        add     esp, 4
        popa
        ret

;
; get_memsize(i) :  return the memory size in KB. i == 0 for conventional
;		memory, i == 1 for extended memory
;	BIOS call "INT 12H" to get conventional memory size
;	BIOS call "INT 15H, AH=88H" to get extended memory size
;		Both have the return value in AX.
;


get_memsize:
	push	ebp
	push	ebx

	mov	ebx, [esp + 0ch]

        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:get_memsize_rm
        push    eax
        call    call_rm
        add     esp, 4

	mov	eax, ebx
	pop	ebx
	pop	ebp
	ret


;
;
; get_eisamemsize() :  return packed EISA memory map, lower 16 bits is
;		memory between 1M and 16M in 1K parts, upper 16 bits is
;		memory above 16M in 64K parts.  If error, return -1.
;	BIOS call "INT 15H, AH=E801H" to get EISA memory map,
;		AX = memory between 1M and 16M in 1K parts.
;		BX = memory above 16M in 64K parts.
;
;

get_eisamemsize:
	push	ebp
	push	ebx

        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:get_eisamemsize_rm
        push    eax
        call    call_rm
        add     esp, 4

	mov	eax, 0FFFFFFFFh
	cmp	bh, 86h
	je	xnoteisa

	mov	eax, ebx

xnoteisa:
	pop	ebx
	pop	ebp
	ret


;
; gateA20(int linear)
;
; Gate address-line 20 for high memory.
;
; This routine is probably overconservative in what it does, but so what?
;
; It also eats any keystrokes in the keyboard buffer.  :-(
;

gateA20:
	; first, try a BIOS call 
	push	ebp
	mov	edx, [esp + 8]
	
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:gateA20_rm
        push    eax
        call    call_rm
        add     esp, 4

	pop	ebp
	test	dl, dl
	jnz	ft3
	ret

ft3:	; use keyboard controller 
	push	eax

	call    gloop1

	mov	al, KC_CMD_WOUT
	out	K_CMD, al

gloopint1:
	in	al, K_STATUS
	and	al, K_IBUF_FUL
	jnz	gloopint1

	mov	al, KB_OUTPUT_MASK
	cmp	byte ptr [esp + 8], 0
	jz	gdoit

	or	al, KB_A20_ENABLE
gdoit:
	out	K_RDWR, al

	call	gloop1

	; output a dummy command (USB keyboard hack)
	mov	al, 0ffh
	out	K_CMD, al
	call	gloop1
	
	pop	eax
	ret

gloop1:
	in	al, K_STATUS
	and	al, K_IBUF_FUL
	jnz	gloop1

gloop2:
	in	al, K_STATUS
	and	al, K_OBUF_FUL
	jz	gloop2ret
	in	al, K_RDWR
	jmp	gloop2

gloop2ret:
	ret


;
; get_code_end() :  return the address of the end of the code
; This is here so that it can be replaced by asmstub.c.
;
get_code_end:
	; will be the end of the bss
        mov     eax, LDR_BASE + 0x20000
        ;mov     eax, offset _TEXT:bss_end
	; Round up to the next word.
	;shr	eax, 2
	;inc	eax
	;shl	eax, 2
	ret


;
;
; get_mmap_entry(addr, cont) :  address and old continuation value (zero to
;		start), for the Query System Address Map BIOS call.
;
;  Sets the first 4-byte int value of "addr" to the size returned by
;  the call.  If the call fails, sets it to zero.
;
;	Returns:  new (non-zero) continuation value, 0 if done.
;
; NOTE: Currently hard-coded for a maximum buffer length of 1024.
;

get_mmap_entry:
	push	ebp
	push	ebx
	push	edi
	push	esi

	; place address (+4) in ES:DI
	mov	eax, [esp + 14h]
	add	eax, 4
	mov	edi, eax
	and	edi, 0fh
	shr	eax, 4
	mov	esi, eax

	; set continuation value
	mov	ebx, [esp + 18h]

	; set default maximum buffer size
	mov	ecx, 14h

	; set EDX to 'SMAP'
	mov	edx, 534d4150h
 
        ; enter real mode
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:get_mmap_entry_rm
        push    eax
        call    call_rm
        add     esp, 4

	; write length of buffer (zero if error) into "addr"
	mov	eax, [esp + 14h]
	mov	[eax], ecx

	; set return value to continuation
	mov	eax, ebx

	pop	esi
	pop	edi
	pop	ebx
	pop	ebp
	ret


;
; get_rom_config_table()
;
; Get the linear address of a ROM configuration table. Return zero,
; if fails.
;
	
get_rom_config_table:
	push	ebp
	push	ebx

	; zero ebx for simplicity
	xor	ebx, ebx

        ; enter real mode
        mov     eax, STAGE0_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:get_rom_config_table_rm
        push    eax
        call    call_rm
        add     esp, 4

	; compute the linear address
	mov	ax, dx
	shl	eax, 4
	add	eax, ebx

	pop	ebx
	pop	ebp
	ret

_TEXT   ends

endif

        end

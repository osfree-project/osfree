;
; linux.asm
; linux multiboot kernel startup
;

name linux

.386p

include fsd.inc
include linux.inc
include struc.inc
include mb_info.inc
include mb_header.inc
include mb_etc.inc

extrn boot_drive      :dword
extrn call_rm         :near
extrn cmain           :near
extrn exe_end         :near
extrn bss_end         :near
extrn gdtsrc          :byte
extrn gdtdesc         :fword
extrn l               :dword
extrn m               :dword

public base

public stop
public start_kernel

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

_TEXT16  segment dword public 'CODE'  use16
_TEXT16  ends
_TEXT    segment dword public 'CODE'  use32
_TEXT    ends
_DATA    segment dword public 'DATA'  use32
_DATA    ends
CONST    segment dword public 'DATA'  use32
CONST    ends
CONST2   segment dword public 'DATA'  use32
CONST2   ends
_end1    segment dword public 'DATA'  use32
_end1    ends
_end2    segment dword public 'DATA'  use32
_end2    ends
_BSS     segment dword public 'BSS'   use32
_BSS     ends
_end3    segment dword public 'BSS'   use32
_end3    ends
_end4    segment dword public 'BSS'   use32
_end4    ends
_STACK   segment dword public 'STACK' use32
_STACK   ends

DGROUP   group _TEXT,_DATA,CONST,CONST2,_end1,_end2,_BSS,_end3,_end4,_STACK

_STACK   segment dword public 'STACK' use32
STACK_SIZE equ 4000h
public stack_top
public stack_bottom
stack_bottom     db STACK_SIZE dup (?)
stack_top        label byte
_STACK   ends

_TEXT16  segment dword public 'CODE' use16
begin:

        org   0h
start:
        jmp   real_start
        org   10h
base    dd    REAL_BASE
        org   20h
real_start:
        ; Start loader
        mov   dl, bl

        push  0
        push  0x7c00
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

hard_stop:
        cli
        hlt

stop_flop:
        xor     dl, dl
        int     13h
        retf

pad1size        equ STARTUP_SIZE - ($ - start)
pad1            db  pad1size dup (0)

_TEXT16  ends


_TEXT    segment dword public 'CODE'  use32

BASE1           equ     KERN_BASE - 0x10000
VIDEO_BUF       equ     0xb8000

        org     BASE1
start1:

        org     BASE1 + _TEXT16_SIZE

start2:

align 4

; Multiboot header
_magic          equ       0x1badb002
_flags          equ       0x00010001
_checksum       equ       - _magic - _flags
_mbhdr          multiboot_header  <_magic,_flags,_checksum,_mbhdr,start1,exe_end,bss_end,entry,0,0,0,0>

        org     start2 + 0x80

entry:
        cmp   eax, MULTIBOOT_VALID
        jne   stop

        ; save multiboot structure address
        mov   m, ebx

        call  set_gdt

        ; set stack
        ;mov   esp, stack_top
        ; at the moment, we use loader stack

        ; copy realmode part of boot_chain at REAL_BASE
        cld
        mov     ecx, 0x80
        mov     esi, KERN_BASE
        mov     edi, REAL_BASE
 
        rep     movsd

        call    cmain

        ; We should not return here
        cli
        hlt
        jmp     $

stop:
        cld
        lea   esi, errmsg
        mov   edi, VIDEO_BUF
        mov   ah, 02h  ; attribute
loop1:
        lodsb          ; symbol
        stosw
        test  al, al   ; copy a string to video buffer
        jnz   loop1

        cli
        hlt
        jmp   $

set_gdt:
        ; set 16-bit segment (_TEXT16) base
        ; in GDT for protected mode
        ;lea  eax, oldgdtdesc
        ;sgdt fword ptr [eax]

        ; copy old gdt
        ;movzx ecx, [eax].g_limit
        ;inc  ecx
        ;shr  ecx, 2
        ;mov  esi, [eax].g_base
        ;mov  edi, offset _TEXT:gdt
        ;mov  ebx, edi
        ;rep  movsd

        ; fix gdt descriptors base
	mov  ebx, GDT_ADDR
        mov  eax, REAL_BASE
        mov  [ebx][7*8].ds_baselo, ax
        mov  [ebx][8*8].ds_baselo, ax
        ror  eax, 16
        mov  [ebx][7*8].ds_basehi1, al
        mov  [ebx][8*8].ds_basehi1, al
        ror  eax, 8
        mov  [ebx][7*8].ds_basehi2, al
        mov  [ebx][8*8].ds_basehi2, al

	; fill GDT descriptor
	mov  ebx, offset _TEXT:gdtdesc
	mov  eax, GDT_ADDR
	mov  [ebx].g_base, eax

        lgdt fword ptr [ebx]

        ret

;
; void start_kernel(void);
;
start_kernel:
        xor  eax, eax
        push eax
	call gateA20	
        add  eax, 4

        mov  ebx, boot_drive   
        mov  eax, REAL_BASE
        shl  eax, 12
        mov  ax, offset _TEXT16:start
        push eax
        call call_rm
        add  esp, 4

        ;ret

        ; we should not return here
        cli
        hlt

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
	
        mov     eax, REAL_BASE
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
;  stop_floppy()
;
;  Stops the floppy drive from spinning, so that other software is
;  jumped to with a known state.
;
stop_floppy:
        pusha
        mov     eax, REAL_BASE
        shl     eax, 12
        mov     ax,  offset _TEXT16:stop_flop
        push    eax
        call    call_rm
        add     esp, 4
        popa
        ret

errmsg  db  "This is not a multiboot loader or no LIP module!",0

oldgdtdesc gdtr <>

_TEXT    ends

         end entry

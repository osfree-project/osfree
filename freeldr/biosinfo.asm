;
; biosinfo.h
;

;/*
; *  GRUB  --  GRand Unified Bootloader
; *  Copyright (C) 1999,2000,2001,2002,2004 Free Software Foundation, Inc.
; *
; *  This program is free software; you can redistribute it and/or modify
; *  it under the terms of the GNU General Public License as published by
; *  the Free Software Foundation; either version 2 of the License, or
; *  (at your option) any later version.
; *
; *  This program is distributed in the hope that it will be useful,
; *  but WITHOUT ANY WARRANTY; without even the implied warranty of
; *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; *  GNU General Public License for more details.
; *
; *  You should have received a copy of the GNU General Public License
; *  along with this program; if not, write to the Free Software
; *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
; */
;
;/*
; * Note: These functions defined in this file may be called from C.
; *       Be careful of that you must not modify some registers. Quote
; *       from gcc-2.95.2/gcc/config/i386/i386.h:
;
;   1 for registers not available across function calls.
;   These must include the FIXED_REGISTERS and also any
;   registers that can be used without being saved.
;   The latter must include the registers where values are returned
;   and the register where structure-value addresses are passed.
;   Aside from that, you can include as many other registers as you like.
;
;  ax,dx,cx,bx,si,di,bp,sp,st,st1,st2,st3,st4,st5,st6,st7,arg
;{  1, 1, 1, 0, 0, 0, 0, 1, 1,  1,  1,  1,  1,  1,  1,  1,  1 }
; */


public get_memsize
public get_eisamemsize
public get_mmap_entry
public get_rom_config_table
public get_code_end
public get_diskinfo_standard
public check_int13_extensions
public biosdisk_standard
public biosdisk_int13_extensions

extrn printk:NEAR
extrn _freeldr_stack_top:WORD
extrn current_seg:WORD

.386

_TEXT segment word public 'CODE' use16

;/*
; *
; * get_memsize(i) :  return the memory size in KB. i == 0 for conventional
; *              memory, i == 1 for extended memory
; *      BIOS call "INT 12H" to get conventional memory size
; *      BIOS call "INT 15H, AH=88H" to get extended memory size
; *              Both have the return value in AX.
; *
; */
;
;long __cdecl get_memsize (long type);
get_memsize proc near

        push bp
        mov  bp, sp

        push bx

        mov  bx, word ptr [bp + 4]

        xor  dx, dx

        cmp  bl, 1
        je   xext

        int  12h
        jmp  xdone

xext:
        mov  ah, 88h
        int  15h

xdone:
        pop  bx
        pop  bp

        ret

get_memsize endp

;/*
; *
; * get_eisamemsize() :  return packed EISA memory map, lower 16 bits is
; *              memory between 1M and 16M in 1K parts, upper 16 bits is
; *              memory above 16M in 64K parts.  If error, return -1.
; *      BIOS call "INT 15H, AH=E801H" to get EISA memory map,
; *              AX = memory between 1M and 16M in 1K parts.
; *              BX = memory above 16M in 64K parts.
; *
; */
;
;long __cdecl get_eisamemsize (void);
get_eisamemsize proc near

        push bp
        push bx
        push si
        push di

        mov  ax, 0e801h
        int  15h

        mov  si, bx
        mov  di, ax

        mov  ax, 0FFFFh
        mov  dx, 0FFFFh
        cmp  bh, 86h
        ; return error
        je   xnoteisa

        ; return memsize in dx:ax
        mov  dx, si
        mov  ax, di

xnoteisa:
        pop  di
        pop  si
        pop  bx
        pop  bp

        ret

get_eisamemsize endp

;/*
; *
; * get_mmap_entry(addr, cont) :  address and old continuation value (zero to
; *              start), for the Query System Address Map BIOS call.
; *
; *  Sets the first 4-byte int value of "addr" to the size returned by
; *  the call.  If the call fails, sets it to zero.
; *
; *      Returns:  new (non-zero) continuation value, 0 if done.
; *
; * NOTE: Currently hard-coded for a maximum buffer length of 1024.
; */
;
;//long get_mmap_entry (struct mmar_desc *desc, long cont);
;long __cdecl get_mmap_entry (unsigned long desc, long cont);
get_mmap_entry proc near

        push bp
        mov  bp, sp

        push bx
        push cx
        push di
        push si
        push es

        ; Print function params
        ;push word ptr [bp + 10]
        ;push word ptr [bp + 8]
        ;push word ptr [bp + 6]
        ;push word ptr [bp + 4]
        ;lea  ax, mmap_msg
        ;push ax
        ;call printk
        ;add  sp, 10

        ; place address (+4) in ES:DI
        mov  eax, dword ptr [bp + 4]
        add  eax, 4
        mov  edi, eax
        and  edi, 0fh
        shr  eax, 4
        mov  esi, eax

        ; set continuation value
        mov  ebx, dword ptr [bp + 8]

        ; set default maximum buffer size
        mov  ecx, 14h

        ; set EDX to 'SMAP'
        mov  edx, 534d4150h

        mov  es, si
        mov  eax, 0e820h
        int  15h

        jc   xnosmap

        cmp  eax, 534d4150h
        jne  xnosmap

        cmp  ecx, 14h
        jl   xnosmap

        cmp  ecx, 400h
        jg   xnosmap

        jmp  xsmap

xnosmap:
        mov  ecx, 0

xsmap:
        ; load phys address into edx
        mov  edx, dword ptr [bp + 4]
        ; convert into far pointer in es:di
        mov  edi, edx
        and  edi, 0fh
        shr  edx, 4
        mov  esi, edx
        mov  es, si
        ; write length of buffer (zero if error) into "addr"
        mov  dword ptr es:[di], ecx

        ; set return value to continuation (return in dx:ax)
        mov  ax, bx
        shr  ebx, 16
        mov  dx, bx

        pop  es
        pop  si
        pop  di
        pop  cx
        pop  bx
        pop  bp

        ret

get_mmap_entry endp

;/*
; * get_rom_config_table()
; *
; * Get the linear address of a ROM configuration table. Return zero,
; * if fails.
; */
;
;unsigned long __cdecl get_rom_config_table (void);
get_rom_config_table proc near

        push bp
        mov  bp, sp

        push bx

        ; zero ebx for simplicity
        xor  ebx, ebx

        mov  ax, 00c0h
        int  15h

        jc   no_rom_table
        test ah, ah
        jnz  no_rom_table

        mov  dx, es
        jmp  found_rom_table

no_rom_table:
        xor  dx, dx
        xor  bx, bx

found_rom_table:
        ; compute the linear address
        mov  ax, dx
        shl  eax, 4
        add  eax, ebx

        ; return address in dx:ax
        mov  edx, eax
        shr  edx, 16

        pop  bx
        pop  bp

        ret

get_rom_config_table endp

;/* get_code_end() :  return the address of the end of the code
; * This is here so that it can be replaced by asmstub.c.
; */
;
;long __cdecl get_code_end (void);
get_code_end proc near

        ; will be the end of the bss
        xor  eax, eax
        xor  edx, edx
        lea  ax, DGROUP:_freeldr_stack_top
        mov  dx, current_seg
        shl  edx, 4
        add  eax, edx

        ; Round up to the next word.
        shr  eax, 2
        inc  eax
        shl  eax, 2

        ; return in dx:ax
        mov  edx, eax
        shr  edx, 16

        ;push ax
        ;push cx
        ;
        ; print dx:ax
        ;push dx
        ;push ax
        ;lea  cx, code_end_msg
        ;push cx
        ;call printk
        ;add  sp, 6
        ;
        ;pop  cx
        ;pop  ax

        ret

get_code_end endp


;long __cdecl get_diskinfo (long drive, struct geometry *geometry);
get_diskinfo_standard proc near



        ret

get_diskinfo_standard endp


check_int13_extensions proc near



        ret

check_int13_extensions endp


biosdisk_standard proc near



        ret

biosdisk_standard endp


biosdisk_int13_extensions proc near



        ret

biosdisk_int13_extensions endp

;mmap_msg      db 'desc_addr = %08lx, cont = %ld',0
;code_end_msg  db 'dx:ax = %08lx',0

_TEXT ends

      end

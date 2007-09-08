;
; vbe.asm
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

.386

name vbe

public get_vbe_controller_info
public get_vbe_mode_info
public set_vbe_mode
public reset_vbe_mode
public get_vbe_pmif

_TEXT segment word public 'CODE' use16


;
; //int get_vbe_controller_info (struct vbe_controller *controller_ptr)
; long get_vbe_controller_info (unsigned long controller_ptr)
;
; Get VBE controller information.
;

get_vbe_controller_info proc near
        push    bp
        mov     bp, sp

        push    esi
        push    edi
        push    ebx

        ; Convert the linear address to segment:offset
        mov     eax, dword ptr [bp + 4]
        mov     edi, eax
        and     edi, 0fh
        shr     eax, 4
        mov     ebx, eax

        mov     es, bx
        mov     ax, 4F00h
        int     10h

        ;mov     bx,  ax
        ;movzx   eax, bx
        xor     dx, dx

        pop     ebx
        pop     edi
        pop     esi

        pop     bp
        ret
get_vbe_controller_info endp


;
; //int get_vbe_mode_info (int mode_number, struct vbe_mode *mode_ptr)
; long get_vbe_mode_info (long mode_number, unsigned long mode_ptr)
;
; Get VBE mode information.
;

get_vbe_mode_info proc near
        push    bp
        mov     bp, sp

        push    esi
        push    edi
        push    ebx

        ; Convert the linear address to segment:offset
        mov     eax, dword ptr [bp + 8]
        mov     edi, eax
        and     edi, 0fh
        shr     eax, 4
        mov     ebx, eax

        ; Save the mode number in cx
        mov     ecx, dword ptr [ebp + 4]

        mov     es, bx
        mov     ax, 4F01h
        int     10h

        ;mov     bx, ax
        ;movzx   eax, bx
        xor     dx, dx

        pop     ebx
        pop     edi
        pop     esi

        pop     bp
        ret
get_vbe_mode_info endp


;
; //int set_vbe_mode (int mode_number)
; long set_vbe_mode (long mode_number)
;
; Set VBE mode. Don't support user-specified CRTC information.
;

set_vbe_mode proc near
        push    bp
        mov     bp, sp

        push    ebx
        push    esi
        push    edi

        ; Save the mode number in bx
        mov     ebx, dword ptr [ebp + 4]
        ; Clear bit D11
        and     ebx, 0F7FFh

        mov     ax, 4F02h
        int     10h

        ;mov     bx, ax
        ;movzx   eax, bx
        xor     dx, dx

        pop     edi
        pop     esi
        pop     ebx

        pop     bp
        ret
set_vbe_mode endp


;
; void reset_vbe_mode (void)
;

reset_vbe_mode proc near
        push    bp
        mov     bp, sp

        push    ebx

        mov     ax, 3
        int     10h

        pop     ebx

        pop     bp
        ret
reset_vbe_mode endp

;
; //void get_vbe_pmif (unsigned long *segoff, unsigned long *len);
; void get_vbe_pmif (unsigned long far *segoff, unsigned long far *len);
;

get_vbe_pmif proc near
        push    bp
        mov     bp, sp

        push    ebx
        push    esi
        push    edi
        push    es

        push    bp

        mov     ax, 4F0Ah
        xor     bx, bx
        xor     di, di
        int     10h

        xor     ebx, ebx
        cmp     ax, 4Fh
        jnz     nopm

        mov     bx, es
        shl     ebx, 16
        mov     bx, di
nopm:
        pop     bp

        mov     eax, dword ptr [bp + 4]
        mov     edx, eax
        and     eax, 0ffffh
        mov     di, ax
        shr     edx, 16
        mov     es, dx
        mov     dword ptr es:[di], ebx

        mov     eax, dword ptr [bp + 8]
        mov     edx, eax
        and     eax, 0ffffh
        mov     di,  ax
        shr     edx, 16
        mov     es, dx
        and     ecx, 0FFFFh
        mov     dword ptr es:[di], ecx

        pop     es
        pop     edi
        pop     esi
        pop     ebx

        pop    bp
        ret
get_vbe_pmif endp

_TEXT ends

      end

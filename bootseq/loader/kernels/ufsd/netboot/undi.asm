;
; undi.asm:
; UNDI realmode call wrapper
;
;/*
; *  GRUB  --  GRand Unified Bootloader
; *  Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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

name undi

.386p

include fsd.inc

public  __undi_call

extrn   call_rm        :near

_TEXT16 segment dword public 'CODE' use16

__undi_call_rm:
       mov     es, cx            ; Seg:off addr of undi_call_info_t struct
       mov     bx, dx            ; into es:bx

       mov     ax,  es:[bx + 8]  ; Transfer contents of undi_call_info_t
       push    ax                ; structure to the real-mode stack
       mov     ax,  es:[bx + 6]
       push    ax
       mov     ax,  es:[bx + 4]
       push    ax

       call    dword ptr es:[bx] ; Do the UNDI call
       cld                       ; Don't know whether or not we need this
                                 ; but pxelinux includes it for some reason,
                                 ; so we put it in just in case.

       pop     cx                ; Tidy up the stack
       pop     cx
       pop     cx
       mov     cx, ax            ; Return ax via cx

       retf

_TEXT16 ends

_TEXT   segment dword public 'CODE' use32

__undi_call:
       ;push    ebp
       ;mov     ebp, esp

       push    esi
       push    edi
       push    ebx
       push    ecx

       ;mov     ecx, dword ptr [ebp + 8]     ; Seg:off addr of undi_call_info_t struct
       ;mov     edx, dword ptr [ebp + 12]    ; Pass to 16-bit code in %cx:%dx
       mov     ecx, eax
       and     ecx, 0ffffh
       and     edx, 0ffffh

       mov     eax, REL1_BASE
       shl     eax, 12
       mov     ax, offset _TEXT16:__undi_call_rm
       push    eax
       xor     eax, eax
       call    call_rm
       add     esp, 4

       xor     eax, eax                     ; ax is returned via cx
       mov     ax, cx

       pop     ecx
       pop     ebx
       pop     edi
       pop     esi

       ;pop     ebp

       ret

_TEXT   ends

        end

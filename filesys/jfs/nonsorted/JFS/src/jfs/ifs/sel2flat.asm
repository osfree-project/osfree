; $Id: sel2flat.asm,v 1.1.1.1 2003/05/21 13:38:42 pasha Exp $ 

SCCSID equ "@(#)1.7.1.2  12/2/99 11:13:41 src/jfs/ifs/sel2flat.asm, sysjfs, w45.fs32, fixbld"
;
;
;   Copyright (c) International Business Machines  Corp., 2000
;
;   This program is free software;  you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or 
;   (at your option) any later version.
; 
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY;  without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
;   the GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program;  if not, write to the Free Software 
;   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
;
;; Change History :
;;
;; 08/30/99 Vamsi D230115 Sel2Flat conversion incorrect for packed LDT selectors
;; 09/14/99 DJK   D230860 Check GDT selector against limit

include basemaca.inc
include devhlp.inc

frame   struc
gdtr    df      ?
fill1   dw      ?
sav_ebx dd      ?
sav_ecx dd      ?
sav_edx dd      ?
linaddr dd      ?
frame   ends

gdtra   equ     gdtr+2


gdtstrc struc
limit   dw      ?
base    dw      ?
base2   db      ?
g1      db      ?
l2      db      ?
base3   db      ?
gdtstrc ends


framein struc
retaddr dd      ?       ; from call
ebase   dd      ?       ; from save of base
argi1   dd      ?
argi3   dd      ?
framein ends


CpuMode 386
assume cs:flat


CODE32 SEGMENT DWORD PUBLIC USE32 'CODE'

extrn   KernelSS:word
extrn   TKSSBase:dword
extrn   KernThunkStackTo16:near
extrn   KernThunkStackTo32:near

        ASSUME DS:FLAT,ES:FLAT,SS:NOTHING
        align 4

Procedure _DosSelToFlat,near
;
;;      let's assert that we're not called at ring 3 because
;;      trashing cx breaks us in ring 0.
;
; grab new stack frame for workarea
;
        push    ebp
        mov     ebp,esp
        sub     esp,size frame
;
; save caller ebx, edx in new frame
;
        mov     [esp].sav_ebx,ebx
        mov     [esp].sav_ecx,ecx
        mov     [esp].sav_edx,edx
;
; if called with an LDT address, we handle things differently
;
        test eax, 00040000h
        jz gdt_address
;
;       Just shift the bits around
;
; 230115 this simple calculation does not work for packed code selectors.
;
;        mov     ebx, eax
;        and     ebx, 0FFF80000h
;        shr     ebx, 3
;        movzx   eax, ax
;        or      eax, ebx
;        jmp     done
;
;
; 230115 **Begin**
;
; We will have to get the base address of the selector from the descriptor
; of the selector from LDT.
;
        mov     [esp].linaddr,eax
;
        sldt    bx
        and     ebx, 0FFF8h

        sgdt    [esp].gdtr
        mov     edx,[esp].gdtra
;
; index to proper GDT entry to get to LDT selector
;
        add     edx,ebx         ; edx -> proper selector entry for LDT
;
        xor     eax,eax
        mov     ax,[edx].base
        mov     cl,[edx].base3
        shl     ecx,8
        mov     cl,[edx].base2
        shl     ecx,16
        or      eax,ecx
;
        mov     edx,eax        ; edx ->base addr of LDT
;
; index to proper LDT entry based on input selector
;
        mov     eax,[esp].linaddr
        mov     ebx,eax
        shr     eax,16
        and     ax, 0FFF8h      ; now eax and ebx contain selector & offset
        movzx   ebx,bx          ; respectively
;
        add     edx,eax         ; LDT entry in edx
;
        xor     eax,eax
        mov     ax,[edx].base
        mov     cl,[edx].base3
        shl     ecx,8
        mov     cl,[edx].base2
        shl     ecx,16
        or      eax,ecx

        add     eax,ebx

        jmp     done
;
; 230115 **end**

gdt_address     label   near

        mov     ebx,eax
        shr     eax,16
        and     ax, 0FFF8h      ; now eax and ebx contain selector & offset
        movzx   ebx,bx          ; respectively

        cmp     ax, KernelSS
        jne     NotStack

; for addresses on the kernel stack, we add TKSSBase, we cannot rely on GDT

        mov     eax, TKSSBase
        mov     eax, dword ptr [eax]
        test    eax,eax
        jnz     useTKSSBase     ; TKSSBase not zero - use it

;; The easiest way to get the value of TKSSBase is to thunk back to the
;; 16-bit stack, grab it, and thunk back
        call    KernThunkStackTo16
        mov     eax, TKSSBase
        mov     ecx, dword ptr [eax]
        call    KernThunkStackTo32
        mov     eax, ecx

useTKSSBase     label   near

        add     eax, ebx
        jmp     done

NotStack        label   near
;
; store GDT base in work field
;
        sgdt    [esp].gdtr

;; BEGIN D230860

; Check gdt limit
	cmp	word ptr [esp].gdtr, ax
	ja	okay

	xor	eax, eax
	jmp	done

okay	label	near

;; END D230860

        mov     edx,[esp].gdtra
;
; index to proper GDT entry based on input selector
;
        add     edx,eax         ; edx -> proper selector entry
;
        xor     eax,eax
        mov     ax,[edx].base
        mov     cl,[edx].base3
        shl     ecx,8
        mov     cl,[edx].base2
        shl     ecx,16
        or      eax,ecx
;
        add     eax,ebx
;
done    label   near
        mov     ebx,[esp].sav_ebx
        mov     ecx,[esp].sav_ecx
        mov     edx,[esp].sav_edx
;
; return to caller here
;
        mov     esp,ebp
        pop     ebp
        ret
;
EndProc _DosSelToFlat

CODE32 ENDS
        end

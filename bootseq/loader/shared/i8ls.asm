;*****************************************************************************
;*
;*                            Open Watcom Project
;*
;*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
;*
;*  ========================================================================
;*
;*    This file contains Original Code and/or Modifications of Original
;*    Code as defined in and that are subject to the Sybase Open Watcom
;*    Public License version 1.0 (the 'License'). You may not use this file
;*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
;*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
;*    provided with the Original Code and Modifications, and is also
;*    available at www.sybase.com/developer/opensource.
;*
;*    The Original Code and all software distributed under the License are
;*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
;*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
;*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
;*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
;*    NON-INFRINGEMENT. Please see the License for the specific language
;*    governing rights and limitations under the License.
;*
;*  ========================================================================
;*
;* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
;*               DESCRIBE IT HERE!
;*
;*****************************************************************************
include struct.inc

.386p
;========================================================================
;==     Name:           I8S                                            ==
;==     Operation:      integer eight byte shift                       ==
;==     Inputs:         EDX;EAX   integer M1                           ==
;==                     ECX;EBX   integer M2  (ECX contents ignored)   ==
;==     Outputs:        EDX;EAX   product                              ==
;==     Volatile:       ECX, EBX destroyed                             ==
;========================================================================
; Special Note:
; If ECX ever takes on a meaningful use (i.e., is no longer ignored)
; then several routines that call these shift routines will require fixing.
; At present, the following routines ignore what is in ECX.


name        i8s

_TEXT   segment dword public 'CODE'  use32

public __U8RS

__U8RS:
        assume cs:_TEXT

        mov     ecx,ebx         ; get shift-count into cl
        and     cl,03fH         ; get mod 64 shift count
        test    cl,020H         ; see if count >= 32
        jnz     L1
        shrd    eax,edx,cl
        shr     edx,cl
        ret                     ; and return!!!

L1:
        mov     eax,edx
        sub     ecx,020H        ; knock off 32-bits of shifting
        xor     edx,edx         ; zero extend result
        shr     eax,cl

        ret

public __I8RS

__I8RS:
        assume cs:_TEXT

        mov     ecx,ebx         ; get shift-count into cl
        and     cl,03fH         ; get mod 64 shift count
        test    cl,020H         ; see if count >= 32
        jnz     L2
        shrd    eax,edx,cl
        sar     edx,cl

        ret                     ; and return!!!

L2:
        mov     eax,edx         ; shift hi into lo (1st 32 bits now shifted)
        sub     cl,020H         ; knock off 32-bits of shifting
        sar     edx,31          ; sign extend hi-word
        sar     eax,cl          ; shift remaining part

        ret

public __I8LS
public __U8LS

__I8LS:
__U8LS:
        assume cs:_TEXT

        mov     ecx,ebx         ; get shift-count into cl
        and     cl,03fH         ; get mod 64 shift count
        test    cl,020H         ; see if count >= 32
        jnz     L3
        shld    edx,eax,cl
        shl     eax,cl

        ret                     ; and return!!!

L3:
        mov     edx,eax         ; shift lo into hi (1st 32 bits now shifted)
        sub     cl,020H         ; knock off 32-bits of shifting
        xor     eax,eax         ; lo 32 bits are now zero
        shl     edx,cl          ; shift remaining part

        ret                     ; and return!!!

_TEXT   ends

        end

;/*
; * $Header$
; */

;/************************************************************************/
;/*                       Linux partition filter.                        */
;/*          (C) Copyright Deon van der Westhuysen, July 1995.           */
;/*                                                                      */
;/*  Dedicated to Jesus Christ, my Lord and Saviour.                     */
;/*                                                                      */
;/* This program is free software; you can redistribute it and/or modify */
;/* it under the terms of the GNU General Public License as published by */
;/* the Free Software Foundation; either version 2, or (at your option)  */
;/* any later version.                                                   */
;/*                                                                      */
;/* This program is distributed in the hope that it will be useful,      */
;/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
;/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
;/* GNU General Public License for more details.                         */
;/*                                                                      */
;/* You should have received a copy of the GNU General Public License    */
;/* along with this program; if not, write to the Free Software          */
;/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            */
;/*                                                                      */
;/*  This code is still under development; expect some rough edges.      */
;/*                                                                      */
;/************************************************************************/

        Title        E2Wrap- Assembly wrapper around interface functions.

ifndef __MASM__
locals                                ; I really don't know the MASM equivalent ! (M.Willm 1995-11-14)
endif

.386P

include devhlp.inc

STRING_MESSAGE=        1178                ; Message ID for Save_Message
ADD_CLASS= 1                        ; Device class for adapter device drivers

ifdef __MASM__
DGROUP        group _DATA,_BSS
else
DGROUP        group _DATA,CONST,_BSS        ; MS Visual C++ defines a CONST segment (M.Willm 1995-11-14)
endif

_DATA        segment word public use16 'DATA'

         extrn _DevHelp: dword

MsgStructure        dw STRING_MESSAGE        ;String replacement message
                dw 1                    ;Only one item.
MsgMessage        dd ?                    ;Address of string to display.

_DATA        ends

ifdef __MASM__                        ; MS Visual C++ defines a CONST segment (M.Willm 1995-11-14)
CONST         segment word public use16 'CONST'
CONST        ends
endif

_BSS        segment word public use16 'BSS' 
_BSS        ends

_TEXT        segment byte public use16 'CODE'
        assume cs:_TEXT, ds:DGROUP

        extrn _E2FilterIORB:near
        extrn _FilterNotify:near
        extrn _PartNotify:near

;
; memset and memcpy are builtin functions in MS Visual C++ 1.5 if -O2 and -G3
; compiler options are set. (M.Willm 1995-11-14)
;
ifndef __MASM__
        public _memset
_memset        proc        near
; void memset (void far* Buffer, UCHAR Value, USHORT Count);
        push bp
        mov bp,sp
        push di
        les di,[bp+4]
        cld
        mov al,[bp+8]
        mov cx,[bp+10]
        rep stosb
        pop di
        pop bp
        ret
_memset        endp

        public _memcpy
_memcpy        proc near
; void memcpy (void far* Dest, void far* Source, USHORT Size);
        push bp
        mov bp,sp
        push ds
        push si
        push di
        lds si,[bp+8]
        les di,[bp+4]
        mov cx,[bp+12]
        cld
        rep movsb
        pop di
        pop si
        pop ds
        pop bp
        ret
_memcpy        endp
endif

        public _VirtToPhys
_VirtToPhys        proc
; void VirtToPhys (void FAR *pDataSeg,ULONG FAR *ppDataSeg);
        push bp
        mov bp,sp
        push ds
        push si
        lds si,[bp+4]
        mov dl,DevHlp_VirtToPhys
        call [_DevHelp]
;
; Changed @@NoError into NoError_VirtToPhys for MASM (M.Willm 1995-11-14)
;
IFNDEF __MASM__
        jnc short @@NoError
        mov ax,0FFFFh
        mov bx,ax
@@NoError:
ELSE
        jnc short NoError_VirtToPhys
        mov ax,0FFFFh
        mov bx,ax
NoError_VirtToPhys:
ENDIF
        lds si,[bp+8]
        mov ds:[si],bx
        mov ds:[si+2],ax
        pop si
        pop ds
        pop bp
        ret
_VirtToPhys        endp

        public _PhysToVirt
_PhysToVirt        proc near
; void far * PhysToVirt(ULONG ppXferBuf, USHORT XferLength);
        push bp
        mov bp,sp
        push es
        push di
        mov ax,[bp+6]
        mov bx,[bp+4]
        mov cx,[bp+8]
        mov dh,1
        mov dl,DevHlp_PhysToVirt
        call [_DevHelp]
        mov dx,es
        mov ax,di
;
; Changed @@NoError into NoError_PhysToVirt for MASM (M.Willm 1995-11-14)
;
IFNDEF __MASM
        jnc short @@NoError
        xor ax,ax
        mov dx,ax
@@NoError:
ELSE
        jnc short NoError_PhysToVirt
        xor ax,ax
        mov dx,ax
NoError_PhysToVirt:
ENDIF
        pop di
        pop es
        pop bp
        ret
_PhysToVirt        endp



        public _E2FilterIORBWrapper
_E2FilterIORBWrapper        proc far
; VOID FAR E2FilterIORBWrapper (PIORB pIORB);
        push bp
        mov bp,sp
        push ds
        push es
        push si
        push di
        mov ax,seg DGROUP
        mov ds,ax
        push dword ptr [bp+6]
        call _E2FilterIORB
        add sp,4
        pop di
        pop si
        pop es
        pop ds
        pop bp
        retf
_E2FilterIORBWrapper        endp;

        public _FilterNotifyWrapper
_FilterNotifyWrapper        proc
; PIORB FAR FilterNotifyWrapper (PIORB pIORB);
        push bp
        mov bp,sp
        push ds
        push es
        push si
        push di
        mov ax,seg DGROUP
        mov ds,ax
        push dword ptr [bp+6]
        call _FilterNotify
        add sp,4
        pop di
        pop si
        pop es
        pop ds                                ; Never mind return value...
        pop bp
        retf
_FilterNotifyWrapper        endp

        public _PartNotifyWrapper
_PartNotifyWrapper        proc
; PIORB FAR PartNotifyWrapper (PIORB pIORB);
        push bp
        mov bp,sp
        push ds
        push es
        push si
        push di
        mov ax,seg DGROUP
        mov ds,ax
        push dword ptr [bp+6]
        call _PartNotify
        add sp,4
        pop di
        pop si
        pop es
        pop ds                                ; Never mind return value...
        pop bp
        retf
_PartNotifyWrapper        endp



        public _ProcRun
_ProcRun        proc far
; int far ProcRun (PIORB pIORB)
        push bp
        mov bp,sp
        push ds
        mov ax,seg DGROUP
        mov ds,ax
        push es
        push si
        push di
        mov ax,[bp+8]
        mov bx,[bp+6]
        mov dl,DevHlp_ProcRun
        call [_DevHelp]
        pop di
        pop si
        pop es
        pop ds
        pop bp
        retf
_ProcRun        endp

        public _ProcBlock
_ProcBlock        proc near
; int ProcBlock(PIORB pIORB)
        push bp
        mov bp,sp
        push di
        mov ax,[bp+6]
        mov bx,[bp+4]
        mov cx,0FFFFh                ;Set timeout to -1: no timeout.
        mov di,cx
        mov dh,1                ;Non interruptable
        mov dl,DevHlp_ProcBlock
        call [_DevHelp]
        pop di
        pop bp
        ret
_ProcBlock         endp



        public _E2GetADDTable
_E2GetADDTable        proc near
; PDevClassTableStruc _E2GetADDTable (void)
        mov al,DHGETDOSV_DEVICECLASSTABLE
        mov cx,ADD_CLASS
        mov dl,DevHlp_GetDOSVar
        call [_DevHelp]
        mov dx,ax
        mov ax,bx
        ret
_E2GetADDTable        endp

        public _RegisterADD
_RegisterADD        proc
; USHORT RegisterADD (PADDEntryPoint pAddEP, PSZ DriverName);
        push bp
        mov bp, sp
        push ds
        push si
        push di
        lds si,[bp+8]
        mov bx,[bp+4]
        mov ax,[bp+6]
        mov di,0
        mov cx,1
        mov dl,DevHlp_RegisterDeviceClass
        call [_DevHelp]
;
; Changed @@NoError into NoError_RegisterADD for MASM (M.Willm 1995-11-14)
;
IFNDEF __MASM__
        jnc short @@NoError
        xor ax,ax
@@NoError:
ELSE
        jnc short NoError_RegisterADD
        xor ax,ax
NoError_RegisterADD:
ENDIF
        pop di
        pop si
        pop ds
        pop bp
        ret
_RegisterADD        endp

        public _E2Print
_E2Print        proc near
; void _E2Print (PSZ Text)
        push bp
        mov bp,sp
        push si
        mov eax,[bp+4]
        mov MsgMessage,eax
        mov si,offset MsgStructure
        mov dl,DevHlp_Save_Message
        call [_DevHelp]
        pop si
        pop bp
        ret
_E2Print        endp

_TEXT        ends

        end

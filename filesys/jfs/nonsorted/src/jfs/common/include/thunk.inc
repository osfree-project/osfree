; $Id: thunk.inc,v 1.1 2000/04/21 10:58:00 ktk Exp $ 

;       SCCSID = @(#)1.16  7/30/98 14:22:46 src/jfs/ifs/thunk.inc, sysjfs, w45.fs32, 990417.1
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
;;---------------------------------------------------------------------------

_JFS_UNICODE    equ 1
INCL_DEF    equ 1       ; include typedef macros

    include mi.inc
    include basemaca.inc
    include seldesc.inc
    include devhlp.inc

    CPUMode 386

;;---------------------------------------------------------------------------
;;
;; change _TEXT and _DATA to be what ICC uses
;;
;;---------------------------------------------------------------------------
DefSeg DATA16,DATA16,DATA,WORD,PUBLIC,USE16,'DATA16'
DefSeg CODE16,CODE16,CODE,WORD,PUBLIC,USE16,'CODE'
DefSeg CONST32_RO,CONST32_RO,DATA,DWORD,PUBLIC,USE32,'CONST'
DefSeg CODE32,CODE32,CODE,DWORD,PUBLIC,USE32,'CODE'
DefSeg DATA32,DATA32,DATA,DWORD,PUBLIC,USE32,'DATA'
DefSeg _DATA,_DATA,DATA,WORD,PUBLIC,USE16,'DATA'

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
MAXPATHLEN  EQU 259
UDC_StrToUcs    EQU 1
UDC_StrFromUcs  EQU 2

STDCALL     EQU 1 ; controls who cleans up arguments after CCALL

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
;; DOS32FLATDS  EQU <seg FLAT:DATA32>
?DefExtrn   DOS32FLATCS,ABS
?DefExtrn   DOS32FLATDS,ABS

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
astubCS         macro
        ASSUME  CS:CODE16
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
astubDS     macro
        ASSUME  DS:DATA16
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
astubES     macro
        ASSUME  ES:DATA16
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
astubContext    macro
        astubDS
        astubES
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
stubContext macro
        mov ax,seg DATA16
        mov ds,ax
        mov es,ax
        astubContext
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
stubDS  macro
    mov ax,seg DATA16
    mov ds,ax
    astubDS
    endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
stubES  macro
    mov ax,seg DATA16
    mov es,ax
    astubDS
    endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
aflatCS     macro
        ASSUME  CS:FLAT
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
aflatDS     macro
        ASSUME  DS:FLAT
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
aflatES     macro
        ASSUME  ES:FLAT
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
aflatContext    macro
        aflatDS
        aflatES
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
flatContext macro   init
        mov     ax,DOS32FLATDS
        ifnb    <init>
         or ax,RPL_RING3
        endif
        mov ds,ax
        mov es,ax
        aflatContext
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
flatDS      macro   init
        mov     ax,DOS32FLATDS
        ifnb    <init>
         or ax,RPL_RING3
        endif
        mov ds,ax
        aflatDS
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
flatES      macro   init
        mov     ax,DOS32FLATDS
        ifnb    <init>
         or ax,RPL_RING3
        endif
        mov es,ax
        aflatES
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
extrn32     macro   name,x
        CODE32  SEGMENT
        extrn   &name:&x
        CODE32  ENDS
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
extrn16     macro   name,x
        CODE16  SEGMENT
        extrn   &name:&x
        CODE16  ENDS
        endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
CCALL32     macro   target,args,modifiers
local       label16,label32
        DefCode CODE32,CODE32,,USE32
        aflatCS
        label32 label   near
        CCall   <target>,<args>,<modifiers>
        jmp far ptr CODE16:label16
        EndCode CODE32
        astubCS
        jmp     far ptr FLAT:label32
        label16 label far
        endm
;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
PCALL16 macro   target,args
local   l16,l32
    DefCode CODE16,CODE16
    astubCS
    l16 label far
    PCall   <target>,<args>
    jmp far ptr FLAT:l32
    EndCode CODE16

    aflatCS
    jmp far ptr CODE16:l16
    l32 label near
    movzx   eax,ax
    endm
;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
CCALL16 macro   target,args
local   l16,l32
    DefCode CODE16,CODE16
    astubCS
    l16 label far
    CCall   <target>,<args>
    jmp far ptr FLAT:l32
    EndCode CODE16

    aflatCS
    jmp far ptr CODE16:l16
    l32 label near
    movzx   eax,ax
    endm

;;---------------------------------------------------------------------------
;;
;;  Thunk - Calls _DosSelToFlat to thunk argument, leaves result in eax
;;
;;  Must be in 32-bit code segment
;;
;;---------------------------------------------------------------------------
Thunk   macro   pointer16
local   nullArg
    mov eax, pointer16
    test    eax, eax
    jz  nullArg
    CCall   _DosSelToFlat
nullArg label   near
    endm

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
DEVHELP16   macro
local       l16,l32
        DefCode CODE16,CODE16
        astubCS
        l16 label far
        call    dword ptr dhrouter
        jmp far ptr FLAT:l32
        EndCode CODE16

        aflatCS
        jmp far ptr CODE16:l16
        l32 label near
        endm

extrn32     KernThunkStackTo16,near
extrn32     KernThunkStackTo32,near
ifndef  THUNK_ASM
 extrn32    gdtBase,dword
endif

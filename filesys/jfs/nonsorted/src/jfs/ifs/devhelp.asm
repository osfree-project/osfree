; $Id: devhelp.asm,v 1.1 2000/04/21 10:58:02 ktk Exp $ 

SCCSID equ "@(#)1.10  7/30/98 14:09:17 src/jfs/ifs/devhelp.asm, sysjfs, w45.fs32, 990417.1"
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
	include thunk.inc

extrn32	dhrouter,dword

;;------------------------------------------------------------------

DefCode EXPORT,CODE32

aflatCS
aflatContext

;;------------------------------------------------------------------
;;	The following are called by functions running on the 16-bit kernel
;;	stack.
;;------------------------------------------------------------------
Procedure	_GetDOSVar@12

ArgVar	VarNumber,DWORD
ArgVar	VarMember,DWORD
ArgVar	KernelVar,DWORD

EnterProc
SaveReg <ebx,esi,edi,ds,es>

mov	al,byte ptr VarNumber
mov	cx,word ptr VarMember
mov	dl,DevHlp_GetDOSVar

DEVHELP16

jc	gdvFail

mov	ecx, KernelVar
mov	word ptr [ecx+2], ax
mov	word ptr [ecx], bx
xor	eax,eax

gdvFail	label	near

RestoreReg <es,ds,edi,esi,ebx>
LeaveProc
RETC
EndProc _GetDOSVar@12

EndCode

	END

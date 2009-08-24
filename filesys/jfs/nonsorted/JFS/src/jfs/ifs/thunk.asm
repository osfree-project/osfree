; $Id: thunk.asm,v 1.2 2003/09/21 08:43:04 pasha Exp $ 

SCCSID equ "@(#)1.26  12/8/98 13:54:59 src/jfs/ifs/thunk.asm, sysjfs, w45.fs32, 990417.1"
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
;;
; Change History :
; PS 2002 insert code for boot from JFS
;

THUNK_ASM       equ     1
        include thunk.inc

;;
;; KSO Apr 20 2000 3:47am
;; To be able to use the old DDK basemaca.inc we'll have to make our own
;; version of ArgVar. The one use internally at IBM has an extra argument.
;;
;; Hope IBM will forgive us when quoting these two macros here.
;;

;*** newArgVar - declare an argument (procedure parameter)
;
;   Assign "name" to an argument (pushed by caller) in the stack frame.
;
;   ENTRY   name     = name of argument
;       length   = "BYTE"
;            = "DBYTE" - double byte. Two new names
;            "name&l" and "name&h" are created to refer
;            to the low and high bytes.
;            = "WORD"
;            = "DWORD" - double word. Two new names
;            "name&l" and "name&h" are created to refer
;            to the low and high words.
;            = a number - declare an argument of this
;            length
;       lowname  = if non-blank this is used instead of
;              "name&l" when length is DBYTE.
;       highname = if non-blank this is used instead of
;              "name&h" when length is DBYTE.
;       nowarn   = "hybrid" to stop newArgVar from generating a warning
;              message about using newArgVar in a hybrid procedure.
;              See example at top.
;
;   (global variables)
;       ?aframe  = number of bytes declared as arguments so
;              far. Set to 0 by Procedure.
;       ?abase   = distance between BP and last byte of
;              argument pushed. Usually 4 for near
;              procedure and 6 for far.
;       ?argfar  = 0 if ArgVars are at fixed offset from BP.
;              This is the case for near, far, faronly and far16.
;            = 4 if ArgVars are at different offset from BP
;              depending on whether the procedure is called near
;              or called far.  This is the case for near
;              + faronly procedures (hybrid).
;            = 8 if ArgVars are at different offset from BP
;              depending on whether the procedure is called near
;              or called 32 bit far.  This is the case for near
;              + far16 procedures.
;            = -1 if multiple thunks exist; ArgVars are broken.
;
;   EXIT    name EQU to the appropriate location in the stack
;       frame (uses BP).
;
;       (global variables)
;        ?aframe = incremented by number of bytes specified
;             in "length".
;
;   NOTES   In 16 bit code segments argvars are word aligned and in
;       32 bit code segments argvars are dword aligned.
;
;   SEE ALSO: Procedure, EnterProc, LeaveProc, EndProc.

newArgVar  macro   name,length,lowname,highname,nowarn,bpsize
    if2
    if ?argfar eq -1
        ProcError <newArgVar cannot be used with multiple thunks>
        .err
    endif
    ifidni <nowarn>,<hybrid>
        ife ?argfar
        ProcError <newArgVar <nowarn> parameter invalid>
        .err
        endif
    else
        if ?argfar
        ProcError <references to newArgVar name may fail when called far>
        .err
        endif
    endif
    if (?distance and ?PD_ENTERED)
        ProcError <newArgVar name invoked inside EnterProc>
        .err
    endif
    endif
    if ?model EQ ?PASCAL
    ?nfields = ?nfields + 1
    ?MArg newArgVar,<name,length,lowname,highname,nowarn>,%?nfields
    else
    if (?distance and ?PD_ESPFRAME)
        ?bp equ <ESP>
    else
        ifnb <bpsize>
           ifidni <bpsize>,<32>
           ?bp equ <EBP>
           else
           ?bp equ <BP>
           endif
        else
           if (?cstype eq ?CS_32bit)
           ?bp equ <EBP>
           else
           ?bp equ <BP>
           endif
        endif
    endif
    ?bm1 = ?aframe + ?abase
    ifidni <length>,<byte>
        ?aframe = ?aframe + 1
        ?BPEqu name,byte,%?bp,+,%?bm1
    elseifidni <length>,<dbyte>
        ?aframe = ?aframe + 2
        ?BPEqu name,word,%?bp,+,%?bm1
        ifb <lowname>
        ?BPEqu name&l,byte,%?bp,+,%?bm1
        else
        ?BPEqu lowname,byte,%?bp,+,%?bm1
        endif
        ifb <highname>
        ?BPEqu name&h,byte,%?bp,+,%(?bm1+1)
        else
        ?BPEqu highname,byte,%?bp,+,%(?bm1+1)
        endif
    elseifidni <length>,<word>
        ?aframe = ?aframe + 2
        ?BPEqu name,word,%?bp,+,%?bm1
    elseifidni <length>,<dword>
        ?aframe = ?aframe + 4
        ?BPEqu  name,dword,%?bp,+,%?bm1
        ?BPEqu  name&l,word,%?bp,+,%?bm1
        ?BPEqu  name&h,word,%?bp,+,%(?bm1+2)
    elseifidni <length>,<fword>
        ?aframe = ?aframe + 8
        ?BPEqu  name,fword,%?bp,+,%?bm1
        ?BPEqu  name&l,dword,%?bp,+,%?bm1
        ?BPEqu  name&h,dword,%?bp,+,%(?bm1+4)
    elseifdef ?&length
        ?aframe = ?aframe + ?&length
        if ?&length EQ 1
          ?BPEqu  name,byte,%?bp,+,%?bm1
        elseif ?&length EQ 2
          ?BPEqu  name,word,%?bp,+,%?bm1
        elseif ?&length EQ 4
          ?BPEqu  name,dword,%?bp,+,%?bm1
        else
          ?BPEqu  name,,%?bp,+,%?bm1
        endif
    else
        ?aframe = ?aframe + length
        ?BPEqu  name,byte,%?bp,+,%?bm1
    endif
    if (?cstype eq ?CS_32bit)       ;; align arguments
        if ?aframe and 03h
        ?aframe = (?aframe and (not 03h)) + 04h
        endif
    else
        if ?aframe and 01h
        ?aframe = ?aframe + 1
        endif
    endif
    endif
endm

;*** newLocalVar - declare a local variable
;
;   Reserves space on the stack frame for a local variable.
;
;   ENTRY   name     = name of local variable
;       length   = "BP" - special case, no space is allocated,
;            "name" labels the location where the old
;            BP is stored.
;            = "BYTE" - reserves a byte
;            = "DBYTE" - reserves two bytes. Two additional
;            names "name&l" and "name&h" are created to
;            refer to the low and high byte.
;            = "WORD" - reserves a word
;            = "DWORD" - reserves a double word (4 bytes).
;            Two additional names "name&l" and "name&h"
;            are created to refer to the low and high
;            word. This is always padded and not
;            affected by "pad".
;            = a number - reserves this many bytes.
;       lowname  = if non-blank and "length" is "DBYTE", then
;              this is used instead of "name&l".
;       highname = if non-blank and "length" is "DBYTE", then
;              this is used instead of "name&h".
;       pad      = "PAD" - make WORD, DWORD, and numeric length
;            variables start on even address.
;            = "NOPAD" - don't pad WORD variables, but pad
;            DWORD variables.
;            = anything else - pad WORD and DWORD
;            variables. Don't pad numeric length
;            variables.
;   (global variables)
;       ?frame = number of bytes reserved as local var
;            (including padding) so far; set to 0 by
;            Procedure.
;
;   EXIT    "name" EQU to a reserved space in the stack frame (uses
;       BP register). No executable code is generated.
;   (global variables)
;       ?pad   = don't care; should really be a local symbol
;       ?nopad = don't care; should really be a local symbol
;       ?frame = incremented by the number of bytes reserved
;            as local var.
;
;   SEE ALSO: Procedure, EnterProc, LeaveProc, EndProc.

newLocalVar macro  name,length,lowname,highname,pad,bpsize
ifdef ALIGNCODE
align 4
endif
    if2
    if (?distance and ?PD_ESPFRAME)
        ProcError <newLocalVar name invoked with ESP frame>
        .err
    endif
    if (?distance and ?PD_ENTERED)
        ProcError <newLocalVar name invoked inside EnterProc>
        .err
    endif
    endif
    ?pad = 0
    ?nopad = 0
    ifidni <pad>,<PAD>
    ?pad = 1
    endif
    ifidni <pad>,<NOPAD>
    ?nopad = 1
    endif
    ifnb <bpsize>
    ifidni <bpsize>,<32>
        ?bp equ <EBP>
    else
        ?bp equ <BP>
    endif
    else
    if (?cstype eq ?CS_32bit)
        ?bp equ <EBP>
    else
        ?bp equ <BP>
    endif
    endif
    ifidni <length>,<BP>    ;; makes a variable point to the old BP
    name EQU  (WORD PTR [BP])
    elseifidni <length>,<EBP>   ;; makes a variable point to the old EBP
    name EQU  (DWORD PTR [EBP])
    elseifidni <length>,<BYTE>
    ?frame =  ?frame + 1
    ?bm1 = ?frame
    ?BPEqu  name,byte,%?bp,-,%?bm1
    elseifidni <length>,<DBYTE>
    if ?frame and 1
        ?frame = ?frame + 1
    endif
    ?frame =  ?frame + 2
    ?bm1 = ?frame
    ?BPEqu  name,word,%?bp,-,%?bm1
    ifb <lowname>
        ?BPEqu  name&l,byte,%?bp,-,%?bm1
    else
        ?BPEqu  lowname,byte,%?bp,-,%?bm1
    endif
    ifb <highname>
        ?BPEqu  name&h,byte,%?bp,-,%(?bm1-1)
    else
       ?BPEqu  highname,byte,%?bp,-,%(?bm1-1)
    endif
    elseifidni <length>,<WORD>
    ife ?nopad
        if ?frame and 1
        ?frame = ?frame + 1
        endif
    endif
    ?frame =  ?frame + 2
    ?bm1 = ?frame
    ?BPEqu  name,word,%?bp,-,%?bm1
    elseifidni <length>,<DWORD>
    if ?frame and 1
        ?frame = ?frame + 1
    endif
    ?frame =  ?frame + 4
    ?bm1 = ?frame
    ?BPEqu  name,dword,%?bp,-,%?bm1
    ?BPEqu  name&l,word,%?bp,-,%?bm1
    ?BPEqu  name&h,word,%?bp,-,%(?bm1-2)
    elseifidni <length>,<FWORD>
    if ?frame and 1
        ?frame = ?frame + 1
    endif
    ?frame =  ?frame + 8
    ?bm1 = ?frame
    ?BPEqu  name,fword,%?bp,-,%?bm1
    ?BPEqu  name&l,dword,%?bp,-,%?bm1
    ?BPEqu  name&h,dword,%?bp,-,%(?bm1-4)
    elseifdef ?&length
    if ?frame and 1
        ?frame = ?frame + 1
    endif
    ?frame =  ?frame + ?&length
    ?bm1 = ?frame
    if ?&length EQ 1
        ?BPEqu  name,byte,%?bp,-,%?bm1
    elseif ?&length EQ 2
        ?BPEqu  name,word,%?bp,-,%?bm1
    elseif ?&length EQ 4
        ?BPEqu  name,dword,%?bp,-,%?bm1
    else
        ?BPEqu  name,,%?bp,-,%?bm1
    endif
    else
    ?frame =  ?frame + length
    if ?pad
        if ?frame and 1
        ?frame = ?frame + 1
        endif
    endif
    ?bm1 = ?frame
    ?BPEqu  name,byte,%?bp,-,%?bm1
    endif
endm




;;---------------------------------------------------------------------------
;;
;; depending on which C compiler is used an '_' may be needed
;; before the name of global variables
;;
;;---------------------------------------------------------------------------
extrn32         FS32_MOUNT,near
extrn32         FS32_ALLOCATEPAGESPACE,near
extrn32         FS32_CANCELLOCKREQUEST,near
extrn32         FS32_CANCELLOCKREQUESTL,near
extrn32         FS32_CHDIR,near
extrn32         FS32_CHGFILEPTRL,near
extrn32         FS32_CLOSE,near
extrn32         FS32_COMMIT,near
extrn32         FS32_DELETE,near
extrn32         FS32_DOPAGEIO,near
extrn32         FS32_EXIT,near
extrn32         FS32_FILEATTRIBUTE,near
extrn32         FS32_FILEINFO,near
extrn32         FS32_FILEIO,near
extrn32         FS32_FILELOCKS,near
extrn32         FS32_FILELOCKSL,near
extrn32         FS32_FINDCLOSE,near
extrn32         FS32_FINDFIRST,near
extrn32         FS32_FINDFROMNAME,near
extrn32         FS32_FINDNEXT,near
extrn32         FS32_FLUSHBUF,near
extrn32         FS32_FSCTL,near
extrn32         FS32_FSINFO,near
extrn32         FS32_MKDIR,near
extrn32         FS32_MOVE,near
extrn32         FS32_NEWSIZEL,near
extrn32         FS32_OPENCREATE,near
extrn32         FS32_OPENPAGEFILE,near
extrn32         FS32_PATHINFO,near
extrn32         FS32_READ,near
extrn32         FS32_RMDIR,near
extrn32         FS32_SETSWAP,near
extrn32         FS32_SHUTDOWN,near
extrn32         FS32_WRITE,near
extrn32         _jfs_init1@0,near
extrn32         _parse_args@4,near
extrn32         _DefineSelector@8,near
extrn32         DosQuerySysInfo,near
extrn32         _DosSelToFlat,near

extrn32         KernelSS,word
extrn32         MemorySize,dword
extrn32     enum_anchor,dword

extrn16         _autocheck, near

PATHBUFSIZE     EQU     (MAXPATHLEN*2+2)

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
FSA_LVL7        EQU     8
FSA_LOCK        EQU     4
FSA_LARGEFILE   EQU 20h

DefData EXPORT,DATA16

        PUBLIC  FS_ATTRIBUTE
FS_ATTRIBUTE    dd      FSA_LVL7 + FSA_LOCK + FSA_LARGEFILE

        PUBLIC  FS_NAME
FS_NAME         db      "JFS",0

                align   4

        PUBLIC  FS_MPSAFEFLAGS2
FS_MPSAFEFLAGS2 dd      41h, 0      ; 01h = don't get r0 spinlock
                    ; 40h = don't acquire subsys spinlock
                    ; FS_MPSAFEFLAGS2 is an array of 64 bits

EndData

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
DefData EXPORT,DATA32

        PUBLIC  gdtReg
        PUBLIC  gdtLimit
        PUBLIC  gdtBase
gdtReg          label   byte
gdtLimit        dw      ?
gdtBase         dd      ?

        PUBLIC  farlabel
farlabel        df      ?

                align 4

        PUBLIC  dhrouter
dhrouter        dd      ?

firstCall       dd      0
EndData

DefData EXPORT,CONST32_RO

        PUBLIC  FS32_ATTRIBUTE
FS32_ATTRIBUTE  dd      FSA_LVL7 + FSA_LOCK + FSA_LARGEFILE

EndData

;;---------------------------------------------------------------------------
;;
;; NOTE: all 16 bit parameters have to be moved into 32 bit registers
;;       for CCALL32
;; put parameters in reverse order for pascal
;;
;;---------------------------------------------------------------------------
DefCode EXPORT,CODE16

                astubCS

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
Procedure       fs_init16,<far,pascal>

newArgVar  pMiniFSD,DWORD,,,,32
newArgVar  devHlp,DWORD,,,,32
newArgVar  szParm,DWORD,,,,32

;;              int     3

                ;int     3

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext     init

;;      Store GDT
                db      MI_OPERANDSIZE
                sgdt    fword ptr gdtReg

;;      Save the address of DevHelp
                mov     eax, devHlp
                mov     dhrouter, eax

                ;int     3

;;      Get size of physical memory
                CCall32 DosQuerySysInfo <17,17,FLAToffset MemorySize,4> cdecl
                test    eax,eax
                jz      okay
                int     3
okay            label   near

;;      Parse arguments

                mov     eax, szParm
                test    eax, eax
                jz      noParams

                movzx   esi, ax
                shr     eax, 16
                mov     dl, DevHlp_VirtToLin
                call    dword ptr dhrouter
                jc      noParams                ;; maybe should fail?

                ;int     3

                CCALL32 _parse_args@4 <eax>

noParams        label   near

                push    ds
                ; set up the proper data segment to call into autocheck
                mov     ax, _DATA
                mov     ds, ax

                ;int     3 ;;;; vs

                CCALL   _autocheck

                pop     ds
ifdef FreeJFS
;; PS Boot Code
		  test	 pMiniFSD,0ffffffffh           ; Test for present mFSD
		  jz	 woMiniFSD
		  les   di,pMiniFSD                   ; Load address saving pointers
		  les	 di,es:[di]                    ; Load address to store parameters
		  cmp	 Word Ptr es:[di],1961h        ; Check for right miniFSD
		  jne	 woMiniFSD 
		  mov	 ax,offset CS:fs_mount16       ; Save pointer to mount procedure
		  stosw
		  mov	 ax,seg fs_mount16             ; Store it selector for Ring0
		  stosw
		  mov	 ax,offset CS:fs_opencreate16  ; Save selector to opencreate procedure
		  stosw
		  mov	 ax,seg fs_opencreate16        ; Store it selector for Ring0
		  stosw
woMiniFSD       label   near
;; End PS Boot Code
endif
                xor     eax,eax

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC

EndProc         fs_init16
;;---------------------------------------------------------------------------
;;
;; This is stuff that can't be done at ring 3, but needs to be done at
;; ring 0 before thunking or calling 32-bit C code.  It may be called from
;; fs_mount16 or fs_fsctl16, whichever is called first.
;;
;;---------------------------------------------------------------------------
Procedure       ring0_init,<far>

                mov     firstCall, 1    ;; Only needs to be called once

;;      Save Kernel Stack Selector.  Thunking on the kernel stack is handled
;;      differently than in other segments.

                mov     ax, ss
                mov     KernelSS, ax

;;      Set up selector table (for DosFlat2Sel).  This can't be done at ring 3
;;      because we can't access the GDT then.
;;      Don't worry about eax being clobbered.  The second argument is only
;;      used on the first call to DefineSelector

                mov     eax, FLAToffset gdtBase
                mov     eax, dword ptr [eax]
                xor     ecx,ecx
                mov     cx, seg DATA16
                CCALL32 _DefineSelector@8 <ecx, eax>
                xor     ecx,ecx
                mov     cx, seg CODE16
                CCALL32 _DefineSelector@8 <ecx, eax>

                CCALL32 _jfs_init1@0

                RETC

EndProc         ring0_init

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------
Procedure       fs_mount16,<far,pascal>

newArgVar  pboot,DWORD,,,,32
newArgVar  hvpb,WORD,,,,32
newArgVar  pvpfsd,DWORD,,,,32
newArgVar  pvpfsi,DWORD,,,,32
newArgVar  flag,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

;; I don't know why, but we're being called with interrupts disabled
        sti

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                mov     eax, firstCall
                test    eax,eax
                jnz     skipinit

                CCall   ring0_init

skipinit        label   near
                jmp     far ptr FLAT:mount_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  mount_32
mount_32        label   near

                CCall   KernThunkStackTo32

                Thunk   pboot
                push    eax

                movzx   eax, hvpb
                push    eax

                Thunk   pvpfsd
                push    eax

                Thunk   pvpfsi
                push    eax

                movzx   eax, flag
                push    eax

                CCall   FS32_MOUNT
                add     esp, 20

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:mount_16

                EndCode CODE32
mount_16        label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC

EndProc         fs_mount16

;;---------------------------------------------------------------------------
;;
;;
;;---------------------------------------------------------------------------

Procedure       fs_allocatepagespace16,<far,pascal>
newArgVar  contig,DWORD,,,,32
newArgVar  newsiz,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx           ebp,bp

                SaveReg         <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:allocateps_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  allocateps_32
        allocateps_32   label   near

                CCall   KernThunkStackTo32

                push    contig

                push    newsiz

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_ALLOCATEPAGESPACE
                add     esp, 16

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:allocateps_16

                EndCode CODE32
        allocateps_16   label   far

                RestoreReg      <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_allocatepagespace16

;;---------------------------------------------------------------------------

Procedure       fs_cancellockrequest16,<far,pascal>
newArgVar  pLockRange,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx           ebp,bp

                SaveReg         <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:cancellockrequest_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  cancellockrequest_32
        cancellockrequest_32    label   near

                CCall   KernThunkStackTo32

                Thunk   pLockRange
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_CANCELLOCKREQUEST
                add     esp, 12

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:cancellockrequest16

                EndCode CODE32
        cancellockrequest16     label   far

                RestoreReg      <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_cancellockrequest16

;;---------------------------------------------------------------------------

Procedure       fs_cancellockrequestl16,<far,pascal>
newArgVar  pLockRange,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx           ebp,bp

                SaveReg         <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:cancellockrequestl_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  cancellockrequestl_32
        cancellockrequestl_32    label   near

                CCall   KernThunkStackTo32

                Thunk   pLockRange
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_CANCELLOCKREQUESTL
                add     esp, 12

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:cancellockrequestl16

                EndCode CODE32
        cancellockrequestl16     label   far

                RestoreReg      <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_cancellockrequestl16

;;---------------------------------------------------------------------------

Procedure       fs_attach16,<far,pascal>
newArgVar  pLen,DWORD,,,,32
newArgVar  pParm,DWORD,,,,32
newArgVar  cdfsd,DWORD,,,,32
newArgVar  pvpfsd,DWORD,,,,32
newArgVar  pDev,DWORD,,,,32
newArgVar  flag,WORD,,,,32

                mov     eax, 50 ;; ERROR_NOT_SUPPORTED

                RETC
EndProc         fs_attach16

;;---------------------------------------------------------------------------

Procedure       fs_chdir16,<far,pascal>
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pDir,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32
newArgVar  flag,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:chdir_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  chdir_32
        chdir_32        label   far

                CCall   KernThunkStackTo32

                movzx   eax, iCurDirEnd
                push    eax

        thunk   pDir
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                movzx   eax, flag
                push    eax

                CCall   FS32_CHDIR
                add     esp, 20

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:chdir_16

                EndCode CODE32
        chdir_16        label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_chdir16

;;---------------------------------------------------------------------------

Procedure       fs_chgfileptr16,<far,pascal>
newArgVar  IOflag,WORD,,,,32
newArgVar  sType,WORD,,,,32
newArgVar  lOffset,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:chgfileptr_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  chgfileptr_32
        chgfileptr_32   label   near

                CCall   KernThunkStackTo32

                movzx   eax,IOflag
                push    eax

                movzx   eax,sType
                push    eax

        mov eax,lOffset
        sar eax,31
        push    eax
                push    lOffset

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_CHGFILEPTRL
                add     esp,24

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:chgfileptr_16

                EndCode CODE32
        chgfileptr_16   label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_chgfileptr16

;;---------------------------------------------------------------------------

Procedure       fs_close16,<far,pascal>
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32
newArgVar  IOflag,WORD,,,,32
newArgVar  sType,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:close_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  close_32
        close_32        label   near

                CCall   KernThunkStackTo32

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                movzx   eax, IOflag
                push    eax

                movzx   eax, sType
                push    eax

                CCall   FS32_CLOSE
                add     esp, 16

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:close_16

                EndCode CODE32
        close_16        label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_close16

;;---------------------------------------------------------------------------

Procedure       fs_commit16,<far,pascal>
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32
newArgVar  IOflag,WORD,,,,32
newArgVar  sType,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:commit_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  commit_32
        commit_32       label   near

                CCall   KernThunkStackTo32

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                movzx   eax, IOflag
                push    eax

                movzx   eax, sType
                push    eax

                CCall   FS32_COMMIT
                add     esp, 16

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:commit_16

                EndCode CODE32
        commit_16       label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_commit16

;;---------------------------------------------------------------------------

Procedure       fs_copy16,<far,pascal>
newArgVar  nameType,WORD,,,,32
newArgVar  iDstCurDirEnd,WORD,,,,32
newArgVar  pDst,DWORD,,,,32
newArgVar  iSrcCurDirEnd,WORD,,,,32
newArgVar  pSrc,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32
newArgVar  flag,WORD,,,,32

                mov     eax, 266        ;; ERROR_CANNOT_COPY

                RETC
EndProc         fs_copy16

;;---------------------------------------------------------------------------

Procedure       fs_delete16,<far,pascal>
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pFile,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:delete_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  delete_32
        delete_32       label   far

                CCall   KernThunkStackTo32

                movzx   eax, iCurDirEnd
                push    eax

        thunk   pFile
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                CCall   FS32_DELETE
                add     esp, 16

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:delete_16

                EndCode CODE32
        delete_16       label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_delete16

;;---------------------------------------------------------------------------

Procedure       fs_dopageio16,<far,pascal>
newArgVar  cmdhdr,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx           ebp,bp

                SaveReg         <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:dopageio_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  dopageio_32
        dopageio_32     label   near

                CCall   KernThunkStackTo32

                Thunk   cmdhdr
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_DOPAGEIO
                add     esp, 12

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:dopageio_16

                EndCode CODE32
        dopageio_16     label   far

                RestoreReg      <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_dopageio16

;;---------------------------------------------------------------------------

Procedure       fs_exit16,<far,pascal>

newArgVar  pdb,WORD,,,,32
newArgVar  pid,WORD,,,,32
newArgVar  uid,WORD,,,,32

        EnterProc       ,,alignesp
        movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

;;  If enum_anchor is zero, skip the 32-bit call and return.

        mov eax, enum_anchor
        test    eax, eax
        jz  bypass

                jmp     far ptr FLAT:exit_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  exit_32
        exit_32     label   far

                CCall   KernThunkStackTo32

                movzx   eax, pdb
                push    eax

                movzx   eax, pid
                push    eax

                movzx   eax, uid
                push    eax

                CCall   FS32_EXIT
                add     esp, 12

                CCall   KernThunkStackTo16

                jmp     far ptr CODE16:exit_16

                EndCode CODE32
        exit_16     label   far
    bypass      label   near

                RestoreReg <es,ds,edi,esi,ebx>
        LeaveProc
                RETC

EndProc         fs_exit16

;;---------------------------------------------------------------------------

Procedure       fs_fileattribute16,<far,pascal>
newArgVar  pAttr,DWORD,,,,32
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pName,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32
newArgVar  flag,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:fileattribute_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  fileattribute_32
        fileattribute_32        label   far

                CCall   KernThunkStackTo32

                Thunk   pAttr
                push    eax

                movzx   eax, iCurDirEnd
                push    eax

        thunk   pName
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                movzx   eax, flag
                push    eax

                CCall   FS32_FILEATTRIBUTE
                add     esp, 24

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:fileattribute_16

                EndCode CODE32
        fileattribute_16        label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_fileattribute16

;;---------------------------------------------------------------------------

Procedure       fs_fileinfo16,<far,pascal>
newArgVar  IOflag,WORD,,,,32
newArgVar  cbData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  level,WORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32
newArgVar  flag,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:fileinfo_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  fileinfo_32
        fileinfo_32     label   near

                CCall   KernThunkStackTo32

                movzx   eax, IOflag
                push    eax

                movzx   eax, cbData
                push    eax

                Thunk   pData
                push    eax

                movzx   eax, level
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                movzx   eax, flag
                push    eax

                CCall   FS32_FILEINFO
                add     esp, 28

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:fileinfo_16

                EndCode CODE32
        fileinfo_16     label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_fileinfo16

;;---------------------------------------------------------------------------

Procedure       fs_fileio16,<far,pascal>
newArgVar  IOflag,WORD,,,,32
newArgVar  poError,DWORD,,,,32
newArgVar  flag,WORD,,,,32
newArgVar  pCmdList,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:fileio_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  fileio_32
        fileio_32       label   near

                CCall   KernThunkStackTo32

                movzx   eax, IOflag
                push    eax

                Thunk   poError
                push    eax

                movzx   eax, flag
                push    eax

                Thunk   pCmdList
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_FILEIO
                add     esp, 24

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:fileio_16

                EndCode CODE32
        fileio_16       label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_fileio16

;;---------------------------------------------------------------------------

Procedure       fs_filelocks16,<far,pascal>
newArgVar  flags,DWORD,,,,32
newArgVar  timeout,DWORD,,,,32
newArgVar  pLockRange,DWORD,,,,32
newArgVar  pUnlockRange,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx           ebp,bp

                SaveReg         <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:filelocks_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  filelocks_32
        filelocks_32    label   near

                CCall   KernThunkStackTo32

                push    flags

                push    timeout

                Thunk   pLockRange
                push    eax

                Thunk   pUnlockRange
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_FILELOCKS
                add     esp, 24

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:filelocks16

                EndCode CODE32
        filelocks16     label   far

                RestoreReg      <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_filelocks16

;;---------------------------------------------------------------------------

Procedure       fs_filelocksl16,<far,pascal>
newArgVar  flags,DWORD,,,,32
newArgVar  timeout,DWORD,,,,32
newArgVar  pLockRange,DWORD,,,,32
newArgVar  pUnlockRange,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx           ebp,bp

                SaveReg         <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:filelocksl_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  filelocksl_32
        filelocksl_32    label   near

                CCall   KernThunkStackTo32

                push    flags

                push    timeout

                Thunk   pLockRange
                push    eax

                Thunk   pUnlockRange
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_FILELOCKSL
                add     esp, 24

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:filelocksl16

                EndCode CODE32
        filelocksl16     label   far

                RestoreReg      <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_filelocksl16

;;---------------------------------------------------------------------------

Procedure       fs_findclose16,<far,pascal>
newArgVar  pfsfsd,DWORD,,,,32
newArgVar  pfsfsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:findclose_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  findclose_32
        findclose_32    label   near

                CCall   KernThunkStackTo32

                Thunk   pfsfsd
                push    eax

                Thunk   pfsfsi
                push    eax

                CCall   FS32_FINDCLOSE
                add     esp, 8

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:findclose_16

                EndCode CODE32
        findclose_16    label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_findclose16

;;---------------------------------------------------------------------------

Procedure       fs_findfirst16,<far,pascal>
newArgVar  flags,WORD,,,,32
newArgVar  level,WORD,,,,32
newArgVar  pcMatch,DWORD,,,,32
newArgVar  cbData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  fsfsd,DWORD,,,,32
newArgVar  fsfsi,DWORD,,,,32
newArgVar  attr,WORD,,,,32
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pName,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32

newLocalVar     lflags,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:findfirst_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  findfirst_32
        findfirst_32    label   far

                CCall   KernThunkStackTo32

                movzx   eax, flags
                push    eax

                movzx   eax, level
                push    eax

                Thunk   pcMatch
                push    eax

                movzx   eax, cbData
                push    eax

                Thunk   pData
        push    eax

                Thunk   fsfsd
                push    eax

                Thunk   fsfsi
                push    eax

                movzx   eax, attr
                push    eax

                movzx   eax, iCurDirEnd
                push    eax

        thunk   pName
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                CCall   FS32_FINDFIRST
                add     esp, 48

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:findfirst_16

                EndCode CODE32
        findfirst_16    label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_findfirst16

;;---------------------------------------------------------------------------

Procedure       fs_findfromname16,<far,pascal>
newArgVar  flags,WORD,,,,32
newArgVar  pName,DWORD,,,,32
newArgVar  position,DWORD,,,,32
newArgVar  level,WORD,,,,32
newArgVar  pcMatch,DWORD,,,,32
newArgVar  cbData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  pfsfsd,DWORD,,,,32
newArgVar  pfsfsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:findfromname_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  findfromname_32
        findfromname_32 label   far

                CCall   KernThunkStackTo32

                movzx   eax, flags
                push    eax

        thunk   pName
                push    eax

                push    position

                movzx   eax, level
                push    eax

                Thunk   pcMatch
                push    eax

                movzx   eax, cbData
                push    eax

                Thunk   pData
        push    eax

                Thunk   pfsfsd
                push    eax

                Thunk   pfsfsi
                push    eax

                CCall   FS32_FINDFROMNAME
                add     esp, 36

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:findfromname_16

                EndCode CODE32
        findfromname_16 label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_findfromname16

;;---------------------------------------------------------------------------

Procedure       fs_findnext16,<far,pascal>
newArgVar  flags,WORD,,,,32
newArgVar  level,WORD,,,,32
newArgVar  pcMatch,DWORD,,,,32
newArgVar  cbData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  pfsfsd,DWORD,,,,32
newArgVar  pfsfsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:findnext_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  findnext_32
        findnext_32     label   far

                CCall   KernThunkStackTo32

                movzx   eax, flags
                push    eax

                movzx   eax, level
                push    eax

                Thunk   pcMatch
                push    eax

                movzx   eax, cbData
                push    eax

                Thunk   pData
        push    eax

                Thunk   pfsfsd
                push    eax

                Thunk   pfsfsi
                push    eax

                CCall   FS32_FINDNEXT
                add     esp, 28

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:findnext_16

                EndCode CODE32
        findnext_16     label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_findnext16

;;---------------------------------------------------------------------------

Procedure       fs_findnotifyclose16,<far,pascal>
newArgVar  handle,WORD,,,,32

                mov     eax, 50         ;; ERROR_NOT_SUPPORTED

                RETC
EndProc         fs_findnotifyclose16

;;---------------------------------------------------------------------------

Procedure       fs_findnotifyfirst16,<far,pascal>
newArgVar  timeout,DWORD,,,,32
newArgVar  level,WORD,,,,32
newArgVar  pMatch,DWORD,,,,32
newArgVar  cbData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  pHandle,DWORD,,,,32
newArgVar  attr,WORD,,,,32
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pName,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32

                mov     eax, 50         ;; ERROR_NOT_SUPPORTED

                RETC
EndProc         fs_findnotifyfirst16

;;---------------------------------------------------------------------------

Procedure       fs_findnotifynext16,<far,pascal>
newArgVar  timeout,DWORD,,,,32
newArgVar  level,WORD,,,,32
newArgVar  pcMatch,DWORD,,,,32
newArgVar  cbData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  handle,WORD,,,,32

                mov     eax, 50         ;; ERROR_NOT_SUPPORTED

                RETC
EndProc         fs_findnotifynext16

;;---------------------------------------------------------------------------

Procedure       fs_flushbuf16,<far,pascal>
newArgVar  flag,WORD,,,,32
newArgVar  hVPB,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:flushbuf_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  flushbuf_32
        flushbuf_32     label   near

                CCall   KernThunkStackTo32

                movzx   eax, flag
                push    eax

                movzx   eax, hVPB
                push    eax

                CCall   FS32_FLUSHBUF
                add     esp, 8

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:flushbuf_16

                EndCode CODE32
        flushbuf_16     label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_flushbuf16

;;---------------------------------------------------------------------------

Procedure       fs_fsctl16,<far,pascal>
newArgVar  plenDataIO,DWORD,,,,32
newArgVar  lenData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  plenParmIO,DWORD,,,,32
newArgVar  lenParm,WORD,,,,32
newArgVar  pParm,DWORD,,,,32
newArgVar  func,WORD,,,,32
newArgVar  iArgType,WORD,,,,32
newArgVar  pArgDat,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                mov     eax, firstCall
                test    eax,eax
                jnz     skipinit2

                CCall   ring0_init

skipinit2       label   near
                jmp     far ptr FLAT:fsctl_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  fsctl_32
        fsctl_32        label   near

                CCall   KernThunkStackTo32

                Thunk   plenDataIO
                push    eax

                movzx   eax, lenData
                push    eax

                Thunk   pData
                push    eax

                Thunk   plenParmIO
                push    eax

                movzx   eax, lenParm
                push    eax

                Thunk   pParm
                push    eax

                movzx   eax, func
                push    eax

                movzx   eax, iArgType
                push    eax

                Thunk   pArgDat
                push    eax

                CCall   FS32_FSCTL
                add     esp,36

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:fsctl_16

                EndCode CODE32
        fsctl_16        label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_fsctl16

;;---------------------------------------------------------------

Procedure       fs_fsinfo16,<far,pascal>

newArgVar  level,WORD,,,,32
newArgVar  cbData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  hVPB,WORD,,,,32
newArgVar  flag,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:fsinfo_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  fsinfo_32
        fsinfo_32       label   near

                CCall   KernThunkStackTo32

                movzx   eax, level
                push    eax

                movzx   eax, cbData
                push    eax

                Thunk   pData
                push    eax

                movzx   eax, hVPB
                push    eax

                movzx   eax, flag
                push    eax

                CCall   FS32_FSINFO
                add     esp, 20

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:fsinfo_16

                EndCode CODE32
        fsinfo_16       label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_fsinfo16

;;---------------------------------------------------------------

Procedure       fs_mkdir16,<far,pascal>
newArgVar  flags,WORD,,,,32
newArgVar  pEABuf,DWORD,,,,32
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pName,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:mkdir_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  mkdir_32
        mkdir_32        label   far

                CCall   KernThunkStackTo32

                movzx   eax, flags
                push    eax

                Thunk   pEABuf
        push    eax

                movzx   eax, iCurDirEnd
                push    eax

        thunk   pName
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                CCall   FS32_MKDIR
                add     esp, 24

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:mkdir_16

                EndCode CODE32
        mkdir_16        label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_mkdir16

;;---------------------------------------------------------------------------

Procedure       fs_move16,<far,pascal>
newArgVar  flags,WORD,,,,32
newArgVar  iDstCurDirEnd,WORD,,,,32
newArgVar  pDst,DWORD,,,,32
newArgVar  iSrcCurDirEnd,WORD,,,,32
newArgVar  pSrc,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:move_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  move_32
        move_32 label   far

                CCall   KernThunkStackTo32

                movzx   eax, flags
                push    eax

                movzx   eax, iDstCurDirEnd
                push    eax

        thunk   pDst
                push    eax

                movzx   eax, iSrcCurDirEnd
                push    eax

        thunk   pSrc
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                CCall   FS32_MOVE
                add     esp, 28

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:move_16

                EndCode CODE32
        move_16 label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_move16

;;---------------------------------------------------------------------------

Procedure       fs_newsizel16,<far,pascal>
newArgVar  IOflag,WORD,,,,32
newArgVar  lenlo,DWORD,,,,32
newArgVar  lenhi,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:newsizel_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  newsizel_32
        newsizel_32     label   far

                CCall   KernThunkStackTo32

                movzx   eax, IOflag
                push    eax

                push    lenhi
                push    lenlo

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_NEWSIZEL
                add     esp, 20

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:newsizel_16

                EndCode CODE32
        newsizel_16     label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_newsizel16

;;---------------------------------------------------------------------------

Procedure       fs_newsize16,<far,pascal>
newArgVar  IOflag,WORD,,,,32
newArgVar  len,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:newsize_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  newsize_32
        newsize_32      label   far

                CCall   KernThunkStackTo32

                movzx   eax, IOflag
                push    eax

        xor eax,eax
        push    eax
                push    len

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_NEWSIZEL
                add     esp, 20

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:newsize_16

                EndCode CODE32
        newsize_16      label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_newsize16

;;---------------------------------------------------------------------------

Procedure       fs_nmpipe16,<far,pascal>
newArgVar  pName,DWORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  pOpRec,DWORD,,,,32
newArgVar  OpType,WORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                mov     eax, 50         ;; ERROR_NOT_SUPPORTED

                RETC
EndProc         fs_nmpipe16

;;---------------------------------------------------------------

Procedure       fs_opencreate16,<far,pascal>

newArgVar  pfgenflag,DWORD,,,,32
newArgVar  pcEABuf,DWORD,,,,32
newArgVar  usAttr,WORD,,,,32
newArgVar  pusAction,DWORD,,,,32
newArgVar  usOpenFlag,WORD,,,,32
newArgVar  ulOpenMode,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pName,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:opencreate_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  opencreate_32
        opencreate_32   label   far

                CCall   KernThunkStackTo32

                Thunk   pfgenflag
                push    eax

                Thunk   pcEABuf
        push    eax

                movzx   eax, usAttr
                push    eax

                Thunk   pusAction
                push    eax

                movzx   eax, usOpenFlag
                push    eax

                push    ulOpenMode

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                movzx   eax, iCurDirEnd
                push    eax

        thunk   pName
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                CCall   FS32_OPENCREATE
                add     esp, 48

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:opencreate_16

                EndCode CODE32
        opencreate_16   label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC

EndProc         fs_opencreate16

;;---------------------------------------------------------------------------

Procedure       fs_openpagefile16,<far,pascal>
newArgVar  reserv,DWORD,,,,32
newArgVar  uattr,WORD,,,,32
newArgVar  openflag,WORD,,,,32
newArgVar  openmode,WORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32
newArgVar  pname,DWORD,,,,32
newArgVar  pmaxrq,DWORD,,,,32
newArgVar  pflag,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx           ebp,bp

                SaveReg         <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:openpagefile_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  openpagefile_32
        openpagefile_32 label   far

                CCall   KernThunkStackTo32

                push    reserv

                movzx   eax, uattr
                push    eax

                movzx   eax, openflag
                push    eax

                movzx   eax, openmode
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

        thunk   pname
                push    eax

                Thunk   pmaxrq
                push    eax

                Thunk   pflag
                push    eax

                CCall   FS32_OPENPAGEFILE
                add     esp, 36

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:openpagefile_16

                EndCode CODE32
        openpagefile_16 label   far

                RestoreReg      <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_openpagefile16

;;---------------------------------------------------------------

Procedure       fs_pathinfo16,<far,pascal>
newArgVar  cbData,WORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  level,WORD,,,,32
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pName,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32
newArgVar  flag,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:pathinfo_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  pathinfo_32
        pathinfo_32     label   far

                CCall   KernThunkStackTo32

                movzx   eax, cbData
                push    eax

                Thunk   pData
        push    eax

                movzx   eax, level
                push    eax

                movzx   eax, iCurDirEnd
                push    eax

        thunk   pName
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                movzx   eax, flag
                push    eax

                CCall   FS32_PATHINFO
                add     esp, 32

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:pathinfo_16

                EndCode CODE32
        pathinfo_16     label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_pathinfo16

;;---------------------------------------------------------------

Procedure       fs_processname16,<far,pascal>

newArgVar  pNameBuf,DWORD,,,,32

                xor     ax,ax

                RETC

EndProc         fs_processname16

;;---------------------------------------------------------------

Procedure       fs_read16,<far,pascal>
newArgVar  IOflag,WORD,,,,32
newArgVar  pLen,DWORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

newLocalVar     Len,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:read_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  read_32
        read_32 label   near

                CCall KernThunkStackTo32

;;              CCALL32 _fs_read@20 <psffsi,psffsd,pData,pLen,eax>

                movzx   eax,IOflag
                push    eax

                push    es
                les     si,pLen
                movzx   ebx, word ptr es:[si]
                pop     es
                lea     eax,Len
                mov     dword ptr [eax], ebx
                push    eax

                Thunk   pData
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_READ
                add     esp,20

                mov     ebx, Len
                push    es
                les     si, pLen
                mov     word ptr es:[si], bx
                pop     es

                mov     ebx,eax
                CCall   KernThunkStackTo16
                mov     eax,ebx

                jmp     far ptr CODE16:read_16

                EndCode CODE32
        read_16 label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_read16

;;---------------------------------------------------------------------------

Procedure       fs_rmdir16,<far,pascal>
newArgVar  iCurDirEnd,WORD,,,,32
newArgVar  pName,DWORD,,,,32
newArgVar  pcdfsd,DWORD,,,,32
newArgVar  pcdfsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:rmdir_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  rmdir_32
        rmdir_32        label   far

                CCall   KernThunkStackTo32

                movzx   eax, iCurDirEnd
                push    eax

        thunk   pName
                push    eax

                Thunk   pcdfsd
                push    eax

                Thunk   pcdfsi
                push    eax

                CCall   FS32_RMDIR
                add     esp, 16

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:rmdir_16

                EndCode CODE32
        rmdir_16        label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_rmdir16

;;---------------------------------------------------------------------------

Procedure       fs_setswap16,<far,pascal>
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:setswap_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  setswap_32
        setswap_32      label   far

                CCall   KernThunkStackTo32

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_SETSWAP
                add     esp, 8

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:setswap_16

                EndCode CODE32
        setswap_16      label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_setswap16

;;---------------------------------------------------------------------------

Procedure       fs_shutdown16,<far,pascal>

newArgVar  reserved,DWORD,,,,32
newArgVar  sdtype,WORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:shutdown_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  shutdown_32
        shutdown_32     label   far

                CCall   KernThunkStackTo32

                push    reserved

                movzx   eax, sdtype
                push    eax

                CCall   FS32_SHUTDOWN
                add     esp, 8

                mov     ebx, eax
                CCall   KernThunkStackTo16
                mov     eax, ebx

                jmp     far ptr CODE16:shutdown_16

                EndCode CODE32
        shutdown_16     label   far

                RestoreReg <es,ds,edi,esi,ebx>
                LeaveProc
                RETC

EndProc         fs_shutdown16

;;---------------------------------------------------------------------------

Procedure       fs_write16,<far,pascal>
newArgVar  IOflag,WORD,,,,32
newArgVar  plen,DWORD,,,,32
newArgVar  pData,DWORD,,,,32
newArgVar  psffsd,DWORD,,,,32
newArgVar  psffsi,DWORD,,,,32

newLocalVar     Len,DWORD,,,,32

                EnterProc       ,,alignesp
                movzx   ebp,bp

                SaveReg <ebx,esi,edi,ds,es>

                flatContext

                jmp     far ptr FLAT:write_32

                DefCode CODE32,CODE32,,USE32
                aflatCS
PUBLIC  write_32
        write_32        label   near

                CCall KernThunkStackTo32

;;              CCALL32 _fs_write@20 <psffsi,psffsd,pData,pLen,eax>

                movzx   eax,IOflag
                push    eax

                push    es
                les     si,pLen
                movzx   ebx, word ptr es:[si]
                pop     es
                lea     eax, Len
                mov     dword ptr [eax], ebx
                push    eax

                Thunk   pData
                push    eax

                Thunk   psffsd
                push    eax

                Thunk   psffsi
                push    eax

                CCall   FS32_WRITE
                add     esp,20

                mov     ebx, Len
                push    es
                les     si, pLen
                mov     word ptr es:[si], bx
                pop     es

                mov     ebx,eax
                CCall   KernThunkStackTo16
                mov     eax,ebx

                jmp     far ptr CODE16:write_16

                EndCode CODE32
        write_16        label   far

                RestoreReg <es,ds,edi,esi,ebx>

                LeaveProc
                RETC
EndProc         fs_write16

;;---------------------------------------------------------------------------

EndCode

        END

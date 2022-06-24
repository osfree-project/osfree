;
; sec32_pre_init_base.asm:
;       this contains additional startup code which gets called
;       by the 32-bits sec32_init_base() strategy handler.
;

;       Copyright (C) 2000-2003 Ulrich M”ller.
;       Based on the MWDD32.SYS example sources,
;       Copyright (C) 1995, 1996, 1997  Matthieu Willm (willm@ibm.net).
;
;       This file is part of the XWorkplace source package.
;       XWorkplace is free software; you can redistribute it and/or modify
;       it under the terms of the GNU General Public License as published
;       by the Free Software Foundation, in version 2 as it comes in the
;       "COPYING" file of the XWorkplace main distribution.
;       This program is distributed in the hope that it will be useful,
;       but WITHOUT ANY WARRANTY; without even the implied warranty of
;       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;       GNU General Public License for more details.

        .386p

        INCL_DOSERRORS equ 1
        include bseerr.inc
        include devhlp.inc
;        include devcmd.inc
;        include devsym.inc
        include sec32_segdef.inc
        include r0thunk.inc

reqPacket       struc
reqLenght       db ?
reqUnit         db ?
reqCommand      db ?
reqStatus       dw ?
reqFlags        db ?
                db 3 dup (?)    ; Reserved field
reqLink         dd ?
reqPacket       ends

rpInitIn        struc
i_rph           db size reqPacket dup (?)
i_unit          db ?
i_devHelp       dd ?
i_initArgs      dd ?
i_driveNum      db ?
rpInitIn        ends

DosTable2 struc
        d2_ErrMap24                     dd ?
        d2_MsgMap24                     dd ?
        d2_Err_Table_24                 dd ?
        d2_CDSAddr                      dd ?
        d2_GDT_RDR1                     dd ?
        d2_InterruptLevel               dd ?
        d2__cInDos                      dd ?
        d2_zero_1                       dd ?
        d2_zero_2                       dd ?
        d2_FlatCS                       dd ?
        d2_FlatDS                       dd ?
        d2__TKSSBase                    dd ?
        d2_intSwitchStack               dd ?
        d2_privateStack                 dd ?
        d2_PhysDiskTablePtr             dd ?
        d2_forceEMHandler               dd ?
        d2_ReserveVM                    dd ?
        d2_pgpPageDir                   dd ?
        d2_unknown                      dd ?
DosTable2 ends

DATA32 segment
        extrn  G_TKSSBase  : dword
DATA32 ends

CODE16 segment
ASSUME CS:CODE16, DS:FLAT

        public thunk16$sec32_pre_init_base

thunk16$sec32_pre_init_base:
        call dword ptr [DevHelp2]
        jmp far ptr FLAT:thunk32$sec32_pre_init_base

CODE16 ends

CODE32 segment
ASSUME CS:FLAT, DS:FLAT, ES:FLAT

        public thunk32$sec32_pre_init_base
        public         sec32_pre_init_base

;
; int sec32_pre_init_base(
;                          PTR16 reqpkt      /* ebp + 8 */
;                         );
;
sec32_pre_init_base proc near
        push ebp
        mov ebp, esp
        push es
        push ebx
        push esi
        push edi

        ;
        ; Saves the DevHelp entry point
        ;
        les bx, dword ptr[ebp + 8]   ; es:bx points to the INIT request packet
        movzx ebx, bx
        mov ebx, es:[ebx].i_devHelp  ; Device Helper entry point
        mov DevHelp2, ebx            ; saved into DevHelp2

        ;
        ; Gets the TKSSBase pointer from DosTable. TKSSBase is used by
        ; __StackToFlat() to convert a stack based address to a FLAT address
        ; without the overhead of DevHlp_VirtToLin
        ;
        ; DosTable is obtained through GetDOSVar with undocumented index 9
        ; The layout is :
        ;     byte      count of following dword (n)
        ;     dword 1   -+
        ;       .        |
        ;       .        | this is DosTable1
        ;       .        |
        ;     dword n   -+
        ;     byte      count of following dword (p)
        ;     dword 1   -+
        ;       .        |
        ;       .        | this is DosTable2
        ;       .        |
        ;     dword p   -+
        ;
        ; Flat CS  is dword number 10 in DosTable2
        ; Flat DS  is dword number 11 in DosTable2
        ; TKSSBase is dword number 12 in DosTable2
        ;
        mov eax, 9                       ; undocumented DosVar : DosTable pointer
        xor ecx, ecx
        mov edx, DevHlp_GetDOSVar
        jmp far ptr thunk16$sec32_pre_init_base ; ax:bx points to DosTable
thunk32$sec32_pre_init_base:
        jc short sec32_pre_init_base_err
        mov es, ax                       ; es:bx points to DosTable
        movzx ebx, bx
        movzx ecx, byte ptr es:[ebx]     ; count of dword in DosTable1
        mov eax, es:[ebx + 4 * ecx + 2].d2__TKSSBase
        mov G_TKSSBase, eax

        mov eax, NO_ERROR
sec32_pre_init_base_err:
        pop edi
        pop esi
        pop ebx
        pop es
        leave
        ret
sec32_pre_init_base endp

CODE32 ends



    end

;
; sec32_devhlp32.asm:
;       32-bit device helper functions. These allow the 16-bit
;       DevHlp* calls to be used from 32-bit C code.
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

;*********************************************************
;
;   Includes
;
;*********************************************************

        INCL_DOSERRORS equ 1
        include bseerr.inc
        include devhlp.inc
        ; include mwdd32_segdef.inc
        include sec32_segdef.inc
        include r0thunk.inc

;*********************************************************
;
;   Defines
;
;*********************************************************

SAS_SIG   equ      "SAS "

SAS struc
    SAS_signature db 4 dup (?)
    SAS_tables_data dw ?            ; offset to tables section
    SAS_flat_sel    dw ?            ; FLAT selector for kernel data
    SAS_config_data dw ?            ; offset to configuration section
    SAS_dd_data     dw ?            ; offset to device driver section
    SAS_vm_data     dw ?            ; offset to Virtual Memory section
    SAS_task_data   dw ?            ; offset to Tasking section
    SAS_RAS_data    dw ?            ; offset to RAS section
    SAS_file_data   dw ?            ; offset to File System section
    SAS_info_data   dw ?            ; offset to infoseg section
SAS ends

; Information Segment section
SAS_info_section struc
    SAS_info_global  dw ?           ; selector for global info seg
    SAS_info_local   dd ?           ; address of curtask local infoseg
    SAS_info_localRM dd ?           ; address of DOS task's infoseg
    SAS_info_CDIB    dw ?           ; selector for Codepage Data Information Block (CDIB)
SAS_info_section ends

    extrn SAS_SEL : abs

    DevHlp_Security equ 044h

    DHSEC_GETEXPORT equ 048a78df8h
    DHSEC_SETIMPORT equ 073ae3627h
    DHSEC_GETINFO   equ 033528882h

_SecHlpUISEPList   struc

   Offset_QuerySubjectHandle        dd    1 dup (?)
   Offset_QuerySubjectInfo          dd    1 dup (?)
   Offset_QueryContextStatus        dd    1 dup (?)
   Offset_QuerySecurityContext      dd    1 dup (?)
   Offset_QuerySubjectHandleInfo    dd    1 dup (?)
   Offset_SetSubjectHandle          dd    1 dup (?)
   Offset_SetSecurityContext        dd    1 dup (?)
   Offset_SetContextStatus          dd    1 dup (?)
   Offset_ResetThreadContext        dd    1 dup (?)
   Offset_QueryAuthID               dd    1 dup (?)
   Offset_SetAuthID                 dd    1 dup (?)
   Offset_ReserveHandle             dd    1 dup (?)
   Offset_ReleaseHandle             dd    1 dup (?)
   Offset_SetChildSecurityContext   dd    1 dup (?)

_SecHlpUISEPList ends

;*********************************************************
;
;   CODE16
;
;*********************************************************

CODE16 segment

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_AllocOneGDTSelector
code16_DevHlp32_AllocOneGDTSelector:
        call [DevHelp2]
        jmp32 code32_DevHlp32_AllocOneGDTSelector

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_Beep
code16_DevHlp32_Beep:
        call [DevHelp2]
        jmp32 code32_DevHlp32_Beep

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_AttachToDD
code16_DevHlp32_AttachToDD:
        call es:[DevHelp2]
        jmp32 code32_DevHlp32_AttachToDD

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_CloseEventSem
code16_DevHlp32_CloseEventSem:
        call [DevHelp2]
        jmp32 code32_DevHlp32_CloseEventSem

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_EOI
code16_DevHlp32_EOI:
        call [DevHelp2]
        jmp32 code32_DevHlp32_EOI

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_FreeGDTSelector
code16_DevHlp32_FreeGDTSelector:
        call [DevHelp2]
        jmp32 code32_DevHlp32_FreeGDTSelector

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_GetDosVar
code16_DevHlp32_GetDosVar:
        call [DevHelp2]
        jmp32 code32_DevHlp32_GetDosVar

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_GetInfoSegs_1
public code16_DevHlp32_GetInfoSegs_2
code16_DevHlp32_GetInfoSegs_1:
        call [DevHelp2]
        jmp32 code32_DevHlp32_GetInfoSegs_1
code16_DevHlp32_GetInfoSegs_2:
        call [DevHelp2]
        jmp32 code32_DevHlp32_GetInfoSegs_2

ASSUME CS:CODE16, DS:DATA16, ES:FLAT
public code16_DevHlp32_InternalError
code16_DevHlp32_InternalError:
        call es:[DevHelp2]
        ;
        ; We should never reach this point
        ;
        int 3

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_LinToPageList
code16_DevHlp32_LinToPageList:
        call [DevHelp2]
        jmp32 code32_DevHlp32_LinToPageList

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_OpenEventSem
code16_DevHlp32_OpenEventSem:
        call [DevHelp2]
        jmp32 code32_DevHlp32_OpenEventSem

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_PageListToLin
code16_DevHlp32_PageListToLin:
        call [DevHelp2]
        jmp32 code32_DevHlp32_PageListToLin

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_PostEventSem
code16_DevHlp32_PostEventSem:
        call [DevHelp2]
        jmp32 code32_DevHlp32_PostEventSem

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_ProcBlock
code16_DevHlp32_ProcBlock:
        call [DevHelp2]
        jmp32 code32_DevHlp32_ProcBlock

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_ProcRun
code16_DevHlp32_ProcRun:
        call [DevHelp2]
        jmp32 code32_DevHlp32_ProcRun

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_ResetEventSem
code16_DevHlp32_ResetEventSem:
        call [DevHelp2]
        jmp32 code32_DevHlp32_ResetEventSem

ASSUME CS:CODE16, DS:FLAT, ES:FLAT
public code16_DevHlp32_SaveMessage_1
public code16_DevHlp32_SaveMessage_2
public code16_DevHlp32_SaveMessage_3
public code16_DevHlp32_SaveMessage_4
code16_DevHlp32_SaveMessage_1:
        call [DevHelp2]
        jmp32 code32_DevHlp32_SaveMessage_1
code16_DevHlp32_SaveMessage_2:
        call [DevHelp2]
        jmp32 code32_DevHlp32_SaveMessage_2
code16_DevHlp32_SaveMessage_3:
        call es:[DevHelp2]
        jmp32 code32_DevHlp32_SaveMessage_3
code16_DevHlp32_SaveMessage_4:
        call [DevHelp2]
        jmp32 code32_DevHlp32_SaveMessage_4

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_Security
code16_DevHlp32_Security:
        call [DevHelp2]
        jmp32 code32_DevHlp32_Security

ASSUME CS:CODE16, DS:NOTHING, ES:FLAT
public code16_DevHlp32_setIRQ
code16_DevHlp32_setIRQ:
        call es:[DevHelp2]
        jmp32 code32_DevHlp32_setIRQ

ASSUME CS:CODE16, ES:FLAT
public code16_DevHlp32_UnSetIRQ
code16_DevHlp32_UnSetIRQ:
        call es:[DevHelp2]
        jmp32 code32_DevHlp32_UnSetIRQ

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_VerifyAccess
code16_DevHlp32_VerifyAccess:
        call [DevHelp2]
        jmp32 code32_DevHlp32_VerifyAccess

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_VirtToLin
code16_DevHlp32_VirtToLin:
        call [DevHelp2]
        jmp32 code32_DevHlp32_VirtToLin

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_VMAlloc
code16_DevHlp32_VMAlloc:
        call [DevHelp2]
        jmp32 code32_DevHlp32_VMAlloc

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_VMFree
code16_DevHlp32_VMFree:
        call [DevHelp2]
        jmp32 code32_DevHlp32_VMFree

; added V0.9.5 (2000-09-27) [umoeller]
ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_VMGlobalToProcess
code16_DevHlp32_VMGlobalToProcess:
        call [DevHelp2]
        jmp32 code32_DevHlp32_VMGlobalToProcess

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_VMLock
code16_DevHlp32_VMLock:
        call [DevHelp2]
        jmp32 code32_DevHlp32_VMLock

; added V0.9.5 (2000-09-27) [umoeller]
ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_VMProcessToGlobal
code16_DevHlp32_VMProcessToGlobal:
        call [DevHelp2]
        jmp32 code32_DevHlp32_VMProcessToGlobal

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_VMUnlock
code16_DevHlp32_VMUnlock:
        call [DevHelp2]
        jmp32 code32_DevHlp32_VMUnlock

ASSUME CS:CODE16, DS:FLAT
public code16_DevHlp32_Yield
code16_DevHlp32_Yield:
        call [DevHelp2]
        jmp32 code32_DevHlp32_Yield

CODE16 ends

;*********************************************************
;
;   DATA16
;
;*********************************************************

DATA16 segment
    public panic_buffer
    panic_buffer db 512 dup (?)

    public begin_msg
    begin_msg label byte
    msgid   dw 1178
    nr      dw 1
    msg_ofs dw 0
    msg_seg dw ?
DATA16 ends

;*********************************************************
;
;   CODE32
;
;*********************************************************

CODE32 segment

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_AllocOneGDTSelector
public        DevHlp32_AllocOneGDTSelector

;
; int DH32ENTRY DevHlp32_AllocOneGDTSelector(
;                    unsigned short *psel       /* ebp + 8 !!! STACK BASED !!! */
;                   );
;
DevHlp32_AllocOneGDTSelector proc near
        enter 2, 0
        push es
        push ebx
        push esi
        push edi

        lea edi, [ebp - 2]         ; tmpsel
        mov eax, ss
        mov es, eax
        mov ecx, 1         ; one selector
        mov dl, DevHlp_AllocGDTSelector
        jmp far ptr code16_DevHlp32_AllocOneGDTSelector
code32_DevHlp32_AllocOneGDTSelector:
        jc short @@errorAllocOneGDTSelector           ; if error, EAX = error code
        mov ax, [ebp - 2]          ; tmpsel
        mov ebx, [ebp + 8]         ; psel
        mov ss:[ebx], ax           ; *psel
        mov eax, NO_ERROR
@@errorAllocOneGDTSelector:
        pop edi
        pop esi
        pop ebx
        pop es
        leave
        ret
DevHlp32_AllocOneGDTSelector endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_AttachToDD
public        DevHlp32_AttachToDD

;
; int _Optlink DevHlp32_AttachToDD(
;                    char *ddname,               /* eax !!! STACK BASED !!! */
;                    void *ddtable               /* edx !!! STACK BASED !!! */
;                   );
;
DevHlp32_AttachToDD proc near
        push ds
        push ebx
        push edi

        mov ebx, eax                    ; offset ds:ddname
        mov edi, edx                    ; offset ds:ddtable
        mov eax, ss
        mov ds, eax
        mov dl, DevHlp_AttachDD
        jmp far ptr code16_DevHlp32_AttachToDD
code32_DevHlp32_AttachToDD:
        jc short @@errorAttachToDD                         ; if error, EAX = error code
        mov eax, NO_ERROR
@@errorAttachToDD:
        pop edi
        pop ebx
        pop ds
        ret
DevHlp32_AttachToDD endp

; added V0.9.5 (2000-10-03) [umoeller]
ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_Beep
public        DevHlp32_Beep

; int DH32ENTRY2 DevHlp32_Beep(
;                                ULONG           ulFreq,       /* ebp + 8  */
;                                ULONG           ulDuration    /* ebp + 12 */
;                                  );

DevHlp32_Beep proc near
        push    ebx
        push    ecx
        mov     ebx, [esp + 8]                ; freq; only BX needed
        mov     ecx, [esp + 12]               ; dur; only CX needed
        mov     dl, DevHlp_Beep
        jmp     far ptr code16_DevHlp32_Beep
code32_DevHlp32_Beep:
        jc      short @@errorBeep

        mov     eax, NO_ERROR
@@errorBeep:
        and     eax, 0000ffffh
        pop     ecx
        pop     ebx
        ret
DevHlp32_Beep endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_CloseEventSem
public        DevHlp32_CloseEventSem

;
; int DH32ENTRY DevHlp32_CloseEventSem(
;                        unsigned long handle        /* ebp + 8  */
;                       );
;
DevHlp32_CloseEventSem proc near
        push  ebx
        mov   eax, [esp + 8]                ; handle
        mov   dl, DevHlp_CloseEventSem
        jmp   far ptr code16_DevHlp32_CloseEventSem
code32_DevHlp32_CloseEventSem:
        jc short @@errorCloseEventSem

        mov   eax, NO_ERROR
@@errorCloseEventSem:
        and   eax, 0000ffffh
        pop   ebx
        ret
DevHlp32_CloseEventSem endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_EOI
public        DevHlp32_EOI

;
; int DH32ENTRY2 DevHlp32_EOI(
;                     unsigned short interrupt_level,   /* ax */
;                            );
;
DevHlp32_EOI proc near
        mov   dl, DevHlp_EOI
        jmp far ptr code16_DevHlp32_EOI
code32_DevHlp32_EOI:
        xor eax, eax
        ret
DevHlp32_EOI endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_FreeGDTSelector
public        DevHlp32_FreeGDTSelector

;
; int DH32ENTRY2 DevHlp32_FreeGDTSelector(
;                    unsigned short sel                  /* ax */
;                   );
;
DevHlp32_FreeGDTSelector proc near
        mov dl, DevHlp_FreeGDTSelector
        jmp far ptr code16_DevHlp32_FreeGDTSelector
code32_DevHlp32_FreeGDTSelector:
        jc short @@errorFreeGDTSelector
        mov eax, NO_ERROR
@@errorFreeGDTSelector:
        ret
DevHlp32_FreeGDTSelector endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_GetDosVar
public        DevHlp32_GetDosVar

;
; int DH32ENTRY DevHlp32_GetDosVar(
;                                  int     index,      /* ebp + 8  */
;                                  PTR16  *value,      /* ebp + 12 */
;                                  int     member      /* ebp + 16 */
;                                 );
;
DevHlp32_GetDosVar proc near
        enter 0, 0
        push ebx
        push esi
        push edi

        mov eax, [ebp + 8]          ; index
        mov ecx, [ebp + 16]         ; member
        mov dl, DevHlp_GetDOSVar
        jmp far ptr code16_DevHlp32_GetDosVar
code32_DevHlp32_GetDosVar:
        jc short @@errorGetDosVar                ; if error, EAX = error code
        mov edi, [ebp + 12]
        mov [edi], bx                       ; value (ofs)
        mov [edi + 2], ax               ; value (seg)
        mov eax, NO_ERROR
@@errorGetDosVar:
        pop edi
        pop esi
        pop ebx
        leave
        ret
DevHlp32_GetDosVar endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public DevHlp32_GetInfoSegs
public code32_DevHlp32_GetInfoSegs_1
public code32_DevHlp32_GetInfoSegs_2

;
; int DH32ENTRY DevHlp32_GetInfoSegs(
;                    struct InfoSeg_LDT **ppLocInfoSeg,   [ebp + 8]
;                    struct InfoSeg_GDT **ppSysInfoSeg    [ebp + 12]
;                   );
;
DevHlp32_GetInfoSegs proc near
        push ebp
        mov  ebp, esp
        push es
        push ebx
        push esi
        push edi

        mov esi, [ebp + 8]
        mov edi, [ebp + 8]

        mov eax, SAS_SEL
        mov es, eax
        xor ebx, ebx
        movzx ebx, word ptr es:[ebx].SAS_info_data
        movzx eax, word ptr es:[ebx].SAS_info_global
        xor esi, esi
        mov edx, DevHlp_VirtToLin
        jmp far ptr code16_DevHlp32_GetInfoSegs_1
code32_DevHlp32_GetInfoSegs_1:
        jc short @@errorGetInfoSegs
        mov ecx, [ebp + 8]
        mov [ecx], eax

        movzx esi, word ptr es:[ebx].SAS_info_local
        movzx eax, word ptr es:[ebx + 2].SAS_info_local
        mov edx, DevHlp_VirtToLin
        jmp far ptr code16_DevHlp32_GetInfoSegs_2
code32_DevHlp32_GetInfoSegs_2:
        jc short @@errorGetInfoSegs
        mov ecx, [ebp + 12]
        mov [ecx], eax


        mov eax, NO_ERROR
@@errorGetInfoSegs:
        pop edi
        pop esi
        pop ebx
        pop es
        leave
        ret
DevHlp32_GetInfoSegs endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public        DevHlp32_InternalError

;
; void DH32ENTRY DevHlp32_InternalError(
;                           char *msg,  /* ebp + 8  */
;                           int   msglen    /* ebp + 12 */
;                          );
;
;   Apparently, the error message must be in the DATA16
;   data segment.    V0.9.5 (2000-09-28) [umoeller]
;
DevHlp32_InternalError proc near
        enter 0, 0
        mov ax, seg DATA16
        mov es, ax
        mov ax, offset DATA16:panic_buffer
        movzx edi, ax
        mov esi, [ebp + 8]
        mov ecx, [ebp + 12]
        and ecx, 511
        rep movsb

        mov edi, [ebp + 12]
        and edi, 511
        push ds
        pop es
        mov ax, seg DATA16
        mov ds, ax
        mov si, offset DATA16:panic_buffer
        mov dl, DevHlp_InternalError
        jmp far ptr code16_DevHlp32_InternalError
DevHlp32_InternalError endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_LinToPageList
public        DevHlp32_LinToPageList

;
; int DH32ENTRY DevHlp32_LinToPageList(
;                                void            *lin,         /* ebp + 8  */
;                                unsigned long    size,        /* ebp + 12 */
;                                struct PageList *pages        /* ebp + 16 */
;                                unsigned long   *nr_pages,    /* ebp + 20 */
;                               );
;
DevHlp32_LinToPageList proc near
        push ebp
        mov  ebp, esp
        push ebx
        push edi

        mov   eax, [ebp + 8]                 ; lin
        mov   ecx, [ebp + 12]                ; size
        mov   edi, [ebp + 16]                ; Flags
        mov   edx, DevHlp_LinToPageList
        jmp far ptr code16_DevHlp32_LinToPageList
code32_DevHlp32_LinToPageList:
        jc short @@errorLinToPageList

        mov   ebx, [ebp + 20]                ; LinAddr
        cmp ebx, 0
        jz short @@outLinToPageList
        mov   [ebx], eax
@@outLinToPageList:
        mov   eax, NO_ERROR
@@errorLinToPageList:
        pop edi
        pop ebx
        leave
        ret
DevHlp32_LinToPageList endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_OpenEventSem
public        DevHlp32_OpenEventSem

;
; int DH32ENTRY DevHlp32_OpenEventSem(
;                        unsigned long handle        /* ebp + 8  */
;                       );
;
DevHlp32_OpenEventSem proc near
        push ebx
        mov eax, [esp + 8]                ; handle
        mov dl, DevHlp_OpenEventSem
        jmp far ptr code16_DevHlp32_OpenEventSem
code32_DevHlp32_OpenEventSem:
        jc short @@errorOpenEventSem

        mov   eax, NO_ERROR
@@errorOpenEventSem:
        and   eax, 0000ffffh
        pop ebx
        ret
DevHlp32_OpenEventSem endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_PageListToLin
public        DevHlp32_PageListToLin

;
; int DH32ENTRY DevHlp32_PageListToLin(
;                                      unsigned long     size,        /* ebp + 8 */
;                                      struct PageList  *pPageList,   /* ebp + 12 */
;                                      void            **pLin         /* ebp + 16 */
;                                     );
;
DevHlp32_PageListToLin proc near
    enter 0, 0
        push edi
    push ebx

        mov   ecx, [ebp + 8]                 ; size
        mov   edi, [ebp + 12]                ; pPageList
        mov   dl, DevHlp_PageListToLin
        jmp far ptr code16_DevHlp32_PageListToLin
code32_DevHlp32_PageListToLin:
        jc short @@errorPageListToLin

        mov   ebx, [ebp + 16]                ; pLin
        cmp ebx, 0
        jz short @@outPageListToLin
        mov   [ebx], eax
        mov   eax, NO_ERROR
@@errorPageListToLin:
        pop ebx
        pop edi
        leave
        ret

@@outPageListToLin:
        mov eax, ERROR_INVALID_PARAMETER
        jmp short @@errorPageListToLin
DevHlp32_PageListToLin endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_PostEventSem
public        DevHlp32_PostEventSem

;
; int DH32ENTRY DevHlp32_PostEventSem(
;                        unsigned long handle       /* ebp + 8  */
;                       );
;
DevHlp32_PostEventSem proc near
        push  ebx
        mov   eax, [esp + 8]                 ; handle
        mov   dl, DevHlp_PostEventSem
        jmp   far ptr code16_DevHlp32_PostEventSem
code32_DevHlp32_PostEventSem:
        jc short @@errorPostEventSem

        mov   eax, NO_ERROR
@@errorPostEventSem:
        and   eax, 0000ffffh
        pop   ebx
        ret
DevHlp32_PostEventSem endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_ProcBlock
public        DevHlp32_ProcBlock

;
; int DH32ENTRY DevHlp32_ProcBlock(
;                          unsigned long  eventid,       /* bp + 8  */
;                          long           timeout,       /* bp + 12 */
;                          short          interruptible  /* bp + 16 */
;                         );
;
DevHlp32_ProcBlock proc near
        push ebp
        mov  ebp, esp
        push ebx
        push edi
        mov bx, [ebp + 8]                 ; eventid_low
        mov ax, [ebp + 10]                ; eventid_high
        mov di, [ebp + 14]                ; timeout_high
        mov cx, [ebp + 12]                ; timeout_low
        mov dh, [ebp + 16]                ; interruptible_flag
        mov dl, DevHlp_ProcBlock
        jmp far ptr code16_DevHlp32_ProcBlock
code32_DevHlp32_ProcBlock:
;        mov ah, 0
        and eax, 0FFh
        pop edi
        pop ebx
        leave
        ret
DevHlp32_ProcBlock endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_ProcRun
public        DevHlp32_ProcRun

;
; int DH32ENTRY DevHlp32_ProcRun(
;                        unsigned long eventid        /* ebp + 8  */
;                       );
;
DevHlp32_ProcRun proc near
        push ebx
        mov bx, [esp + 8]                 ; eventid_low
        mov ax, [esp + 10]                ; eventid_high
        mov dl, DevHlp_ProcRun
        jmp far ptr code16_DevHlp32_ProcRun
code32_DevHlp32_ProcRun:
        pop ebx
        ret
DevHlp32_ProcRun endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_ResetEventSem
public        DevHlp32_ResetEventSem

;
; int DH32ENTRY DevHlp32_ResetEventSem(
;                        unsigned long handle        /* ebp + 8  */
;            int *nposts             /* ebp +12 */
;                       );
;
DevHlp32_ResetEventSem proc near
        push  ebx
        push  edi
        mov   eax, [esp + 8]                 ; handle
        mov   edi, [esp + 12]            ; address of var for posts
        mov   dl, DevHlp_ResetEventSem
        jmp   far ptr code16_DevHlp32_ResetEventSem
code32_DevHlp32_ResetEventSem:
        jc short @@errorResetEventSem
        mov eax, NO_ERROR
@@errorResetEventSem:
        and   eax,0000ffffh
        pop   edi
        pop   ebx
        ret
DevHlp32_ResetEventSem endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public        DevHlp32_SaveMessage
public code32_DevHlp32_SaveMessage_1
public code32_DevHlp32_SaveMessage_2
public code32_DevHlp32_SaveMessage_3
public code32_DevHlp32_SaveMessage_4

;
; int DH32ENTRY DevHlp32_SaveMessage(
;                                    char *msg,         /* ebp + 8  */
;                                    int   len          /* ebp + 12 */
;                                   );
;
DevHlp32_SaveMessage proc near
        enter   10, 0
        push    ds
        push    es
        push    ebx
        push    esi
        push    edi
;
; ebp - 4  -> msg16  (dword)
; ebp - 6  -> nr     (word)
; ebp - 8  -> msgid  (word)
; ebp - 10 -> rc     (word)
;

        ;
        ; rc = 0
        ;
        mov     word ptr [ebp - 10], 0

        mov     ax, seg DATA16
        mov     es, ax
ASSUME ES:DATA16
        mov     di, offset DATA16:msg_seg
        mov     ecx, 1                              ; one selector
        mov     dl, DevHlp_AllocGDTSelector
        jmp     far ptr code16_DevHlp32_SaveMessage_1
code32_DevHlp32_SaveMessage_1:
        jc      @@errorSaveMessage

        mov     ax, es:msg_seg                      ; sel
        mov     ebx, [ebp + 8]                      ; msg
        mov     ecx, [ebp + 12]                     ; len
        mov     dl, DevHlp_LinToGDTSelector
        jmp     far ptr code16_DevHlp32_SaveMessage_2
code32_DevHlp32_SaveMessage_2:
        mov     [ebp - 10], ax                       ; rc
        jc      short @@error_2SaveMessage

        mov     ax, es               ; DATA16
        mov     bx, ds               ; FLAT
        mov     ds, ax
        mov     es, bx
ASSUME DS:DATA16, ES:FLAT
        mov     si, offset DATA16:msgid
        xor     bx, bx
        mov     dl, DevHlp_Save_Message
        jmp     far ptr code16_DevHlp32_SaveMessage_3
code32_DevHlp32_SaveMessage_3:
        jc      @@error_2SaveMessage
        mov     [ebp - 10], ax                       ; rc

@@error_2SaveMessage:
        mov     ax, msg_seg
        push    es
        pop     ds                   ; FLAT
    ASSUME DS:FLAT, ES:FLAT
        mov     dl, DevHlp_FreeGDTSelector
        jmp     far ptr code16_DevHlp32_SaveMessage_4
code32_DevHlp32_SaveMessage_4:
        jc      @@errorSaveMessage

        mov     ax, [ebp - 10]                    ; rc
        cmp     ax, 0
        jnz     @@errorSaveMessage

@@errorSaveMessage:
        movzx   eax, ax
        pop     edi
        pop     esi
        pop     ebx
        pop     es
        pop     ds
        leave
        ret
DevHlp32_SaveMessage endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_Security
public        DevHlp32_Security

;
; int DH32ENTRY DevHlp32_Security(
;                 unsigned long   func,     /* ebp + 8  */
;                                 void           *ptr       /* ebp + 12 */
;                                );
;
DevHlp32_Security proc near
        push ebp
        mov  ebp, esp
        push ebx

        mov ecx, [ebp + 12]   ; ptr
        mov eax, [ebp + 8]    ; func
        mov edx, DevHlp_Security
        jmp far ptr code16_DevHlp32_Security
code32_DevHlp32_Security:
        mov eax, NO_ERROR
        pop ebx
        leave
        ret
DevHlp32_Security endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_setIRQ
public        DevHlp32_setIRQ

;
; int DH32ENTRY DevHlp32_setIRQ(
;                               unsigned short offset_irq,  /* ebp + 8  */
;                       unsigned short interrupt_level, /* ebp + 12 */
;                       unsigned short sharing_flag,    /* ebp + 16 */
;                       unsigned short default_dataseg  /* ebp + 20 */
;                              );
;
DevHlp32_setIRQ proc near
        enter 0, 0
        push ds
        push ebx
        mov ax, [ebp + 8]
        mov bx, [ebp + 12]
        mov dh, [ebp + 16]
        mov cx, [ebp + 20]
        mov ds, cx
        mov   dl, DevHlp_SetIRQ
        jmp far ptr code16_DevHlp32_setIRQ
code32_DevHlp32_setIRQ:
        jc short @@errorsetIRQ
        mov ax, NO_ERROR
@@errorsetIRQ:
        movzx eax, ax
        pop ebx
        pop ds
        leave
        ret
DevHlp32_setIRQ endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_UnSetIRQ
public        DevHlp32_UnSetIRQ

;
; int DH32ENTRY DevHlp32_UnSetIRQ(
;                                 unsigned short interrupt_level,   /* ebp + 8  */
;                         unsigned short data16_segment     /* ebp + 12 */
;                                );
;
DevHlp32_UnSetIRQ proc near
        enter 0, 0
        push ds
        push ebx
        mov bx, [ebp + 8]
        mov cx, [ebp + 12]
        mov ds, cx
        mov   dl, DevHlp_UnSetIRQ
        jmp far ptr code16_DevHlp32_UnSetIRQ
code32_DevHlp32_UnSetIRQ:
        xor eax, eax
        pop ebx
        pop ds
        leave
        ret
DevHlp32_UnSetIRQ endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_VerifyAccess
public        DevHlp32_VerifyAccess

;
; int DH32ENTRY DevHlp32_VerifyAccess(
;                                PTR16          Seg,      /* ebp + 8  */
;                                unsigned short Size,     /* ebp + 12 */
;                                int            Flags     /* ebp + 16 */
;                               );
;
DevHlp32_VerifyAccess proc near
        push  ebp
        mov   ebp, esp
        push  ebx
        push  ecx
        push  edi

        mov   di, [ebp + 8]                 ; Off
        mov   ax, [ebp + 10]                ; Seg
        mov   cx, [ebp + 12]                ; Size
        mov   dh, [ebp + 16]                ; Flags
        mov   dl, DevHlp_VerifyAccess
        jmp far ptr code16_DevHlp32_VerifyAccess
code32_DevHlp32_VerifyAccess:
        jc short @@errorVerifyAccess

        mov   eax, NO_ERROR
@@errorVerifyAccess:
        and   eax, 0000ffffh
        pop   edi
        pop   ecx
        pop   ebx
        leave
        ret
DevHlp32_VerifyAccess endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_VirtToLin
public        DevHlp32_VirtToLin

;
; int DH32ENTRY DevHlp32_VirtToLin(
;      PTR16  virt  [ebp + 8]
;      void **plin  [ebp + 12]
;     );
;
DevHlp32_VirtToLin proc near
        push ebp
        mov  ebp, esp
        push esi
        push edi

        movzx esi, word ptr[ebp + 8]        ; ofs
        mov ax, word ptr[ebp + 10]          ; seg
        mov dl, DevHlp_VirtToLin
        jmp far ptr code16_DevHlp32_VirtToLin
code32_DevHlp32_VirtToLin:
        jc short @@errorVirtToLin           ; if error, EAX = error code
        mov edi, [ebp + 12]        ; plin
        mov [edi], eax             ;*plin
        mov eax, NO_ERROR
@@errorVirtToLin:
        pop edi
        pop esi
        leave
        ret
DevHlp32_VirtToLin endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_VMAlloc
public        DevHlp32_VMAlloc

;
; int DH32ENTRY DevHlp32_VMAlloc(
;                                unsigned long  Length,      /* ebp + 8  */
;                                unsigned long  PhysAddr,    /* ebp + 12 */
;                                unsigned long  Flags,       /* ebp + 16 */
;                                void         **LinAddr      /* ebp + 20 */
;                               );
;
DevHlp32_VMAlloc proc near
        push ebp
        mov  ebp, esp
        push ebx
        push edi

        mov   ecx, [ebp + 8]                 ; Length
        mov   edi, [ebp + 12]                ; PhysAddr
        mov   eax, [ebp + 16]                ; Flags
        mov   dl, DevHlp_VMAlloc
        jmp far ptr code16_DevHlp32_VMAlloc
code32_DevHlp32_VMAlloc:
        jc short @@errorVMAlloc

        mov   ebx, [ebp + 20]                ; LinAddr
        mov   [ebx], eax
        mov   eax, NO_ERROR
@@errorVMAlloc:
        pop edi
        pop ebx
        leave
        ret
DevHlp32_VMAlloc endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_VMFree
public        DevHlp32_VMFree

;
; int _Optlink VMFree_32(
;                        void *addr        /* eax */
;                       );
;
DevHlp32_VMFree proc near
        mov   dl, DevHlp_VMFree
        jmp far ptr code16_DevHlp32_VMFree
code32_DevHlp32_VMFree:
        ret
DevHlp32_VMFree endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_VMGlobalToProcess
public        DevHlp32_VMGlobalToProcess

;
; int _Optlink VMGlobalToProcess_32( ULONG ulFlags,          /* ebp + 8  */
;                                    LIN LinearAddr,         /* ebp + 12 */
;                                    ULONG Length,           /* ebp + 16 */
;                                    PLIN ProcessLinearAddr  /* ebp + 20 */
;                       );
;
DevHlp32_VMGlobalToProcess proc near
        push ebp
        mov  ebp, esp
        push ebx
        push edi

        mov   ebx, [ebp + 12]                ; LinearAddr
        mov   ecx, [ebp + 16]                ; Length
        mov   eax, [ebp + 8]                 ; ActionFlags
        mov   dl, DevHlp_VMGlobalToProcess
        jmp far ptr code16_DevHlp32_VMGlobalToProcess
code32_DevHlp32_VMGlobalToProcess:
        jc short @@errorVMGlobalToProcess
                ; on error (C set): eax has error code
                ; no error (C clear): eax has linear address in process space
        ; no error:
        mov   ebx, [ebp + 20]                ; LinAddr
        mov   [ebx], eax
        mov   eax, NO_ERROR
@@errorVMGlobalToProcess:
        pop edi
        pop ebx
        leave
        ret
DevHlp32_VMGlobalToProcess endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_VMLock
public        DevHlp32_VMLock

;
; int DH32ENTRY DevHlp32_VMLock(
;                    unsigned long flags,                 [ebp + 8]
;                    unsigned long lin,                   [ebp + 12]
;                    unsigned long length,                [ebp + 16]
;                    void          *pPageList,            [ebp + 20]
;                    void          *pLockHandle,          [ebp + 24]
;                    unsigned long *pPageListCount        [ebp + 28]
;                   );
;
DevHlp32_VMLock proc near
        push ebp
        mov  ebp, esp
        push ebx
        push esi
        push edi

        mov ebx, [ebp + 12]        ; lin
        mov ecx, [ebp + 16]        ; length
        mov edi, [ebp + 20]        ; pPageList    (FLAT)
        mov esi, [ebp + 24]        ; pLockHandle  (FLAT)
        mov eax, [ebp + 8]         ; flags
        mov dl, DevHlp_VMLock
        jmp far ptr code16_DevHlp32_VMLock
code32_DevHlp32_VMLock:
        jc short @@errorVMLock           ; if error, EAX = error code
        mov edi, [ebp + 28]        ; pPageListCount
        mov [edi], eax             ;*pPageListCount
        mov eax, NO_ERROR
@@errorVMLock:
        pop edi
        pop esi
        pop ebx
        leave
        ret
DevHlp32_VMLock endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_VMProcessToGlobal
public        DevHlp32_VMProcessToGlobal

;
; int _Optlink VMProcessToGlobal_32( ULONG ulFlags,          /* ebp + 8  */
;                                       --  MDHPG_READONLY - If bit 0 is clear, its
;                                           mapping will be read only.
;                                       --  VMDHPG_WRITEV  - If bit 0 is set, the
;                                           mapping created will be writable.
;                                    LIN LinearAddr,         /* ebp + 12 */
;                                       linear addr in process addr space; must
;                                       be on page boundary
;                                    ULONG Length,           /* ebp + 16 */
;                                    PLIN GlobalLinearAddr   /* ebp + 20 */
;                       );
;   This service converts an address in the current process address space to
;   an address in the system region of the global address space.
;
;   The address range must be on a page boundary and must not cross object
;   boundaries.  This call copies the linear mapping from the process's address
;   space to the system-shared address space.  This allows the physical
;   device driver to access the data independent of the context of the
;   current process. The following figure shows the mapping that is performed
;   when a physical device driver calls VMProcessToGlobal.
;
;   The following figure shows Mapping Performed by VMProcessToGlobal.
;
;
;            Before                                       After
;   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ 4 Gigabytes ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;   ³                       ³             ³                       ³
;   ³                       ³             ³                       ³
;   ³     System Region     ³             ³     System Region     ³
;   ³                       ³             ³                       ³
;   ³                       ³             ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
;   ³                       ³             ³       Object A        ³
;   ³                       ³             ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´
;   ³                       ³             ³                       ³
;   ÃÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ´             ÃÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄ´
;   ³       ³       ³       ³             ³       ³       ³       ³
;   ³Process³Process³Process³             ³Process³Process³Process³
;   ³   A   ³   B   ³   C   ³             ³   A   ³   B   ³   C   ³
;   ³Address³Address³Address³             ³Address³Address³Address³
;   ³ Space ³ Space ³ Space ³             ³ Space ³ Space ³ Space ³
;   ÃÄÄÄÄÄÄÄ´       ³       ³             ÃÄÄÄÄÄÄÄ´       ³       ³
;   ³ Obj A ³       ³       ³             ³ Obj A ³       ³       ³
;   ÃÄÄÄÄÄÄÄ´       ³       ³             ÃÄÄÄÄÄÄÄ´       ³       ³
;   ³       ³       ³       ³             ³       ³       ³       ³
;   ÀÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÙ      0      ÀÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÙ
;
;  Mapping Performed by VMProcessToGlobal
;
;  The following steps show to use the DevHlp services to gain interrupt
;  time access to the buffer of a process:
;
;     1. If the user's buffer was allocated in the Local Descriptor Table (LDT)
;        tiled region, use the SelToFlat macro to convert the address to a
;        linear address.
;
;     2. Call VMLock to verify the address and to lock the range of memory
;        needed into physical memory.
;
;     3. Call VMProcessToGlobal to map the private address of a process into
;        global address space.  See the previous figure for more details on the
;        mapping performed.
;
;     4. If the physical device driver requests it, an array of physical
;        addresses corresponding to the locked region is returned.
;
;     5. Access the memory using the FLAT selector and its linear address as
;        returned by VMProcessToGlobal.
;
;     6. Call VMFree to remove global mapping to process address space.
;
;     7. Call VMUnLock to unlock the object.
;
DevHlp32_VMProcessToGlobal proc near
        push ebp
        mov  ebp, esp
        push ebx
        push edi

        mov   ebx, [ebp + 12]                ; LinearAddr
        mov   ecx, [ebp + 16]                ; Length
        mov   eax, [ebp + 8]                 ; ActionFlags
        mov   dl, DevHlp_VMProcessToGlobal
        jmp far ptr code16_DevHlp32_VMProcessToGlobal
code32_DevHlp32_VMProcessToGlobal:
        jc short @@errorVMProcessToGlobal
                ; on error (C set): eax has error code
                ; no error (C clear): eax has global ofs to region of memory
        ; no error:
        mov   ebx, [ebp + 20]                ; GlobalLinAddr
        mov   [ebx], eax
        mov   eax, NO_ERROR
@@errorVMProcessToGlobal:
        pop edi
        pop ebx
        leave
        ret
DevHlp32_VMProcessToGlobal endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_VMUnlock
public        DevHlp32_VMUnlock

;
; int DH32ENTRY DevHlp32_VMUnlock(
;                                 void *pLockHandle   /* ebp + 8 */
;                                );
;
DevHlp32_VMUnlock proc near
        push ebp
        mov  ebp, esp
        push esi

        mov esi, [ebp + 8]        ; pLockHandle  (FLAT)
        mov edx, DevHlp_VMUnlock
        jmp far ptr code16_DevHlp32_VMUnlock
code32_DevHlp32_VMUnlock:
        jc short @@errorVMUnlock           ; if error, EAX = error code
        mov eax, NO_ERROR
@@errorVMUnlock:
        pop esi
        leave
        ret
DevHlp32_VMUnlock endp

ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:NOTHING
public code32_DevHlp32_Yield
public        DevHlp32_Yield

;
; void DH32ENTRY DevHlp32_Yield(void);
;
DevHlp32_Yield proc near
        mov   dl, DevHlp_Yield
        jmp far ptr code16_DevHlp32_Yield
code32_DevHlp32_Yield:
        ret
DevHlp32_Yield endp


CODE32 ends

        end

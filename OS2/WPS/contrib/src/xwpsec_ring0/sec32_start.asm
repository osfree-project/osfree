;
; sec32_start.asm:
;       this contains the head of the driver binary, most importantly,
;       the device driver header and the 16-bit strategy routine,
;       which calls the 32-bits sec32_strategy() routine.
;
;       This is linked FIRST by the makefile to make sure it appears
;       at the start of the binary.
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
        include devcmd.inc
        include devsym.inc

        include sec32_segdef.inc
        include r0thunk.inc

; Definition of the request packet header.

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

; *****************************************************
; *
; *     DATA16
; *
; *****************************************************

DATA16 segment
        extrn data16_end : byte

        public device_header

; *****************************************************
; *      Device Driver Header
; *****************************************************
device_header   dd -1                               ; far pointer to next driver
                dw 1000100110000000b                ; device attributes
;                  ³³³³³ ÃÄ´   ³³³³
;                  ³³³³³ ³ ³   ³³³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 0: is STDIN
;                  ³³³³³ ³ ³   ³³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 1: is STDOUT
;                  ³³³³³ ³ ³   ³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 2: is NULL
;                  ³³³³³ ³ ³   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 3: is CLOCK
;                  ³³³³³ ³ ³
;                  ³³³³³ ³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂ 7-9: -- (001) OS/2
;                  ³³³³³ ³                          ³      -- (010) DosDevIOCtl2 + SHUTDOWN
;                  ³³³³³ ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁ      -- (011) level 3: capabilities bit strip (below)
;                  ³³³³³
;                  ³³³³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 11:
;                  ³³³³                               for char DDs: if 1, driver req's. open/close
;                  ³³³³                               for block DDs: if 1, driver handles removeable media
;                  ³³³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 12: sharing support:
;                  ³³³                                if 0, DD provides contention control itself
;                  ³³³                                if 1, file-system sharing rules apply to the device
;                  ³³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 13: block-DDs only:
;                  ³³                                 if 1, use BPB
;                  ³³                                 if 0 = use media descriptor byte
;                  ³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 14: IDC capability
;                  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 15: device type bit: 1 = char, 0 = block

                dw offset CODE16:sec32_stub_strategy; 16-bit offset to strategy routine
                dw 0                                ; 16-bit offset to IDC routine

                db 'XWPSEC$ '                       ; device driver name
                db 8 dup (0)                        ; reserved
                dw 0000000000011011b                ; capabilities bit strip (level 3 PDDs only)
;                             ³³³³³
;                             ³³³³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 0: DosDevIOCtl2 + Shutdown
;                             ³³³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 1: char only: if set, DD supports mem above 16 MB
;                             ³³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 2: DD supports parallel ports
;                             ³ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 3: DD participates in ADD strategy (alternate INIT package)
;                             ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ 4: DD supports InitComplete
                dw 0000000000000000b
DATA16 ends

; *****************************************************
; *
; *     CODE16
; *
; *****************************************************

CODE16 segment
        assume cs:CODE16, ds:DATA16

        extrn code16_end : byte

        public sec32_stub_strategy
;        public sec32_stub_IDC

;
; sec32_stub_strategy:
;       16-bit strategy routine, which calls
;       the 32-bit version.
;

sec32_stub_strategy proc far
        movzx eax, byte ptr es:[bx].reqCommand
        cmp eax, 0
        jz short @@error

        ; call int DRV32ENTRY sec32_strategy(PTR16 reqpkt, int index)
        push es                                 ; seg reqpkt
        push bx                                 ; ofs reqpkt
        push eax                                ; command
        mov word ptr es:[bx].reqStatus, 0       ; updates the request status
        call far ptr FLAT:SEC32_STRATEGY        ; 32 bits strategy entry point
                                                ; (sec32_strategy.c)
                                                ; _Far32 _Pascal _loadds,
                                                ; callee cleans up stack
        mov word ptr es:[bx].reqStatus, ax      ; updates the request status
        retf

@@error:
        ;
        ; Cannot be initialized as a DEVICE statement. MUST be
        ; initialized as a BASEDEV statement. (ring 0 FLAT CS is
        ; unreachable at DEVICE INIT time ...)
        ;
        mov dword ptr es:[bx].i_devHelp, 0
        mov  word ptr es:[bx].reqStatus, STDON + STERR + ERROR_I24_BAD_COMMAND
        retf
sec32_stub_strategy endp

;sec32_stub_IDC proc far
;        call far ptr FLAT:SEC32_IDC
;        retf
;sec32_stub_IDC endp

;*********************************************************************************************
;**************** Everything below this line will be unloaded after init *********************
;*********************************************************************************************
end_of_code:

CODE16 ends

CODE32 segment
ASSUME CS:FLAT, DS:FLAT, ES:FLAT

        public code32_begin

        extrn  SEC32_STRATEGY  : far
;        extrn  SEC32_IDC       : far

code32_begin:

CODE32 ends

DATA32 segment
        public  codeend
        public  dataend
        codeend dw offset CODE16:code16_end
        dataend dw offset DATA16:data16_end

DATA32 ends

BSS32 segment
        public  data32_begin
        data32_begin dd (?)
BSS32 ends

end


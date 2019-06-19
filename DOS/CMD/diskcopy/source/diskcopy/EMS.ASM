;;
;;   EMS.ASM - routines to use Extended Memory from a DOS program.
;;
;;   Copyright (C) 2000, 2001 Imre Leber.
;;
;;   This program is free software; you can redistribute it and/or modify
;;   it under the terms of the GNU General Public License as published by
;;   the Free Software Foundation; either version 2 of the License, or
;;   (at your option) any later version.
;;
;;   This program is distributed in the hope that it will be useful,
;;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;   GNU General Public License for more details.
;;
;;   You should have recieved a copy of the GNU General Public License
;;   along with this program; if not, write to the Free Software
;;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;;
;;   If you have any questions, comments, suggestions, or fixes please
;;   email me at:  imre.leber@worlonline.be
;;
;;
;;*************************************************************************
;; Routines to use Epanded Memory from a DOS program.
;;
;; NOTE: translation from the EMS routines by Cliff Rhodes to NASM by
;;       Imre Leber.
;;
;; The C version was released to the public domain by Cliff Rhodes with
;; no guarantees of any kind.
;;
;; The assembly version is hereby put under GNU General Public License by
;; Imre Leber.
;;
;;*************************************************************************
;; Last changed:
;;
;;      april 16, 2001: Fixed situation where si and di not saved.
;;

%assign EMS_INT      67h      ;; EMS interrupt number.
%assign EMS_VERSION  32h      ;; Version 3.2 of EMS. 
%assign RES_VERSION  40h      ;; At least version 4.0 to be resizable.

; EMS service codes.
%assign EMSservice1  40h    ;; Get EMS status.
%assign EMSservice2  41h    ;; Get segment address of page 0. 
%assign EMSservice3  42h    ;; Get total number of expanded pages.
%assign EMSservice4  43h    ;; Get handle and assign pages to it.
%assign EMSservice5  44h    ;; Map a page into one of the page frames.
%assign EMSservice6  45h    ;; Close EMS handle.
%assign EMSservice7  46h    ;; Get the EMS version number.

;=============================== DATA =====================================

segment _DATA class=DATA

%assign ID_LEN 8/2
        EMS_ID db "EMMXXXX0" ;; EMS identification string.

;=============================== CODE =====================================

segment _TEXT class=CODE

;==========================================================================
;===                           EMSbaseaddress                           ===
;==========================================================================
;=== unsigned int EMSbaseaddress(void);                                 ===
;===                                                                    ===
;=== Determines if EMS present. If so returns base segment of EMS.      ===
;=== Returns 0 if EMS not available. The base segment is necessary      ===
;=== for mapping EMS memory pages into the user address space (see      ===
;=== EMSmap() below).                                                   ===
;==========================================================================

        global _EMSbaseaddress
_EMSbaseaddress:
        push  es
	push  si
	push  di

        cld

        mov  ax, 3567h
        int  21h

        mov  si, EMS_ID
        mov  cx, ID_LEN
        mov  di, 10              ;; EMS_ID must be at offset 10

        repe cmpsw
        je   .next

        xor  ax, ax
        jmp  short .EndOfProc

.next:
        mov  ah, EMSservice2     ;; Get page frame segment.
        int  EMS_INT

        cmp  ah, 0
        je   .GotFrame

        xor  ax, ax
        jmp  short .EndOfProc

.GotFrame:
        mov  ax, bx
        
.EndOfProc:
	pop  di
	pop  si
        pop  es
        ret

;==========================================================================
;===                              EMSversion                            ===
;==========================================================================
;=== int EMSversion(void);                                              ===
;===                                                                    ===
;=== Returns current EMS version, -1 if not found or obsolete.          ===
;==========================================================================

        global _EMSversion
_EMSversion:

        mov  ah, EMSservice7
        int  EMS_INT

        cmp  ah, 0
        jne  .NotGood

        cmp  al, EMS_VERSION
        jb   .NotGood
        jmp  short .EndOfProc

.NotGood:
        mov  ax, -1

.EndOfProc:
        ret

;==========================================================================
;===                             EMSstatus                              ===
;==========================================================================
;=== int  EMSstatus(void);                                              ===
;===                                                                    ===
;=== Returns 0 if EMS system OK, -1 if not.                             ===
;==========================================================================

        global _EMSstatus
_EMSstatus:

        mov  ah, EMSservice1
        int  EMS_INT

        cmp  ah, 0
        je   .next1

        mov  ax, -1
        jmp  short .EndOfProc

.next1:
        xor  ax, ax

.EndOfProc:
        ret

;==========================================================================
;===                             EMSpages                               ===
;==========================================================================
;=== int EMSpages(void);                                                ===
;===                                                                    ===
;=== Returns number of free EMS pages (each page is 16k), -1 if error.  === 
;==========================================================================

        global _EMSpages
_EMSpages:

        mov  ah, EMSservice3
        int  EMS_INT

        cmp  ah, 0
        je   .next

        mov  ax, -1
        jmp  short .EndOfProc

.next:
        mov  ax, bx

.EndOfProc:
        ret

;==========================================================================
;===                              EMSalloc                              ===
;==========================================================================
;=== int EMSalloc(int pages);                                           ===
;===                                                                    ===
;=== Returns handle to block of size pages or -1 if error.              ===
;===                                                                    ===
;=== NOTE: always free any handles when you are done!.                  ===
;==========================================================================

        global _EMSalloc
_EMSalloc:
        mov  bx, sp

        mov  ah, EMSservice4
        mov  bx, [ss:bx+02h]
        int  EMS_INT

        cmp  ah, 0
        je   .next

        mov  ax, -1
        jmp  short .EndOfProc

.next:
        mov  ax, dx

.EndOfProc:
        ret

;==========================================================================
;===                              EMSfree                               ===
;==========================================================================
;=== int EMSfree(int handle);                                           ===
;===                                                                    ===
;=== Frees handle block, returns 0 if successful, -1 if error.          ===
;==========================================================================

        global _EMSfree
_EMSfree:
        mov   bx, sp

        mov  ah, EMSservice6
        mov  dx, [ss:bx+02h]
        int  EMS_INT

        cmp  ah, 0
        je   .next

        mov  ax, -1
        jmp  short .EndOfProc

.next:
        xor  ax, ax

.EndOfProc:
        ret

;=============================================================================
;===                                EMSmap                                 ===
;=============================================================================
;=== int  EMSmap(int bank, int handle, int page);                          ===
;===                                                                       ===
;=== Maps page of handle into bank. Returns 0 if successful, -1 if error.  ===
;=== Each handle controls 1 or more 16k pages of EMS memory.               ===
;=== There are four banks 0-3. bank 0 starts at the segment returned by    ===
;=== EMSbaseaddress(), bank 1 starts at that segment with offset 16k, etc. ===
;=============================================================================

        global _EMSmap
_EMSmap:
        push bp
        mov  bp, sp

        mov  ax, [bp+04h]      ; bank.
        mov  bx, [bp+08h]      ; page.
        mov  dx, [bp+06h]      ; handle.
        mov  ah, EMSservice5
        int  EMS_INT

        cmp  ah, 0
        je   .EndOfProc

        mov  ax, -1

.EndOfProc:
        pop  bp
        ret

%if 0
;=========================================================================
;===                            EMSResizable                           ===
;=========================================================================
;=== int EMSResizable();                                               ===
;===                                                                   ===
;=== Returns wether the pages allocated for a certain handle can be    ===
;=== changed.                                                          ===
;=========================================================================

        global _EMSResizable
_EMSResizable:

        mov  ah, EMSservice7                    ;; Get version number, 
        int  EMS_INT

        cmp  al, RES_VERSION                    ;; and see if it is at 
        jb   .NotSupported                      ;; least version 4.0.

        mov  ax, 1
        jmp  short .EndOfProc

.NotSupported:
        xor  ax, ax

.EndOfProc:
        ret

;=========================================================================
;===                            EMSResize                              ===
;=========================================================================
;=== int EMSResize(int handle, int pages);                             ===
;===                                                                   ===
;=== Change the amount of pages allocated for a certain handle.        ===
;===                                                                   ===
;=== Remark: check first wether this function is supported.            ===
;===                                                                   ===
;=== Returns: 0 on success, -1 or error.                               ===
;=========================================================================

        global _EMSResize
_EMSResize:

        push bp
        mov  bp, sp

        mov  ah, 51h
        mov  dx, [bp+04h]
        mov  bx, [bp+06h]
        int  EMS_INT

        cmp  ah, 0
        je   .next

        mov  ax, -1
        jmp  short .EndOfProc

.next:
        xor  ax, ax

.EndOfProc:
        pop  bp
        ret
%endif

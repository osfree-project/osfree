;    
;   hicritcl.asm - the effective critical handler for this application.
;   Copyright (C) 2000, 2001 Imre Leber
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;
;   If you have any questions, comments, suggestions, or fixes please
;   email me at:  imre.leber@worlonline.be
;

%assign FAIL 3              ;; fail on critical error.

extern _SetCriticalHandler

segment _DATA class=DATA

        CriterrOccured DB 0 ;; has there been a critical error. 

        status  DB 0        ;; status byte.                     
        cause   DB 0        ;; cause of error.                  

segment _TEXT class=CODE

;***********************************************************************
;***                     criticalhandler                             ***
;***********************************************************************
;*** Our critical error handler.                                     ***
;***                                                                 ***
;*** Remembers status and cause of critical error.                   ***
;***********************************************************************

criticalhandler:
        mov  bx, sp
        mov  ax, [ss:bx+02h]

        mov  [status], ah              ;; Save status byte.
        mov  [cause],  al              ;; Save cause of error.
        mov  [CriterrOccured], byte 1  ;; Remember that a critical 
                                       ;; error has occured.
        mov  ax, FAIL
        ret

;***********************************************************************
;***                     CriticalHandlerOn                           ***
;***********************************************************************
;*** void CriticalHandlerOn(void);                                   ***
;***                                                                 ***
;*** Installs our critical error handler.                            ***
;***********************************************************************

        global _CriticalHandlerOn
_CriticalHandlerOn:

        mov  ax, criticalhandler       ;; Point the real critical handler
        push ax                        ;; to our handler.
        call _SetCriticalHandler
        pop  ax
        
        mov  [CriterrOccured], byte 0  ;; Make sure that there is no pending
                                       ;; critical error.
        ret

;***********************************************************************
;***                     CriticalErrorOccured                        ***
;***********************************************************************
;*** int CriticalErrorOccured(void);                                 ***
;***                                                                 ***
;*** Returns wether there has been a critical error.                 ***
;***                                                                 ***
;*** Remark: can only be called once.                                ***
;***********************************************************************
        
        global _CriticalErrorOccured
_CriticalErrorOccured:

        xor  ax, ax
        mov  al, [CriterrOccured]      ;; See if there hasn't been a critical
                                       ;; error.
        mov  [CriterrOccured], byte 0
        ret

%if 0
;***********************************************************************
;***                     GetCriticalCause                            ***
;***********************************************************************
;*** int GetCriticalCause(void);                                     ***
;***                                                                 ***
;*** Returns the cause of the critical error.                        ***
;***********************************************************************

        global _GetCriticalCause
_GetCriticalCause:

        xor  ax, ax                    ;; Return critical error cause.
        mov  al, [cause]
        ret

;***********************************************************************
;***                     GetCriticalStatus                           ***
;***********************************************************************
;*** int GetCriticalStatus(void);                                    ***
;***                                                                 ***
;*** Returns the status byte for the critical error.                 ***
;***********************************************************************

        global _GetCriticalStatus
_GetCriticalStatus:

        xor  ax, ax
        mov  al, [status]              ;; Return status byte.
        ret
%endif

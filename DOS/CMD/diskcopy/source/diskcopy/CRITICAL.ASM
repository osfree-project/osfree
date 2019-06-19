;    
;   Critical.asm - critical error handler.
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


segment _DATA class=DATA

        handler     DW 0
        
segment _TEXT class=CODE

;************************************************************************
;***                    call_real_handler                             ***
;************************************************************************
;*** Private function to encapsulate the critical error handler.      ***
;************************************************************************

call_real_handler:
        push bx
        push cx
        push dx
        push si              ;; Save all registers.
        push di
        push es
        push ss
        push ds
        
        sti
        mov  dx, _DATA       ;; Set data segment to _DATA.
        mov  ds, dx          

        and  di, 0FFh        ;; Prepare the parameter to
        xor  al, al          ;; give to the real handler.
        add  ax, di

        push ax              ;; Call real handler.
        call [handler]
        pop  dx                        ;; take parameter from the stack.

        pop  ds
        pop  ss
        pop  es              ;; Restore all registers (except AX = retval.).
        pop  di
        pop  si
        pop  dx
        pop  cx
        pop  bx
        iret

;************************************************************************
;***                         SetCriticalHandler                       ***
;************************************************************************
;*** void SetCriticalHandler(int (*handler)(int status));             ***
;***                                                                  ***
;*** Installs the critical error handler and sets the routine to call ***
;*** to handler.                                                      ***
;************************************************************************

        global _SetCriticalHandler      
_SetCriticalHandler:
        push bp
        mov  bp, sp
        push es
        push ds
 
        mov  ax, [bp+04h]               ; Get real handler and
        mov  [handler], ax              ; save it.

        mov  dx, call_real_handler      
        push cs
        pop  ds                         ; Install the new critical handler.
        mov  ax, 2524h
        int  21h
       
        pop  ds
        pop  es
        pop  bp
        ret

%if 0
;**************************************************************************
;***                       RenewCriticalHandler                         ***
;**************************************************************************
;*** void RenewCriticalHandler(int (*handler)(int status));             ***
;***                                                                    ***
;*** Sets the routine to call to handler.                               ***
;**************************************************************************

        global _RenewCriticalHandler
_RenewCriticalHandler:
        push ds

        mov  bx, sp
        mov  ax, [ss:bx+04h]            ; Save the newly given handler
        mov  [handler], ax              ; as the handler to call.

        pop  ds
        ret

%endif

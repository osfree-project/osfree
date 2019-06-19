;    
;   Mouse.asm - general mouse routines.
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

;**************************************************************************
;***                              ToTextCos                             ***
;***++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++***
;*** Convert to text coordinates.                                       ***   
;***                                                                    ***
;*** Parameter: %1: coordinate to convert to text coordinate.           ***
;**************************************************************************
%macro ToTextCos 1  
 shr  %1, 1
 shr  %1, 1
 shr  %1, 1
 inc  %1
%endmacro

;**************************************************************************
;***                            FromTextCos                             ***
;***++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++***
;*** Convert from text coordinates.                                     ***   
;***                                                                    ***
;*** Parameter: %1: coordinate to convert to text coordinate.           ***
;**************************************************************************

%macro FromTextCos 1
 shl  %1, 1
 shl  %1, 1
 shl  %1, 1
 dec  %1 
%endmacro

;--------------------------------- DATA ----------------------------------

segment _DATA class=DATA

before db 0           ;Remembers wether any of these routines have been
                      ;called before.

mousepresent db 0     ;Remembers wether a mouse driver is present.

;--------------------------------- CODE ----------------------------------

segment _TEXT class=CODE

;=========================================================================        
;===                           InitMouse                               ===
;===-------------------------------------------------------------------===
;=== Initialises the mouse driver.                                     ===
;===                                                                   ===
;=== int InitMouse (void);                                             ===
;===                                                                   ===
;=== Returns: 0 if mouse driver not installed.                         ===
;===          1 if mouse driver installed.                             ===
;=========================================================================                                                                   
        
        global _CloseMouse
        global _InitMouse
_CloseMouse:
_InitMouse:
        push es

        mov  ax, 3533h               ; Get vector address of int 33h.
        int  21h

        xor  ax, ax                  ; AX = 0 for: 
                                     ;    a) false if handler not installed.
                                     ;    b) to make the init call to int 33h.

        mov  dx, es
        cmp  dx, 0                   ; Check to be sure that the vector is
        jne  .installed              ; not 0000:0000.

        cmp  bx, 0
        jne  .installed

        jmp  short .EndOfProc        ; Exit if 0000:000, return value 
                                     ; already in ax.

.installed:
        int  33h                     ; Initialise mouse driver.
        neg  ax
        
        mov  [mousepresent], al      ; and return wether initialisation
                                     ; worked.

.EndOfProc:
        pop  es
        ret

;=========================================================================        
;===                           MousePresent                            ===
;===-------------------------------------------------------------------===
;=== Returns wether a mouse driver is installed and if any of these    ===
;=== routines haven't been called before initialises the driver.       ===
;===                                                                   ===
;=== int MousePresent (void);                                          ===
;===                                                                   ===
;=== Returns: 0 if mouse not installed (assembly: zf == 1).            ===
;===          1 if mouse installed(assembly: zf == 0).                 ===
;=========================================================================                                                                   
       
        global _MousePresent
_MousePresent:
        cmp  [before], byte 0        ; Check wether we already have
        jne  .gotit                  ; initialised.

        mov  [before], byte 1        

        call _InitMouse              ; Initialise if we haven't done this
                                     ; before.

.gotit:
        mov  al, [mousepresent]      ; return wether mouse available.
        xor  ah, ah
        cmp  ax,  0
        ret

%if 0

;=========================================================================        
;===                           ShowMouse                               ===
;===-------------------------------------------------------------------===
;=== Makes the mouse cursor visible.                                   ===
;===                                                                   ===
;=== void ShowMouse (void);                                            ===
;=========================================================================                                                                   
        
        global _ShowMouse
_ShowMouse:
        call _MousePresent
        jz   .EndOfProc              ; If mouse available

        mov  ax, 1                   ; show mouse cursor.
        int  33h

.EndOfProc:
        ret

%endif

%if 0

;=========================================================================        
;===                           HideMouse                               ===
;===-------------------------------------------------------------------===
;=== Makes the mouse cursor invisible.                                 ===
;===                                                                   ===
;=== void HideMouse (void);                                            ===
;=========================================================================                                                                   


        global _HideMouse
_HideMouse:
        call _MousePresent
        jz   .EndOfProc              ; If Mouse available.

        mov  ax, 2                   ; Hide mouse cursor.
        int  33h

.EndOfProc:
        ret

%endif

%if 0

;=========================================================================        
;===                           WhereMouse                              ===
;===-------------------------------------------------------------------===
;=== Returns the status of the buttons and the position of the mouse.  ===
;===                                                                   ===
;=== int WhereMouse (int* x, int* y);                                  ===
;===                                                                   ===
;=== Returns: in x: x position.                                        ===
;===          in y: y position.                                        ===
;===        + status mouse buttons.                                    ===
;=========================================================================                                                                   

        global _WhereMouse
_WhereMouse:
        push bp
        mov  bp, sp
        
        call _MousePresent           ; If mouse is not available:
        jnz  .available
        
        mov  bx, [bp+04h]            ; Return:
        mov  [bx], word 0            ;     *x = 0
        mov  bx, [bp+06h]
        mov  [bx], word 0            ;     *y = 0

        jmp  short .EndOfProc
.available:                          ; If mouse is available

        mov  ax, 3                   ; Get Mouse cursor position.
        int  33h

        ToTextCos cx                 ; Convert to screen coordinates.
        ToTextCos dx

        mov  ax, bx                  ; Return *x 
        mov  bx, [bp+04h]
        mov  [bx], cx
        mov  bx, [bp+06h]            ; and *y.
        mov  [bx], dx

.EndOfProc:
        pop  bp
        ret

%endif

%if 0

;=========================================================================        
;===                           MouseGotoXY                             ===
;===-------------------------------------------------------------------===
;=== Sets the position of the mouse cursor.                            ===
;===                                                                   ===
;=== void MouseGotoXY (int x, int y);                                  ===
;===                                                                   ===
;=== Parameters: x: x position.                                        ===
;===             y: y position.                                        ===
;=========================================================================                                                                   
       
        global _MouseGotoXY
_MouseGotoXY:
        push bp
        mov  bp, sp
        
        call _MousePresent           ; Exit if mouse not available.
        jz   .EndOfProc

        mov  ax, 4                   
        mov  cx, [bp+4]              ; Get x and y coordinates.
        mov  dx, [bp+6]
        
        FromTextCos cx               ; Convert to mouse coordinates.
        FromTextCos dx
        
        int  33h                     ; Set mouse cursor.

.EndOfProc:
        pop  bp
        ret

%endif

%if 1

;=========================================================================        
;===                      CountButtonPresses                           ===
;===-------------------------------------------------------------------===
;=== Returns the number of mouse presses of a certain button and       ===
;=== the position of the last press.                                   ===
;===                                                                   ===
;=== int CountButtonPresses  (int button, int* presses,                ===
;===                          int* x, int* y);                         ===
;===                                                                   ===
;=== Returns: in x: x position.                                        ===
;===          in y: y position.                                        ===
;===        + status mouse buttons.                                    ===
;=========================================================================                                                                   

        global _CountButtonPresses
_CountButtonPresses:
        push bp
        mov  bp, sp

        call _MousePresent           ; If mouse not available.
        jnz  .available

        mov  ax, 0                   ; Return:
        mov  bx, [bp+06h]
        mov  [bx], word 0            ;  *presses = 0
        mov  bx, [bp+08h]
        mov  [bx], word 0            ;  *x = 0
        mov  bx, [bp+0Ah]
        mov  [bx], word 0            ;  *y = 0
        
        jmp  short .EndOfProc
.available:
        push di        
        mov  ax, 5                   
        mov  bx, [bp+04h]            ; Count for given button.
        int  33h

        ToTextCos cx                 ; Convert return coordinates
        ToTextCos dx                 ; to screen coordinates.
        
        mov  di, [bp+06h]
        mov  [di], bx                ; Return press count.

        mov  di, [bp+08h]
        mov  [di], cx                ; Return x.

        mov  di, [bp+0Ah]
        mov  [di], dx                ; Return y.

        pop  di
.EndOfProc:
        pop  bp
        ret

%endif

%if 0

;=========================================================================        
;===                      CountButtonReleases                          ===
;===-------------------------------------------------------------------===
;=== Returns the number of mouse releases of a certain button and      ===
;=== the position of the last release.                                 ===
;===                                                                   ===
;=== int CountButtonReleases (int button, int* releases,               ===
;===                          int* x, int* y);                         ===
;===                                                                   ===
;=== Returns: in x: x position.                                        ===
;===          in y: y position.                                        ===
;===        + status mouse buttons.                                    ===
;=========================================================================                                                                   

        global _CountButtonReleases
_CountButtonReleases:
        
        push bp
        mov  bp, sp
        
        call _MousePresent           ; If mouse not available.
        jnz  .available

        mov  ax, 0                   ;Return:
        mov  bx, [bp+06h]            ;   *presses = 0
        mov  [bx], word 0
        mov  bx, [bp+08h]            ;   *x = 0
        mov  [bx], word 0
        mov  bx, [bp+0Ah]            ;   *y = 0
        mov  [bx], word 0
        
        jmp  short .EndOfProc

.available:        
        push di

        mov  bx, [bp+04h]            ; Count for given button.
        mov  ax, 6
        int  33h

        ToTextCos cx                 ; Convert the returned coordinates
        ToTextCos dx                 ; to screen coordinates.
        
        mov  di, [bp+06h]
        mov  [di], bx                ; Return release count.
        mov  di, [bp+08h]
        mov  [di], cx                ; Return x count.
        mov  di, [bp+0Ah]
        mov  [di], dx                ; Return y count.
        
        pop  di
.EndOfProc:
        pop  bp
        ret

%endif

%if 0

;=========================================================================        
;===                           MouseWindow                             ===
;===-------------------------------------------------------------------===
;=== Defines the mouse window.                                         ===
;===                                                                   ===
;=== void MouseWindow (int x1, int y1, int x2, int y2);                ===
;=========================================================================                                                                   

        global _MouseWindow
_MouseWindow:
        
        push bp
        mov  bp, sp
        
        call _MousePresent           ; Exit if mouse is not available.
        jz   .EndOfProc

        mov  ax, 7
        mov  cx, [bp+04h]            ; Get x1.
        mov  dx, [bp+08h]            ; Get x2.
        
        FromTextCos cx               ; Convert to mouse coordinates.
        FromTextCos dx
        
        int  33h                     ; Set horizontal range.

        mov  ax, 8
        mov  cx, [bp+06h]            ; Get y1.
        mov  dx, [bp+0Ah]            ; Get y2.
        
        FromTextCos cx               ; Convert to mouse coordinates.
        FromTextCos dx
        
        int  33h                     ; Set vertical range.

.EndOfProc:
        pop  bp
        ret

%endif

%if 0

;=========================================================================        
;===                     DefineTextMouseCursor                         ===
;===-------------------------------------------------------------------===
;=== Defines the form of the text mouse cursor.                        ===
;===                                                                   ===
;=== void DefineTextMouseCursor(int type, int andmask, int ormask);    ===
;===                                                                   ===
;=== Parameters: type: software(0), then                               ===      
;===                      andmask: AND mask.                           ===
;===                      ormask : OR mask.                            ===
;===                                                                   ===
;===                   hardware(1), then                               ===                  
;===                      andmask: first screen line.                  ===
;===                      ormask:  last  screen line.                  ===
;=========================================================================                                                                   

        global _DefineTextMouseCursor
_DefineTextMouseCursor:
        push bp
        mov  bp, sp
                                     
        call _MousePresent           ; Exit if mouse not available.
        jz   .EndOfProc

        mov  ax, 0Ah                 ; Set:
        mov  bx, [bp+04h]            ;    type of cursor.
        mov  cx, [bp+06h]            ;    and mask.
        mov  dx, [bp+08h]            ;    or  mask.
        int  33h

.EndOfProc:
        pop  bp
        ret

%endif

%if 0

;=========================================================================        
;===                          GetMouseMoved                            ===
;===-------------------------------------------------------------------===
;=== Returns the distance between the actual mouse position and the    ===
;=== previous position on the screen.                                  ===
;===                                                                   ===
;=== void GetMouseMoved (int* distx, int* disty);                      ===
;===                                                                   ===
;=== Returns: in distx: horizontal distance.                           ===      
;===          in disty: vertical distance.                             ===
;=========================================================================                                                                   

        global _GetMouseMoved
_GetMouseMoved:
        push bp
        mov  bp, sp
        
        call _MousePresent           ; If mouse not available
        jnz  .available
                                     ; Return:
        mov  bx, [bp+04h]            
        mov  [bx], word 0            ;     distx = 0
        mov  bx, [bp+06h]
        mov  [bx], word 0            ;     disty = 0
        
        jmp  short .EndOfProc

.available:
        mov  ax, 0Bh                 
        int  33h                     
                                     ; Return: 
        mov  bx, [bp+04h]             
        mov  [bx], cx                ;     horizontal distance.

        mov  bx, [bp+06h]            
        mov  [bx], dx                ;     vertical distance.

.EndOfProc:
        pop  bp
        ret

%endif

%if 0

;=========================================================================        
;===                          SetLightPenOn                            ===
;===-------------------------------------------------------------------===
;=== Turns light pen emulation on.                                     ===
;===                                                                   ===
;=== void SetLightPenOn(void);                                          ===
;=========================================================================                                                                   

        global _SetLightPenOn
_SetLightPenOn:
        
        call _MousePresent
        jz   .EndOfProc              ; If it is available,
        
        mov  ax, 0Dh                 ; then turn on pen emulation.
        int  33h

.EndOfProc:
        ret

%endif

%if 0

;=========================================================================        
;===                          SetLightPenOff                           ===
;===-------------------------------------------------------------------===
;=== Turns light pen emulation off.                                    ===
;===                                                                   ===
;=== void SetLightPenOff(void);                                        ===
;=========================================================================                                                                   

        global _SetLightPenOff
_SetLightPenOff:
        
        call _MousePresent           ; If it is available,
        jz   .EndOfProc
        
        mov  ax, 0Eh                 ; then turn off pen emulation.
        int  33h

.EndOfProc:
        ret

%endif

%if 0

;=========================================================================        
;===                           SetMickey                               ===
;===-------------------------------------------------------------------===
;=== Sets the speed of the mouse cursor.                               ===
;===                                                                   ===
;=== void SetMickey (int hm, int vm);                                  ===
;===                                                                   ===
;=== Parameters: hm: horizontal mickeys.                               ===
;===             vm: vertical mickeys.                                 ===
;=========================================================================                                                                   

        global _SetMickey
_SetMickey:
        push bp
        mov  bp, sp
        
        call _MousePresent           ; If the mouse is available.
        jz   .EndOfProc

        mov  cx, [bp+04h]            ; Set vertical and
        mov  dx, [bp+06h]            ; horizontal mickey's.
	mov  ax, 0fh
        int  33h

.EndOfProc:
        pop  bp
        ret
%endif

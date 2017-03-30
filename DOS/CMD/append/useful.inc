; FreeDOS APPEND
; Copyright (c) 2004 Eduardo Casino <casino_e@terra.es>
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	USA
;
; USEFUL.MAC - Some useful macros
;
; 04-06-01  casino_e@terra.es	First version
;

; TOUPPER - Uppercase the contents of the specified register or address
; ARGS	  - 1 - Register or address
;
toupper macro x,y
    cmp  x, 'a'
    jb	 a&y
    cmp	 x, 'z'
    jnbe a&y
    sub	 x, 'a' - 'A'
a&y:
endm


; UPDATEF - Update caller's flags
;	    Replaces the caller's flags with the ones stored in the 1st argument
; ARGS	  - 1 : Register or address
;
updatef macro x
    push ax
    push bx

    mov	 bx, sp
    mov	 ax, x			; Update caller's flags
    mov	 ss:[bx+8], ax		; in sp+2(bx)+2(ax)+4(cs:ip)

    pop  bx
    pop  ax
endm


; SAVEF - Save flags into the 1st argument
;
; ARGS	- 1 : Register or address
;
savef macro x
    push  ax
    pushf
    pop	  ax
    mov	  x, ax			; save result
    pop	  ax
endm

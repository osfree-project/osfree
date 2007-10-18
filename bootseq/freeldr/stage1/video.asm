;
; $Header: /cur/cvsroot/boot/muFSD/video.asm,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
;

; 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
; access your Linux ext2fs partitions as normal drive letters.
; Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

        .286

        PUBLIC VIDEO_INIT
        PUBLIC VIDEO_CRLF
        PUBLIC VIDEO_OUTPUT

;****************************************************************************
;*** Segment definitions                                                  ***
;****************************************************************************
_TEXT   SEGMENT

        EXTRN  _current_seg : word

VIDEO_CRLF proc near
        enter 0, 0
        push ds
        push es
        pusha

        ;
        ; Reads current cursor position - returned in line = dh, column = dl
        ;
        mov ah, 03h
        mov bh, 0
        int 10h

        ;
        ; Column <= 24 then no scrolling else scrolling
        ;
        inc dh
        cmp dh, 24
        jbe noscroll

        mov ah, 06h     ; scroll up
        mov al, 1       ; one line up
        mov cx, 0       ; line in CH, column in CL
        mov dh, 24      ; line
        mov dl, 79      ; column
        mov bh, 7       ; attribute (white on black)
        int 10h

noscroll:
        ;
        ; Puts the cursor either one line lower (noscroll) or on the last line (scroll)
        ;
        mov ah, 02h     ; move cursor
        mov bh, 0
        mov dl, 0       ; column
        int 10h

        popa
        pop es
        pop ds
        leave
        ret
VIDEO_CRLF endp

VIDEO_OUTPUT proc near
        enter 0, 0
        push ds
        push es
        pusha

        ;
        ; Reads current cursor position - puts it in dh (line) dl (column)
        ;
        mov ah, 03h     ; read cursor pos.
        mov bh, 0       ; screen page
        int 10h

        ;
        ; Writes the string on the screen
        ;
        mov ah, 13h             ; string output
        mov al, 1               ; attribute in bl, update cursor position
        mov bl, 7               ; attribute (white on black)
        mov bh, 0               ; screen page
        mov cx, [bp + 4]        ; number of characters in the string
        mov bp, [bp + 6]        ; offset to the string
        push ds                 ; was cs // valerius
        pop es                  ; segment to the string
        int 10h

        ; NOTE here bp and es are lost

        popa
        pop es
        pop ds
        leave
        ret 4
VIDEO_OUTPUT endp

VIDEO_INIT proc near
        enter 0, 0
        pusha
        push ds
        push es
        mov ax, 2
        int 10h
        pop es
        pop ds
        popa
        leave
        ret
VIDEO_INIT endp

_TEXT   ENDS
        end

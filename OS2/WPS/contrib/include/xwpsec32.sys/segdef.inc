
; 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
; services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
; (device drivers and installable file system drivers).
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

ifdef FSD32
        ;
        ; segment definitions for ext2-os2.ifs
        ;
        include fsd32_segdef.inc
endif

ifdef MWDD32
        ;
        ; segment definitions for mwdd32.sys
        ;
        include mwdd32_segdef.inc
endif



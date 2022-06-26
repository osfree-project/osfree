;
; sec32_end.asm:
;       this contains the tail of the driver binary.
;
;       This is linked LAST by the makefile to make sure it appears
;       at the end of the binary.
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

    include sec32_segdef.inc


CODE16 segment
    public code16_end
code16_end label byte
CODE16 ends

DATA16 segment
    public data16_end
data16_end label byte
DATA16 ends

CODE32 segment
    public code32_end
code32_end label byte
CODE32 ends

DATA32 segment
    public data32_end
data32_end label byte
DATA32 ends

    end

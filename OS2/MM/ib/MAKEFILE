# makefile - image
# Copyright (c) 1997, 2001 Mark Kimes
# Copyright (c) 2001 Steven Levine & Associates, Inc.

# $TLIB$: $ &(#) %n - Ver %v, %f $
# TLIB: $ &(#) MAKEFILE. - Ver 2, 27-Oct-01,17:15:32 $


DEBUG=0
BASE=image

CFLAGS = /G5 /Gm+ /Gs- /Gt- /Mp /O- /Q+ /Sp4 /Ss /W3 /C .\$*.c

!IF $(DEBUG)
LFLAGS = /DE /ALIGN:4 /EXEPACK:2 /NOI /MAP /PMTYPE:PM
!ELSE
LFLAGS = /ALIGN:4 /EXEPACK /MAP /NOI /PMTYPE:PM
!ENDIF

.SUFFIXES: .c .rc

.c.obj:
    icc $(CFLAGS)

OFILES= image.obj

ALL: $(BASE).EXE \
     $(BASE).RES

$(BASE).exe: $(OFILES) $(BASE).res $(BASE).def
    @REM @<<image.@0
    $(LFLAGS)
    $(OFILES)
    mmpm2.lib
    $(BASE).def
<<
    type $(BASE).@0
    ilink @$(BASE).@0
    rc -x2 $(BASE).res $(BASE).exe

$(BASE).res: $*.rc $*.dlg $*.h
    rc -r -x2 $*

$image.obj:     $*.c $*.h

# The end

#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = clibexts
TRGT = $(PROJ).lib
srcfiles = $(p)cgetsos2$(e) $(p)getchos2$(e) $(p)putchos2$(e) $(p)kbhitos2$(e)
# defines additional options for C compiler
ADD_COPT    = -3s
NODEFAULTLIBS = 1
ADD_LINKOPT = lib sub32.lib
UNI2H = 1
C = s

#TARGETS  = $(PATH)$(PROJ).lib

!ifndef TARGETBLD
DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)lib
!else
DEST = os2tk45$(SEP)lib
SRC  = $(MYDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)os2386$(SEP)libc$(SEP)s$(SEP)
ADD_COPT += -i=$(SRC)..
!endif

!include $(%ROOT)tools/mk/libsos2.mk

.SUFFIXES:
.SUFFIXES: .lib .obj .c .h

.c: $(MYDIR)..

.c: $(SRC)..

.h: $(MYDIR)..

.h: $(SRC)..

.obj: $(PATH)

.c.obj: .AUTODEPEND
 @echo CC       $^. $(LOG)
 @$(CC) $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

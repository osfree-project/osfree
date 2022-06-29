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

DEST = host$(SEP)$(%HOST)$(SEP)os2tk45$(SEP)lib

!include $(%ROOT)/mk/libsos2.mk

.SUFFIXES:
.SUFFIXES: .lib .obj .c .h

.c: $(MYDIR)..

.h: $(MYDIR)..

.obj: $(PATH)

.c.obj: .AUTODEPEND
 @echo CC      $^. $(LOG)
 @wcc386 $(COPT) -fr=$^*.err -fo=$^@ $[@ $(LOG2)

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ = criter1
TRGT = $(PROJ).exe
DESC = Critical Error & Dummy ^Break handler & Kernel Swap Context
#defines object file names in format objname.$(O)
srcfiles = $(p)..$(SEP)criter$(e)

# defines additional options for C compiler
ADD_ASMOPT = -dAUTO_FAIL  -dMODEL=l -i=$(PATH)..$(SEP)..$(SEP)strings$(SEP)critstrs -i=$(MYDIR)..
ADD_LINKOPT = lib suppl.lib
#COM = 1

#TARGETS = $(PATH)$(PROJ).exe

!include $(%ROOT)/mk/appsdos.mk

.SUFFIXES: .def .x

.def: $(PATH)..

.x: $(MYDIR)

$(PATH)criter.obj: $(PATH)..$(SEP)context.def criter.asm dmy_cbrk.asm ..$(SEP)include$(SEP)stuff.inc

$(PATH)..$(SEP)context.def: $(MYDIR)context.x
 @mkctxt $<
 @$(CP) $(MYDIR)context.h_c $(PATH).. $(BLACKHOLE)
 @$(CP) $(MYDIR)context.inc $(PATH).. $(BLACKHOLE)


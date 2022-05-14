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
ADD_ASMOPT = -dAUTO_FAIL  -dMODEL=l -i=$(PATH)..$(SEP)..$(SEP)strings$(SEP)critstrs -i=$(PATH) -i=$(MYDIR)
ADD_LINKOPT = lib suppl.lib
#COM = 1

!include $(%ROOT)/mk/appsdos.mk

TARGETS = $(PATH)context.def

.SUFFIXES: .def .x

.def: $(PATH)

.x: $(MYDIR)..

.asm: $(MYDIR)..

$(PATH)context.def: $(MYDIR)..$(SEP)context.x
 @mkctxt.exe $<
 @$(RN) $(MYDIR)..$(SEP)context.h_c $(PATH) $(BLACKHOLE)
 @$(RN) $(MYDIR)..$(SEP)context.inc $(PATH) $(BLACKHOLE)
 @$(RN) $(MYDIR)..$(SEP)context.def $(PATH) $(BLACKHOLE)

$(PATH)criter.obj: $(PATH)context.def criter.asm dmy_cbrk.asm ..$(SEP)..$(SEP)include$(SEP)stuff.inc

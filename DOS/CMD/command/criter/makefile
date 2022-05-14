#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ = criter
TRGT = $(PROJ).exe
DESC = Critical Error & Dummy ^Break handler & Kernel Swap Context
DIRS = criter1
#defines object file names in format objname.$(O)
srcfiles = $(p)criter$(e)

# defines additional options for C compiler
ADD_ASMOPT =  -dMODEL=l -i=$(PATH)..$(SEP)strings$(SEP)critstrs 
ADD_LINKOPT = lib suppl.lib
#COM = 1

!include $(%ROOT)/mk/appsdos.mk

TARGETS = $(PATH)context.def subdirs

.SUFFIXES: .def .x

.def: $(PATH)

.x: $(MYDIR)

$(PATH)criter.obj: $(PATH)context.def criter.asm dmy_cbrk.asm ..$(SEP)include$(SEP)stuff.inc

$(PATH)context.def: $(MYDIR)context.x
 @mkctxt.exe $<
 @$(RN) $(MYDIR)context.h_c $(PATH) $(BLACKHOLE)
 @$(RN) $(MYDIR)context.inc $(PATH) $(BLACKHOLE)
 @$(RN) $(MYDIR)context.def $(PATH) $(BLACKHOLE)


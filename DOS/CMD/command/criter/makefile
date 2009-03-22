#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ = criter
DESC = Critical Error & Dummy ^Break handler & Kernel Swap Context
#defines object file names in format objname.$(O)
srcfiles = $(p)criter$(e)

# defines additional options for C compiler
ADD_ASMOPT = -i=$(PATH)..$(SEP)strings$(SEP)critstrs
ADD_LINKOPT = lib suppl.lib
COM = 1

!include $(%ROOT)/mk/appsdos.mk

TARGETS = $(PATH)$(PROJ).exe $(PATH)..$(SEP)context.def

.SUFFIXES: .def .x

.def: $(PATH)..

.x: $(MYDIR)

$(PATH)..$(SEP)context.def: $(MYDIR)context.x
 mkctxt $<
 $(CP) $(MYDIR)context.h_c $(PATH)..
 $(CP) $(MYDIR)context.inc $(PATH)..
 

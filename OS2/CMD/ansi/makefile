#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = ansi
DESC = Control utility for ANSI
#defines object file names in format objname.$(O)
srcfiles = $(p)ansi$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)..$(SEP)include
NODEFAULTLIBS = 1
ADD_LINKOPT = lib $(BLD)$(SEP)lib$(SEP)os2386.lib, clib3r.lib
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

IMPORTS = &
      VioGetAnsi_                  EMXWRAP.103, &
      VioSetAnsi_                  EMXWRAP.105

!include $(%ROOT)/mk/appsos2.mk

qwe: .symbolic
 $(SAY) $(COPT)

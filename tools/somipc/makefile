#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = somipc
TRGT = $(PROJ).exe
DESC = SOM Compiler IPC
#defines object file names in format objname.$(O)
srcfiles = $(p)somipc$(e)
# defines additional options for C compiler
ADD_COPT    = -d2 -i=$(%ROOT)$(SEP)tools$(SEP)include -sg -dTRUE=1 -dFALSE=0
ADD_LINKOPT = lib tools_shared.lib
DEBUG = 1


!include $(%ROOT)/mk/tools.mk

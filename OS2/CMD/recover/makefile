#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = recover
DESC = RECOVER front-end
#defines object file names in format objname.$(O)
srcfiles = $(p)recover$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib cmd_shared.lib
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
IMPORTS = DosFlatToSel_ DOSCALLS.425

!include $(%ROOT)/mk/appsos2.mk

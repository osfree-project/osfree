#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = format
DESC = FORMAT front-end
#defines object file names in format objname.$(O)
srcfiles = $(p)format$(e)
# defines additional options for C compiler
ADD_COPT = -d2 -db -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = debug all
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
IMPORTS = DosFlatToSel_ DOSCALLS.425
UNI2H = 1

!include $(%ROOT)/mk/appsos2_cmd.mk

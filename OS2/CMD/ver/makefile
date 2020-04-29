#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = ver
TRGT = $(PROJ).exe
DESC = Displays System Version Number
#defines object file names in format objname.$(O)
srcfiles = $(p)ver$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include
# ADD_LINKOPT = debug all
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1

!include $(%ROOT)/mk/appsos2_cmd.mk

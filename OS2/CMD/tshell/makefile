#
# A Makefile for TSHELL
# (c) osFree project,
# author, date
#

PROJ = tshell
DESC = Text Shell
#defines object file names in format objname.$(O)
srcfiles = $(p)tshell$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)include
#ADD_LINKOPT = lib cmd_shared.lib,all_shared.lib
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2.mk

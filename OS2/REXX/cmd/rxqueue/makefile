#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = rxqueue
DESC = REXX queue tool
srcfiles = $(p)rxqueue$(e)
# defines additional options for C compiler
# ADD_COPT    = -i=$(MYDIR)..$(SEP)..$(SEP)interp
#NODEFAULTLIBS = 1
#ADD_LINKOPT = lib $(BLD)$(SEP)lib$(SEP)os2386.lib, clib3r.lib
# STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
# UNI2H=1
IMPORTS      = RexxAddQueue     REXXAPI.3, &
    	       RexxPullQueue    REXXAPI.4, &
    	       RexxQueryQueue   REXXAPI.5

!include $(%ROOT)/mk/appsos2_cmd.mk

#
# A Makefile for system shutdown command
# (c) osFree project,
# author, date
#

PROJ        = shutdown
TRGT         = $(PROJ).exe
DESC         = osFree System Shutdown
#defines object file names in format objname.$(O)
srcfiles     = $(p)shutdown$(e)
# defines additional options for C compiler
ADD_COPT     = -i=$(MYDIR) -i=$(%WATCOM)$(SEP)h$(SEP)os2 -sg -3s 
ADD_LINKOPT  = lib os2386.lib, clib3s.lib
# STUB       = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
WINDOWCOMPAT = 1
HEAPSIZE     = 10240
STACKSIZE    = 24576
# UNI2H = 1

!include $(%ROOT)/mk/appsos2_pm.mk


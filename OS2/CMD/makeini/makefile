#
# A Makefile for MiniCMD
# (c) osFree project,
# prokushev, 2008
#

PROJ = makeini
DESC = INI file compiler
#defines object file names in format objname.$(O)
srcfiles = $(p)xmakeini$(e) $(p)linklist$(e) $(p)stringh$(e) $(p)xprf$(e) $(p)xstring$(e) &
           $(p)nls$(e) $(p)dosh$(e) $(p)prfh$(e) $(p)dosh2$(e) $(p)level$(e)
# defines additional options for C compiler
ADD_COPT = -i=..$(SEP)include
OPTIONS  = caseexact
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2.mk

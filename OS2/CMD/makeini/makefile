#
# A Makefile for MiniCMD
# (c) osFree project,
# prokushev, 2008
#

PROJ = makeini
TRGT = $(PROJ).exe
DESC = INI file compiler
#defines object file names in format objname.$(O)
srcfiles = $(p)xmakeini$(e) $(p)linklist$(e) $(p)stringh$(e) $(p)xprf$(e) $(p)xstring$(e) &
           $(p)nls$(e) $(p)dosh$(e) $(p)prfh$(e) $(p)dosh2$(e) $(p)level$(e)
# defines additional options for C compiler
ADD_COPT = -i=. -i=$(MYDIR)..$(SEP)include
OPTIONS  = caseexact
IMPORTS     = KbdCharIn                   EMXWRAP.204
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1
# c++ source
CXX   = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk

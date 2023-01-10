#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = sysinstx
TRGT = $(PROJ).exe
DESC = System Installation Program 0
#defines object file names in format objname.$(O)
srcfiles = $(p)sysinstx$(e)
# defines additional options for C compiler
ADD_COPT = -d2 -db  -i=$(MYDIR)..$(SEP)include
# ADD_COPT = -d2 -db -i=$(%WATCOM)$(SEP)h -i=$(%WATCOM)$(SEP)h$(SEP)os2 -i=$(MYDIR)..$(SEP)include
# ADD_LINKOPT = lib math387s, os2$(SEP)os2, os2386
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
#IMPORTS = DosFlatToSel_ DOSCALLS.425, &
#          DosSelToFlat_ DOSCALLS.426
UNI2H = 1

!include $(%ROOT)tools/mk/appsos2_cmd.mk

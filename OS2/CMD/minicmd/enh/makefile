#
# A Makefile for MiniCMD
# (c) osFree project,
# prokushev, 2008
#

PROJ = minicmd
TRGT = $(PROJ).exe
DESC = Minimalistic Command line processor
ADD_COPT = -s -od -d3 -db
DEBUG = watcom all
# defines object file names in format objname.$(O)
srcfiles = $(p)minicmd$(e) $(p)commands$(e) $(p)dir$(e) $(p)env$(e)
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
UNI2H = 1

IMPORTS = KbdStringIn        EMXWRAP.209, &
          VioWrtTTY          EMXWRAP.119

!include $(%ROOT)tools/mk/appsos2_cmd.mk

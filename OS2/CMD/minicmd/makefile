#
# A Makefile for MiniCMD
# (c) osFree project,
# prokushev, 2008
#

PROJ = minicmd
DESC = Minimalistic Command line processor
ADD_COPT = -s -od -d3 -db
DEBUG = watcom all
#defines object file names in format objname.$(O)
srcfiles = $(p)minicmd$(e) $(p)commands$(e)
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

IMPORTS = KbdStringIn        EMXWRAP.209

!include $(%ROOT)/mk/appsos2.mk

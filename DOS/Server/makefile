#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = dos
DESC = DOS emulation
#defines object file names in format objname.$(O)
srcfiles = $(p)dos$(e) $(p)emu$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)include -d__ELF__ -dX86_EMULATOR
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2.mk

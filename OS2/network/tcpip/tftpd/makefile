#
# A Makefile for tftpd
# (c) osFree project,
# author, date
#

PROJ = tftpd
DESC = TFTP protocol daemon
#defines object file names in format objname.$(O)
srcfiles = $(p)tftpd$(e)
# defines additional options for C compiler
ADD_COPT = -zp=1 -i=$(%WATCOM)$(SEP)h$(SEP)os2
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
STACKSIZE=65535

!include $(%ROOT)/mk/appsos2.mk

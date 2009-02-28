#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = makeproc
DESC = Control utility for ANSI
#defines object file names in format objname.$(O)
srcfiles = $(p)makeproc$(e)
# defines additional options for C compiler
#ADD_COPT    = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
ADD_LINKOPT  = lib mmpm2.lib, gbm.lib, libtiff.lib, libjpeg.lib, png.lib, libz.lib, ojpeg.lib

!include $(%ROOT)/mk/appsos2.mk

#
# (c) osFree project,
#

PROJ = test201
TRGT = $(PROJ).exe
DESC = test application
#defines object file names in format objname.$(O)
srcfiles = $(p)test201$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
DEST        = os2$(SEP)test

!include $(%ROOT)tools/mk/appsos2_cmd.mk

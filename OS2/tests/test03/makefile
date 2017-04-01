#
# (c) osFree project,
#

PROJ = test03
DESC = test application
TRGT = $(PROJ).exe
#defines object file names in format objname.$(O)
srcfiles = $(p)test03$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)..$(SEP)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe
DEST        = os2$(SEP)test

!include $(%ROOT)/mk/appsos2_cmd.mk

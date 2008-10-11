#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ = lvm
DESC = Logical volume management tool
#defines object file names in format objname.$(O)
srcfiles = $(p)lvm$(e) $(p)lvmcmdline$(e)
# defines additional options for C compiler
ADD_COPT = -i=$(MYDIR)..$(SEP)lib$(SEP)datastruct -i=$(MYDIR)..$(SEP)lib$(SEP)misc -i=$(MYDIR)..$(SEP)lib$(SEP)config -i=$(MYDIR)..$(SEP)lib$(SEP)commands &
           -i=$(MYDIR)..$(SEP)lib$(SEP)log -i=$(MYDIR)..$(SEP)lib$(SEP)metadata -i=$(MYDIR)..$(SEP)lib$(SEP)device -i=$(MYDIR)..$(SEP)lib$(SEP)uuid &
           -i=$(MYDIR)..$(SEP)lib$(SEP)display -i=$(MYDIR)..$(SEP)lib$(SEP)format_text -i=$(MYDIR)..$(SEP)lib$(SEP)activate -i=$(MYDIR)..$(SEP)lib$(SEP)mm &
           -i=$(MYDIR)..$(SEP)lib$(SEP)locking -i=$(MYDIR)..$(SEP)lib$(SEP)cache -i=$(MYDIR)..$(SEP)lib$(SEP)label -i=$(MYDIR)..$(SEP)lib$(SEP)filters &
           -i=$(MYDIR)..$(SEP)lib$(SEP)activate -i=$(MYDIR)..$(SEP)daemons$(SEP)clvmd -bd
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2.mk

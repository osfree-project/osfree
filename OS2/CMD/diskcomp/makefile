#

PROJ = diskcomp
DESC = Disk compare tool
#defines object file names in format objname.$(O)
srcfiles = $(p)diskcomp$(e) $(p)misc$(e) $(p)md5$(e) $(p)drive$(e)
# defines additional options for C compiler
ADD_COPT = -i=..$(SEP)include
STACK    = 65536
STUB     = $(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)diskcomp.exe

!include $(%ROOT)/mk/appsos2.mk

# FORMAT OS2 LX PMCOMPATIBLE

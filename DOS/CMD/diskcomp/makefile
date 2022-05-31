#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ = diskcomp
TRGT = $(PROJ).exe
DESC = Compare disks
#defines object file names in format objname.$(O)
srcfiles = $(p)diskcomp$(e) $(p)drive$(e) $(p)md5$(e) $(p)misc$(e)
# defines additional options for C compiler
ADD_COPT  = -ml -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT = lib $(BLD)lib$(SEP)doscmd_shared.lib

!include $(%ROOT)/mk/appsdos.mk

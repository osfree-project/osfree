#
# A Makefile for APPEND
# (c) osFree project,
# author, date
#

PROJ      = exit_vdm
TRGT      = $(PROJ).exe
DESC      = Binary comparison of files
srcfiles  = $(p)exit_vdm$(e)

ADD_COPT  = -i=$(MYDIR)..$(SEP)..$(SEP)include
ADD_LINKOPT = lib $(BLD)lib$(SEP)dos_shared.lib

!include $(%ROOT)/mk/appsdos.mk

$(PATH)append.exe: .symbolic

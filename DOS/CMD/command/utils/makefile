#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

PROJ      = mkinfres
TRGT      = $(PROJ).exe
DESC      = DOS command interpreter
srcfiles = $(p)mkinfres$(e)
!ifeq %OS LINUX
ADD_COPT = -dLINUX
!endif
ADD_COPT += -i=$(MYDIR)..$(SEP)include -i=$(MYDIR)..$(SEP)suppl

!include $(%ROOT)/mk/tools.mk

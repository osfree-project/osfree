#
# A Makefile for OS/2 Server
# (c) osFree project,
# author, date
#

ARCH=win32 # os2 linux win32
PROJ = os2server_$(ARCH)
DESC = OS/2 Personality Server
DEST = .
DIRS = Shared $(ARCH)
srcfiles = $(p)main$(e)
# defines additional options for C compiler
ADD_COPT = #-i=$(MYDIR)$(SEP)include

!include $(%ROOT)/mk/os2server.mk

TARGETS  = subdirs $(PATH)$(PROJ).exe

qwe: .symbolic
 @echo $(SrvDir)

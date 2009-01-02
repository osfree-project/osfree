#
# A Makefile for OS/2 Server
# (c) osFree project,
# author, date
#

PROJ = os2server
DESC = OS/2 Personality Server
DEST = .
ARCH=OS2
#ARCH=Linux
!ifeq ENV OS/2
sadfsd
!endif
DIRS = Shared $(ARCH)
srcfiles = $(p)main$(e)
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)$(SEP)include 

ADD_LINKOPT = lib os2server_shared.lib, os2server_arch.lib

!include $(%ROOT)/mk/appsos2.mk
#!include $(%ROOT)/mk/tools.mk

TARGETS  = subdirs $(PATH)$(PROJ).exe
TARGET = all

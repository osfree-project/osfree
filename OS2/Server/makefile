#
# A Makefile for OS/2 Server
# (c) osFree project,
# author, date
#

PROJ = os2server
DESC = OS/2 Personality Server
DIRS = shared $(ENV)
srcfiles = $(p)stub$(e) $(p)main$(e) 
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)include
ADD_LINKOPT = lib os2server_shared.lib, os2server_arch.lib

#!include $(%ROOT)/mk/appsos2.mk
!include $(%ROOT)/mk/tools.mk

TARGETS  = subdirs $(PATH)$(PROJ).exe
TARGET = all

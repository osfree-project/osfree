#
# A Makefile for OS/2 Server
# (c) osFree project,
# author, date
#

PROJ = os2server
DESC = OS/2 Personality Server
DIRS = shared $(ENV)
srcfiles = &
# defines additional options for C compiler
ADD_COPT    = -i=$(MYDIR)include
STUB=$(FILESDIR)$(SEP)os2$(SEP)mdos$(SEP)os2stub.exe

!include $(%ROOT)/mk/appsos2.mk

TARGETS  = subdirs $(PATH)$(PROJ).exe
TARGET = all

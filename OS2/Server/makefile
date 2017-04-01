#
# A Makefile for OS/2 Server
# (c) osFree project,
# author, date
#

# host operating system
# os2 linux win32
#

ARCH  = $(%SERVERENV)
PROJ0 = main

!ifeq ARCH linux
EXESUF = l
!else ifeq ARCH win32
EXESUF = w
!else ifeq ARCH os2
EXESUF = p
!endif

PROJ = $(PROJ0)$(EXESUF)
TRGT = $(PROJ).exe

DESC = OS/2 Personality Server
# DEST = .
DIRS = kal Shared $(ARCH)
srcfiles = $(p)main$(e)
# defines additional options for C compiler
INSTALL_ADD = 1
ADD_COPT    = -d2 -db
ADD_LINKOPT = debug all lib os2server_shared.lib, os2server_$(ARCH).lib, apistub.lib
UNI2H   = 1

!include $(%ROOT)/mk/os2server.mk

#TARGETS  = subdirs $(PATH)$(PROJ).exe

install_add: $(MYDIR)CONFIG.SYS
 @$(CP) $< $(DEST) $(BLACKHOLE)


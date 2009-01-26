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
EXE_SUF = l
!else
!ifeq ARCH win32
EXE_SUF = w
!else
!ifeq ARCH os2
EXE_SUF = p
!endif
!endif
!endif

PROJ = $(PROJ0)$(EXE_SUF)

DESC = OS/2 Personality Server
DEST = .
DIRS = kal Shared $(ARCH) test
srcfiles = $(p)main$(e)
# defines additional options for C compiler
INSTALL_ADD = 1
ADD_COPT= -d2 -db

!include $(%ROOT)/mk/os2server.mk

TARGETS  = subdirs $(PATH)$(PROJ).exe

install_add: $(MYDIR)CONFIG.SYS
 $(CP) $< $(DEST)

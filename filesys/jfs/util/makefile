#
# A makefile for JFS utility DLL
#

!include ../../../build.conf
!include ../../../mk/site.mk

FS         = jfs
ADD_ASMOPT =
DEFINES    = -dOS2 -d__WATCOM__

!ifeq UNIX FALSE

DIR      = filesys\$(FS)\util
MYDIR    = $(ROOT)\$(DIR)
ADD_COPT = $(DEFINES) -i=$(MYDIR)\include -i=.

!else

DIR      = filesys/$(FS)/util
MYDIR    = $(ROOT)/$(DIR)
ADD_COPT = $(DEFINES) -i=$(MYDIR)/include -i=.

!endif

!include ../../../mk/bootseq.mk

STARTUP =
ALL     =
OBJS    = $(STARTUP) $(ALL)


all: .SYMBOLIC
 $(SAY) Making $(FS) utility DLL... $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

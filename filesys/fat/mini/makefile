#
# A makefile for fat MiniFSD
#

!include ../../../build.conf
!include ../../../mk/site.mk

FS         = fat
ADD_ASMOPT =
DEFINES  = -dOS2 -d__WATCOM__

!ifeq UNIX FALSE

DIR      = filesys\$(FS)\mini
MYDIR    = $(ROOT)\$(DIR)
ADD_COPT = $(DEFINES) -i=$(MYDIR)\include -i=.

!else

DIR      = filesys/$(FS)/mini
MYDIR    = $(ROOT)/$(DIR)
ADD_COPT = $(DEFINES) -i=$(MYDIR)/include -i=.

!endif

!include ../../../mk/bootseq.mk

STARTUP =
ALL     =
OBJS    = $(STARTUP) $(ALL)


all: .SYMBOLIC
 $(SAY) Making $(FS) MiniFSD... $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

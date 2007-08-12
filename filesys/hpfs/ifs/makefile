#
# A makefile for hpfs.ifs
#

!include ../../../build.conf
!include ../../../mk/site.mk

FS         = hpfs
ADD_ASMOPT =
DEFINES    = -dOS2 -d__WATCOM__

!ifeq UNIX FALSE

DIR      = filesys\$(FS)\ifs
MYDIR    = $(ROOT)\$(DIR)
ADD_COPT = $(DEFINES) -i=$(MYDIR)\include -i=.

!else

DIR      = filesys/$(FS)/ifs
MYDIR    = $(ROOT)/$(DIR)
ADD_COPT = $(DEFINES) -i=$(MYDIR)/include -i=.

!endif

!include ../../../mk/bootseq.mk

STARTUP =
ALL     =
OBJS    = $(STARTUP) $(ALL)


all: .SYMBOLIC
 $(SAY) Making $(FS).ifs... $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)


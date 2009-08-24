#
# A makefile for FAT utility DLL
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

FS         = fat
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

!include $(%ROOT)/mk/bootseq.mk

STARTUP =
ALL     =
OBJS    = $(STARTUP) $(ALL)


all: .SYMBOLIC
 $(SAY) Making $(FS) utility DLL... $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)


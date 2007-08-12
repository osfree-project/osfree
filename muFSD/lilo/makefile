#
# Makefile for OS/3 loader muFSD's common code.
# 06/09/29 valerius
#

!include ../../build.conf
!include ../../mk/site.mk

ADD_ASMOPT =
DEFINES    = -dMICROFSD_TEST -dOS2 -d__KERNEL__ -d__WATCOM__

!ifeq UNIX FALSE

DIR        = muFSD\lilo
MYDIR      = $(ROOT)\$(DIR)
ADD_COPT   = $(DEFINES) -i=$(ROOT)\include\muFSD -i=.

!else

DIR        = muFSD/lilo
MYDIR      = $(ROOT)/$(DIR)
ADD_COPT   = $(DEFINES) -i=$(ROOT)/include/muFSD -i=.

!endif

DIRS       =

OBJS       = build.$(O) boot.$(O)

!include ../../mk/bootseq.mk

all: $(OBJS) .SYMBOLIC
 $(MAKE) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) TARGET=$^@ subdirs

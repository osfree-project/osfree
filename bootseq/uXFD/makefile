#
# A Makefile for OS/3 boot sequence project
# micro IIF drivers
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

DIR        = uXFD
DIRS       = bin # mz ne a.out elf coff lx
ADD_COPT   = -od -s $(DEFINES) -i=. -i=$(%ROOT)$(SEP)include
ADD_ASMOPT =

STARTUP    =
ALL        = init.$(O)

OBJS       = $(STARTUP) $(ALL)

!include $(%ROOT)/mk/bootseq.mk

all: $(OBJS)
 $(MAKE) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) TARGET=$^@ subdirs

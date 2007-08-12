#
# A Makefile for OS/3 boot sequence project
# micro IIF drivers
# (c) osFree project,
# valerius, 2006/10/30
#

!include ../build.conf
!include ../mk/site.mk

DIRS = # bin mz ne a.out elf coff lx

!include ../mk/bootseq.mk

all: .SYMBOLIC
 $(MAKE) TARGET=$^@ subdirs

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) TARGET=$^@ subdirs

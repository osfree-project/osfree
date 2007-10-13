#
# A Makefile for mkmsgf
# (c) osFree project,
# valerius, 2006/10/30
#
# $Id: makefile,v 1.1 2004/08/16 06:27:30 prokushev Exp $
#

DIR = $(ROOT)$(SEP)tools$(SEP)bin
PROJ = mkmsgf
DESC = Make message file tool

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

!include $(%ROOT)/mk/all.mk

all: $(DIR)$(SEP)$(PROJ).exe .SYMBOLIC $(DIR)$(SEP)$(PROJ).sym

#fpc386  -o<x>  change the name of the executable produced to <x>
#On GNU/Linux this file is an ELF file without the exe extension.

$(DIR)$(SEP)$(PROJ).exe: .symbolic
 $(PC) $(PCOPT) -o$(PROJ).exe $(PROJ)
 $(CP) $(PROJ).exe $(DIR)

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

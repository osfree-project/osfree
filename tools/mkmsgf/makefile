#
# A Makefile for mkmsgf
# (c) osFree project,
# valerius, 2006/10/30
#
# $Id: makefile,v 1.1 2004/08/16 06:27:30 prokushev Exp $
#

ROOT=..\..
DIR = $(ROOT)\tools\bin
PROJ = mkmsgf
DESC = Make message file tool

!include ../../build.conf
!include ../../mk/site.mk

32_BITS  = 1

!include ../../mk/cmd.mk

all: $(DIR)\$(PROJ).exe .SYMBOLIC # $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).exe: .symbolic
 $(PC) $(PCOPT) $(PROJ)
 $(CP) $(PROJ).exe $(DIR)

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

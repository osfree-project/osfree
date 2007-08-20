# $Id: makefile,v 1.4 2004/08/16 04:51:09 prokushev Exp $

!include $(ROOT)/build.conf
!include $(ROOT)/mk/site.mk

ROOT=..$(SEP)..$(SEP)..
DIR = $(ROOT)$(SEP)bin$(SEP)os2
PROJ = bldlevel
DESC = Build level display facility

!include $(ROOT)/mk/cmd.mk

all: $(DIR)$(SEP)$(PROJ).cmd

$(DIR)$(SEP)$(PROJ).cmd: $(PROJ).cmd
  @$(CP) $< $@

# Clean all

clean: .symbolic

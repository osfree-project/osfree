# $Id: makefile,v 1.4 2004/08/16 04:51:09 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2
PROJ = bldlevel

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).cmd

$(DIR)\$(PROJ).cmd: $(PROJ).cmd
  @copy $< $@

# Clean all

clean: .symbolic

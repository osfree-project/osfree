# $Id: makefile,v 1.3 2003/10/29 11:04:06 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2
PROJ = syslevel

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).cmd

$(DIR)\$(PROJ).cmd: $(PROJ).cmd
  copy $< $@

# Clean all

clean: .symbolic


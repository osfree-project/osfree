# $Id: makefile,v 1.2 2003/10/29 11:04:05 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2
PROJ = fdisk

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).cmd

$(DIR)\$(PROJ).cmd: $(PROJ).cmd
  copy $< $@

clean: .symbolic

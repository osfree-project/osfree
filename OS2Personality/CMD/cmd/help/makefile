# $Id: makefile,v 1.2 2004/08/16 04:51:11 prokushev Exp $

ROOT=..\..\..\..

DIR = $(ROOT)\bin\os2\nls\book
PROJ = cmdref
PROJ2 = external

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).inf $(DIR)\$(PROJ2).inf

# Clean target
clean : .symbolic
    @if exist *.inf @del /q *.inf $(LOG)

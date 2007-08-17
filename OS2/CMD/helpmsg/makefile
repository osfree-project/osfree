# $Id: makefile,v 1.2 2004/08/16 04:51:14 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2
PROJ = helpmsg
DESC = Show help message

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).exe

# Clean all

clean: .symbolic
 -@del *.oo2 $(LOG)
 -@del *.ao2 $(LOG)
 -@del *.ppo $(LOG)
 -@del *.o   $(LOG)

# $Id: set.makefile,v 1.1.1.1 2003/10/04 08:27:50 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2
PROJ = set
DESC = Sets environment variables
OBJS = set.obj
ADD_COPT = -i=..\include 
ADD_LIB = cmd_shared.lib,all_shared.lib

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).exe $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).exe: $(OBJS)
 %create $^&.lnk
 %append $^&.lnk $(LINKOPT)
 %append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 %append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 %append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 %append $^&.lnk OPTION STACK=40960
 %append $^&.lnk OPTION MAP=$^&.wmp
 %append $^&.lnk LIBRARY $(LINKLIB)
 for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(LINKER) @$^&.lnk

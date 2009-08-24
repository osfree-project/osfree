# $Id: makefile,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2
PROJ = sysinstx
DESC = System Installation Program 0
OBJS = sysinstx.obj
ADD_COPT = -i=..\include
ADD_LIB = cmd_shared.lib,all_shared.lib

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).exe $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=40960
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) @$^&.lnk $(LOG)

# Clean all

clean: .symbolic
 -@del *.obj $(LOG)
 -@del *.sbr $(LOG)
 -@del *.map $(LOG)
 -@del *.err $(LOG)
 -@del *.wmp $(LOG)
 -@del *.lnk $(LOG)

# $Id: makefile,v 1.4 2004/08/16 06:26:36 prokushev Exp $

ROOT=..\..\..\..\..

DIR = $(ROOT)\bin\os2
PROJ = ping
DESC = PING utility
OBJS = $(PROJ).obj
ADD_COPT = -i=..\include -i=$(ROOT)\src\cmd\include
ADD_LIB = all_shared.lib,tcpip32.lib,math387s.lib, cmd_shared.lib

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).exe $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).exe: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk NAME $^@ OPTION MODNAME=$^&
 @%append $^&.lnk FORMAT OS2 LX PMCOMPATIBLE
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION STACK=65536
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

# Clean all

clean: .symbolic
 -@del *.obj $(LOG)
 -@del *.sbr $(LOG)
 -@del *.map $(LOG)
 -@del *.err $(LOG)
 -@del *.wmp $(LOG)
 -@del *.lnk $(LOG)

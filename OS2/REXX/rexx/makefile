# $Id: makefile,v 1.4 2004/08/16 04:51:07 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2\dll
PROJ = rexx
DESC = Classic REXX
OBJS = rexx.obj rexx16.obj
ADD_COPT = -i=..\include -bd -s

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).dll $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(DIR)\$(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initinstance terminstance
 @%append $^&.lnk option    ManyAutoData
 @%append $^&.lnk option    caseexact
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk import
 @%append $^&.lnk   RexxStart REGINA.RexxStart,
 @%append $^&.lnk   RexxVariablePool REGINA.RexxVariablePool,
 @%append $^&.lnk   RexxSetHalt REGINA.RexxSetHalt
 @%append $^&.lnk export
 @%append $^&.lnk   RexxStart.1,
 @%append $^&.lnk   RexxVariablePool.2,
 @%append $^&.lnk   RexxSetHalt.3,
 @%append $^&.lnk   RexxSetTrace.4,
 @%append $^&.lnk   RexxResetTrace.5,
 @%append $^&.lnk   RXTRACERESET.6 RESIDENT,
 @%append $^&.lnk   RXVAR.8 RESIDENT,
 @%append $^&.lnk   REXXSAA.9 RESIDENT,
 @%append $^&.lnk   RXHALTSET.10 RESIDENT,
 @%append $^&.lnk   RXTRACESET.11 RESIDENT,
# This function not found in any of OS/2 Toolkits
 @%append $^&.lnk   RXBREAKCLEANUP.7 RESIDENT
 @for %i in ($(OBJS)) do @%append $^&.lnk FILE %i
 $(SAY) Linking $^@
 @$(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

# Clean all

clean: .symbolic
 -@del /q *.obj $(LOG)
 -@del /q *.sbr $(LOG)
 -@del /q *.map $(LOG)
 -@del /q *.err $(LOG)
 -@del /q *.wmp $(LOG)
 -@del /q *.lnk $(LOG)
 -@del /q *.sym $(LOG)

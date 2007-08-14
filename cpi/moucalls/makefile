# $Id: makefile,v 1.3 2004/08/16 04:51:20 prokushev Exp $

ROOT=..\..\..

DIR = $(ROOT)\bin\os2\dll
PROJ = moucalls
DESC = Mouse API
OBJS = ..\shared\stub.obj
ADD_COPT = -i=..\include

!include $(ROOT)\makefile.inc

all: $(DIR)\$(PROJ).dll $(DIR)\$(PROJ).sym

$(DIR)\$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(DIR)\$(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    map
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk LIBRARY $(LINKLIB)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk import
 @%append $^&.lnk   MOUGETPTRSHAPE       DOSCALLS.719,
 @%append $^&.lnk   MOUSETPTRSHAPE       DOSCALLS.720,
 @%append $^&.lnk   MOUGETNUMMICKEYS     DOSCALLS.721,
 @%append $^&.lnk   MOUGETHOTKEY         DOSCALLS.853,
 @%append $^&.lnk   MOUSHELLINIT         DOSCALLS.723,
 @%append $^&.lnk   MOUGETSCALEFACT      DOSCALLS.724,
 @%append $^&.lnk   MOUFLUSHQUE          DOSCALLS.725,
 @%append $^&.lnk   MOUGETNUMBUTTONS     DOSCALLS.726,
 @%append $^&.lnk   MOUCLOSE             DOSCALLS.727,
 @%append $^&.lnk   MOUSETHOTKEY         DOSCALLS.854,
 @%append $^&.lnk   MOUSETSCALEFACT      DOSCALLS.729,
 @%append $^&.lnk   MOUGETNUMQUEEL       DOSCALLS.730,
 @%append $^&.lnk   MOUDEREGISTER        DOSCALLS.731,
 @%append $^&.lnk   MOUGETEVENTMASK      DOSCALLS.732,
 @%append $^&.lnk   MOUSETEVENTMASK      DOSCALLS.733,
 @%append $^&.lnk   MOUOPEN              DOSCALLS.734,
 @%append $^&.lnk   MOUREMOVEPTR         DOSCALLS.735,
 @%append $^&.lnk   MOUGETPTRPOS         DOSCALLS.736,
 @%append $^&.lnk   MOUREADEVENTQUE      DOSCALLS.737,
 @%append $^&.lnk   MOUSETPTRPOS         DOSCALLS.738,
 @%append $^&.lnk   MOUGETDEVSTATUS      DOSCALLS.739,
 @%append $^&.lnk   MOUSYNCH             DOSCALLS.740,
 @%append $^&.lnk   MOUREGISTER          DOSCALLS.741,
 @%append $^&.lnk   MOUSETDEVSTATUS      DOSCALLS.742,
 @%append $^&.lnk   MOUDRAWPTR           DOSCALLS.743,
 @%append $^&.lnk   MOUINITREAL          DOSCALLS.744,
 @%append $^&.lnk   MOUGETTHRESHOLD      DOSCALLS.722,
 @%append $^&.lnk   MOUSETTHRESHOLD      DOSCALLS.728
 @%append $^&.lnk export
 @%append $^&.lnk   MOUGETPTRSHAPE.1,
 @%append $^&.lnk   MOUSETPTRSHAPE.2,
 @%append $^&.lnk   MOUGETNUMMICKEYS.3,
 @%append $^&.lnk   MOUGETHOTKEY.4,
 @%append $^&.lnk   MOUSHELLINIT.5,
 @%append $^&.lnk   MOUGETSCALEFACT.6,
 @%append $^&.lnk   MOUFLUSHQUE.7,
 @%append $^&.lnk   MOUGETNUMBUTTONS.8,
 @%append $^&.lnk   MOUCLOSE.9,
 @%append $^&.lnk   MOUSETHOTKEY.10,
 @%append $^&.lnk   MOUSETSCALEFACT.11,
 @%append $^&.lnk   MOUGETNUMQUEEL.13,
 @%append $^&.lnk   MOUDEREGISTER.14,
 @%append $^&.lnk   MOUGETEVENTMASK.15,
 @%append $^&.lnk   MOUSETEVENTMASK.16,
 @%append $^&.lnk   MOUOPEN.17,
 @%append $^&.lnk   MOUREMOVEPTR.18,
 @%append $^&.lnk   MOUGETPTRPOS.19,
 @%append $^&.lnk   MOUREADEVENTQUE.20,
 @%append $^&.lnk   MOUSETPTRPOS.21,
 @%append $^&.lnk   MOUGETDEVSTATUS.22,
 @%append $^&.lnk   MOUSYNCH.23,
 @%append $^&.lnk   MOUREGISTER.24,
 @%append $^&.lnk   MOUSETDEVSTATUS.25,
 @%append $^&.lnk   MOUDRAWPTR.26,
 @%append $^&.lnk   MOUINITREAL.27,
 @%append $^&.lnk   MOUGETTHRESHOLD.29,
 @%append $^&.lnk   MOUSETTHRESHOLD.30
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
 -@del *.sym $(LOG)

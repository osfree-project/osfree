#
# A Makefile for moucalls.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = moucalls
DESC     = Mouse API
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
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
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll

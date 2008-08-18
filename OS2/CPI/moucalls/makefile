#
# A Makefile for moucalls.dll
# (c) osFree project,
# author, date
#

PROJ     = moucalls
DESC     = Mouse API
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = MOUGETPTRSHAPE       DOSCALLS.719, &
          MOUSETPTRSHAPE       DOSCALLS.720, &
          MOUGETNUMMICKEYS     DOSCALLS.721, &
          MOUGETHOTKEY         DOSCALLS.853, &
          MOUSHELLINIT         DOSCALLS.723, &
          MOUGETSCALEFACT      DOSCALLS.724, &
          MOUFLUSHQUE          DOSCALLS.725, &
          MOUGETNUMBUTTONS     DOSCALLS.726, &
          MOUCLOSE             DOSCALLS.727, &
          MOUSETHOTKEY         DOSCALLS.854, &
          MOUSETSCALEFACT      DOSCALLS.729, &
          MOUGETNUMQUEEL       DOSCALLS.730, &
          MOUDEREGISTER        DOSCALLS.731, &
          MOUGETEVENTMASK      DOSCALLS.732, &
          MOUSETEVENTMASK      DOSCALLS.733, &
          MOUOPEN              DOSCALLS.734, &
          MOUREMOVEPTR         DOSCALLS.735, &
          MOUGETPTRPOS         DOSCALLS.736, &
          MOUREADEVENTQUE      DOSCALLS.737, &
          MOUSETPTRPOS         DOSCALLS.738, &
          MOUGETDEVSTATUS      DOSCALLS.739, &
          MOUSYNCH             DOSCALLS.740, &
          MOUREGISTER          DOSCALLS.741, &
          MOUSETDEVSTATUS      DOSCALLS.742, &
          MOUDRAWPTR           DOSCALLS.743, &
          MOUINITREAL          DOSCALLS.744, &
          MOUGETTHRESHOLD      DOSCALLS.722, &
          MOUSETTHRESHOLD      DOSCALLS.728
EXPORTS = MOUGETPTRSHAPE.1,    &
          MOUSETPTRSHAPE.2,    &
          MOUGETNUMMICKEYS.3,  &
          MOUGETHOTKEY.4,      &
          MOUSHELLINIT.5,      &
          MOUGETSCALEFACT.6,   &
          MOUFLUSHQUE.7,       &
          MOUGETNUMBUTTONS.8,  &
          MOUCLOSE.9,          &
          MOUSETHOTKEY.10,     &
          MOUSETSCALEFACT.11,  &
          MOUGETNUMQUEEL.13,   &
          MOUDEREGISTER.14,    &
          MOUGETEVENTMASK.15,  &
          MOUSETEVENTMASK.16,  &
          MOUOPEN.17,          &
          MOUREMOVEPTR.18,     &
          MOUGETPTRPOS.19,     &
          MOUREADEVENTQUE.20,  &
          MOUSETPTRPOS.21,     &
          MOUGETDEVSTATUS.22,  &
          MOUSYNCH.23,         &
          MOUREGISTER.24,      &
          MOUSETDEVSTATUS.25,  &
          MOUDRAWPTR.26,       &
          MOUINITREAL.27,      &
          MOUGETTHRESHOLD.29,  &
          MOUSETTHRESHOLD.30
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

#
# A Makefile for moucalls.dll
# (c) osFree project,
# author, date
#

PROJ     = moucalls
DESC     = Mouse API
srcfiles = $(p)wrap$(e) $(p)dllstart$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod OPTION REDEFSOK
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS =  MouGetPtrShape              SUB32.301, &
           MouSetPtrShape              SUB32.302, &
           MouGetNumMickeys            SUB32.303, &
           MouGetScaleFact             SUB32.306, &
           MouFlushQue                 SUB32.307, &
           MouGetNumButtons            SUB32.308, &
           MouClose                    SUB32.309, &
           MouSetScaleFact             SUB32.311, &
           MouGetNumQueEl              SUB32.313, &
           MouDeRegister               SUB32.314, &
           MouGetEventMask             SUB32.315, &
           MouSetEventMask             SUB32.316, &
           MouOpen                     SUB32.317, &
           MouRemovePtr                SUB32.318, &
           MouGetPtrPos                SUB32.319, &
           MouReadEventQue             SUB32.320, &
           MouSetPtrPos                SUB32.321, &
           MouGetDevStatus             SUB32.322, &
           MouSynch                    SUB32.323, &
           MouRegister                 SUB32.324, &
           MouSetDevStatus             SUB32.325, &
           MouDrawPtr                  SUB32.326, &
           MouInitReal                 SUB32.327, &
           MouSetThreshold             SUB32.329, &
           MouGetThreshold             SUB32.330

EXPORTS = MOUGETPTRSHAPE.1,    &
          MOUSETPTRSHAPE.2,    &
          MOUGETNUMMICKEYS.3,  &
#          MOUGETHOTKEY.4,      &
#          MOUSHELLINIT.5,      &
          MOUGETSCALEFACT.6,   &
          MOUFLUSHQUE.7,       &
          MOUGETNUMBUTTONS.8,  &
          MOUCLOSE.9,          &
#          MOUSETHOTKEY.10,     &
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

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

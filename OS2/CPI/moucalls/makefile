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
IMPORTS =  MouGetPtrShape              EMXWRAP.301, &
           MouSetPtrShape              EMXWRAP.302, &
           MouGetNumMickeys            EMXWRAP.303, &
           MouGetScaleFact             EMXWRAP.306, &
           MouFlushQue                 EMXWRAP.307, &
           MouGetNumButtons            EMXWRAP.308, &
           MouClose                    EMXWRAP.309, &
           MouSetScaleFact             EMXWRAP.311, &
           MouGetNumQueEl              EMXWRAP.313, &
           MouDeRegister               EMXWRAP.314, &
           MouGetEventMask             EMXWRAP.315, &
           MouSetEventMask             EMXWRAP.316, &
           MouOpen                     EMXWRAP.317, &
           MouRemovePtr                EMXWRAP.318, &
           MouGetPtrPos                EMXWRAP.319, &
           MouReadEventQue             EMXWRAP.320, &
           MouSetPtrPos                EMXWRAP.321, &
           MouGetDevStatus             EMXWRAP.322, &
           MouSynch                    EMXWRAP.323, &
           MouRegister                 EMXWRAP.324, &
           MouSetDevStatus             EMXWRAP.325, &
           MouDrawPtr                  EMXWRAP.326, &
           MouInitReal                 EMXWRAP.327, &
           MouSetThreshold             EMXWRAP.329, &
           MouGetThreshold             EMXWRAP.330

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

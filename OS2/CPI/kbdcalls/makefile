#
# A Makefile for kbdcalls.dll
# (c) osFree project,
# author, date
#

PROJ     = kbdcalls
TRGT     = $(PROJ).dll
DESC     = Keyboard API
srcfiles = $(p)dllstart$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod OPTION REDEFSOK
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS =  KBDSETCUSTXT      DOSCALLS.745, &
           KBDINIT           DOSCALLS.746, &
           KBDGETCP          DOSCALLS.747, &
           KBDCHARIN         DOSCALLS.748, &
           KBDSETCP          DOSCALLS.749, &
           KBDLOADINSTANCE   DOSCALLS.750, &
           KBDSYNCH          DOSCALLS.751, &
           KBDREGISTER       DOSCALLS.752, &
           KBDSTRINGIN       DOSCALLS.753, &
           KBDGETSTATUS      DOSCALLS.754, &
           KBDSETSTATUS      DOSCALLS.755, &
           KBDGETFOCUS       DOSCALLS.756, &
           KBDFLUSHBUFFER    DOSCALLS.757, &
           KBDXLATE          DOSCALLS.758, &
           KBDSWITCHFGND     DOSCALLS.759, &
           KBDSHELLINIT      DOSCALLS.760, &
           KBDCLOSE          DOSCALLS.761, &
           KBDFREEFOCUS      DOSCALLS.762, &
           KBDFREE           DOSCALLS.763, &
           KBDDEREGISTER     DOSCALLS.764, &
           KBDSETFGND        DOSCALLS.765, &
           KBDPEEK           DOSCALLS.766, &
           KBDOPEN           DOSCALLS.767, &
           KBDGETHWID        DOSCALLS.768, &
           KBDSETHWID        DOSCALLS.769

UNI2H = 1

EXPORTS =  KBDSETCUSTXT.1,     &
           KBDINIT.2,   &
           KBDGETCP.3,         &
           KBDCHARIN.4,        &
           KBDSETCP.5,         &
           KBDLOADINSTANCE.6,  &
           KBDSYNCH.7,         &
           KBDREGISTER.8,      &
           KBDSTRINGIN.9,      &
           KBDGETSTATUS.10,    &
           KBDSETSTATUS.11,    &
           KBDGETFOCUS.12,     &
           KBDFLUSHBUFFER.13,  &
           KBDXLATE.14,        &
           KBDSWITCHFGND.15,   &
           KBDSHELLINIT.16,    &
           KBDCLOSE.17,        &
           KBDFREEFOCUS.18,    &
           KBDFREE.19,         &
           KBDDEREGISTER.20,   &
           KBDSETFGND.21,      &
           KBDPEEK.22,         &
           KBDOPEN.23,         &
           KBDGETHWID.24,      &
           KBDSETHWID.25

DEST     = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk

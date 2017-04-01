#
# A Makefile for kbdcalls.dll
# (c) osFree project,
# author, date
#

PROJ     = kbdcalls
TRGT     = $(PROJ).dll
DESC     = Keyboard API
srcfiles = $(p)wrap$(e) $(p)dllstart$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod OPTION REDEFSOK
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS =  KbdSetCustXt                EMXWRAP.201, &
           KbdGetCp                    EMXWRAP.203, &
           KbdCharIn                   EMXWRAP.204, &
           KbdSetCp                    EMXWRAP.205, &
           KbdSynch                    EMXWRAP.207, &
           KbdRegister                 EMXWRAP.208, &
           KbdStringIn                 EMXWRAP.209, &
           KbdGetStatus                EMXWRAP.210, &
           KbdSetStatus                EMXWRAP.211, &
           KbdGetFocus                 EMXWRAP.212, &
           KbdFlushBuffer              EMXWRAP.213, &
           KbdXlate                    EMXWRAP.214, &
           KbdClose                    EMXWRAP.217, &
           KbdFreeFocus                EMXWRAP.218, &
           KbdDeRegister               EMXWRAP.220, &
           KbdSetFgnd                  EMXWRAP.221, &
           KbdPeek                     EMXWRAP.222, &
           KbdOpen                     EMXWRAP.223, &
           KbdGetHWID                  EMXWRAP.224, &
           KbdSetHWID                  EMXWRAP.225

UNI2H = 1

EXPORTS =  KBDSETCUSTXT.1,     &
#           KBDPROCESSINIT.2,   &
           KBDGETCP.3,         &
           KBDCHARIN.4,        &
           KBDSETCP.5,         &
#           KBDLOADINSTANCE.6,  &
           KBDSYNCH.7,         &
           KBDREGISTER.8,      &
           KBDSTRINGIN.9,      &
           KBDGETSTATUS.10,    &
           KBDSETSTATUS.11,    &
           KBDGETFOCUS.12,     &
           KBDFLUSHBUFFER.13,  &
           KBDXLATE.14,        &
#           KBDSWITCHFGND.15,   &
#           KBDSHELLINIT.16,    &
           KBDCLOSE.17,        &
           KBDFREEFOCUS.18,    &
#           KBDFREE.19,         &
           KBDDEREGISTER.20,   &
           KBDSETFGND.21,      &
           KBDPEEK.22,         &
           KBDOPEN.23,         &
           KBDGETHWID.24,      &
           KBDSETHWID.25

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk

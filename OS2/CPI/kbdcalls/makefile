#
# A Makefile for kbdcalls.dll
# (c) osFree project,
# author, date
#

PROJ     = kbdcalls
DESC     = Keyboard API
srcfiles = $(p)wrap$(e) $(p)dllstart$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod OPTION REDEFSOK
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS =  KbdSetCustXt                SUB32.201, &
           KbdGetCp                    SUB32.203, &
           KbdCharIn                   SUB32.204, &
           KbdSetCp                    SUB32.205, &
           KbdSynch                    SUB32.207, &
           KbdRegister                 SUB32.208, &
           KbdStringIn                 SUB32.209, &
           KbdGetStatus                SUB32.210, &
           KbdSetStatus                SUB32.211, &
           KbdGetFocus                 SUB32.212, &
           KbdFlushBuffer              SUB32.213, &
           KbdXlate                    SUB32.214, &
           KbdClose                    SUB32.217, &
           KbdFreeFocus                SUB32.218, &
           KbdDeRegister               SUB32.220, &
           KbdSetFgnd                  SUB32.221, &
           KbdPeek                     SUB32.222, &
           KbdOpen                     SUB32.223, &
           KbdGetHWID                  SUB32.224, &
           KbdSetHWID                  SUB32.225

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

!include $(%ROOT)/mk/appsos2.mk

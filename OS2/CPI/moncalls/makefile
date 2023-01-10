#
# A Makefile for moncalls.dll
# (c) osFree project,
# author, date
#

PROJ     = moncalls
TRGT     = $(PROJ).dll
DESC     = Monitor API
srcfiles = $(p)dllstart$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod OPTION REDEFSOK
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS =  DosMonWrite                 DOSCALLS.609, &
           DosMonRead                  DOSCALLS.608, &
           DosMonClose                 DOSCALLS.607, &
           DosMonOpen                  DOSCALLS.606, &
           DosMonReg                   DOSCALLS.610 

UNI2H = 1

EXPORTS =  DosMonWrite.1, &
           DosMonRead.2,  &
           DosMonClose.3, &
           DosMonOpen.4,  &
           DosMonReg.5

DEST     = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk

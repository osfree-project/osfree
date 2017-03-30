#
# A Makefile for moncalls.dll
# (c) osFree project,
# author, date
#

PROJ     = moncalls
DESC     = Monitor API
srcfiles = $(p)dllstart$(e) # $(p)wrap$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod OPTION REDEFSOK
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS =  DosMonWrite                 EMXWRAP.401, &
           DosMonRead                  EMXWRAP.402, &
           DosMonClose                 EMXWRAP.403, &
           DosMonOpen                  EMXWRAP.404, &
           DosMonReg                   EMXWRAP.405

UNI2H = 1

EXPORTS =  DosMonWrite.1, &
           DosMonRead.2,  &
           DosMonClose.3, &
           DosMonOpen.4,  &
           DosMonReg.5

#EXPORTS =  DOSMONWRITE.1
#           DOSMONREAD.2
#           DOSMONCLOSE.3
#           DOSMONOPEN.4
#           DOSMONREG.5

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk

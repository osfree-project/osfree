#
# A Makefile for moncalls.dll
# (c) osFree project,
# author, date
#

PROJ     = moncalls
DESC     = Monitor API
srcfiles = $(p)wrap$(e) $(p)dllstart$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod OPTION REDEFSOK
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS =  DosMonWrite                 SUB32.401, &
           DosMonRead                  SUB32.402, &
           DosMonClose                 SUB32.403, &
           DosMonOpen                  SUB32.404, &
           DosMonReg                   SUB32.405

EXPORTS =  DOSMONWRITE.1, &
           DOSMONREAD.2,  &
           DOSMONCLOSE.3, &
           DOSMONOPEN.4,  &
           DOSMONREG.5

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

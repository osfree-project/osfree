#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ     = doscalls
DESC     = Control Programm API
srcfiles = $(p)msg$(e) # $(p)doscopy$(e) $(p)doseditname$(e)
ADD_COPT = -i=..$(SEP)include # -i=$(ROOT)$(SEP)include$(SEP)os3$(SEP)os2 # -i=..\include
ADD_LINKOPT  = op nod lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = DosFSCtl        KAL.1, &
          DosWrite        KAL.2, &
          DosExit         KAL.3, &
          DosQuerySysInfo KAL.4
EXPORTS = DosPutMessage.387, &
          DosWrite.282,      &
          DosExit.234,       &
          DosQuerySysInfo.348

#DOSCOPY.201 RESIDENT, &
#          DOSICOPY.200,         &
#          Dos32Copy.258,        &
#          Dos32EditName.261
DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

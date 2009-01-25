#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ     = msg
DESC     = Message files API
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata, caseexact



IMPORTS = DOSPUTMESSAGE DOSCALLS.383,      &
          DOSTRUEGETMESSAGE DOSCALLS.384,  &
          DOSINSMESSAGE DOSCALLS.385,      &
          DosInsertMessage DOSCALLS.386,   &
          DosPutMessage DOSCALLS.387,      &
          DosTrueGetMessage DOSCALLS.388,  &
          DOSIQUERYMESSAGECP DOSCALLS.389, &
          DosIQueryMessageCP DOSCALLS.390, &
          DOSCLOSEMESSAGEFILE DOSCALLS.649
EXPORTS = DOSPUTMESSAGE.1,      &
          DOSTRUEGETMESSAGE.2,  &
          DOSINSMESSAGE.3,      &
          DosInsertMessage.4,   &
          DosPutMessage.5,      &
          DosTrueGetMessage.6,  &
          DOSIQUERYMESSAGECP.7, &
          DosIQueryMessageCP.8, &
          DOSCLOSEMESSAGEFILE.9
#EXPORTS = &
#          DOSINSMESSAGE.3, &
#          DOSPUTMESSAGE.5, &
#          DOSTRUEGETMESSAGE.2, &
#          DOSIQUERYMESSAGECP.7, &
#          DOSCLOSEMESSAGEFILE.9

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

#
# A Makefile for MSG.DLL
# (c) osFree project,
#

PROJ     = msg
DESC     = Message files API
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..$(SEP)include -s
ADD_LINKOPT  = segment type DATA shared # lib os2386.lib lib clib3r.lib
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata, caseexact



IMPORTS = DosInsertMessage DOSCALLS.386,   &
          DosPutMessage DOSCALLS.387,      &
          DosTrueGetMessage DOSCALLS.388,  &
          DosIQueryMessageCP DOSCALLS.390, &
# Old 16-bit imports. Not required.
          DOSPUTMESSAGE DOSCALLS.383,      &
          DOSTRUEGETMESSAGE DOSCALLS.384,  &
          DOSINSMESSAGE DOSCALLS.385,      &
          DOSIQUERYMESSAGECP DOSCALLS.389, &
          DOSCLOSEMESSAGEFILE DOSCALLS.649
EXPORTS = DosInsertMessage.4,   &
          DosPutMessage.5,      &
          DosTrueGetMessage.6,  &
          DosIQueryMessageCP.8, &
# Old 16-bit exports. Not required.
          DOSPUTMESSAGE.1,      &
          DOSTRUEGETMESSAGE.2,  &
          DOSINSMESSAGE.3,      &
          DOSIQUERYMESSAGECP.7, &
          DOSCLOSEMESSAGEFILE.9


DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

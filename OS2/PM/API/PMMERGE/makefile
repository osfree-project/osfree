#
# A Makefile for pmmerge.dll
# (c) osFree project,
# author, date
#

PROJ     = pmmerge
DESC     = OS/2 PM Merged Library
DIRS     = rectangles habmgr messagemgr framemgr windowmgr input
srcfiles = $(p)framemgr$(SEP)framemgr$(e) $(p)habmgr$(SEP)habmgr$(e) $(p)rectangles$(SEP)rectangles$(e) &
           $(p)input$(SEP)input$(e) $(p)windowmgr$(SEP)WindowClass$(e) &
           $(p)messagemgr$(SEP)queue$(e) $(p)messagemgr$(SEP)messages$(e) $(p)windowmgr$(SEP)windowmgr$(e)
ADD_COPT = -bd -i=$(MYDIR)..$(SEP)include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib &
               segment type DATA shared
DLL     = 1
DLLOPT  = pm initinstance terminstance
OPTIONS = manyautodata
IMPORTS = RectCopy utlapi.1,                    &
          RectSet utlapi.2,                     &
          RectIsEmpty utlapi.3,                 &
          RectEqual utlapi.4,                   &
          RectSetEmpty utlapi.5,                &
          RectOffset utlapi.6,                  &
          RectInflate utlapi.7,                 &
          RectPtIn utlapi.8,                    &
          RectIntersect utlapi.9,               &
          RectUnion utlapi.10,                  &
          RectSubtract utlapi.11,               &
          RectMakeRect utlapi.12,               &
          RectMakePoints utlapi.13,             &
# PMClient communication library
          InitServerConnection pmclient.1,      &
          CloseServerConnection pmclient.2,     &
          _F_SendCmdToServer pmclient.3,        &
          _F_SendDataToServer pmclient.4,       &
          _F_RecvDataFromServer pmclient.5,     &
          _F_SendGenCmdToServer pmclient.6,     &
          _F_SendGenCmdDataToServer pmclient.7, &
          fatal pmclient.8
EXPORTS = WinCopyRect.05127,             &
          WinCreateMsgQueue.05136,       &
          WinCreateStdWindow.05139,      &
          WinDefWindowProc.05147,        &
          WinDestroyMsgQueue.05154,      &
          WinDestroyWindow.05156,        &
          WinDispatchMsg.05158,          &
          WinEqualRect.05173,            &
          WinGetMsg.05192,               &
          WinInflateRect.05200,          &
          WinInitialize.05201,           &
          WinIntersectRect.05203,        &
          WinIsRectEmpty.05209,          &
          WinMakePoints.05229,           &
          WinMakeRect.05230,             &
          WinOffsetRect.05239,           &
          WinPtInRect.05248,             &
          WinRegisterClass.05315,        &
          WinSendMsg.05334,              &
          WinSetRect.05364,              &
          WinSetRectEmpty.05365,         &
          WinSubtractRect.05391,         &
          WinTerminate.05396,            &
          WinUnionRect.05402
DEST     = os2$(SEP)dll
#INSTALL_ADD = 1

!include $(%ROOT)/mk/appsos2.mk

TARGETS = subdirs $(PATH)$(PROJ).dll
TARGET = all

#install_add: .SYMBOLIC
# renmodul /r $^@ $^& fmmerge
# $(CP) pmmerge.dll fmmerge.dll

qwe: .SYMBOLIC
 $(SAY) $(COPT)
 
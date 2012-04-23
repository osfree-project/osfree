#
# A Makefile for pmmerge.dll
# (c) osFree project,
# author, date
#

PROJ     = pmmerge
DESC     = OS/2 PM Merged Library
DIRS     = rectangles habmgr messagemgr framemgr windowmgr input load dialogs errors
srcfiles = $(p)init$(e) $(p)framemgr$(SEP)framemgr$(e) $(p)habmgr$(SEP)habmgr$(e) $(p)rectangles$(SEP)rectangles$(e) &
           $(p)input$(SEP)input$(e) $(p)messagemgr$(SEP)queue$(e) $(p)messagemgr$(SEP)messages$(e) &
           $(p)windowmgr$(SEP)windowmgr$(e) $(p)windowmgr$(SEP)WindowClass$(e) &
           $(p)errors$(SEP)errors$(e) $(p)errors$(SEP)f_errors$(e) $(p)load$(SEP)load$(e) &
           $(p)fc_config$(e) $(p)util$(e) # $(p)dialogs$(SEP)windlgbox$(e)
ADD_COPT = -i=$(MYDIR)..$(SEP)include  -sg -of+
ADD_LINKOPT  = segment   type DATA nonshared lib os2386.lib lib clib3r.lib
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata, caseexact, start=dll_initterm
IMPORTS = RectCopy_ utlapi.1,                    &
          RectSet_ utlapi.2,                     &
          RectIsEmpty_ utlapi.3,                 &
          RectEqual_ utlapi.4,                   &
          RectSetEmpty_ utlapi.5,                &
          RectOffset_ utlapi.6,                  &
          RectInflate_ utlapi.7,                 &
          RectPtIn_ utlapi.8,                    &
          RectIntersect_ utlapi.9,               &
          RectUnion_ utlapi.10,                  &
          RectSubtract_ utlapi.11,               &
          RectMakeRect_ utlapi.12,               &
          RectMakePoints_ utlapi.13
EXPORTS = WinCopyRect.05127,             &
          WinCreateMsgQueue.05136,       &
          WinCreateStdWindow.05139,      &
          WinDefWindowProc.05147,        &
          WinDeleteLibrary.05149,        &
          WinDeleteProcedure.05150,      &
          WinDestroyMsgQueue.05154,      &
          WinDestroyWindow.05156,        &
          WinDispatchMsg.05158,          &
          WinEqualRect.05173,            &
          WinGetMsg.05192,               &
          WinInflateRect.05200,          &
          WinInitialize.05201,           &
          WinIntersectRect.05203,        &
          WinIsRectEmpty.05209,          &
          WinIsWindow.05211,             &
          WinLoadLibrary.05219,          &
          WinLoadProcedure.05223,        &
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

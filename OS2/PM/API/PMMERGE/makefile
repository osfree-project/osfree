#
# A Makefile for pmmerge.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
PROJ     = pmmerge
DESC     = OS/2 PM Merged Library
OBJS     = framemgr.$(O) habmgr.$(O) rectangles.$(O) input.$(O) WindowClass.$(O) queue.$(O) messages.$(O) windowmgr.$(O)
ADD_COPT = -bd -i=..$(SEP)include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib path rectangles$(PS)habmgr$(PS)messagemgr$(PS)framemgr$(PS)windowmgr$(PS)input
DIRS     = rectangles habmgr messagemgr framemgr windowmgr input

!include $(%ROOT)/mk/all.mk

all: subs $(PROJ).dll .SYMBOLIC

subs: .SYMBOLIC
  $(MAKE) $(MAKEOPT) TARGET=all subdirs

$(PROJ).dll: .symbolic
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk SYSTEM os2v2 dll initglobal pm
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk import
 @%append $^&.lnk   RectCopy utlapi.1,
 @%append $^&.lnk   RectSet utlapi.2,
 @%append $^&.lnk   RectIsEmpty utlapi.3,
 @%append $^&.lnk   RectEqual utlapi.4,
 @%append $^&.lnk   RectSetEmpty utlapi.5,
 @%append $^&.lnk   RectOffset utlapi.6,
 @%append $^&.lnk   RectInflate utlapi.7,
 @%append $^&.lnk   RectPtIn utlapi.8,
 @%append $^&.lnk   RectIntersect utlapi.9,
 @%append $^&.lnk   RectUnion utlapi.10,
 @%append $^&.lnk   RectSubtract utlapi.11,
 @%append $^&.lnk   RectMakeRect utlapi.12,
 @%append $^&.lnk   RectMakePoints utlapi.13,
# PMClient communication library
 @%append $^&.lnk   InitServerConnection pmclient.1,
 @%append $^&.lnk   CloseServerConnection pmclient.2,
 @%append $^&.lnk   _F_SendCmdToServer pmclient.3,
 @%append $^&.lnk   _F_SendDataToServer pmclient.4,
 @%append $^&.lnk   _F_RecvDataFromServer pmclient.5,
 @%append $^&.lnk   _F_SendGenCmdToServer pmclient.6,
 @%append $^&.lnk   _F_SendGenCmdDataToServer pmclient.7,
 @%append $^&.lnk   fatal pmclient.8
 @%append $^&.lnk export
 @%append $^&.lnk   WinCopyRect.05127,
 @%append $^&.lnk   WinCreateMsgQueue.05136,
 @%append $^&.lnk   WinCreateStdWindow.05139,
 @%append $^&.lnk   WinDefWindowProc.05147,
 @%append $^&.lnk   WinDestroyMsgQueue.05154,
 @%append $^&.lnk   WinDestroyWindow.05156,
 @%append $^&.lnk   WinDispatchMsg.05158,
 @%append $^&.lnk   WinEqualRect.05173,
 @%append $^&.lnk   WinGetMsg.05192,
 @%append $^&.lnk   WinInflateRect.05200,
 @%append $^&.lnk   WinInitialize.05201,
 @%append $^&.lnk   WinIntersectRect.05203,
 @%append $^&.lnk   WinIsRectEmpty.05209,
 @%append $^&.lnk   WinMakePoints.05229,
 @%append $^&.lnk   WinMakeRect.05230,
 @%append $^&.lnk   WinOffsetRect.05239,
 @%append $^&.lnk   WinPtInRect.05248,
 @%append $^&.lnk   WinRegisterClass.05315,
 @%append $^&.lnk   WinSendMsg.05334,
 @%append $^&.lnk   WinSetRect.05364,
 @%append $^&.lnk   WinSetRectEmpty.05365,
 @%append $^&.lnk   WinSubtractRect.05391,
 @%append $^&.lnk   WinTerminate.05396,
 @%append $^&.lnk   WinUnionRect.05402

 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)
 renmodul /r $^@ $^& fmmerge
 $(CP) pmmerge.dll fmmerge.dll

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
 $(MAKE) $(MAKEOPT) TARGET=clean subdirs

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) fmmerge.dll $(FILESDIR)$(SEP)os2$(SEP)dll

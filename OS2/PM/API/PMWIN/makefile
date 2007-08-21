#
# A Makefile for pmdrag.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ     = pmwin
DESC     = Drag and Drop API
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    quiet
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk $(LINKOPT)
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.map
 @%append $^&.lnk import
 @%append $^&.lnk   WinInitialize      PMMERGE.5201,
 @%append $^&.lnk   WinTerminate       PMMERGE.5396,
 @%append $^&.lnk   WinCreateMsgQueue  PMMERGE.5136,
 @%append $^&.lnk   WinDestroyMsgQueue PMMERGE.5154,
 @%append $^&.lnk   WinDispatchMsg     PMMERGE.5158,
 @%append $^&.lnk   WinGetMsg          PMMERGE.5192,
 @%append $^&.lnk   WinSendMsg         PMMERGE.5334,
 @%append $^&.lnk   WinRegisterClass   PMMERGE.5315,
 @%append $^&.lnk   WinCreateStdWindow PMMERGE.5139,
 @%append $^&.lnk   WinDefWindowProc   PMMERGE.5147,
 @%append $^&.lnk   WinDestroyWindow   PMMERGE.5156
 @%append $^&.lnk export
 @%append $^&.lnk   WinInitialize.            00763,
 @%append $^&.lnk   WinTerminate.             00888,
 @%append $^&.lnk   WinCreateMsgQueue.        00716,
 @%append $^&.lnk   WinDestroyMsgQueue.       00726,
 @%append $^&.lnk   WinDestroyWindow.         00728,
 @%append $^&.lnk   WinCreateStdWindow.       00908,
 @%append $^&.lnk   WinDefWindowProc.         00911,
 @%append $^&.lnk   WinDispatchMsg.           00912,
 @%append $^&.lnk   WinGetMsg.                00915,
 @%append $^&.lnk   WinSendMsg.               00920,
 @%append $^&.lnk   WinRegisterClass.         00926
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) @$^&.lnk $(LOG)
 renmodul /r $^@ pmwin fmwin
 $(CP) pmwin.dll fmwin.dll
 renmodul /i fmwin.dll pmmerge fmmerge

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) fmwin.dll $(FILESDIR)$(SEP)os2$(SEP)dll

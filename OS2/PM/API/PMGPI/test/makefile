PROJECT = testgpi
DESCRIPTION = Minimal PMGPI tests

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
!endif

ADD_COPT += -i=$(SRCDIR)..

IMPORTS = WinInitialize      PMWIN.246 &
          WinCreateMsgQueue  PMWIN.58  &
          WinTerminate  PMWIN.247 &
          WinGetScreenPS  PMWIN.42 &
          WinDestroyMsgQueue   PMWIN.59 &
          GpiSetColor   PMGPI.108 &
          GpiMove PMGPI.128 &
          GpiLine PMGPI.129 &
          GpiBox PMGPI.131 &
          WinReleasePS PMWIN.44

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

.c: $(SRCDIR)

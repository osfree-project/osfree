PROJECT = testgpi
DESCRIPTION = Minimal PMGPI tests
SOURCES = testgpi

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
SOURCES += win_compat
!endif

ADD_COPT += -i=$(SRCDIR)..

IMPORTS = WinInitialize      PMWIN.246, &
          WinCreateMsgQueue  PMWIN.58,  &
          WinTerminate       PMWIN.247, &
          WinGetScreenPS     PMWIN.42,  &
          WinDestroyMsgQueue PMWIN.59,  &
          GpiSetColor        PMGPI.517, &
          GpiMove            PMGPI.404, &
          GpiLine            PMGPI.398, &
          GpiBox             PMGPI.356, &
          WinReleasePS       PMWIN.44

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

.c: $(SRCDIR)

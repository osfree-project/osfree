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
          WinReleasePS       PMWIN.44, &
          GpiSetDrawingMode  PMGPI.522, &
          GpiOpenSegment     PMGPI.408, &
          GpiCloseSegment    PMGPI.361, &
          GpiDrawSegment     PMGPI.383, &
          GpiQuerySegmentNames    PMGPI.483, &
          GpiQuerySegmentAttrs    PMGPI.482, &
          GpiSetSegmentAttrs PMGPI.547, &
          GpiGetData         PMGPI.394, &
          GpiPutData         PMGPI.421, &
          GpiDeleteSegment   PMGPI.376, &
          GpiDrawChain       PMGPI.380, &
          GpiDrawDynamics    PMGPI.381, &
          GpiRemoveDynamics  PMGPI.496, &
          GpiDrawFrom        PMGPI.382, &
          GpiQuerySegmentPriority         PMGPI.484, &
          GpiSetSegmentPriority           PMGPI.548


!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

.c: $(SRCDIR)

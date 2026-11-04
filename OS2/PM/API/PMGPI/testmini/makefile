PROJECT = testgpi
DESCRIPTION = Minimal PMGPI tests
SOURCES = testmini

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
SOURCES += win_compat
!endif

ADD_COPT += -i=$(SRCDIR).. -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)include

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
          GpiSetSegmentPriority           PMGPI.548, &
          GpiDeleteSegments               PMGPI.377, &
          GpiSetEditMode                  PMGPI.523, &
          GpiLabel                        PMGPI.397, &
          GpiBeginElement                 PMGPI.353, &
          GpiEndElement                   PMGPI.386, &
          GpiSetElementPointer            PMGPI.524, &
          GpiQueryElementType             PMGPI.451, &
          GpiQueryElement                 PMGPI.449, &
          GpiQueryElementPointer          PMGPI.450, &
          GpiSetElementPointerAtLabel     PMGPI.525, &
          GpiOffsetElementPointer         PMGPI.406, &
          GpiElement                      PMGPI.384, &
          GpiDeleteElement                PMGPI.372, &
          GpiDeleteElementRange           PMGPI.373, &
          GpiDeleteElementsBetweenLabels  PMGPI.374, &
          GpiBeginPath                    PMGPI.354, &
          GpiEndPath                      PMGPI.387, &
          GpiFillPath                     PMGPI.392, &
          GpiCloseFigure                  PMGPI.360, &
          GpiOutlinePath                  PMGPI.563, &
          GpiModifyPath                   PMGPI.403, &
          GpiStrokePath                   PMGPI.554, &
          GpiSetClipPath                  PMGPI.515, &
          GpiCharString                   PMGPI.358, &
          GpiPolyLine                     PMGPI.415, &
          GpiCreatePS                     PMGPI.369, &
          GpiDestroyPS                    PMGPI.379, &
          GpiQueryPS                      PMGPI.471, &
          GpiSetPS                        PMGPI.539, &
          GpiQueryDevice                  PMGPI.444, &
          GpiAssociate                    PMGPI.351, &
          GpiErase                        PMGPI.389, &
          GpiResetPS                      PMGPI.498, &
          GpiQueryDrawingMode             PMGPI.447, &
          GpiSavePS                       PMGPI.501, &
          GpiRestorePS                    PMGPI.499, &
          GpiSetStopDraw                  PMGPI.550, &
          GpiSetDrawControl               PMGPI.521, &
          GpiMarker                       PMGPI.402, &
          GpiPolyMarker                   PMGPI.416, &
          GpiCharStringAt                 PMGPI.359, &
          GpiSetArcParams                 PMGPI.502, &
          GpiSetAttrs                     PMGPI.588, &
          GpiQueryColor                   PMGPI.437, &
          GpiQueryCurrentPosition         PMGPI.441, &
          GpiQueryMix                     PMGPI.467, &
          GpiQueryDrawControl             PMGPI.446, &
          GpiQueryStopDraw                PMGPI.487, &
          GpiQueryEditMode                PMGPI.448, &
          GpiSetInitialSegmentAttrs       PMGPI.527, &
          GpiQueryInitialSegmentAttrs     PMGPI.455

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

.c: $(SRCDIR)

#
# A Makefile for gre.dll
# (c) osFree project
#

PROJECT      = gre
DESCRIPTION  = FreePM gre
TARGET_CLASS = LIBRARY
SOURCES      = gre

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
OPTIONS = manyautodata,
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT += -bt=nt
!endif

IMPORTS = F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16

ADD_COPT     += -sg -of+ -bd -5s -bm -fp5 -ei -i=$(SRCDIR) &
                          -i=$(SRCDIR)..$(SEP)include &
                          -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)include


ADD_LINKOPT  = segment   type DATA nonshared
NOLIBS       = 1
DLLOPT       = initinstance terminstance
OPTIONS     += caseexact



!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

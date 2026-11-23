#
# A Makefile for goca.dll
# (c) osFree project
#

PROJECT      = goca
DESCRIPTION  = FreePM goca engine
TARGET_CLASS = LIBRARY

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
OPTIONS = manyautodata,
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT += -bt=nt
!endif

IMPORTS = F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16, &
          Gre32Entry4      PMGRE, &
          Gre32Entry5      PMGRE, &
          Gre32Entry6      PMGRE, &
          Gre32Entry7      PMGRE, &
          Gre32Entry10     PMGRE

EXPORTS = GocaInit, GocaParseStream

ADD_COPT     +=  -fp5 -ei -i=$(SRCDIR) -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)include -i=$(SRCDIR)..$(SEP)include  -sg -of+ -bd -5s -bm

ADD_LINKOPT  = segment   type DATA nonshared
NOLIBS       = 1
DLLOPT       = initinstance terminstance
OPTIONS     += caseexact

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

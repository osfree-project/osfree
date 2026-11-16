#
# A Makefile for gre.dll
# (c) osFree project
#

PROJECT      = gre
DESCRIPTION  = FreePM gre
TARGET_CLASS = LIBRARY

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
OPTIONS = manyautodata,
IMPORTS = F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT += -bt=nt
IMPORTS = F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16
!endif


ADD_COPT     +=  -fp5 -ei -i=$(SRCDIR) -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)include -i=$(SRCDIR)..$(SEP)include  -sg -of+ -bd -5s -bm

ADD_LINKOPT  = segment   type DATA nonshared
NOLIBS       = 1
DLLOPT       = initinstance terminstance
OPTIONS     += caseexact



!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

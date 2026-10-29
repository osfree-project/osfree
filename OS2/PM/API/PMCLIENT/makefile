#
# A Makefile for pmclient.dll
# (c) osFree project
#

PROJECT      = pmclient
DESCRIPTION  = FreePM client
TARGET_CLASS = LIBRARY

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
OPTIONS = manyautodata,
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT += -bt=nt
!endif

ADD_COPT     = -i=$(SRCDIR). -sg -of+ -bd -3s

ADD_LINKOPT  = segment   type DATA nonshared
NOLIBS       = 1
DLLOPT       = initinstance terminstance
OPTIONS     += caseexact

EXPORTS      = &
    PmcPostCommand             .1, &
    PmcSendData                .2, &
    PmcGetData                 .3, &
    PmcSendCommand             .4, &
    PmcSendCommandData         .5

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

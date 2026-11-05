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

ADD_COPT     += -i=$(SRCDIR) -i=$(SRCDIR)..$(SEP)..$(SEP)include -i=$(SRCDIR)..$(SEP)include  -sg -of+ -bd -3s -bm

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

IMPORTS      =  &
#                F_DbgInit        PMDEBUG.1, &
                F_DbgSetLevels   PMDEBUG.2, &
#                F_DbgRotate      PMDEBUG.3, &
#                F_DbgSetWireSink PMDEBUG.4, &
                F_DbgPrint       PMDEBUG.5, &
#                F_DbgTrace       PMDEBUG.6, &
#                F_DbgFatal       PMDEBUG.7, &
#                F_DbgFatalf      PMDEBUG.8, &
#                F_DbgAssert      PMDEBUG.9, &
#                F_DbgCtxEnter    PMDEBUG.10, &
#                F_DbgCtxExit     PMDEBUG.11, &
                F_DbgSetShuttingDown PMDEBUG.12, &
#                F_DbgSetConfig   PMDEBUG.13, &
#                F_DbgFlush       PMDEBUG.14, &
                F_DbgCtxEnterf   PMDEBUG.15, &
                F_DbgCtxExitf    PMDEBUG.16

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

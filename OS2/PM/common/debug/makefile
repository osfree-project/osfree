#
# A Makefile for pmclient.dll
# (c) osFree project
#

PROJECT      = pmdebug
DESCRIPTION  = FreePM debug subsystem
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

EXPORTS      =  F_DbgInit        .1, &
                F_DbgSetLevels   .2, &
                F_DbgRotate      .3, &
                F_DbgSetWireSink .4, &
                F_DbgPrint       .5, &
#                F_DbgTrace       .6, &
                F_DbgFatal       .7, &
                F_DbgFatalf      .8, &
                F_DbgAssert      .9, &
                F_DbgCtxEnter    .10, &
                F_DbgCtxExit     .11, &
                F_DbgSetShuttingDown .12, &
                F_DbgSetConfig   .13, &
                F_DbgFlush       .14, &
                F_DbgCtxEnterf   .15, &
                F_DbgCtxExitf    .16

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

PROJECT = debugtest
DESCRIPTION = tests
IMPORTS      =  F_DbgInit        PMDEBUG.1, &
                F_DbgSetLevels   PMDEBUG.2, &
                F_DbgRotate      PMDEBUG.3, &
                F_DbgSetWireSink PMDEBUG.4, &
                F_DbgPrint       PMDEBUG.5, &
                F_DbgTrace       PMDEBUG.6, &
                F_DbgFatal       PMDEBUG.7, &
                F_DbgFatalf      PMDEBUG.8, &
                F_DbgAssert      PMDEBUG.9, &
                F_DbgCtxEnter    PMDEBUG.10, &
                F_DbgCtxExit     PMDEBUG.11, &
                F_DbgSetShuttingDown PMDEBUG.12

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
LIBS = tcpip32
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
ADD_LINKOPT = lib wsock32.lib
!endif

ADD_COPT += -bm -i=$(SRCDIR).. -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)include -i=$(SRCDIR)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

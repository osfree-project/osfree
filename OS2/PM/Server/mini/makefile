PROJECT = pmserver
DESCRIPTION = tests
IMPORTS = _InitServerConnection PMTCP.1,       &
           _CloseServerConnection PMTCP.2,     &
           _startServerThreads PMTCP.3,        &
           _F_SendCmdToServer PMTCP.4,         &
           _F_SendDataToServer PMTCP.5,        &
           _F_RecvDataFromServer PMTCP.6,      &
           _F_SendGenCmdToServer PMTCP.7,      &
           _F_SendGenCmdDataToServer PMTCP.8,  &
           _F_RecvCmdFromClient PMTCP.9,       &
           _F_RecvDataFromClient PMTCP.10,     &
           _F_SendDataToClient PMTCP.11,       &
                F_DbgInit        PMDEBUG.1, &
#                F_DbgSetLevels   PMDEBUG.2, &
#                F_DbgRotate      PMDEBUG.3, &
#                F_DbgSetWireSink PMDEBUG.4, &
                F_DbgPrint       PMDEBUG.5, &
#                F_DbgTrace       PMDEBUG.6, &
#                F_DbgFatal       PMDEBUG.7, &
#                F_DbgFatalf      PMDEBUG.8, &
#                F_DbgAssert      PMDEBUG.9, &
#                F_DbgCtxEnter    PMDEBUG.10, &
#                F_DbgCtxExit     PMDEBUG.11, &
#                F_DbgSetShuttingDown PMDEBUG.12, &
#                F_DbgSetConfig   PMDEBUG.13, &
#                F_DbgFlush       PMDEBUG.14, &
                F_DbgCtxEnterf   PMDEBUG.15, &
                F_DbgCtxExitf    PMDEBUG.16

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
!endif

ADD_COPT += -bm -i=$(SRCDIR) -i=$(SRCDIR)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

.c: $(SRCDIR)

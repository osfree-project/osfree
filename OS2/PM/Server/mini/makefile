PROJECT = pmserver
DESCRIPTION = tests
IMPORTS = PmcSendCommand PMCLIENT.1
IMPORTS = _InitServerConnection PMTCP.1,      &
           _CloseServerConnection PMTCP.2,     &
           _startServerThreads PMTCP.3,        &
           _F_SendCmdToServer PMTCP.4,         &
           _F_SendDataToServer PMTCP.5,        &
           _F_RecvDataFromServer PMTCP.6,      &
           _F_SendGenCmdToServer PMTCP.7,      &
           _F_SendGenCmdDataToServer PMTCP.8,  &
           _F_RecvCmdFromClient PMTCP.9,       &
           _F_RecvDataFromClient PMTCP.10,     &
           _F_SendDataToClient PMTCP.11

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
!endif

ADD_COPT += -i=$(SRCDIR)..

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

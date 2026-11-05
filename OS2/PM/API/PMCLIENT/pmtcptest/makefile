PROJECT = pmtcptest
DESCRIPTION = tests
IMPORTS = PmcPostCommand             PMCLIENT.1, &
          PmcSendData                PMCLIENT.2, &
          PmcGetData                 PMCLIENT.3, &
          PmcSendCommand             PMCLIENT.4, &
          PmcSendCommandData         PMCLIENT.5

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
LIBS = tcpip32
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
ADD_LINKOPT = lib wsock32.lib
!endif

ADD_COPT += -i=$(SRCDIR).. -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)include -i=$(SRCDIR)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

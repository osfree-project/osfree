PROJECT = pmtcptest
DESCRIPTION = tests
IMPORTS = Fc_TrnConnect PMTCP.1,              &
          Fc_TrnDisconnect PMTCP.2,           &
          Fc_TrnPostCommand PMTCP.3,          &
          Fc_TrnPostData PMTCP.4,             &
          Fc_TrnSendCommand PMTCP.5,          &
          Fc_TrnSendCommandData PMTCP.6,      &
          Fc_TrnGetData PMTCP.7,              &
          Fs_TrnRunListener PMTCP.8,          &
          Fs_TrnPostData PMTCP.9,             &
          Fs_TrnGetCommand PMTCP.10,          &
          Fs_TrnGetData PMTCP.11

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
LIBS = tcpip32
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
ADD_LINKOPT = lib wsock32.lib
!endif

ADD_COPT += -bm -i=$(SRCDIR).. -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

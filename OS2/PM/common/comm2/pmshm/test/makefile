PROJECT = test_pmshm
DESCRIPTION = tests
IMPORTS = Fc_TrnConnect PMSHM.1,              &
          Fc_TrnDisconnect PMSHM.2,           &
          Fc_TrnPostCommand PMSHM.3,          &
          Fc_TrnPostData PMSHM.4,             &
          Fc_TrnSendCommand PMSHM.5,          &
          Fc_TrnSendCommandData PMSHM.6,      &
          Fc_TrnGetData PMSHM.7,              &
          Fs_TrnRunListener PMSHM.8,          &
          Fs_TrnPostData PMSHM.9,             &
          Fs_TrnGetCommand PMSHM.10,          &
          Fs_TrnGetData PMSHM.11, &
          F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16


!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
LIBS = tcpip32
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
#ADD_LINKOPT = lib wsock32.lib
!endif

ADD_COPT += -bm -i=$(SRCDIR).. -i=$(SRCDIR)..$(SEP)..$(SEP)include -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

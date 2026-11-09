PROJECT = test_pmpipe
DESCRIPTION = tests
IMPORTS = Fc_TrnConnect PMPIPE.1,              &
          Fc_TrnDisconnect PMPIPE.2,           &
          Fc_TrnPostCommand PMPIPE.3,          &
          Fc_TrnPostData PMPIPE.4,             &
          Fc_TrnSendCommand PMPIPE.5,          &
          Fc_TrnSendCommandData PMPIPE.6,      &
          Fc_TrnGetData PMPIPE.7,              &
          Fs_TrnRunListener PMPIPE.8,          &
          Fs_TrnPostData PMPIPE.9,             &
          Fs_TrnGetCommand PMPIPE.10,          &
          Fs_TrnGetData PMPIPE.11, &
          F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16


!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
#LIBS = tcpip32
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
#ADD_LINKOPT = lib wsock32.lib
!endif

ADD_COPT += -bm -i=$(SRCDIR).. -i=$(SRCDIR)..$(SEP)..$(SEP)include -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)..$(SEP)include

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

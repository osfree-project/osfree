PROJECT = test_pmhab_mt
DESCRIPTION = tests
IMPORTS = Fc_SesPostCommand         PMCLIENT.1, &
          Fc_SesPostData            PMCLIENT.2, &
          Fc_SesGetData             PMCLIENT.3, &
          Fc_SesSendCommand         PMCLIENT.4, &
          Fc_SesConnect             PMCLIENT.6, &
          Fc_SesDisconnect          PMCLIENT.7, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16

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

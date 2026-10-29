PROJECT = pmclient
DESCRIPTION = tests
IMPORTS = PmcSendCommand PMCLIENT.4
#    PmcPostCommand             .1, &
#    PmcSendData                .2, &
#    PmcGetData                 .3, &
#    PmcSendCommand             .4, &
#    PmcSendCommandData         .5

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT = -bt=nt
!endif

ADD_COPT += -i=$(SRCDIR)..

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)

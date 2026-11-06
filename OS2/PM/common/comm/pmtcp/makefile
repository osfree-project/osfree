#  Makefile variables for FreePM
#
#  Viking

#
# A Makefile for pmclient.dll
# (c) osFree project
#

PROJECT      = pmtcp
DESCRIPTION  = FreePM TCP communication backend
TARGET_CLASS = LIBRARY

ADD_COPT = -bm -sg

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
OPTIONS = manyautodata,
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT += -bt=nt
ADD_LINKOPT = lib wsock32.lib
!endif

ADD_COPT     += -i=$(SRCDIR). -i=$(SRCDIR)..$(SEP)..$(SEP)..$(SEP)include -sg -of+ -bd -3s

ADD_LINKOPT += segment   type DATA nonshared
NOLIBS       = 1
DLLOPT       = initinstance terminstance
OPTIONS     += caseexact

EXPORTS = Fc_TrnConnect.1,              &
          Fc_TrnDisconnect.2,           &
          Fc_TrnPostCommand.3,          &
          Fc_TrnPostData.4,             &
          Fc_TrnSendCommand.5,          &
          Fc_TrnSendCommandData.6,      &
          Fc_TrnGetData.7,              &
          Fs_TrnRunListener.8,          &
          Fs_TrnPostData.9,             &
          Fs_TrnGetCommand.10,          &
          Fs_TrnGetData.11

IMPORTS = F_DbgPrint       PMDEBUG.5, &
          F_DbgCtxEnterf   PMDEBUG.15, &
          F_DbgCtxExitf    PMDEBUG.16

!include $(%ROOT)tools/mk/build.mk

.cpp: $(SRCDIR)


#
#  option stack=3m
# Strange, the server crashes with debug build! No, it crashes for some else reason.
# Found it! To little heapsize! Was only 287 KiB while it assumed 2.6 MiB!
# It must be set with stack= not with heapsize= !
# Changing the stack size has no effect at all.
# Increased the stack for threads
# 65 KiB from 32 KB. Hope it works or else I have to change the stack manually.
# Fixed! With help from Valerius. Added the switch "-sg" which grows the stack.

#ADD_COPT = -bm -sg -s
#ADD_LINKOPT  = segment type DATA nonshared lib tcpip32.lib
#
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

#srcfiles = $(p)F_tcp$(e) $(p)Ft_main$(e) $(p)F_globals$(e) $(p)F_utils$(e) &
#           $(p)F_errors$(e) $(p)F_debug$(e)

!ifndef TARGETBLD
SRCDIR  = $(MYDIR)
OPTIONS = manyautodata,
!else
SRCDIR  = $(MYDIR)..$(SEP)
ADD_COPT += -bt=nt
ADD_LINKOPT = lib wsock32.lib
!endif

ADD_COPT     = -i=$(SRCDIR). -sg -of+ -bd -3s

ADD_LINKOPT += segment   type DATA nonshared
NOLIBS       = 1
DLLOPT       = initinstance terminstance
OPTIONS     += caseexact

EXPORTS  = _InitServerConnection.1,      &
           _CloseServerConnection.2,     &
           _startServerThreads.3,        &
           _F_SendCmdToServer.4,         &
           _F_SendDataToServer.5,        &
           _F_RecvDataFromServer.6,      &
           _F_SendGenCmdToServer.7,      &
           _F_SendGenCmdDataToServer.8,  &
           _F_RecvCmdFromClient.9,       &
           _F_RecvDataFromClient.10,     &
           _F_SendDataToClient.11

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
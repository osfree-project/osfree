DBG = 0

!ifeq DBG 1
OPT         = -d3 -of -od
!else
OPT         = -d0 -oteanx
!endif

PROJ        = rxsock
TRGT        = $(PROJ).dll
DESC        = REXX Socket interface
ADD_COPT    = -dDYNAMIC -dHAVE_CONFIG_H -i=$(MYDIR) -bm -wx -s -mf $(OPT) -6r -fp6 # -6s
ADD_LINKOPT = lib tcpip32.lib #lib libuls.lib, libconv.lib
DLL         = 1
# UNI2H       = 1
DLLOPT      = initinstance terminstance
OPTIONS     = manyautodata
STACKSIZE   = 0x20000

srcfiles    = $(p)conversions$(e) $(p)getopt$(e) $(p)loader$(e) $(p)rxpack$(e) $(p)rxsock$(e)
EXPORTS     = &
        SOCKACCEPT.1          = SockAccept, &
        SOCKBIND.2            = SockBind, &
        SOCKCLOSE.3           = SockClose, &
        SOCKCONNECT.4         = SockConnect, &
        SOCKDROPFUNCS.5       = SockDropFuncs, &
        SOCKGETHOSTBYADDR.6   = SockGetHostByAddr, &
        SOCKGETHOSTBYNAME.7   = SockGetHostByName, &
        SOCKGETHOSTID.8       = SockGetHostId, &
        SOCKGETPEERNAME.9     = SockGetPeerName, &
        SOCKGETSOCKNAME.10    = SockGetSockName, &
        SOCKGETSOCKOPT.11     = SockGetSockOpt, &
        SOCKINIT.12           = SockInit, &
        SOCKIOCTL.13          = SockIoctl, &
        SOCKLISTEN.14         = SockListen, &
        SOCKPSOCK_ERRNO.15    = SockPSock_Errno, &
        SOCKRECV.16           = SockRecv, &
        SOCKRECVFROM.17       = SockRecvFrom, &
        SOCKSELECT.18         = SockSelect, &
        SOCKSEND.19           = SockSend, &
        SOCKSENDTO.20         = SockSendTo, &
        SOCKSETSOCKOPT.21     = SockSetSockOpt, &
        SOCKSHUTDOWN.22       = SockShutDown, &
        SOCKSOCK_ERRNO.23     = SockSock_Errno, &
        SOCKSOCKET.24         = SockSocket, &
        SOCKSOCLOSE.25        = SockSoClose, &
        SOCKVERSION.26        = SockVersion, &
        RXSOCKVERSION.27      = RxsockVersion, &
        SOCKVARIABLE.28       = SockVariable, &
        SOCKLOADFUNCS.29      = SockLoadFuncs

# missing!
#        SockFunctionGateWay.2 = SockFunctionGateWay

DEST        = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk

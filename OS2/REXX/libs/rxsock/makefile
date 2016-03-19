DBG = 0

!ifeq DBG 1
OPT         = -d3 -of -od
!else
OPT         = -d0 -oteanx
!endif

PROJ        = rxsock
DESC        = REXX Socket interface
ADD_COPT    = -dUSE_OS2REXX -dHAVE_CONFIG_H -i=$(MYDIR) -bd -bm -wx -s -mf $(OPT) -6s -fp6 -bd
ADD_LINKOPT = #lib libuls.lib, libconv.lib
DLL         = 1
DLLOPT      = initinstance
OPTIONS     = manyautodata
STACKSIZE   = 0x20000

srcfiles    = $(p)conversions$(e) $(p)getopt$(e) $(p)loader$(e) $(p)rxpack$(e) $(p)rxsock$(e)
EXPORTS     = &
        SockLoadFuncs.1       = SockLoadFuncs, &
        SockAccept, &
        SockBind, &
        SockClose, &
        SockConnect, &
        SockDropFuncs, &
        SockGetHostByAddr, &
        SockGetHostByName, &
        SockGetHostId, &
        SockGetPeerName, &
        SockGetSockName, &
        SockGetSockOpt, &
        SockInit, &
        SockIoctl, &
        SockListen, &
        SockLoadFuncs, &
        SockPSock_Errno, &
        SockRecv, &
        SockRecvFrom, &
        SockSelect, &
        SockSend, &
        SockSendTo, &
        SockSetSockOpt, &
        SockShutDown, &
        SockSock_Errno, &
        SockSocket, &
        SockSoClose, &
        SockVersion, &
        SockVariable, &
        RxsockVersion

# missing!
#        SockFunctionGateWay.2 = SockFunctionGateWay

DEST        = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk

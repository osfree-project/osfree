#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = ssm
TRGT     = $(PROJ).dll
DESC     = SSM
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = SMHEntryPoint                 MMPM.0131, &
          SpiCreateStream               MMPM.0132, &
          SpiGetHandler                 MMPM.0133, &
          SpiStartStream                MMPM.0134, &
          SpiAssociate                  MMPM.0135, &
          SpiDestroyStream              MMPM.0136, &
          SpiSeekStream                 MMPM.0137, &
          SpiStopStream                 MMPM.0138, &
          SpiEnableEvent                MMPM.0139, &
          SpiDisableEvent               MMPM.0140, &
          SpiEnableSync                 MMPM.0141, &
          SpiDisableSync                MMPM.0142, &
          SpiInstallProtocol            MMPM.0143, &
          SpiGetProtocol                MMPM.0144, &
          SpiGetTime                    MMPM.0145, &
          SpiEnumerateHandlers          MMPM.0146, &
          SpiEnumerateProtocols         MMPM.0147, &
          SpiDetermineSyncMaster        MMPM.0148, &
          SpiSendMsg                    MMPM.0149, &
          SpiDebug                      MMPM.0150, &
          SpiAddStreamToNetwork         MMPM.0151, &
          SpiAssembleStreamNetwork      MMPM.0211, &
          SpiCloseStreamNetwork         MMPM.0212, &
          SpiConnectStream              MMPM.0213, &
          SpiOpenStreamNetwork          MMPM.0214, &
          SpiQueryStreamInfo            MMPM.0215, &
          SpiReactivateStream           MMPM.0216, &
          SpiRemoveStreamFromNetwork    MMPM.0217, &
          SpiSeekStreamNetwork          MMPM.0218, &
          SpiStartStreamNetwork         MMPM.0219, &
          SpiStopStreamNetwork          MMPM.0220, &
          SpiUnassembleStreamNetwork    MMPM.0221, &
          SpiEnableNetworkEvent         MMPM.0222, &
          SpiGetNetworkTime             MMPM.0223, &
          SpiUnloadHandler              MMPM.0229, &
          SpiGetHandler2                MMPM.0230
EXPORTS = SMHEntryPoint.1, &
          SpiCreateStream.2, &
          SpiGetHandler.3, &
          SpiStartStream.4, &
          SpiAssociate.5, &
          SpiDestroyStream.6, &
          SpiSeekStream.7, &
          SpiStopStream.8, &
          SpiEnableEvent.9, &
          SpiDisableEvent.10, &
          SpiEnableSync.11, &
          SpiDisableSync.12, &
          SpiInstallProtocol.13, &
          SpiGetProtocol.14, &
          SpiGetTime.15, &
          SpiEnumerateHandlers.16, &
          SpiEnumerateProtocols.17, &
          SpiDetermineSyncMaster.18, &
          SpiSendMsg.19, &
          SpiDebug.20, &
          SpiAddStreamToNetwork.21, &
          SpiAssembleStreamNetwork.22, &
          SpiCloseStreamNetwork.23, &
          SpiConnectStream.24, &
          SpiOpenStreamNetwork.25, &
          SpiQueryStreamInfo.26, &
          SpiReactivateStream.27, &
          SpiRemoveStreamFromNetwork.28, &
          SpiSeekStreamNetwork.29, &
          SpiStartStreamNetwork.30, &
          SpiStopStreamNetwork.31, &
          SpiUnassembleStreamNetwork.32, &
          SpiEnableNetworkEvent.33, &
          SpiGetNetworkTime.34, &
          SpiUnloadHandler.35, &
          SpiGetHandler2.36

DEST    = mmos2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk

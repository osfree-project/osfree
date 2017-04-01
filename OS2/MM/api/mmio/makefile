#
# A Makefile for ansicall.dll
# (c) osFree project
#

PROJ     = mmio
TRGT     = $(PROJ).dll
DESC     = MMIO
OBJS     = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
IMPORTS = mmiohlpCleanupInstance                MMPM.0015, &
          mmiohlpListCreate                     MMPM.0016, &
          mmiohlpListAdd                        MMPM.0017, &
          mmiohlpListGet                        MMPM.0018, &
          mmiohlpListFind                       MMPM.0019, &
          mmiohlpListInsert                     MMPM.0020, &
          mmiohlpListChange                     MMPM.0021, &
          mmiohlpListDelete                     MMPM.0022, &
          mmiohlpListGetHeader                  MMPM.0023, &
          mmiohlpConvertAddressToHmmio          MMPM.0024, &
          mmiohlpConvertHmmioToAddress          MMPM.0025, &
          mmiohlpFreeHmmioEntry                 MMPM.0026, &
          mmiohlpGrowHmmioTable                 MMPM.0027, &
          mmiohlpGenerateHmmio                  MMPM.0028, &
          mmiohlpAcquireMmioSem                 MMPM.0029, &
          mmiohlpDiscardMmioSem                 MMPM.0030, &
          mmiohlpReadIniFile                    MMPM.0031, &
          mmiohlpLoadDefIOProcs                 MMPM.0032, &
          mmiohlpUnloadDefIOProcs               MMPM.0033, &
          mmiohlpRestoreInfoState               MMPM.0034, &
          mmiohlpAllocateInternalBuffer         MMPM.0035, &
          mmiohlpExpandMemFile                  MMPM.0036, &
          mmioStringToFOURCC                    MMPM.0037, &
          mmioGetLastError                      MMPM.0037, &
          mmioInstallIOProc                     MMPM.0039, &
          mmioOpen                              MMPM.0040, &
          mmioRead                              MMPM.0041, &
          mmioWrite                             MMPM.0042, &
          mmioSeek                              MMPM.0043, &
          mmioFlush                             MMPM.0044, &
          mmioClose                             MMPM.0045, &
          DOSIOProc                             MMPM.0046, &
          MEMIOProc                             MMPM.0047, &
          BNDIOProc                             MMPM.0048, &
          mmioAscend                            MMPM.0049, &
          mmioDescend                           MMPM.0050, &
          mmioCreateChunk                       MMPM.0051, &
          mmioGetInfo                           MMPM.0052, &
          mmioSetInfo                           MMPM.0053, &
          mmioSendMessage                       MMPM.0054, &
          mmioAdvance                           MMPM.0055, &
          mmioSetBuffer                         MMPM.0056, &
          mmioCFOpen                            MMPM.0057, &
          mmioCFClose                           MMPM.0058, &
          mmioCFAddEntry                        MMPM.0059, &
          mmioCFDeleteEntry                     MMPM.0060, &
          mmioCFChangeEntry                     MMPM.0061, &
          mmioCFFindEntry                       MMPM.0062, &
          mmioCFAddElement                      MMPM.0063, &
          mmioCFGetInfo                         MMPM.0064, &
          mmioCFSetInfo                         MMPM.0065, &
          mmioCFCopy                            MMPM.0066, &
          mmiohlpBuildCtoc                      MMPM.0067, &
          mmiohlpCreateCtoc                     MMPM.0068, &
          mmiohlpRewriteCtoc                    MMPM.0069, &
          mmiohlpGetHeaderInfo                  MMPM.0070, &
          mmiohlpUpdateCtocHeader               MMPM.0071, &
          mmiohlpChangeCtocHeader               MMPM.0072, &
          mmiohlpGetRemainingSize               MMPM.0073, &
          mmiohlpFindOpenCtoc                   MMPM.0074, &
          mmiohlpGetFullPathName                MMPM.0075, &
          mmiohlpAddCtocInstance                MMPM.0076, &
          mmiohlpRemoveCtocInstance             MMPM.0077, &
          mmiohlpLastModTime                    MMPM.0078, &
          mmiohlpOpenTempElement                MMPM.0079, &
          mmiohlpCloseTempElement               MMPM.0080, &
          mmiohlpTempName                       MMPM.0081, &
          mmiohlpRemoveSharingEntry             MMPM.0082, &
          mmiohlpAddSharingEntry                MMPM.0083, &
          mmiohlpCheckSharingMode               MMPM.0084, &
          mmiohlpCheckHmmcfSharingMode          MMPM.0085, &
          mmiohlpListTransferNode               MMPM.0086, &
          mmioQueryFormatCount                  MMPM.0087, &
          mmioGetFormats                        MMPM.0088, &
          mmioQueryHeaderLength                 MMPM.0089, &
          mmioGetHeader                         MMPM.0090, &
          mmioSetHeader                         MMPM.0091, &
          mmioIdentifyFile                      MMPM.0092, &
          mmioGetFormatName                     MMPM.0093, &
          mmiohlpGetFormatStringLength          MMPM.0094, &
          mmiohlpGetFormatString                MMPM.0095, &
          mmiohlpGetNLSData                     MMPM.0096, &
          CFIOProc                              MMPM.0097, &
          mmioIniFileHandler                    MMPM.0098, &
          mmioIdentifyFileFormat                MMPM.0099, &
          mmioIdentifyStorageSystem             MMPM.0100, &
          mmioDetermineSSIOProc                 MMPM.0101, &
          mmioDetermineFFIOProc                 MMPM.0102, &
          mmioDetermineLastIOProc               MMPM.0103, &
          mmiohlpIsFccSS                        MMPM.0104, &
          mmiohlpIsFccFF                        MMPM.0105, &
          mmioQueryIOProcModuleHandle           MMPM.0106, &
          mmiohlpGetElementStart                MMPM.0107, &
          mmiohlpChangeSharingEntry             MMPM.0108, &
          mmiohlpLoadNextIOProc                 MMPM.0109, &
          mmiohlpLoadIOProc                     MMPM.0110, &
          mmioMigrateIniFile                    MMPM.0111, &
          mmioIniFileCODEC                      MMPM.0112, &
          mmioCFCompact                         MMPM.0113, &
          mmioSet                               MMPM.0114, &
          mmioQueryCODECName                    MMPM.0115, &
          mmiohlpCFName                         MMPM.0116, &
          mmioLoadCODECProc                     MMPM.0117, &
          mmioQueryCODECNameLength              MMPM.0118, &
          mmioGetData                           MMPM.0119
EXPORTS = mmiohlpCleanupInstance.2, &
          mmiohlpListCreate.3, &
          mmiohlpListAdd.4, &
          mmiohlpListGet.5, &
          mmiohlpListFind.6, &
          mmiohlpListInsert.7, &
          mmiohlpListChange.8, &
          mmiohlpListDelete.9, &
          mmiohlpListGetHeader.10, &
          mmiohlpConvertAddressToHmmio.11, &
          mmiohlpConvertHmmioToAddress.12, &
          mmiohlpFreeHmmioEntry.13, &
          mmiohlpGrowHmmioTable.14, &
          mmiohlpGenerateHmmio.15, &
          mmiohlpAcquireMmioSem.16, &
          mmiohlpDiscardMmioSem.17, &
          mmiohlpReadIniFile.18, &
          mmiohlpLoadDefIOProcs.19, &
          mmiohlpUnloadDefIOProcs.20, &
          mmiohlpRestoreInfoState.21, &
          mmiohlpAllocateInternalBuffer.22, &
          mmiohlpExpandMemFile.23, &
          mmioStringToFOURCC.24, &
          mmioGetLastError.25, &
          mmioInstallIOProc.26, &
          mmioOpen.27, &
          mmioRead.28, &
          mmioWrite.29, &
          mmioSeek.30, &
          mmioFlush.31, &
          mmioClose.32, &
          DOSIOProc.33, &
          MEMIOProc.34, &
          BNDIOProc.35, &
          mmioAscend.36, &
          mmioDescend.37, &
          mmioCreateChunk.38, &
          mmioGetInfo.39, &
          mmioSetInfo.40, &
          mmioSendMessage.41, &
          mmioAdvance.42, &
          mmioSetBuffer.43, &
          mmioCFOpen.44, &
          mmioCFClose.45, &
          mmioCFAddEntry.46, &
          mmioCFDeleteEntry.47, &
          mmioCFChangeEntry.48, &
          mmioCFFindEntry.49, &
          mmioCFAddElement.50, &
          mmioCFGetInfo.51, &
          mmioCFSetInfo.52, &
          mmioCFCopy.53, &
          mmiohlpBuildCtoc.54, &
          mmiohlpCreateCtoc.55, &
          mmiohlpRewriteCtoc.56, &
          mmiohlpGetHeaderInfo.57, &
          mmiohlpUpdateCtocHeader.58, &
          mmiohlpChangeCtocHeader.59, &
          mmiohlpGetRemainingSize.60, &
          mmiohlpFindOpenCtoc.61, &
          mmiohlpGetFullPathName.62, &
          mmiohlpAddCtocInstance.63, &
          mmiohlpRemoveCtocInstance.64, &
          mmiohlpLastModTime.65, &
          mmiohlpOpenTempElement.66, &
          mmiohlpCloseTempElement.67, &
          mmiohlpTempName.68, &
          mmiohlpRemoveSharingEntry.69, &
          mmiohlpAddSharingEntry.70, &
          mmiohlpCheckSharingMode.71, &
          mmiohlpCheckHmmcfSharingMode.72, &
          mmiohlpListTransferNode.73, &
          mmioQueryFormatCount.74, &
          mmioGetFormats.75, &
          mmioQueryHeaderLength.76, &
          mmioGetHeader.77, &
          mmioSetHeader.78, &
          mmioIdentifyFile.79, &
          mmioGetFormatName.80, &
          mmiohlpGetFormatStringLength.81, &
          mmiohlpGetFormatString.82, &
          mmiohlpGetNLSData.83, &
          CFIOProc.84, &
          mmioIniFileHandler.85, &
          mmioIdentifyFileFormat.86, &
          mmioIdentifyStorageSystem.87, &
          mmioDetermineSSIOProc.88, &
          mmioDetermineFFIOProc.89, &
          mmioDetermineLastIOProc.90, &
          mmiohlpIsFccSS.91, &
          mmiohlpIsFccFF.92, &
          mmioQueryIOProcModuleHandle.93, &
          mmiohlpGetElementStart.94, &
          mmiohlpChangeSharingEntry.95, &
          mmiohlpLoadNextIOProc.96, &
          mmiohlpLoadIOProc.97, &
          mmioMigrateIniFile.98, &
          mmioIniFileCODEC.99, &
          mmioCFCompact.100, &
          mmioSet.101, &
          mmioQueryCODECName.102, &
          mmiohlpCFName.103, &
          mmioLoadCODECProc.104, &
          mmioQueryCODECNameLength.105, &
          mmioGetData.106

DEST    = mmos2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk

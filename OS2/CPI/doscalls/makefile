#
# A Makefile for doscalls.dll
# (c) osFree project,
# authors:
# Yuri Prokushev,   2002-2006,
# Valery Sedletski, 2010-2011.
#

PROJ     = doscalls
DESC     = Control Program API
srcfiles = $(p)msg$(e) &
           $(p)nls$(e) &
           $(p)doscopy$(e) &
           $(p)dllstart$(e) &
           $(p)strlcpy$(e) &
           $(p)strlcat$(e) &
           $(p)strnlen$(e) &
           $(p)strncmp$(e) &
           $(p)file$(e) &
           $(p)memory$(e) &
           $(p)doscancellockrequest$(e) &
           $(p)dosopen$(e) &
           $(p)doslistio$(e) &
           $(p)dossetfileptr$(e) &
           $(p)dossetfilesize$(e) &
           $(p)dossetfilelocks$(e) &
           $(p)dosprotectopen$(e) &
           $(p)dosprotectsetfileptr$(e) &
           $(p)dosprotectsetfilesize$(e) &
           $(p)dosprotectsetfilelocks$(e) &
           $(p)dosquerysysinfo$(e) &
           $(p)dosread$(e)  &
           $(p)doswrite$(e) &
           $(p)dosexit$(e) &
           $(p)doserror$(e) &
           $(p)dosexecpgm$(e) &
           $(p)dossetcurrentdir$(e) &
           $(p)dosquerycurrentdir$(e) &
           $(p)dosquerycurrentdisk$(e) &
           $(p)dossetdefaultdisk$(e) &
           $(p)modmgr$(e) &
           $(p)log$(e)    &
           $(p)env$(e)    &
           $(p)thunk$(e)  &
           $(p)sched$(e)  &
           $(p)unimplemented$(e) # $(p)monitors$(e) $(p)dossearchpath$(e) # $(p)doseditname$(e)

ADD_COPT = -s -od
ADD_LINKOPT  = lib clib3r op nod segment   type DATA shared
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata
IMPORTS = DosFSCtl             KAL.KalFSCtl, &
          _KalRead             KAL.KalRead,  &
          _KalWrite            KAL.KalWrite, &
          _KalExit             KAL.KalExit,  &
          _KalOpenL            KAL.KalOpenL, &
          _KalClose            KAL.KalClose, &
          _KalSetCurrentDir    KAL.KalSetCurrentDir, &
          _KalQueryCurrentDir  KAL.KalQueryCurrentDir, &
          _KalQueryCurrentDisk KAL.KalQueryCurrentDisk, &
          _KalSetDefaultDisk   KAL.KalSetDefaultDisk, &
          _KalError            KAL.KalError, &
          _KalExecPgm          KAL.KalExecPgm, &
          _KalAllocMem         KAL.KalAllocMem, &
          _KalFreeMem          KAL.KalFreeMem, &
          _KalQueryCp          KAL.KalQueryCp, &
          _KalQueryDBCSEnv     KAL.KalQueryDBCSEnv, &
          _KalQueryHType       KAL.KalQueryHType,   &
          _KalLoadModule       KAL.KalLoadModule,   &
          _KalQueryModuleHandle KAL.KalQueryModuleHandle, &
          _KalQueryModuleName  KAL.KalQueryModuleName, &
          _KalQueryProcAddr    KAL.KalQueryProcAddr, &
          _KalLogWrite         KAL.KalLogWrite, &
          _KalGetInfoBlocks    KAL.KalGetInfoBlocks, &
          _KalResetBuffer      KAL.KalResetBuffer, &
          _KalSetFilePtrL      KAL.KalSetFilePtrL, &
          _KalScanEnv          KAL.KalScanEnv,     &
          _KalSleep            KAL.KalSleep,       &
          _KalDupHandle        KAL.KalDupHandle,   &
          _KalDelete           KAL.KalDelete,      &
          _KalForceDelete      KAL.KalForceDelete, &
          _KalDeleteDir        KAL.KalDeleteDir,   &
          _KalCreateDir        KAL.KalCreateDir,   &
          _KalSetMaxFH         KAL.KalSetMaxFH,    &
          _KalSetRelMaxFH      KAL.KalSetRelMaxFH

EXPORTS = &
        DosICreateThread           .1, &
        DosCwait                   .2, &
        DosEnterCritSec            .3, &
        DosIExecPgm                .4, &
        Dos16Exit                  .5, &
        Dos16ExitCritSec           .6, &
        Dos16ExitList              .7, &
        Dos16GetInfoSeg            .8, &
        Dos16GetPrty               .9, &
        Dos16KillProcess           .10, &
        Dos16SetPrty               .11, &
        DosPtrace                  .12, &
        DosHoldSignal              .13, &
        DosSetSigHandler           .14, &
        DosFlagProcess             .15, &
        DosMakePipe                .16, &
        DosISysSemClear            .17, &
        DosISemRequest             .18, &
        DosISysSemSet              .19, &
        Dos16SemSetWait            .20, &
        DosISemWait                .21, &
        Dos16MuxSemWait            .22, &
        Dos16CloseSem              .23, &
        Dos16CreateSem             .24, &
        Dos16OpenSem               .25, &
        Dos16ResumeThread          .26, &
        Dos16SuspendThread         .27, &
        Dos16SetDateTime           .28, &
        Dos16TimerAsync            .29, &
        Dos16TimerStart            .30, &
        Dos16TimerStop             .31, &
        Dos16Sleep                 .32, &
        Dos16GetDateTime           .33, &
        Dos16AllocSeg              .34, &
        Dos16AllocShrSeg           .35, &
        Dos16GetShrSeg             .36, &
        Dos16GiveSeg               .37, &
        Dos16ReallocSeg            .38, &
        Dos16FreeSeg               .39, &
        Dos16AllocHuge             .40, &
        Dos16GetHugeShift          .41, &
        Dos16ReallocHuge           .42, &
        Dos16CreateCSAlias         .43, &
        Dos16LoadModule            .44, &
        Dos16GetProcAddr           .45, &
        Dos16FreeModule            .46, &
        Dos16GetModHandle          .47, &
        Dos16GetModName            .48, &
        Dos16GetMachineMode        .49, &
        Dos16Beep                  .50, &
        Dos16CLIAccess             .51, &
        Dos16DevConfig             .52, &
        Dos16DevIOCtl              .53, &
        Dos16SGSwitch              .54, &
        Dos16SGSwitchMe            .55, &
        Dos16BufReset              .56, &
        Dos16ChDir                 .57, &
        Dos16ChgFilePtr            .58, &
        Dos16Close                 .59, &
        Dos16Delete                .60, &
        Dos16DupHandle             .61, &
        Dos16FileLocks             .62, &
        Dos16FindClose             .63, &
        Dos16FindFirst             .64, &
        Dos16FindNext              .65, &
        Dos16MkDir                 .66, &
        Dos16Move                  .67, &
        Dos16NewSize               .68, &
        Dos16PortAccess            .69, &
        Dos16Open                  .70, &
        Dos16QCurDir               .71, &
        Dos16QCurDisk              .72, &
        Dos16QFHandState           .73, &
        Dos16QFileInfo             .74, &
        Dos16QFileMode             .75, &
        Dos16QFSInfo               .76, &
        Dos16QHandType             .77, &
        Dos16QVerify               .78, &
        DosIRead                   .79, &
        Dos16RmDir                 .80, &
        Dos16SelectDisk            .81, &
        Dos16SetFHandState         .82, &
        Dos16SetFileInfo           .83, &
        Dos16SetFileMode           .84, &
        Dos16SetMaxFH              .85, &
        Dos16SetVerify             .86, &
        DosIWrite                  .87, &
        Dos16SystemService         .88, &
        Dos16SetVec                .89, &
        Dos16SysTrace              .90, &
        Dos16GetEnv                .91, &
        Dos16GetVersion            .92, &
        Dos16QTraceInfo            .93, &
        Dos16GetPID                .94, &
        Dos16Open2                 .95, &
        Dos16LibInit               .96, &
        Dos16SetFSInfo             .97, &
        Dos16QPathInfo             .98, &
        Dos16DevIOCtl2             .99, &
        DosICanonicalize           .100, &
        Dos16SetFgnd               .101, &
        Dos16SwapTaskInit          .102, &
        Dos16ReadPhys              .103, &
        Dos16SetPathInfo           .104, &
        DosSGSwitchProc2           .105, &
        STRUCHECK                  .106, &
        STRURESUPDATE              .107, &
        DosISetRelMaxFH            .108, &
        DosIDevIOCtl               .109, &
        DosForceDelete             .110, &
        DosKillThread              .111, &
        DosQueryRASInfo            .112, &
        DosDumpProcess             .113, &
        DosSuppressPopUps          .114, &
        THK32ALLOCMEM              .115, &
        THK32FREEMEM               .116, &
        THK32ALLOCSTACK            .117, &
        DosOpen2Compt              .118, &
        DosGetSTDA                 .119, &
        Dos16Error                 .120, &
        Dos16GetSeg                .121, &
        Dos16LockSeg               .122, &
        Dos16UnlockSeg             .123, &
        Dos16SGSwitchProc          .124, &
        DosIRamSemWake             .125, &
        Dos16SizeSeg               .126, &
        Dos16MemAvail              .127, &
        DosIRamSemRequest          .128, &
        Dos16PhysicalDisk          .129, &
        Dos16GetCP                 .130, &
        DosISetCP                  .131, &
        Dos16GlobalSeg             .132, &
        Dos16Profile               .133, &
        Dos16SendSignal            .134, &
        Dos16HugeShift             .135, &
        Dos16HugeIncr              .136, &
        Dos16Read                  .137, &
        Dos16Write                 .138, &
        Dos16ErrClass              .139, &
        Dos16SemRequest            .140, &
        Dos16SemClear              .141, &
        Dos16SemWait               .142, &
        Dos16SemSet                .143, &
        Dos16ExecPgm               .144, &
        Dos16CreateThread          .145, &
        Dos16SubSet                .146, &
        Dos16SubAlloc              .147, &
        Dos16SubFree               .148, &
        Dos16ReadAsync             .149, &
        Dos16WriteAsync            .150, &
        Dos16SearchPath            .151, &
        Dos16ScanEnv               .152, &
        Dos16SetCP                 .153, &
        Dos16QProcStatus           .154, &
        Dos16GetResource           .155, &
        Dos16GetPPID               .156, &
        Dos16CallBack              .157, &
        DosICallBack               .158, &
        Dos16RetForward            .159, &
        Dos16R2StackRealloc        .160, &
        Dos16FSRamSemRequest       .161, &
        Dos16FSRamSemClear         .162, &
        Dos16QAppType              .163, &
        Dos16SetProcCP             .164, &
        Dos16DynamicTrace          .165, &
        Dos16QSysInfo              .166, &
        DosIMakeNmPipe             .167, &
        DosICallNmPipe             .168, &
        DosIConnectNmPipe          .169, &
        DosIDisconnectNmPipe       .170, &
        DosIPeekNmPipe             .171, &
        DosIQNmPipeInfo            .172, &
        DosIQNmPHandState          .173, &
        DosISetNmPHandState        .174, &
        DosITransactNmPipe         .175, &
        DosIWaitNmPipe             .176, &
        DosISetNmPipeSem           .177, &
        DosIQNmPipeSemState        .178, &
        DosIRawReadNmPipe          .179, &
        DosIRawWriteNmPipe         .180, &
        Dos16FSAttach              .181, &
        Dos16QFSAttach             .182, &
        Dos16FSCtl                 .183, &
        Dos16FindFirst2            .184, &
        Dos16MkDir2                .185, &
        Dos16FileIO                .186, &
        Dos16FindNotifyClose       .187, &
        Dos16FindNotifyFirst       .188, &
        Dos16FindNotifyNext        .189, &
        DosSetTraceInfo            .190, &
        Dos16EditName              .191, &
        DosLogMode                 .192, &
        DosLogEntry                .193, &
        DosGetLogBuffer            .194, &
        DosLogRegister             .195, &
        DosLogRead                 .196, &
        DosFindFromName            .197, &
        DosOPLockRelease           .198, &
        DosOPLockWait              .199, &
        DosICopy                   .200, &
        Dos16Copy                  .201, &
        DosIQAppType               .202, &
        Dos16ForceDelete           .203, &
        Dos16EnumAttribute         .204, &
        DosOPLockShutdown          .205, &
        Dos16Shutdown              .206, &
        Dos16GetResource2          .207, &
        Dos16FreeResource          .208, &
        DosSetMaxFH                .209, &
        DosSetVerify               .210, &
        DosErrClass                .211, &
        DosError                   .212, &
        DosCreateVDM               .213, &
        Dos16MaxPathLen            .214, &
        Dos16PageSize              .215, &
        Dos16LocalInfo             .216, &
        Dos16GlobalInfo            .217, &
        DosSetFileInfo             .218, &
        DosSetPathInfo             .219, &
        DosSetDefaultDisk          .220, &
        DosSetFHState              .221, &
        DosSetFSInfo               .222, &
        DosQueryPathInfo           .223, &
        DosQueryHType              .224, &
        DosQueryVerify             .225, &
        DosDeleteDir               .226, &
        DosScanEnv                 .227, &
        DosSearchPath              .228, &
        DosSleep                   .229, &
        DosGetDateTime             .230, &
        DosDevConfig               .231, &
        DosEnterCritSec            .232, &
        DosExitCritSec             .233, &
        DosExit                    .234, &
        DosKillProcess             .235, &
        DosSetPriority             .236, &
        DosResumeThread            .237, &
        DosSuspendThread           .238, &
        DosCreatePipe              .239, &
        DosCallNPipe               .240, &
        DosConnectNPipe            .241, &
        DosDisConnectNPipe         .242, &
        DosCreateNPipe             .243, &
        DosPeekNPipe               .244, &
        DosQueryNPHState           .245, &
        DosRawReadNPipe            .246, &
        DosRawWriteNPipe           .247, &
        DosQueryNPipeInfo          .248, &
        DosQueryNPipeSemState      .249, &
        DosSetNPHState             .250, &
        DosSetNPipeSem             .251, &
        DosTransactNPipe           .252, &
        DosWaitNPipe               .253, &
        DosResetBuffer             .254, &
        DosSetCurrentDir           .255, &
        DosSetFilePtr              .256, &
        DosClose                   .257, &
        DosCopy                    .258, &
        DosDelete                  .259, &
        DosDupHandle               .260, &
        DosEditName                .261, &
        THK32FREESTACK             .262, &
        DosFindClose               .263, &
        DosFindFirst               .264, &
        DosFindNext                .265, &
        Dos16OpenVDD               .266, &
        Dos16RequestVDD            .267, &
        Dos16CloseVDD              .268, &
        DosFSAttach                .269, &
        DosCreateDir               .270, &
        DosMove                    .271, &
        DosSetFileSize             .272, &
        DosOpen                    .273, &
        DosQueryCurrentDir         .274, &
        DosQueryCurrentDisk        .275, &
        DosQueryFHState            .276, &
        DosQueryFSAttach           .277, &
        DosQueryFSInfo             .278, &
        DosQueryFileInfo           .279, &
        DosWaitChild               .280, &
        DosRead                    .281, &
        DosWrite                   .282, &
        DosExecPgm                 .283, &
        DosDevIOCtl                .284, &
        DosFSCtl                   .285, &
        DosBeep                    .286, &
        DosPhysicalDisk            .287, &
        DosSetCp                   .288, &
        DosSetProcessCp            .289, &
        DosStopTimer               .290, &
        DosQueryCp                 .291, &
        DosSetDateTime             .292, &
        Thk32AllocBlock            .293, &
        Thk32FreeBlock             .294, &
        Thk32R3DS                  .295, &
        DosExitList                .296, &
        DosAllocProtectedMem       .297, &
        DosAliasMem                .298, &
        DosAllocMem                .299, &
        DosAllocSharedMem          .300, &
        DosGetNamedSharedMem       .301, &
        DosGetSharedMem            .302, &
        DosGiveSharedMem           .303, &
        DosFreeMem                 .304, &
        DosSetMem                  .305, &
        DosQueryMem                .306, &
        DosQueryMemState           .307, &
        DosOpenVDD                 .308, &
        DosRequestVDD              .309, &
        DosCloseVDD                .310, &
        DosCreateThread            .311, &
        DosGetInfoBlocks           .312, &
        DosAllocProtSeg            .313, &
        DosAllocShrProtSeg         .314, &
        DosAllocProtHuge           .315, &
        DosDynamicTrace            .316, &
        DosDebug                   .317, &
        DosLoadModule              .318, &
        DosQueryModuleHandle       .319, &
        DosQueryModuleName         .320, &
        DosQueryProcAddr           .321, &
        DosFreeModule              .322, &
        DosQueryAppType            .323, &
        DosCreateEventSem          .324, &
        DosOpenEventSem            .325, &
        DosCloseEventSem           .326, &
        DosResetEventSem           .327, &
        DosPostEventSem            .328, &
        DosWaitEventSem            .329, &
        DosQueryEventSem           .330, &
        DosCreateMutexSem          .331, &
        DosOpenMutexSem            .332, &
        DosCloseMutexSem           .333, &
        DosRequestMutexSem         .334, &
        DosReleaseMutexSem         .335, &
        DosQueryMutexSem           .336, &
        DosCreateMuxWaitSem        .337, &
        DosOpenMuxWaitSem          .338, &
        DosCloseMuxWaitSem         .339, &
        DosWaitMuxWaitSem          .340, &
        DosAddMuxWaitSem           .341, &
        DosDeleteMuxWaitSem        .342, &
        DosQueryMuxWaitSem         .343, &
        DosSubSetMem               .344, &
        DosSubAllocMem             .345, &
        DosSubFreeMem              .346, &
        DosSubUnsetMem             .347, &
        DosQuerySysInfo            .348, &
        DosWaitThread              .349, &
        DosAsyncTimer              .350, &
        DosStartTimer              .351, &
        DosGetResource             .352, &
        DosFreeResource            .353, &
        DosSetExceptionHandler     .354, &
        DosUnsetExceptionHandler   .355, &
        DosRaiseException          .356, &
        DosUnwindException         .357, &
        DosQueryPageUsage          .358, &
        DosQueryModFromCS          .359, &
        DosQueryModFromEIP         .360, &
        DosFPDataArea              .361, &
        DosTmrQueryFreq            .362, &
        DosTmrQueryTime            .363, &
        DosAliasPerfCtrs           .364, &
        DosConfigPerf              .365, &
        DosDeconPerf               .366, &
        DosRegisterPerfCtrs        .367, &
        DosQuerySysState           .368, &
        DosFlatCS                  .369, &
        DosFlatDS                  .370, &
        DosQueryABIOSSupport       .371, &
        DosEnumAttribute           .372, &
        DosQueryDOSProperty        .373, &
        DosSetDOSProperty          .374, &
        Dos16QueryDosProperty      .375, &
        Dos16SetDosProperty        .376, &
        Dos32Profile               .377, &
        DosSetSignalExceptionFocus .378, &
        DosSendSignalException     .379, &
        DosEnterMustComplete       .380, &
        DosExitMustComplete        .381, &
        DosSetRelMaxFH             .382, &
# msg
        Dos16PutMessage            .383, &
        Dos16TrueGetMessage        .384, &
        Dos16InsMessage            .385, &
        DosInsertMessage           .386, &
        DosPutMessage              .387, &
        Dos32TrueGetMessage        .388, &
        Dos16IQueryMessageCP       .389, &
        Dos32IQueryMessageCP       .390, &
# nls
        DosCaseMap                 .391, &
        DosGetCollate              .392, &
        DosGetCtryInfo             .393, &
        DosGetDBCSEv               .394, &
        DosQueryCtryInfo           .395, &
        DosQueryDBCSEnv            .396, &
        DosMapCase                 .397, &
        DosQueryCollate            .398, &
# nampipes
        DosMakeNmPipe              .399, &
        DosQNmPipeInfo             .400, &
        DosConnectNmPipe           .401, &
        DosDisconnectNmPipe        .402, &
        DosQNmPHandState           .403, &
        DosSetNmPHandState         .404, &
        DosPeekNmPipe              .405, &
        DosWaitNmPipe              .406, &
        DosTransactNmPipe          .407, &
        DosCallNmPipe              .408, &
        DosRawReadNmPipe           .409, &
        DosRawWriteNmPipe          .410, &
        DosSetNmPipeSem            .411, &
        DosQNmPipeSemState         .412, &
# startlw
        StartLazyWriter            .413, &
# ???
        _QueInst_Data              .414, &
#
        DosShutdown                .415, &
        DosICacheModule            .416, &
        DosReplaceModule           .417, &
        DosAcknowledgeSignalException .418, &
        DosTIB                     .419, &
        Dos16TmrQueryFreq          .420, &
        Dos16TmrQueryTime          .421, &
        Dos16RegisterPerfCtrs      .422, &
        Dos16FlatToSel             .423, &
        Dos16SelToFlat             .424, &
        DosFlatToSel               .425, &
        DosSelToFlat               .426, &
        DosIODelayCnt              .427, &
        DosSetFileLocks            .428, &
        DosCancelLockRequest       .429, &
        LogOpen                    .430, &
        LogClose                   .431, &
        LogAddEntries              .432, &
        LogGetEntries              .433, &
        LogSetState                .434, &
        LogSetName                 .435, &
        LogQueryState              .436, &
        Dos16OpenChangeNotify      .437, &
        Dos16ResetChangeNotify     .438, &
        Dos16CloseChangeNotify     .439, &
        DosOpenChangeNotify        .440, &
        DosResetChangeNotify       .441, &
        DosCloseChangeNotify       .442, &
        Dos16QueryABIOSSupport     .443, &
        DosForceSystemDump         .444, &
                                         &
        Dos32GetProcessorStatus    .447, &
        Dos32SetProcessorStatus    .448, &
        Dos16CreateSpinLock        .449, &
        Dos16AcquireSpinLock       .450, &
        Dos16ReleaseSpinLock       .451, &
        Dos16FreeSpinLock          .452, &
        Dos32TestPSD               .453, &
        DosAllocThreadLocalMemory  .454, &
        DosFreeThreadLocalMemory   .455, &
                                         &
        DosVerifyPidTid            .460, &
                                         &
        PTDA_LANMAN_SEC            .464, &
        PTDA_PID                   .465, &
        SAS_SEL                    .466, &
        TCB_OPCOOKIE               .467, &
        TCB_OPFLAGS                .468, &
        TCB_NEWFLAGS               .469, &
        TCB_USER_ID                .470, &
        TCB_PROC_ID                .471, &
        TCB_FSHARING               .472, &
        TCB_SRVATTRIB              .473, &
        TCB_ALLOWED                .474, &
        TCB_PRTCB                  .475, &
        TCB_NUMBER                 .476, &
        TCB_THISSFT                .477, &
        TCB_THISCDS                .478, &
        TKOPTDA                    .479, &
        PTDA_CRITSEC               .480, &
        PTDA_HOLDSIGCNT            .481, &
        PTDA_PPTDAPARENT           .482, &
        PTDA_PGDATA                .483, &
        PTDA_HANDLE                .484, &
        PTDA_MODULE                .485, &
        PTDA_LDTHANDLE             .486, &
        PTDA_CODEPAGE_TAG          .487, &
        PTDA_JFN_LENGTH            .488, &
        PTDA_JFN_PTABLE            .489, &
        PTDA_JFN_FLG_PTR           .490, &
        PTDA_EXTERR_LOCUS          .491, &
        PTDA_EXTERR                .492, &
        PTDA_EXTERR_ACTION         .493, &
        PTDA_EXTERR_CLASS          .494, &
        PTDA_PPID                  .495, &
        PTDA_PROCTYPE              .496, &
        PTDA_CURRTCB               .497, &
        PTDA_CURRTSD               .498, &
        PTDA_SIGNATURE             .499, &
        T32EXITLIST                .500, &
        T32ALLOCPROTECTEDMEM       .501, &
        T32ALIASMEM                .502, &
        T32ALLOCMEM                .503, &
        T32ALLOCSHAREDMEM          .504, &
        T32GETNAMEDSHAREDMEM       .505, &
        T32GETSHAREDMEM            .506, &
        T32GIVESHAREDMEM           .507, &
        T32FREEMEM                 .508, &
        T32SETMEM                  .509, &
        T32QUERYMEM                .510, &
        T32QUERYMEMSTATE           .511, &
        T32OPENVDD                 .512, &
        T32REQUESTVDD              .513, &
        T32CLOSEVDD                .514, &
        T32CREATETHREAD            .515, &
        T32DYNAMICTRACE            .516, &
        T32DEBUG                   .517, &
        T32QUERYPROCADDR           .518, &
        T32CREATEEVENTSEM          .519, &
        T32OPENEVENTSEM            .520, &
        T32CLOSEEVENTSEM           .521, &
        T32RESETEVENTSEM           .522, &
        T32POSTEVENTSEM            .523, &
        T32WAITEVENTSEM            .524, &
        T32QUERYEVENTSEM           .525, &
        T32CREATEMUTEXSEM          .526, &
        T32OPENMUTEXSEM            .527, &
        T32CLOSEMUTEXSEM           .528, &
        T32REQUESTMUTEXSEM         .529, &
        T32RELEASEMUTEXSEM         .530, &
        T32QUERYMUTEXSEM           .531, &
        T32CREATEMUXWAITSEM        .532, &
        T32OPENMUXWAITSEM          .533, &
        T32CLOSEMUXWAITSEM         .534, &
        T32WAITMUXWAITSEM          .535, &
        T32ADDMUXWAITSEM           .536, &
        T32DELETEMUXWAITSEM        .537, &
        T32QUERYMUXWAITSEM         .538, &
        T32QUERYSYSINFO            .539, &
        T32WAITTHREAD              .540, &
        T32GETRESOURCE             .541, &
        T32FREERESOURCE            .542, &
        T32EXCEPTIONCALLBACK       .543, &
        T32QUERYPAGEUSAGE          .544, &
        DosExceptionCallBack       .545, &
        TI32ASYNCTIMER             .546, &
        TI32STARTTIMER             .547, &
        DosR3ExceptionDispatcher   .548, &
        DosLibiDisp                .549, &
        DosLibiDisp16              .550, &
        DosLibiDisp32              .551, &
        Dos16R3ExitAddr            .552, &
        Dos32R3ExitAddr            .553, &
        DosIRead                   .554, &
        T32IREAD                   .555, &
        DosIWrite                  .556, &
        T32IWRITE                  .557, &
        T32TMRQUERYFREQ            .558, &
        T32TMRQUERYTIME            .559, &
        T32ALIASPERFCTRS           .560, &
        T32CONFIGUREPERF           .561, &
        T32DECONPERF               .562, &
        T32REGISTERPERFCTRS        .563, &
        T32QUERYSYSSTATE           .564, &
        DosISetFileInfo            .565, &
        DosISetPathInfo            .566, &
        T32QUERYDOSPROPERTY        .567, &
        T32SETDOSPROPERTY          .568, &
        DosIFindNext               .569, &
        T32PROFILE                 .570, &
        T32QUERYRESOURCESIZE       .571, &
        Dos32QueryResourceSize     .572, &
        Dos16QueryResourceSize     .573, &
        T32SETSIGNALEXCEPTIONFOCUS .574, &
        T32SENDSIGNALEXCEPTION     .575, &
        T32STARTTIMER              .576, &
        T32STOPTIMER               .577, &
        T32ASYNCTIMER              .578, &
        T32INITIALIZEPORTHOLE      .579, &
        DosInitializePorthole      .580, &
        T32QUERYHEADERINFO         .581, &
        DosQueryHeaderInfo         .582, &
        Dos16InitializePorthole    .583, &
        Dos16QueryHeaderInfo       .584, &
        T32QUERYPROCTYPE           .585, &
        Dos32QueryProcType         .586, &
        Dos16QueryProcType         .587, &
        T32IEXITMUSTCOMPLETE       .588, &
        DosISigDispatch            .589, &
        T32ICACHEMODULE            .590, &
        T32DLLTERM                 .591, &
        DosDllTermDisp             .592, &
        T32IRAISEEXCEPTION         .593, &
        DosIRaiseException         .594, &
        T32ACKNOWLEDGESIGNALEXC    .595, &
        DosPM16SemChk              .596, &
        DosIQueryFHState           .597, &
        DosISetFHState             .598, &
        DosLDTSel                  .599, &
        DosR3Frestor               .600, &
        DosIFindFirst              .601, &
        T32SETFILELOCKS            .602, &
        T32CANCELLOCKREQUEST       .603, &
        T32IMONREAD                .604, &
        T32IMONWRITE               .605, &
# moncalls
        DosMonOpen                 .606, &
        DosMonClose                .607, &
        DosMonRead                 .608, &
        DosMonWrite                .609, &
        DosMonReg                  .610, &
#
        T32KILLTHREAD              .611, &
        TQUERYRASINFO              .612, &
                                         &
        Dos32IProtectWrite         .615, &
                                         &
        DosIProtectSetFileInfo     .617, &
        Dos32IProtectSetFileInfo   .618, &
        Dos32IProtectSetFHState    .619, &
        Dos32IProtectQueryFHState  .620, &
        DosProtectSetFilePtr       .621, &
        Dos16ProtectClose          .622, &
        Dos16ProtectFileIO         .623, &
        Dos16ProtectFileLocks      .624, &
        DosIProtectRead            .625, &
        DosIProtectWrite           .626, &
        Dos16ProtectNewSize        .627, &
        Dos16ProtectOpen           .628, &
        Dos16ProtectQFHandState    .629, &
        Dos16ProtectSetFHandState  .630, &
        Dos16ProtectQFileInfo      .631, &
        Dos16ProtectSetFileInfo    .632, &
                                         &
        Dos16ProtectChgFilePtr     .634, &
        Dos16ProtectEnumAttribute  .635, &
        DosProtectEnumAttribute    .636, &
        DosProtectOpen             .637, &
        DosProtectClose            .638, &
        DosProtectSetFileLocks     .639, &
        DosProtectSetFileSize      .640, &
        DosProtectRead             .641, &
        DosProtectWrite            .642, &
        DosProtectSetFileInfo      .643, &
        DosProtectSetFHState       .644, &
        DosProtectQueryFHState     .645, &
        DosProtectQueryFileInfo    .646, &
        Dos32IProtectRead          .647, &
# msg
        DosCloseMessageFile        .649, &
#
        DosLDRDirtyWorker          .650, &
        Dos16LDRDirtyWorker        .651, &
        Dos32IGetInfoBlocks        .652, &
        T32IGETINFOBLOCKS          .653, &
                                         &
        T32GETPROCESSORSTATUS      .656, &
        T32SETPROCESSORSTATUS      .657, &
        T32TESTPSD                 .658, &
        T32QUERYTHREADAFFINITY     .659, &
        T32SETTHREADAFFINITY       .660, &
# quecalls 32
        Dos32ReadQueue             .661, &
        Dos32PurgeQueue            .662, &
        Dos32CloseQueue            .663, &
        Dos32QueryQueue            .664, &
        Dos32PeekQueue             .665, &
        Dos32WriteQueue            .666, &
        Dos32OpenQueue             .667, &
        Dos32CreateQueue           .668, &
# sesmgr 32
        Dos32StartSession          .669, &
        Dos32SelectSession         .670, &
        Dos32SetSession            .671, &
        Dos32StopSession           .672, &
        DosRegisterNotification    .673, &
# quecalls 16
        Dos16ReadQueue             .674, &
        Dos16PurgeQueue            .675, &
        Dos16CloseQueue            .676, &
        Dos16QueryQueue            .677, &
        Dos16PeekQueue             .678, &
        Dos16WriteQueue            .679, &
        Dos16OpenQueue             .680, &
        Dos16CreateQueue           .681, &
# os2char 16
        Dos16SMGetmem              .682, &
        Dos16SMFreemem             .683, &
        Dos16SMGetSGCB             .684, &
        Dos16SMInitSGCB            .685, &
# sesmgr 16
        Dos16SMSGDoPopup           .686, &
        Dos16SMSwitch              .687, &
        Dos16SMServeAppReq         .688, &
        Dos16GetTimes              .689, &
        Dos16SMSetTitle            .690, &
        Dos16ScrUnlock             .691, &
        Dos16SMDoAppReq            .692, &
        Dos16StopSession           .693, &
        Dos16SelectSession         .694, &
        Dos16ScrLock               .695, &
        Dos16SavRedrawWait         .696, &
        Dos16SavRedrawUndo         .697, &
        Dos16SMSGEndPopup          .698, &
        Dos16SetSession            .699, &
        Dos16SetMNLockTime         .700, &
        Dos16ModeUndo              .701, &
        Dos16StartSession          .702, &
        Dos16SMGetStatus           .703, &
        Dos16ModeWait              .704, &
        Dos16SMTerminate           .705, &
        Dos16SMGetAppReq           .706, &
        Dos16SMInitialize          .707, &
        Dos16SMStart               .708, &
        Dos16SMParentSwitch        .709, &
        Dos16SMPause               .710, &
        Dos16SMHDeinit             .711, &
        Dos16SMPMPresent           .712, &
        Dos16SMRegisterDD          .713, &
        Dos16SMNotifyDD            .714, &
        Dos16SMNotifyDD2           .715, &
        Dos16SMOpenDD              .716, &
        Dos16SMSetSessionType      .717, &
# os2char
        OS2BaseInit                .718, &
# moucalls
        MouGetPtrShape             .719, &
        MouSetPtrShape             .720, &
        MouGetNumMickeys           .721, &
        MouGetThreshold            .722, &
        MouShellInit               .723, &
        MouGetScaleFact            .724, &
        MouFlushQue                .725, &
        MouGetNumButtons           .726, &
        MouClose                   .727, &
        MouSetThreshold            .728, &
        MouSetScaleFact            .729, &
        MouGetNumQueEl             .730, &
        MouDeRegister              .731, &
        MouGetEventMask            .732, &
        MouSetEventMask            .733, &
        MouOpen                    .734, &
        MouRemovePtr               .735, &
        MouGetPtrPos               .736, &
        MouReadEventQue            .737, &
        MouSetPtrPos               .738, &
        MouGetDevStatus            .739, &
        MouSynch                   .740, &
        MouRegister                .741, &
        MouSetDevStatus            .742, &
        MouDrawPtr                 .743, &
        MouInitReal                .744, &
# kbdcalls
        KbdSetCustXt               .745, &
        KbdInit                    .746, &
        KbdGetCp                   .747, &
        KbdCharIn                  .748, &
        KbdSetCp                   .749, &
        KbdLoadInstance            .750, &
        KbdSynch                   .751, &
        KbdRegister                .752, &
        KbdStringIn                .753, &
        KbdGetStatus               .754, &
        KbdSetStatus               .755, &
        KbdGetFocus                .756, &
        KbdFlushBuffer             .757, &
        KbdXlate                   .758, &
        KbdSwitchFgnd              .759, &
        KbdShellInit               .760, &
        KbdClose                   .761, &
        KbdFreeFocus               .762, &
        KbdFree                    .763, &
        KbdDeRegister              .764, &
        KbdSetFgnd                 .765, &
        KbdPeek                    .766, &
        KbdOpen                    .767, &
        KbdGetHWId                 .768, &
        KbdSetHWId                 .769, &
# viocalls
        VioEndPopup                .770, &
        VioGetPhysBuf              .771, &
        VioGetAnsi                 .772, &
        VioFree                    .773, &
        VioSetAnsi                 .774, &
        VioDeRegister              .775, &
        VioScrollUp                .776, &
        VioPrtSc                   .777, &
        VioGetCurPos               .778, &
        VioWrtCellStr              .779, &
        VioPopUp                   .780, &
        VioScrollRt                .781, &
        VioWrtCharStr              .782, &
        Avs_Prtsc                  .783, &
        VioSetCurPos               .784, &
        VioSrfUnblock              .785, &
        VioSrfBlock                .786, &
        VioScrUnlock               .787, &
        VioWrtTty                  .788, &
        VioSave                    .789, &
        VioGetMode                 .790, &
        VioSetMode                 .791, &
        VioScrLock                 .792, &
        VioReadCellStr             .793, &
        VioSavRedrawWait           .794, &
        VioWrtNAttr                .795, &
        VioGetCurType              .796, &
        VioSavRedrawUndo           .797, &
        VioGetFont                 .798, &
        VioReadCharStr             .799, &
        VioGetBuf                  .800, &
        VioSetCurType              .801, &
        VioSetFont                 .802, &
        VioHetInit                 .803, &
        VioModeUndo                .804, &
        VioSswSwitch               .805, &
        VioModeWait                .806, &
        Avs_PrtscToggle            .807, &
        VioGetCp                   .808, &
        VioRestore                 .809, &
        VioSetCp                   .810, &
        VioShowBuf                 .811, &
        VioScrollLf                .812, &
        VioRegister                .813, &
        VioGetConfig               .814, &
        VioScrollDn                .815, &
        VioWrtCharStrAtt           .816, &
        VioGetState                .817, &
        VioPrtScToggle             .818, &
        VioSetState                .819, &
        VioWrtNCell                .820, &
        VioWrtNChar                .821, &
        VioShellInit               .822, &
        VioAssociate               .823, &
        VioCreatePS                .824, &
        VioDeleteSetId             .825, &
        VioGetDeviceCellSize       .826, &
        VioGetOrg                  .827, &
        VioCreateLogFont           .828, &
        VioDestroyPS               .829, &
        VioQuerySetIds             .830, &
        VioSetOrg                  .831, &
        VioQueryFonts              .832, &
        VioSetDeviceCellSize       .833, &
        VioShowPS                  .834, &
        VioGetPSAddress            .835, &
        VioQueryConsole            .836, &
        VioRedrawSize              .837, &
        VioGlobalReg               .838, &
        XVioSetCAState             .839, &
        XVioCheckCharType          .840, &
        XVioDestroyCA              .841, &
        XVioCreateCA               .842, &
        VioCheckCharType           .843, &
        XVioGetCAState             .844, &
# bvscalls
        BVSMAIN                    .845, &
        BVSREDRAWSIZE              .846, &
        BVSGETPTRDRAWNAME          .847, &
# ansicall
        AnsiInject                 .848, &
        AnsiKeydef                 .849, &
        AnsiInterp                 .850, &
# bkscalls
        BKSMAIN                    .851, &
# bmscalls
        BMSMAIN                    .852, &
# moucalls
        MouGetHotKey               .853, &
        MouSetHotKey               .854, &
# sesmgr
        Dos16SMSysInit             .855, &
# queue handle variables ???
        QhKeybdHandle              .856, &
        QhMouseHandle              .857, &
# os2char
        SMQueueRamSem              .858, &
        SMArray                    .859, &
        SMPIDArray                 .860, &
        SMInitialized              .861, &
        SMArraySize                .862, &
        BVSGLOBAL                  .863, &
        SMGINSTANCE                .864, &
        BVHINSTANCE                .865, &
#
        THK32ALIASMEM              .866, &
        THK32FREEALIAS             .867, &
        THK32ALLOCVARLEN           .868, &
        THK32HANDLEBOUNDARY        .869, &
        THK32HANDLESTRING          .870, &
        THK32DEALLOC               .871, &
        THK32XHNDLR                .872, &
        Dos32SetExtLIBPATH         .873, &
        Dos32QueryExtLIBPATH       .874, &
        DosPM16SemRst              .875, &
        DosSysCtl                  .876, &
        Dos32QueryThreadContext    .877, &
        DosSGQueryTopmost          .878, &
#
        DosPerfSysCall             .976, &
                                         &
        DosOpenL                   .981, &
        DosProtectOpenL            .982, &
        DosProtectSetFileLocksL    .983, &
        DosProtectSetFilePtrL      .984, &
        DosProtectSetFileSizeL     .985, &
        DosSetFileLocksL           .986, &
        DosCancelLockRequestL      .987, &
        DosSetFilePtrL             .988, &
        DosSetFileSizeL            .989, &
        DosListIOL                 .990, &
        Dos16FSRamSemRequest2      .991, &
        Dos16FSRamSemClear2        .992, &
                                         &
# Needed for Java
        DosSystemService           .995, &
#
        DosListIO                  .996, &
        DosCreateThread2           .997, &
        Dos16SetExtLIBPATH         .998, &
        Dos16QueryExtLIBPATH       .999, &
        T32EXITLIST                .1000, &
        T32ALLOCPROTECTEDMEM       .1001, &
        T32ALIASMEM                .1002, &
        T32ALLOCMEM                .1003, &
        T32ALLOCSHAREDMEM          .1004, &
        T32GETNAMEDSHAREDMEM       .1005, &
        T32GETSHAREDMEM            .1006, &
        T32GIVESHAREDMEM           .1007, &
        T32FREEMEM                 .1008, &
        T32SETMEM                  .1009, &
        T32QUERYMEM                .1010, &
        T32QUERYMEMSTATE           .1011, &
        T32OPENVDD                 .1012, &
        T32REQUESTVDD              .1013, &
        T32CLOSEVDD                .1014, &
        T32CREATETHREAD            .1015, &
        T32DYNAMICTRACE            .1016, &
        T32DEBUG                   .1017, &
        T32QUERYPROCADDR           .1018, &
        T32CREATEEVENTSEM          .1019, &
        T32OPENEVENTSEM            .1020, &
        T32CLOSEEVENTSEM           .1021, &
        T32RESETEVENTSEM           .1022, &
        T32POSTEVENTSEM            .1023, &
        T32WAITEVENTSEM            .1024, &
        T32QUERYEVENTSEM           .1025, &
        T32CREATEMUTEXSEM          .1026, &
        T32OPENMUTEXSEM            .1027, &
        T32CLOSEMUTEXSEM           .1028, &
        T32REQUESTMUTEXSEM         .1029, &
        T32RELEASEMUTEXSEM         .1030, &
        T32QUERYMUTEXSEM           .1031, &
        T32CREATEMUXWAITSEM        .1032, &
        T32OPENMUXWAITSEM          .1033, &
        T32CLOSEMUXWAITSEM         .1034, &
        T32WAITMUXWAITSEM          .1035, &
        T32ADDMUXWAITSEM           .1036, &
        T32DELETEMUXWAITSEM        .1037, &
        T32QUERYMUXWAITSEM         .1038, &
        T32QUERYSYSINFO            .1039, &
        T32WAITTHREAD              .1040, &
        T32GETRESOURCE             .1041, &
        T32FREERESOURCE            .1042, &
        T32EXCEPTIONCALLBACK       .1043, &
        T32QUERYPAGEUSAGE          .1044, &
        T32FORCESYSTEMDUMP         .1045, &
        TI32ASYNCTIMER             .1046, &
        TI32STARTTIMER             .1047, &
        T32QUERYABIOSSUPPORT       .1048, &
        T32QUERYMODFROMEIP         .1049, &
        T32ALIASPERFCTRS           .1050, &
        T32CONFIGUREPERF           .1051, &
        T32DECONPERF               .1052, &
        T32REGISTERPERFCTRS        .1053, &
        T32QUERYSYSSTATE           .1054, &
        T32IREAD                   .1055, &
        T32IWRITE                  .1056, &
        T32TMRQUERYFREQ            .1057, &
        T32TMRQUERYTIME            .1058, &
        T32IMONREAD                .1059, &
        T32IMONWRITE               .1060, &
        T32QUERYRESOURCESIZE       .1061, &
        T32PROFILE                 .1062, &
        T32SETSIGNALEXCEPTIONFOCUS .1063, &
        T32SENDSIGNALEXCEPTION     .1064, &
        T32STARTTIMER              .1065, &
        T32STOPTIMER               .1066, &
        T32ASYNCTIMER              .1067, &
        T32INITIALIZEPORTHOLE      .1068, &
        T32QUERYHEADERINFO         .1069, &
        T32QUERYPROCTYPE           .1070, &
        T32IEXITMUSTCOMPLETE       .1071, &
        T32ICACHEMODULE            .1072, &
        T32DLLTERM                 .1073, &
        T32IRAISEEXCEPTION         .1074, &
        T32ACKNOWLEDGESIGNALEXCEPTION .1075, &
        T32QUERYDOSPROPERTY        .1076, &
        T32SETDOSPROPERTY          .1077, &
        T32SETFILELOCKS            .1078, &
        T32CANCELLOCKREQUEST       .1079, &
        T32KILLTHREAD              .1080, &
        TQUERYRASINFO              .1081, &
        T32DUMPPROCESS             .1082, &
        T32SUPPRESSPOPUPS          .1083, &
        T32IPROTECTWRITE           .1084, &
        T32PROTECTSETFILELOCKS     .1085, &
        T32IPROTECTREAD            .1086, &
        T32PMPOSTEVENTSEM          .1087, &
        T32PMWAITEVENTSEM          .1088, &
        T32PMREQUESTMUTEXSEM       .1089, &
        T32PMWAITMUXWAITSEM        .1090, &
        T32PM16SEMCHK              .1091, &
        T32ALLOCTHREADLOCALMEMORY  .1092, &
        T32FREETHREADLOCALMEMORY   .1093, &
        T32SETEXTLIBPATH           .1094, &
        T32QUERYEXTLIBPATH         .1095, &
        T32PM16SEMRST              .1096, &
        T32VERIFYPIDTID            .1097, &
        T32SYSCTL                  .1098, &
        T32QUERYTHREADCONTEXT      .1099, &
#
        T32PERFSYSCALL             .1101, &
        T32LISTIO                  .1102, &
        T32PMR3WAITEVENTSEM        .1103, &
        T32PMR3POSTEVENTSEM        .1104, &
        T32CREATETHREAD2           .1105, &
        T32PROTECTSETFILELOCKSL    .1106, &
        T32IPROTECTSETFILEPTRL     .1107, &
        T32SETFILELOCKSL           .1108, &
        T32CANCELLOCKREQUESTL      .1109, &
        T32ISETFILEPTRL            .1110, &
        T32LISTIOL                 .1111, &
# osFree extension
        DosLogWrite                .1112, &
#
        T32ISETFILEPTR             .1116, &
        T32IPROTECTSETFILEPTR      .1117, &
#
        Dos32CreateRamSem          .9004, &
        Dos32CloseRamSem           .9005, &
        Dos32OpenRamSem            .9006, &
        Dos32RequestRamSem         .9007, &
        Dos32ReleaseRamSem         .9008, &
#
        Dos32QueryProcessInfo      .9010, &
        Dos32QueryThreadInfo       .9011, &
        Dos32FindFromName          .9018

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

# <04:47> <_dixie_> объявляецца конкурс знание того, что делает doscalls.995 :)
# <04:48> <[Pasha]> _dixie_ DosJAVAIOCtl ? ;-)
# <04:48> <[Pasha]> _dixie_ думается у них оно обзывалось DosJITDispatcherThread
# <04:49> <_dixie_> [Pasha] похоже ;)
# <valeriusN> _dixie_ [Pasha] ничего не видно про Java:
# <valeriusN> DosSystemService .995
# <04:50> <[Pasha]> valeriusN а у вас ея и нету ;-) этож аврора
# <valeriusN> вот оно у нас в .lnk как обзывается
# <valeriusN> [Pasha] а у нас что? мерлин типа?
# <04:51> <[Pasha]> видимо хто-то обозвал чем-то приблизительным ;-)
# <04:51> <[Pasha]> valeriusN: у вас в начале было среднее между мерлиным и warp 3
# <04:51> <[Pasha]> что сейчас - не знаю
# <valeriusN> [Pasha] наверное, разные источники противоречивы ;)
# <valeriusN> [Pasha] то было давно и неправда ;)
# <04:54> <_dixie_> valeriusN DosSystemService это 88 ;)
# <04:54> <_dixie_> valeriusN Dos32SystemService нету, т.е. похоже, что кто-то от фонаря записал,
#         да ;)
# <04:55> <[Pasha]> _dixie_ ну жабу с нуля в отсусвии док всунуть проблематично ;-)
# <valeriusN> _dixie_ ну, это я щас глянул -- если верить Прокушеву, он мейкфайл составлял ;)
# <valeriusN> [Pasha] поэтому его надо похерить? ;)
# * <04:56> [Pasha] в таких делах бы Прокушеву не верил, вот Белову можно, но явно это не он
#   делал ;-)
# <04:59> <[Pasha]> valeriusN: ну тогда намотай Прокушеву на ус, что это жабоские internal
#         function ;-)

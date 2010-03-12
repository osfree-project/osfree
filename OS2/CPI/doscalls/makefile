#
# A Makefile for ANSI
# (c) osFree project,
# author, date
#

PROJ     = doscalls
DESC     = Control Program API
srcfiles = $(p)msg$(e) &
           $(p)doscopy$(e) &
           $(p)dllstart$(e) &
           $(p)strlcpy$(e) &
           $(p)strlcat$(e) &
           $(p)strnlen$(e) &
           $(p)strncmp$(e) &
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
           $(p)unimplemented$(e) # $(p)monitors$(e) $(p)dossearchpath$(e) # $(p)doseditname$(e)

ADD_COPT = -s
ADD_LINKOPT  = op nod segment   type DATA shared
DLL     = 1
DLLOPT  = initinstance terminstance
#OPTIONS = manyautodata
IMPORTS = DosFSCtl        KAL.KalFSCtl.2, &
          DosWrite        KAL.KalWrite.3, &
          DosExit         KAL.KalExit.4,  &
          DosOpenL        KAL.KalOpenL.5, &
          DosWrite        KAL.KalWrite.6, &
          DosClose        KAL.KalClose.7

EXPORTS = &
     DosICreateThread       .1, &
     DosPtrace              .12, &
     DosISysSemClear        .17, &
     DosISemRequest         .18, &
     DosISysSemSet          .19, &
     DosISemWait            .21, &
     DosSGSwitch            .54, &
     DosSGSwitchMe          .55, &
     DosQTraceInfo          .93, &
     DosICanonicalize       .100, &
     DosSetFgnd             .101, &
     DosSwapTaskInit        .102, &
     DosReadPhys            .103, &
     DosSGSwitchProc2       .105, &
     DosISetRelMaxFH        .108, &
     DosIDevIOCtl           .109, &
     DosForceDelete         .110, &
     DosKillThread          .111, &
     DosQueryRASInfo        .112, &
     DosDumpProcess         .113, &
     DosSuppressPopUps      .114, &
     DosOpen2Compt          .118, &
     DosGetSTDA             .119, &
     DosSGSwitchProc        .124, &
     DosIRamSemWake         .125, &
     DosIRamSemRequest      .128, &
     DosISetCP              .131, &
     DosGlobalSeg           .132, &
     DosQProcStatus         .154, &
     DosICallBack           .158, &
     DosRetForward          .159, &
     DosIMakeNmPipe         .167, &
     DosICallNmPipe         .168, &
     DosIConnectNmPipe      .169, &
     DosIDisconnectNmPipe   .170, &
     DosIPeekNmPipe         .171, &
     DosIQNmPipeInfo        .172, &
     DosIQNmPHandState      .173, &
     DosISetNmPHandState    .174, &
     DosITransactNmPipe     .175, &
     DosIWaitNmPipe         .176, &
     DosISetNmPipeSem       .177, &
     DosIQNmPipeSemState    .178, &
     DosIRawReadNmPipe      .179, &
     DosIRawWriteNmPipe     .180, &
     DosSetTraceInfo      .190  , &
     DosLogMode           .192  , &
     DosLogEntry          .193  , &
     DosGetLogBuffer      .194  , &
     DosLogRegister       .195  , &
     DosLogRead           .196  , &
     DosFindFromName      .197  , &
     DosOPLockRelease     .198  , &
     DosOPLockWait        .199  , &
     DosICopy             .200  , &
     DosIQAppType         .202  , &
     DosOPLockShutdown    .205  , &
     DosSetMaxFH          .209  , &
     DosSetVerify         .210  , &
     DosErrClass          .211  , &
     DosError             .212  , &
     DosCreateVDM         .213  , &
     DosSetFileInfo       .218  , &
     DosSetPathInfo       .219  , &
     DosSetDefaultDisk    .220  , &
     DosSetFHState        .221  , &
     DosSetFSInfo         .222  , &
     DosQueryPathInfo     .223  , &
     DosQueryHType        .224  , &
     DosQueryVerify       .225  , &
     DosDeleteDir         .226  , &
     DosScanEnv           .227  , &
     DosSearchPath        .228  , &
     DosSleep             .229  , &
     DosGetDateTime       .230  , &
     DosDevConfig         .231  , &
     DosEnterCritSec      .232  , &
     DosExitCritSec       .233  , &
     DosExit              .234 , &
     DosKillProcess        .235  , &
     DosSetPriority        .236  , &
     DosResumeThread       .237  , &
     DosSuspendThread      .238  , &
     DosCreatePipe         .239  , &
     DosCallNPipe          .240  , &
     DosConnectNPipe       .241  , &
     DosDisConnectNPipe    .242  , &
     DosCreateNPipe        .243  , &
     DosPeekNPipe          .244  , &
     DosQueryNPHState      .245  , &
     DosRawReadNPipe       .246  , &
     DosRawWriteNPipe      .247  , &
     DosQueryNPipeInfo     .248  , &
     DosQueryNPipeSemState .249  , &
     DosSetNPHState        .250  , &
     DosSetNPipeSem        .251  , &
     DosTransactNPipe      .252  , &
     DosWaitNPipe          .253  , &
     DosResetBuffer        .254  , &
     DosSetCurrentDir      .255  , &
     DosSetFilePtr         .256  , &
     DosClose              .257  , &
     DosCopy               .258  , &
     DosDelete             .259  , &
     DosDupHandle          .260  , &
     DosEditName           .261  , &
     DosFindClose          .263  , &
     DosFindFirst          .264  , &
     DosFindNext           .265  , &
     DosFSAttach           .269  , &
     DosCreateDir          .270  , &
     DosMove               .271  , &
     DosSetFileSize        .272  , &
     DosOpen               .273  , &
     DosQueryCurrentDir    .274  , &
     DosQueryCurrentDisk   .275  , &
     DosQueryFHState       .276  , &
     DosQueryFSAttach      .277  , &
     DosQueryFSInfo        .278  , &
     DosQueryFileInfo      .279  , &
     DosWaitChild          .280  , &
     DosRead               .281  , &
     DosWrite              .282  , &
     DosExecPgm            .283  , &
     DosDevIOCtl           .284  , &
     DosFSCtl              .285  , &
     DosBeep               .286  , &
     DosPhysicalDisk       .287  , &
     DosSetCp              .288  , &
     DosSetProcessCp       .289  , &
     DosStopTimer          .290  , &
     DosQueryCp            .291  , &
     DosSetDateTime        .292  , &
     Thk32AllocBlock       .293  , &
     Thk32FreeBlock        .294  , &
     Thk32R3DS             .295  , &
     DosExitList           .296  , &
     DosAllocProtectedMem  .297  , &
     DosAliasMem           .298  , &
     DosAllocMem           .299  , &
     DosAllocSharedMem     .300  , &
     DosGetNamedSharedMem  .301  , &
     DosGetSharedMem       .302  , &
     DosGiveSharedMem      .303  , &
     DosFreeMem            .304  , &
     DosSetMem             .305  , &
     DosQueryMem           .306  , &
     DosQueryMemState      .307  , &
     DosOpenVDD            .308  , &
     DosRequestVDD         .309  , &
     DosCloseVDD           .310  , &
     DosCreateThread       .311  , &
     DosGetInfoBlocks      .312  , &
     DosAllocProtSeg       .313  , &
     DosAllocShrProtSeg    .314  , &
     DosAllocProtHuge      .315  , &
     DosDynamicTrace       .316  , &
     DosDebug              .317  , &
     DosLoadModule         .318  , &
     DosQueryModuleHandle  .319  , &
     DosQueryModuleName    .320  , &
     DosQueryProcAddr      .321  , &
     DosFreeModule         .322  , &
     DosQueryAppType       .323  , &
     DosCreateEventSem     .324  , &
     DosOpenEventSem       .325  , &
     DosCloseEventSem      .326  , &
     DosResetEventSem      .327  , &
     DosPostEventSem       .328  , &
     DosWaitEventSem       .329  , &
     DosQueryEventSem      .330  , &
     DosCreateMutexSem     .331  , &
     DosOpenMutexSem       .332  , &
     DosCloseMutexSem      .333  , &
     DosRequestMutexSem    .334  , &
     DosReleaseMutexSem    .335  , &
     DosQueryMutexSem      .336  , &
     DosCreateMuxWaitSem   .337  , &
     DosOpenMuxWaitSem     .338  , &
     DosCloseMuxWaitSem    .339  , &
     DosWaitMuxWaitSem     .340  , &
     DosAddMuxWaitSem      .341  , &
     DosDeleteMuxWaitSem   .342  , &
     DosQueryMuxWaitSem    .343  , &
     DosSubSetMem          .344  , &
     DosSubAllocMem        .345  , &
     DosSubFreeMem         .346  , &
     DosSubUnsetMem        .347  , &
     DosQuerySysInfo       .348 , &
     DosWaitThread              .349 , &
     DosAsyncTimer              .350 , &
     DosStartTimer              .351 , &
     DosGetResource             .352 , &
     DosFreeResource            .353 , &
     DosSetExceptionHandler     .354 , &
     DosUnsetExceptionHandler   .355 , &
     DosRaiseException          .356 , &
     DosUnwindException         .357 , &
     DosQueryPageUsage          .358 , &
     DosQueryModFromCS          .359 , &
     DosQueryModFromEIP         .360 , &
     DosFPDataArea              .361 , &
     DosTmrQueryFreq            .362 , &
     DosTmrQueryTime            .363 , &
     DosAliasPerfCtrs           .364 , &
     DosConfigPerf              .365 , &
     DosDeconPerf               .366 , &
     DosRegisterPerfCtrs        .367 , &
     DosQuerySysState           .368 , &
     DosFlatCS                  .369 , &
     DosFlatDS                  .370 , &
     DosQueryABIOSSupport       .371 , &
     DosEnumAttribute           .372 , &
     DosQueryDOSProperty        .373 , &
     DosSetDOSProperty          .374 , &
     DosSetSignalExceptionFocus .378   , &
     DosSendSignalException     .379   , &
     DosEnterMustComplete       .380   , &
     DosExitMustComplete        .381   , &
     DosSetRelMaxFH             .382   , &
     DosInsertMessage           .386   , &
     Dos32PutMessage            .387 = DosPutMessage  , &
     DosCaseMap                     .391, &
     DosGetCollate                  .392, &
     DosGetCtryInfo                 .393, &
     DosGetDBCSEv                   .394, &
     DosQueryCtryInfo               .395, &
     DosQueryDBCSEnv                .396, &
     DosMapCase                     .397, &
     DosQueryCollate                .398, &
     DosMakeNmPipe                  .399, &
     DosQNmPipeInfo                 .400, &
     DosConnectNmPipe               .401, &
     DosDisconnectNmPipe            .402, &
     DosQNmPHandState               .403, &
     DosSetNmPHandState             .404, &
     DosPeekNmPipe                  .405, &
     DosWaitNmPipe                  .406, &
     DosTransactNmPipe              .407, &
     DosCallNmPipe                  .408, &
     DosRawReadNmPipe               .409, &
     DosRawWriteNmPipe              .410, &
     DosSetNmPipeSem                .411, &
     DosQNmPipeSemState             .412, &
     StartLazyWriter                .413, &
     _QueInst_Data                  .414, &
     DosShutdown                    .415, &
     DosICacheModule                .416, &
     DosReplaceModule               .417, &
     DosAcknowledgeSignalException  .418, &
     DosTIB                         .419, &
     DosIODelayCnt                  .427  , &
     DosSetFileLocks                .428  , &
     DosCancelLockRequest           .429  , &
     LogOpen                        .430  , &
     LogClose                       .431  , &
     LogAddEntries                  .432  , &
     LogGetEntries                  .433  , &
     LogSetState                    .434  , &
     LogSetName                     .435  , &
     LogQueryState                  .436  , &
     DosOpenChangeNotify            .440  , &
     DosResetChangeNotify           .441  , &
     DosCloseChangeNotify           .442  , &
     DosForceSystemDump             .444  , &
     DosAllocThreadLocalMemory                   .454, &
     DosFreeThreadLocalMemory                    .455, &
     DosVerifyPidTid                             .460, &
     PTDA_LANMAN_SEC                .464              , &
     PTDA_PID                       .465              , &
     SAS_SEL                        .466              , &
     TCB_OPCOOKIE                   .467              , &
     TCB_OPFLAGS                    .468              , &
     TCB_NEWFLAGS                   .469              , &
     TCB_USER_ID                    .470              , &
     TCB_PROC_ID                    .471              , &
     TCB_FSHARING                   .472              , &
     TCB_SRVATTRIB                  .473              , &
     TCB_ALLOWED                    .474              , &
     TCB_PRTCB                      .475              , &
     TCB_NUMBER                     .476              , &
     TCB_THISSFT                    .477              , &
     TCB_THISCDS                    .478              , &
     TKOPTDA                        .479              , &
     PTDA_CRITSEC                   .480              , &
     PTDA_HOLDSIGCNT                .481              , &
     PTDA_PPTDAPARENT               .482              , &
     PTDA_PGDATA                    .483              , &
     PTDA_HANDLE                    .484              , &
     PTDA_MODULE                    .485              , &
     PTDA_LDTHANDLE                 .486              , &
     PTDA_CODEPAGE_TAG              .487              , &
     PTDA_JFN_LENGTH                .488              , &
     PTDA_JFN_PTABLE                .489              , &
     PTDA_JFN_FLG_PTR               .490              , &
     PTDA_EXTERR_LOCUS              .491              , &
     PTDA_EXTERR                    .492              , &
     PTDA_EXTERR_ACTION             .493              , &
     PTDA_EXTERR_CLASS              .494              , &
     PTDA_PPID                      .495              , &
     PTDA_PROCTYPE                  .496              , &
     PTDA_CURRTCB                   .497              , &
     PTDA_CURRTSD                   .498              , &
     PTDA_SIGNATURE                 .499              , &
     T32EXITLIST                    .500              , &
     T32ALLOCPROTECTEDMEM           .501              , &
     T32ALIASMEM                    .502              , &
     T32ALLOCMEM                    .503              , &
     T32ALLOCSHAREDMEM              .504              , &
     T32GETNAMEDSHAREDMEM           .505              , &
     T32GETSHAREDMEM                .506              , &
     T32GIVESHAREDMEM               .507              , &
     T32FREEMEM                     .508              , &
     T32SETMEM                      .509              , &
     T32QUERYMEM                    .510              , &
     T32QUERYMEMSTATE               .511              , &
     T32OPENVDD                     .512              , &
     T32REQUESTVDD                  .513              , &
     T32CLOSEVDD                    .514              , &
     T32CREATETHREAD                .515              , &
     T32DYNAMICTRACE                .516              , &
     T32DEBUG                       .517              , &
     T32QUERYPROCADDR               .518              , &
     T32CREATEEVENTSEM              .519              , &
     T32OPENEVENTSEM                .520              , &
     T32CLOSEEVENTSEM               .521              , &
     T32RESETEVENTSEM               .522              , &
     T32POSTEVENTSEM                .523              , &
     T32WAITEVENTSEM                .524              , &
     T32QUERYEVENTSEM               .525              , &
     T32CREATEMUTEXSEM              .526              , &
     T32OPENMUTEXSEM                .527              , &
     T32CLOSEMUTEXSEM               .528              , &
     T32REQUESTMUTEXSEM             .529              , &
     T32RELEASEMUTEXSEM             .530              , &
     T32QUERYMUTEXSEM               .531              , &
     T32CREATEMUXWAITSEM            .532              , &
     T32OPENMUXWAITSEM              .533              , &
     T32CLOSEMUXWAITSEM             .534              , &
     T32WAITMUXWAITSEM              .535              , &
     T32ADDMUXWAITSEM               .536              , &
     T32DELETEMUXWAITSEM            .537              , &
     T32QUERYMUXWAITSEM             .538              , &
     T32QUERYSYSINFO                .539              , &
     T32WAITTHREAD                  .540              , &
     T32GETRESOURCE                 .541              , &
     T32FREERESOURCE                .542              , &
     T32EXCEPTIONCALLBACK           .543              , &
     T32QUERYPAGEUSAGE              .544              , &
     DosExceptionCallBack           .545              , &
     TI32ASYNCTIMER                 .546              , &
     TI32STARTTIMER                 .547              , &
     DosR3ExceptionDispatcher       .548              , &
     DosLibiDisp                    .549              , &
     DosLibiDisp16                  .550              , &
     DosLibiDisp32                  .551              , &
     DosIRead                       .554              , &
     T32IREAD                       .555              , &
     DosIWrite                      .556              , &
     T32IWRITE                      .557              , &
     T32TMRQUERYFREQ                .558              , &
     T32TMRQUERYTIME                .559              , &
     T32ALIASPERFCTRS               .560              , &
     T32CONFIGUREPERF               .561              , &
     T32DECONPERF                   .562              , &
     T32REGISTERPERFCTRS            .563              , &
     T32QUERYSYSSTATE               .564              , &
     DosISetFileInfo                .565              , &
     DosISetPathInfo                .566              , &
     T32QUERYDOSPROPERTY            .567              , &
     T32SETDOSPROPERTY              .568              , &
     DosIFindNext                   .569              , &
     T32PROFILE                     .570              , &
     T32QUERYRESOURCESIZE           .571              , &
     DosQueryResourceSize           .572              , &
     T32SETSIGNALEXCEPTIONFOCUS     .574    , &
     T32SENDSIGNALEXCEPTION         .575    , &
     T32STARTTIMER                  .576    , &
     T32STOPTIMER                   .577    , &
     T32ASYNCTIMER                  .578    , &
     T32INITIALIZEPORTHOLE          .579    , &
     DosInitializePorthole          .580    , &
     T32QUERYHEADERINFO             .581    , &
     DosQueryHeaderInfo             .582    , &
     T32QUERYPROCTYPE               .585    , &
     DosQueryProcType               .586    , &
     T32IEXITMUSTCOMPLETE           .588    , &
     DosISigDispatch                .589    , &
     T32ICACHEMODULE                .590    , &
     T32DLLTERM                     .591    , &
     DosDllTermDisp                 .592    , &
     T32IRAISEEXCEPTION             .593    , &
     DosIRaiseException             .594    , &
     T32ACKNOWLEDGESIGNALEXC        .595    , &
     DosPM16SemChk                 .596, &
     DosIQueryFHState               .597              , &
     DosISetFHState                 .598              , &
     DosLDTSel                      .599              , &
     DosR3Frestor                   .600              , &
     DosIFindFirst                  .601              , &
     T32SETFILELOCKS                .602              , &
     T32CANCELLOCKREQUEST           .603              , &
     T32IMONREAD                    .604              , &
     T32IMONWRITE                   .605              , &
     DosMonOpen                     .606              , &
     DosMonClose                    .607              , &
     DosMonRead                     .608              , &
     DosMonWrite                    .609              , &
     DosMonReg                      .610              , &
     T32KILLTHREAD                  .611              , &
     TQUERYRASINFO                  .612              , &
     DosProtectSetFilePtr           .621              , &
     DosProtectEnumAttribute        .636              , &
     DosProtectOpen                 .637              , &
     DosProtectClose                .638              , &
     DosProtectSetFileLocks         .639              , &
     DosProtectSetFileSize          .640              , &
     DosProtectRead                 .641              , &
     DosProtectWrite                .642              , &
     DosProtectSetFileInfo          .643              , &
     DosProtectSetFHState           .644              , &
     DosProtectQueryFHState         .645              , &
     DosProtectQueryFileInfo        .646              , &
     DosCloseMessageFile            .649             , &
     DosSetExtLIBPATH                            .873, &
     DosQueryExtLIBPATH                          .874, &
#     DosPM16SemRst                               .875, &
     DosSysCtl                                   .876, &
     DosQueryThreadContext                       .877, &
     DosSGQueryTopmost                           .878, &
     DosPerfSysCall                              .976, &
     DosOpenL                                    .981, &
     DosProtectOpenL                             .982, &
     DosProtectSetFileLocksL                     .983, &
     DosProtectSetFilePtrL                       .984, &
     DosProtectSetFileSizeL                      .985, &
     DosSetFileLocksL                            .986, &
     DosCancelLockRequestL                       .987, &
     DosSetFilePtrL                              .988, &
     DosSetFileSizeL                             .989, &
     DosListIOL                                  .990, &
     DosSystemService                            .995, &
     DosListIO                                   .996, &
     DosCreateThread2                            .997

# alias     DosSetSignalExceptionFoc                  @378
# conflict? DOS16PMSEMWAIT                              @574
# conflict? DOS16PMMUXSEMWAIT                           @575
# conflict? DosPMPostEventSem                           @590
# conflict? DosPMWaitEventSem                           @591
# conflict? DosPMRequestMutexSem                        @593
# conflict? DosPMWaitMuxWaitSem                         @595
# need to detect which 16bit/32bit
#      DosProfile             .133, &
#     DosProfile                .377   , &
#     DosPutMessage             .387, &
#     DosR3ExitAddr                  .552              , &
#     DosR3ExitAddr                  .553              , &
#     DosTrueGetMessage          .384   , &
#     DosTrueGetMessage              .388, &
#     DosIQueryMessageCP             .389, &
#     DosIQueryMessageCP             .390, &
#     Dos16IRead               .79, &
#     Dos16IWrite            .87, &
#     Dos16Cwait             .2, &
#     Dos16EnterCritSec      .3, &
#     Dos16Exit              .5, &
#     Dos16ExitCritSec       .6, &
#     Dos16ExitList          .7, &
#     Dos16GetInfoSeg        .8, &
#     Dos16GetPrty           .9, &
#     Dos16KillProcess       .10, &
#     Dos16SetPrty           .11, &
#     Dos16HoldSignal        .13, &
#     Dos16SetSigHandler     .14, &
#     Dos16FlagProcess       .15, &
#     Dos16MakePipe          .16, &

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

#DOSCOPY.201 RESIDENT, &
#     DosFlatToSel                   .425  , &
#     DosSelToFlat                   .426  , &
#          DOSICOPY.200,         &
#     DosIExecPgm            .4, &
#          Dos32Copy.258,        &
#          Dos32EditName.261
# 16-bit functions. No need to implements because 32-bit versions exists and calls to
# 16-bit will be raplaced by 32-bit calls.
#     Dos16QueryResourceSize         .573            , &
#     Dos16QueryProcType             .587  , &
#     DosInsMessage              .385   , &
#     DosPutMessage              .383   , &
#     Dos16SemSetWait        .20, &
#     Dos16MuxSemWait        .22, &
#     Dos16CloseSem          .23, &
#     Dos16CreateSem         .24, &
#     Dos16OpenSem           .25, &
#     Dos16ResumeThread      .26, &
#     Dos16SuspendThread     .27, &
#     Dos16SetDateTime       .28, &
#     Dos16TimerAsync        .29, &
#     Dos16TimerStart        .30, &
#     Dos16TimerStop         .31, &
#     Dos16Sleep             .32, &
#     Dos16GetDateTime       .33, &
#     Dos16AllocSeg          .34, &
#     Dos16AllocShrSeg       .35, &
#     Dos16GetShrSeg         .36, &
#     Dos16GiveSeg           .37, &
#     Dos16ReallocSeg        .38, &
#     Dos16FreeSeg           .39, &
#     Dos16AllocHuge         .40, &
#     Dos16GetHugeShift      .41, &
#     Dos16ReallocHuge       .42, &
#     Dos16CreateCSAlias     .43, &
#     Dos16LoadModule        .44, &
#     Dos16GetProcAddr       .45, &
#     Dos16FreeModule        .46, &
#     Dos16GetModHandle      .47, &
#     Dos16GetModName        .48, &
#     Dos16GetMachineMode    .49, &
#     Dos16Beep              .50, &
#     Dos16CLIAccess         .51, &
#     Dos16DevConfig         .52, &
#     Dos16DevIOCtl          .53, &
#     Dos16BufReset          .56, &
#     Dos16ChDir             .57, &
#     Dos16ChgFilePtr        .58, &
#     Dos16Close             .59, &
#     Dos16Delete            .60, &
#     Dos16DupHandle         .61, &
#     Dos16FileLocks         .62, &
#     Dos16FindClose         .63, &
#     Dos16FindFirst         .64, &
#     Dos16FindNext          .65, &
#     Dos16MkDir             .66, &
#     Dos16Move              .67, &
#     Dos16NewSize           .68, &
#     Dos16PortAccess        .69, &
#     Dos16Open              .70, &
#     Dos16QCurDir           .71, &
#     Dos16QCurDisk          .72, &
#     Dos16QFHandState       .73, &
#     Dos16QFileInfo         .74, &
#     Dos16QFileMode         .75, &
#     Dos16QFSInfo           .76, &
#     Dos16QHandType         .77, &
#     Dos16QVerify           .78, &
#     Dos16RmDir             .80, &
#     Dos16SelectDisk        .81, &
#     Dos16SetFHandState     .82, &
#     Dos16SetFileInfo       .83, &
#     Dos16SetFileMode       .84, &
#     Dos16SetMaxFH          .85, &
#     Dos16SetVerify         .86, &
#     Dos16SystemService     .88, &
#     Dos16SetVec            .89, &
#     Dos16SysTrace          .90, &
#     Dos16GetEnv            .91, &
#     Dos16GetVersion        .92, &
#     Dos16GetPID            .94, &
#     Dos16Open2             .95, &
#     Dos16LibInit           .96, &
#     Dos16SetFSInfo         .97, &
#     Dos16QPathInfo         .98, &
#     Dos16DevIOCtl2         .99, &
#     Dos16SetPathInfo       .104, &
#     Dos16Error             .120, &
#     Dos16GetSeg            .121, &
#     Dos16LockSeg           .122, &
#     Dos16UnlockSeg         .123, &
#     Dos16SizeSeg           .126, &
#     Dos16MemAvail          .127, &
#     Dos16PhysicalDisk      .129, &
#     Dos16GetCP             .130, &
#     Dos16SendSignal        .134, &
#     Dos16HugeShift         .135, &
#     Dos16HugeIncr          .136, &
#     Dos16Read              .137, &
#     Dos16Write             .138, &
#     Dos16ErrClass          .139, &
#     Dos16SemRequest        .140, &
#     Dos16SemClear          .141, &
#     Dos16SemWait           .142, &
#     Dos16SemSet            .143, &
#     Dos16ExecPgm           .144, &
#     Dos16CreateThread      .145, &
#     Dos16SubSet            .146, &
#     Dos16SubAlloc          .147, &
#     Dos16SubFree           .148, &
#     Dos16ReadAsync         .149, &
#     Dos16WriteAsync        .150, &
#     Dos16SearchPath        .151, &
#     Dos16ScanEnv           .152, &
#     Dos16SetCP             .153, &
#     Dos16GetResource       .155, &
#     Dos16GetPPID           .156, &
#     Dos16CallBack          .157, &
#     Dos16R2StackRealloc    .160, &
#     Dos16FSRamSemRequest   .161, &
#     Dos16FSRamSemClear     .162, &
#     Dos16QAppType          .163, &
#     Dos16SetProcCP         .164, &
#     Dos16DynamicTrace      .165, &
#     Dos16QSysInfo          .166, &
#     Dos16FSAttach          .181, &
#     Dos16QFSAttach         .182, &
#     Dos16FSCtl             .183, &
#     Dos16FindFirst2        .184, &
#     Dos16MkDir2            .185, &
#     Dos16FileIO            .186, &
#     Dos16FindNotifyClose   .187, &
#     Dos16FindNotifyFirst   .188, &
#     Dos16FindNotifyNext    .189, &
#     Dos16EditName          .191, &
#     Dos16Copy              .201, &
#     Dos16ForceDelete       .203, &
#     Dos16EnumAttribute     .204, &
#     Dos16Shutdown          .206, &
#     Dos16GetResource2      .207, &
#     Dos16FreeResource      .208, &
#     Dos16MaxPathLen        .214, &
#     Dos16PageSize          .215, &
#     Dos16LocalInfo         .216, &
#     Dos16GlobalInfo        .217, &
#     Dos16OpenVDD            .266, &
#     Dos16RequestVDD         .267, &
#     Dos16CloseVDD           .268, &
#     Dos16QueryDosProperty        .375 , &
#     Dos16SetDosProperty          .376 , &
#     Dos16TmrQueryFreq              .420, &
#     Dos16TmrQueryTime              .421, &
#     Dos16RegisterPerfCtrs          .422, &
#     Dos16FlatToSel                 .423, &
#     Dos16SelToFlat                 .424, &
#     Dos16OpenChangeNotify            .437, &
#     Dos16ResetChangeNotify           .438, &
#     Dos16CloseChangeNotify           .439, &
#     Dos16InitializePorthole        .583  , &
#     Dos16QueryHeaderInfo           .584  , &
#     Dos16ProtectClose                .622            , &
#     Dos16ProtectFileIO               .623            , &
#     Dos16ProtectFileLocks            .624            , &
#     Dos16ProtectNewSize              .627            , &
#     Dos16ProtectOpen                 .628            , &
#     Dos16ProtectQFHandState          .629            , &
#     Dos16ProtectSetFHandState        .630            , &
#     Dos16ProtectQFileInfo            .631            , &
#     Dos16ProtectSetFileInfo          .632            , &
#     Dos16ProtectChgFilePtr           .634            , &
#     Dos16ProtectEnumAttribute        .635            , &
#     DOS16FSRAMSEMREQUEST2                       .991, &
#     DOS16FSRAMSEMCLEAR2                         .992, &
#     DOS16SETEXTLIBPATH                          .998, &
#     DOS16QUERYEXTLIBPATH                        .999


DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk

#
# A Makefile for doscalls.dll
# (c) osFree project,
# authors:
# Yuri Prokushev,   2002-2006,
# Valery Sedletski, 2010-2011.
#

PROJ     = doscalls
TRGT     = $(PROJ).dll
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
           $(p)dosfsctl$(e) &
           $(p)memory$(e) &
           $(p)submem$(e) &
           $(p)xcpt$(e) &
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
           $(p)dos16exit$(e) &
           $(p)dos16getmachinemode$(e)    &
           $(p)dos16cwait$(e) &
           $(p)dos16bufreset$(e) &
           $(p)dos16chdir$(e) &
           $(p)dos16close$(e) &
           $(p)dos16delete$(e) &
           $(p)dos16duphandle$$(e) &
           $(p)dos16getversion$$(e) &
           $(p)dos16mkdir$$(e) &
           $(p)dos16qverify$$(e) &
           $(p)dos16setverify$$(e) &
           $(p)dos16rmdir$$(e) &
           $(p)dos16selectdisk$(e) &
           $(p)dos16truegetmessage$(e) &
		   $(p)dos16putmessage$(e) &
		   $(p)dos16entercritsec$(e) &
		   $(p)dos16exitcritsec$(e) &
		   $(p)dos16purgequeue$(e) &
		   $(p)dos16closequeue$(e) &
		   $(p)dos16protectclose$(e) &
		   $(p)dos16freespinlock$(e) &
		   $(p)dos16findclose$(e) &
		   $(p)dos16setmaxfh$(e) &
		   $(p)dos16error$(e) &
		   $(p)dos16setcp$(e) &
		   $(p)dos16setproccp$(e) &
		   $(p)dos16copy$(e) &
		   $(p)dos16connectnmpipe$(e) &
		   $(p)dos16disconnectnmpipe$(e) &
		   $(p)dos16setnmphandstate$(e) &
		   $(p)dos16killprocess$(e) &
		   $(p)dos16setprty$(e) &
		   $(p)dos16waitnmpipe$(e) &
		   $(p)dos16setnmpipesem$(e) &
		   $(p)dos16timerstart$(e) &
		   $(p)dos16freemodule$(e) &
		   $(p)dos16getmodhandle$(e) &
		   $(p)dos16getmodname$(e) &
		   $(p)dos16beep$(e) &
		   $(p)dos16move$(e) &
		   $(p)dos16qcurdir$(e) &
		   $(p)dos16qnmphstate$(e) &
		   $(p)dos16makepipe$(e) &
		   $(p)dos16newsize$(e) &
		   $(p)dos16editname$(e) &
           $(p)doserror$(e) &
           $(p)dosexecpgm$(e) &
           $(p)dossetcurrentdir$(e) &
           $(p)dosquerycurrentdir$(e) &
           $(p)dosquerycurrentdisk$(e) &
           $(p)dossetdefaultdisk$(e) &
           $(p)modmgr$(e) &
           $(p)log$(e)    &
           $(p)env$(e)    &
           $(p)thread$(e) &
           $(p)thunk$(e)  &
           $(p)sched$(e)  &
           $(p)sem$(e)    &
           $(p)unimplemented$(e) &
           $(p)unimp16$(e) &
           $(p)doseditname$(e) &
           $(p)dossearchpath$(e) &
           $(p)token$(e) &
           $(p)monitors$(e) &
           $(p)monitors16$(e) &
           $(p)viocalls$(e) &
           $(p)moucalls$(e) &
           $(p)BmsMain$(e) &
           $(p)BksMain$(e) &
           $(p)KbdRoute$(e) &
           $(p)kbddebug$(e) &
#           $(p)MouRoute$(e) &
#           $(p)moudebug$(e) &
		   $(p)nlsmsg$(e) &
           $(p)vars$(e)

ADD_COPT = -s -od
ADD_ASMOPT  = -I=$(%OS2TK)$(SEP)inc -I=$(MYDIR)..$(SEP)macrolib -I=$(PATH)
ADD_LINKOPT  = lib clib3r op nod segment   type DATA shared
DLL     = 1
DLLOPT  = initinstance terminstance
OPTIONS = manyautodata OPTION CASEEXACT
# UNI2H = 1
JWASM   = 1

IMPORTS = &
        _KalAllocMem             KAL.KalAllocMem, &
        _KalAllocSharedMem       KAL.KalAllocSharedMem, &
        _KalClose                KAL.KalClose, &
        _KalCloseEventSem        KAL.KalCloseEventSem, &
        _KalCreateDir            KAL.KalCreateDir, &
        _KalCreateEventSem       KAL.KalCreateEventSem, &
        _KalCreateThread         KAL.KalCreateThread, &
        _KalDelete               KAL.KalDelete, &
        _KalDeleteDir            KAL.KalDeleteDir, &
        _KalDupHandle            KAL.KalDupHandle, &
        _KalEnter                KAL.KalEnter, &
        _KalError                KAL.KalError, &
        _KalExecPgm              KAL.KalExecPgm, &
        _KalExit                 KAL.KalExit, &
        _KalFSCtl                KAL.KalFSCtl, &
        _KalFindClose            KAL.KalFindClose, &
        _KalFindFirst            KAL.KalFindFirst, &
        _KalFindNext             KAL.KalFindNext, &
        _KalForceDelete          KAL.KalForceDelete, &
        _KalFreeMem              KAL.KalFreeMem, &
        _KalGetInfoBlocks        KAL.KalGetInfoBlocks, &
        _KalGetNamedSharedMem    KAL.KalGetNamedSharedMem, &
        _KalGetSharedMem         KAL.KalGetSharedMem, &
        _KalGiveSharedMem        KAL.KalGiveSharedMem, &
        _KalKillThread           KAL.KalKillThread, &
        _KalLoadModule           KAL.KalLoadModule, &
        _KalLogWrite             KAL.KalLogWrite, &
        _KalMove                 KAL.KalMove, &
        _KalOpenEventSem         KAL.KalOpenEventSem, &
        _KalOpenL                KAL.KalOpenL, &
        _KalQueryAppType         KAL.KalQueryAppType, &
        _KalQueryCp              KAL.KalQueryCp, &
        _KalQueryCurrentDir      KAL.KalQueryCurrentDir, &
        _KalQueryCurrentDisk     KAL.KalQueryCurrentDisk, &
        _KalQueryDBCSEnv         KAL.KalQueryDBCSEnv, &
        _KalQueryFHState         KAL.KalQueryFHState, &
        _KalQueryFileInfo        KAL.KalQueryFileInfo, &
        _KalQueryHType           KAL.KalQueryHType, &
        _KalQueryMem             KAL.KalQueryMem, &
        _KalQueryModuleHandle    KAL.KalQueryModuleHandle, &
        _KalQueryModuleName      KAL.KalQueryModuleName, &
        _KalQueryPathInfo        KAL.KalQueryPathInfo, &
        _KalQueryProcAddr        KAL.KalQueryProcAddr, &
        _KalQueryProcType        KAL.KalQueryProcType, &
        _KalQuit                 KAL.KalQuit, &
        _KalRead                 KAL.KalRead, &
        _KalResetBuffer          KAL.KalResetBuffer, &
        _KalResumeThread         KAL.KalResumeThread, &
        _KalSetCurrentDir        KAL.KalSetCurrentDir, &
        _KalSetDefaultDisk       KAL.KalSetDefaultDisk, &
        _KalSetFHState           KAL.KalSetFHState, &
        _KalSetFilePtrL          KAL.KalSetFilePtrL, &
        _KalSetFileSizeL         KAL.KalSetFileSizeL, &
        _KalSetMaxFH             KAL.KalSetMaxFH, &
        _KalSetMem               KAL.KalSetMem, &
        _KalSetRelMaxFH          KAL.KalSetRelMaxFH, &
        _KalSleep                KAL.KalSleep, &
        _KalSuspendThread        KAL.KalSuspendThread, &
        _KalWaitThread           KAL.KalWaitThread, &
        _KalWrite                KAL.KalWrite, &
											&
        WinDefAVioWindowProc     SUB32.30, &
                                            &
        Vio32EndPopUp              SUB32.101, &
        Vio32GetPhysBuf            SUB32.102, &
        Vio32GetAnsi               SUB32.103, &
        Vio32SetAnsi               SUB32.105, &
        Vio32WrtTTY                SUB32.119, &
        Vio32DeRegister            SUB32.106, &
        Vio32ScrollUp              SUB32.107, &
        Vio32PrtSc                 SUB32.108, &
        Vio32GetCurPos             SUB32.109, &
        Vio32WrtCellStr            SUB32.110, &
        Vio32PopUp                 SUB32.111, &
        Vio32ScrollRt              SUB32.112, &
        Vio32WrtCharStr            SUB32.113, &
        Vio32SetCurPos             SUB32.115, &
        Vio32ScrUnLock             SUB32.118, &
        Vio32GetMode               SUB32.121, &
        Vio32SetMode               SUB32.122, &
        Vio32ScrLock               SUB32.123, &
        Vio32ReadCellStr           SUB32.124, &
        Vio32SavRedrawWait         SUB32.125, &
        Vio32WrtNAttr              SUB32.126, &
        Vio32GetCurType            SUB32.127, &
        Vio32SavRedrawUndo         SUB32.128, &
        Vio32GetFont               SUB32.129, &
        Vio32ReadCharStr           SUB32.130, &
        Vio32GetBuf                SUB32.131, &
        Vio32SetCurType            SUB32.132, &
        Vio32SetFont               SUB32.133, &
        Vio32ModeUndo              SUB32.135, &
        Vio32ModeWait              SUB32.137, &
        Vio32GetCp                 SUB32.140, &
        Vio32SetCp                 SUB32.142, &
        Vio32ShowBuf               SUB32.143, &
        Vio32ScrollLf              SUB32.144, &
        Vio32Register              SUB32.145, &
        Vio32GetConfig             SUB32.146, &
        Vio32ScrollDn              SUB32.147, &
        Vio32WrtCharStrAtt         SUB32.148, &
        Vio32GetState              SUB32.149, &
        Vio32PrtScToggle           SUB32.150, &
        Vio32SetState              SUB32.151, &
        Vio32WrtNCell              SUB32.152, &
        Vio32WrtNChar              SUB32.153, &
        Vio32Associate             SUB32.155, &
        Vio32CreatePS              SUB32.156, &
        Vio32DeleteSetId           SUB32.157, &
        Vio32GetDeviceCellSize     SUB32.158, &
        Vio32GetOrg                SUB32.159, &
        Vio32CreateLogFont         SUB32.160, &
        Vio32DestroyPS             SUB32.161, &
        Vio32QuerySetIds           SUB32.162, &
        Vio32SetOrg                SUB32.163, &
        Vio32QueryFonts            SUB32.164, &
        Vio32SetDeviceCellSize     SUB32.165, &
        Vio32ShowPS                SUB32.166, &
        Vio32GlobalReg             SUB32.170, &
        Vio32CheckCharType         SUB32.175, &
                                            &
        KbdSetCustXt             SUB32.201, &
        KbdGetCp                 SUB32.203, &
        KbdCharIn                SUB32.204, &
        KbdSetCp                 SUB32.205, &
        KbdSynch                 SUB32.207, &
        KbdRegister              SUB32.208, &
        KbdStringIn              SUB32.209, &
        KbdGetStatus             SUB32.210, &
        KbdSetStatus             SUB32.211, &
        KbdGetFocus              SUB32.212, &
        KbdFlushBuffer           SUB32.213, &
        KbdXlate                 SUB32.214, &
        KbdClose                 SUB32.217, &
        KbdFreeFocus             SUB32.218, &
        KbdDeRegister            SUB32.220, &
        KbdSetFgnd               SUB32.221, &
        KbdPeek                  SUB32.222, &
        KbdOpen                  SUB32.223, &
        KbdGetHWID               SUB32.224, &
        KbdSetHWID               SUB32.225, &
                                            &
        Mou32GetPtrShape           SUB32.301, &
        Mou32SetPtrShape           SUB32.302, &
        Mou32GetNumMickeys         SUB32.303, &
        Mou32GetScaleFact          SUB32.306, &
        Mou32FlushQue              SUB32.307, &
        Mou32GetNumButtons         SUB32.308, &
        Mou32Close                 SUB32.309, &
        Mou32SetScaleFact          SUB32.311, &
        Mou32GetNumQueEl           SUB32.313, &
        Mou32DeRegister            SUB32.314, &
        Mou32GetEventMask          SUB32.315, &
        Mou32SetEventMask          SUB32.316, &
        Mou32Open                  SUB32.317, &
        Mou32RemovePtr             SUB32.318, &
        Mou32GetPtrPos             SUB32.319, &
        Mou32ReadEventQue          SUB32.320, &
        Mou32SetPtrPos             SUB32.321, &
        Mou32GetDevStatus          SUB32.322, &
        Mou32Synch                 SUB32.323, &
        Mou32Register              SUB32.324, &
        Mou32SetDevStatus          SUB32.325, &
        Mou32DrawPtr               SUB32.326, &
        Mou32InitReal              SUB32.327, &
        Mou32SetThreshold          SUB32.329, &
        Mou32GetThreshold          SUB32.330, &
                                            &
        Dos32MonWrite            SUB32.401, &
        Dos32MonRead             SUB32.402, &
        Dos32MonClose            SUB32.403, &
        Dos32MonOpen             SUB32.404, &
        Dos32MonReg              SUB32.405

EXPORTS = &
        DOSICREATETHREAD           .1, &
        DOS16CWAIT                 .2, &
        DOS16ENTERCRITSEC          .3, &
        DOSIEXECPGM                .4, &
        DOS16EXIT                  .5, &
        DOS16EXITCRITSEC           .6, &
        DOS16EXITLIST              .7, &
        DOS16GETINFOSEG            .8=DOSGETINFOSEG, &
        DOS16GETPRTY               .9, &
        DOS16KILLPROCESS           .10, &
        DOS16SETPRTY               .11, &
        DOS16PTRACE                .12, &
        DOS16HOLDSIGNAL            .13, &
        DOS16SETSIGHANDLER         .14, &
        DOS16FLAGPROCESS           .15, &
        DOS16MAKEPIPE              .16, &
        DOSISYSSEMCLEAR            .17, &
        DOSISEMREQUEST             .18, &
        DOSISYSSEMSET              .19, &
        DOS16SEMSETWAIT            .20, &
        DOSISEMWAIT                .21, &
        DOS16MUXSEMWAIT            .22, &
        DOS16CLOSESEM              .23=DOSCLOSE, &
        DOS16CREATESEM             .24, &
        DOS16OPENSEM               .25, &
        DOS16RESUMETHREAD          .26, &
        DOS16SUSPENDTHREAD         .27, &
        DOS16SETDATETIME           .28, &
        DOS16TIMERASYNC            .29, &
        DOS16TIMERSTART            .30, &
        DOS16TIMERSTOP             .31, &
        DOS16SLEEP                 .32, &
        DOS16GETDATETIME           .33, &
        DOS16ALLOCSEG              .34, &
        DOS16ALLOCSHRSEG           .35, &
        DOS16GETSHRSEG             .36, &
        DOS16GIVESEG               .37, &
        DOS16REALLOCSEG            .38, &
        DOS16FREESEG               .39, &
        DOS16ALLOCHUGE             .40, &
        DOS16GETHUGESHIFT          .41, &
        DOS16REALLOCHUGE           .42, &
        DOS16CREATECSALIAS         .43, &
        DOS16LOADMODULE            .44=DOSLOADMODULE, &
        DOS16GETPROCADDR           .45=DOSGETPROCADDR, &
        DOS16FREEMODULE            .46=DOSFREEMODULE, &
        DOS16GETMODHANDLE          .47, &
        DOS16GETMODNAME            .48, &
        DOS16GETMACHINEMODE        .49, &
        DOS16BEEP                  .50, &
        DOS16CLIACCESS             .51, &
        DOS16DEVCONFIG             .52, &
        DOS16DEVIOCTL              .53=DOSDEVIOCTL, &
        DOS16SGSWITCH              .54, &
        DOS16SGSWITCHME            .55, &
        DOS16BUFRESET              .56, &
        DOS16CHDIR                 .57, &
        DOS16CHGFILEPTR            .58, &
        DOS16CLOSE                 .59=DOSCLOSE, &
        DOS16DELETE                .60, &
        DOS16DUPHANDLE             .61, &
        DOS16FILELOCKS             .62, &
        DOS16FINDCLOSE             .63, &
        DOS16FINDFIRST             .64, &
        DOS16FINDNEXT              .65, &
        DOS16MKDIR                 .66, &
        DOS16MOVE                  .67, &
        DOS16NEWSIZE               .68, &
        DOS16PORTACCESS            .69, &
        DOS16OPEN                  .70=DOSOPEN, &
        DOS16QCURDIR               .71, &
        DOS16QCURDISK              .72, &
        DOS16QFHANDSTATE           .73, &
        DOS16QFILEINFO             .74, &
        DOS16QFILEMODE             .75, &
        DOS16QFSINFO               .76, &
        DOS16QHANDTYPE             .77, &
        DOS16QVERIFY               .78, &
        DOSIREAD                   .79, &
        DOS16RMDIR                 .80, &
        DOS16SELECTDISK            .81, &
        DOS16SETFHANDSTATE         .82, &
        DOS16SETFILEINFO           .83, &
        DOS16SETFILEMODE           .84, &
        DOS16SETMAXFH              .85, &
        DOS16SETVERIFY             .86, &
        DOSIWRITE                  .87, &
        DOS16SYSTEMSERVICE         .88, &
        DOS16SETVEC                .89, &
        DOS16SYSTRACE              .90, &
        DOS16GETENV                .91, &
        DOS16GETVERSION            .92, &
        DOS16QTRACEINFO            .93, &
        DOS16GETPID                .94, &
        DOS16OPEN2                 .95, &
        DOS16LIBINIT               .96, &
        DOS16SETFSINFO             .97, &
        DOS16QPATHINFO             .98, &
        DOS16DEVIOCTL2             .99, &
        DOSICANONICALIZE           .100, &
        DOS16SETFGND               .101, &
        DOS16SWAPTASKINIT          .102, &
        DOS16READPHYS              .103, &
        DOS16SETPATHINFO           .104, &
        DOS16SGSWITCHPROC2         .105, &
        STRUCHECK                  .106, &
        STRURESUPDATE              .107, &
        DOSISETRELMAXFH            .108, &
        DOSIDEVIOCTL               .109, &
        DosForceDelete             .110, &
        DosKillThread              .111, &
        DosQueryRASInfo            .112, &
        DosDumpProcess             .113, &
        DosSuppressPopUps          .114, &
        THK32ALLOCMEM              .115, &
        THK32FREEMEM               .116, &
        THK32ALLOCSTACK            .117, &
        DosOpen2Compt              .118, &
        DOS16GETSTDA               .119, &
        DOS16ERROR                 .120, &
        DOS16GETSEG                .121, &
        DOS16LOCKSEG               .122, &
        DOS16UNLOCKSEG             .123, &
        DOS16SGSWITCHPROC          .124, &
        DOSIRAMSEMWAKE             .125, &
        DOS16SIZESEG               .126, &
        DOS16MEMAVAIL              .127, &
        DOSIRAMSEMREQUEST          .128, &
        DOS16PHYSICALDISK          .129, &
        DOS16GETCP                 .130, &
        DOSISETCP                  .131, &
        DOS16GLOBALSEG             .132, &
        DOS16PROFILE               .133, &
        DOS16SENDSIGNAL            .134, &
        DOS16HUGESHIFT             .135, &
        DOS16HUGEINCR              .136, &
        DOS16READ                  .137, &
        DOS16WRITE                 .138, &
        DOS16ERRCLASS              .139, &
        DOS16SEMREQUEST            .140, &
        DOS16SEMCLEAR              .141, &
        DOS16SEMWAIT               .142, &
        DOS16SEMSET                .143, &
        DOS16EXECPGM               .144, &
        DOS16CREATETHREAD          .145, &
        DOS16SUBSET                .146, &
        DOS16SUBALLOC              .147, &
        DOS16SUBFREE               .148, &
        DOS16READASYNC             .149, &
        DOS16WRITEASYNC            .150, &
        DOS16SEARCHPATH            .151, &
        DOS16SCANENV               .152, &
        DOS16SETCP                 .153, &
        DOS16QPROCSTATUS           .154, &
        DOS16GETRESOURCE           .155, &
        DOS16GETPPID               .156, &
        DOS16CALLBACK              .157, &
        DOSICALLBACK               .158, &
        DOS16RETFORWARD            .159, &
        DOS16R2STACKREALLOC        .160, &
        DOS16FSRAMSEMREQUEST       .161, &
        DOS16FSRAMSEMCLEAR         .162, &
        DOS16QAPPTYPE              .163, &
        DOS16SETPROCCP             .164, &
        DOS16DYNAMICTRACE          .165, &
        DOS16QSYSINFO              .166, &
        DOSIMAKENMPIPE             .167, &
        DOSICALLNMPIPE             .168, &
        DOSICONNECTNMPIPE          .169, &
        DOSIDISCONNECTNMPIPE       .170, &
        DOSIPEEKNMPIPE             .171, &
        DOSIQNMPIPEINFO            .172, &
        DOSIQNMPHANDSTATE          .173, &
        DOSISETNMPHANDSTATE        .174, &
        DOSITRANSACTNMPIPE         .175, &
        DOSIWAITNMPIPE             .176, &
        DOSISETNMPIPESEM           .177, &
        DOSIQNMPIPESEMSTATE        .178, &
        DOSIRAWREADNMPIPE          .179, &
        DOSIRAWWRITENMPIPE         .180, &
        DOS16FSATTACH              .181, &
        DOS16QFSATTACH             .182, &
        DOS16FSCTL                 .183, &
        DOS16FINDFIRST2            .184, &
        DOS16MKDIR2                .185, &
        DOS16FILEIO                .186, &
        DOS16FINDNOTIFYCLOSE       .187, &
        DOS16FINDNOTIFYFIRST       .188, &
        DOS16FINDNOTIFYNEXT        .189, &
        DOS16SETTRACEINFO          .190, &
        DOS16EDITNAME              .191, &
        DOS16LOGMODE               .192, &
        DOS16LOGENTRY              .193, &
        DOS16GETLOGBUFFER          .194, &
        DOS16LOGREGISTER           .195, &
        DOS16LOGREAD               .196, &
        DOS16FINDFROMNAME          .197, &
        DOS16OPLOCKRELEASE         .198, &
        DOS16OPLOCKWAIT            .199, &
        DOSICOPY                   .200, &
        DOS16COPY                  .201, &
        DOSIQAPPTYPE               .202, &
        DOS16FORCEDELETE           .203, &
        DOS16ENUMATTRIBUTE         .204, &
        DOS16OPLOCKSHUTDOWN        .205, &
        DOS16SHUTDOWN              .206, &
        DOS16GETRESOURCE2          .207, &
        DOS16FREERESOURCE          .208, &
        DosSetMaxFH                .209, &
        DosSetVerify               .210, &
        DosErrClass                .211, &
        DosError                   .212, &
        DosCreateVDM               .213, &
# Max Path len value
        DOS16MAXPATHLEN            .214, &
# Page size value
        DOS16PAGESIZE              .215, &
# Local Infoseg selector
        DOS16LOCALINFO             .216, &
# Global Infoseg selector
        DOS16GLOBALINFO            .217, &
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
# pipes 32-bit API
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
#
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
        DOS16OPENVDD               .266, &
        DOS16REQUESTVDD            .267, &
        DOS16CLOSEVDD              .268, &
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
        THK32ALLOCBLOCK            .293, &
        THK32FREEBLOCK             .294, &
        THK32R3DS                  .295, &
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
        DOS16ALLOCPROTSEG          .313, &
        DOS16ALLOCSHRPROTSEG       .314, &
        DOS16ALLOCPROTHUGE         .315, &
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
        _DosFPDataArea             .361, &
        DosTmrQueryFreq            .362, &
        DosTmrQueryTime            .363, &
        DosAliasPerfCtrs           .364, &
        DosConfigPerf              .365, &
        DosDeconPerf               .366, &
        DosRegisterPerfCtrs        .367, &
        DosQuerySysState           .368, &
        _DosFlatCS                 .369, &
        _DosFlatDS                 .370, &
        DosQueryABIOSSupport       .371, &
        DosEnumAttribute           .372, &
        DosQueryDOSProperty        .373, &
        DosSetDOSProperty          .374, &
        DOS16QUERYDOSPROPERTY      .375, &
        DOS16SETDOSPROPERTY        .376, &
        DosProfile                 .377, &
        DosSetSignalExceptionFocus .378, &
        DosSendSignalException     .379, &
        DosEnterMustComplete       .380, &
        DosExitMustComplete        .381, &
        DosSetRelMaxFH             .382, &
# msg
        DOS16PUTMESSAGE            .383, &
        DOS16TRUEGETMESSAGE        .384, &
        DOS16INSMESSAGE            .385, &
        DosInsertMessage           .386, &
        DosPutMessage              .387, &
        DosTrueGetMessage          .388, &
        DOS16IQUERYMESSAGECP       .389, &
        DosIQueryMessageCP         .390, &
# nls
        DOS16CASEMAP               .391, &
        DOS16GETCOLLATE            .392, &
        DOS16GETCTRYINFO           .393, &
        DOS16GETDBCSEV             .394, &
        DosQueryCtryInfo           .395, &
        DosQueryDBCSEnv            .396, &
        DosMapCase                 .397, &
        DosQueryCollate            .398, &
# nampipes
        DOS16MAKENMPIPE            .399, &
        DOS16QNMPIPEINFO           .400, &
        DOS16CONNECTNMPIPE         .401, &
        DOS16DISCONNECTNMPIPE      .402, &
        DOS16QNMPHANDSTATE         .403, &
        DOS16SETNMPHANDSTATE       .404, &
        DOS16PEEKNMPIPE            .405, &
        DOS16WAITNMPIPE            .406, &
        DOS16TRANSACTNMPIPE        .407, &
        DOS16CALLNMPIPE            .408, &
        DOS16RAWREADNMPIPE         .409, &
        DOS16RAWWRITENMPIPE        .410, &
        DOS16SETNMPIPESEM          .411, &
        DOS16QNMPIPESEMSTATE       .412, &
# startlw
        StartLazyWriter            .413, &
# some data ???
        _QueInst_Data              .414, &
#
        DosShutdown                .415, &
        DOSICACHEMODULE            .416, &
        DosReplaceModule           .417, &
        DosAcknowledgeSignalException .418, &
        _DosTIB                    .419, &
        DOS16TMRQUERYFREQ          .420, &
        DOS16TMRQUERYTIME          .421, &
        DOS16REGISTERPERFCTRS      .422, &
        DOS16FLATTOSEL             .423, &
        DOS16SELTOFLAT             .424, &
        DosFlatToSel               .425, &
        DosSelToFlat               .426, &
        _DosIODelayCnt             .427, &
        DosSetFileLocks            .428, &
        DosCancelLockRequest       .429, &
        LogOpen                    .430, &
        LogClose                   .431, &
        LogAddEntries              .432, &
        LogGetEntries              .433, &
        LogSetState                .434, &
        LogSetName                 .435, &
        LogQueryState              .436, &
        DOS16OPENCHANGENOTIFY      .437, &
        DOS16RESETCHANGENOTIFY     .438, &
        DOS16CLOSECHANGENOTIFY     .439, &
        DosOpenChangeNotify        .440, &
        DosResetChangeNotify       .441, &
        DosCloseChangeNotify       .442, &
        DOS16QUERYABIOSSUPPORT     .443, &
        DosForceSystemDump         .444, &
                                         &
        Dos32GetProcessorStatus    .447, &
        Dos32SetProcessorStatus    .448, &
        DOS16CREATESPINLOCK        .449, &
        DOS16ACQUIRESPINLOCK       .450, &
        DOS16RELEASESPINLOCK       .451, &
        DOS16FREESPINLOCK          .452, &
        Dos32TestPSD               .453, &
        DosAllocThreadLocalMemory  .454, &
        DosFreeThreadLocalMemory   .455, &
                                         &
        DosVerifyPidTid            .460, &
                                         &
# not valid after HPFS & LS init:
        _PTDA_LANMAN_SEC           .464, &
        _PTDA_PID                  .465, &
        _SAS_SEL                   .466, &
        _TCB_OPCOOKIE              .467, &
        _TCB_OPFLAGS               .468, &
        _TCB_NEWFLAGS              .469, &
        _TCB_USER_ID               .470, &
        _TCB_PROC_ID               .471, &
        _TCB_FSHARING              .472, &
        _TCB_SRVATTRIB             .473, &
        _TCB_ALLOWED               .474, &
        _TCB_PRTCB                 .475, &
        _TCB_NUMBER                .476, &
        _TCB_THISSFT               .477, &
        _TCB_THISCDS               .478, &
        _TKOPTDA                   .479, &
        _PTDA_CRITSEC              .480, &
        _PTDA_HOLDSIGCNT           .481, &
        _PTDA_PPTDAPARENT          .482, &
        _PTDA_PGDATA               .483, &
        _PTDA_HANDLE               .484, &
        _PTDA_MODULE               .485, &
        _PTDA_LDTHANDLE            .486, &
        _PTDA_CODEPAGE_TAG         .487, &
        _PTDA_JFN_LENGTH           .488, &
        _PTDA_JFN_PTABLE           .489, &
        _PTDA_JFN_FLG_PTR          .490, &
        _PTDA_EXTERR_LOCUS         .491, &
        _PTDA_EXTERR               .492, &
        _PTDA_EXTERR_ACTION        .493, &
        _PTDA_EXTERR_CLASS         .494, &
        _PTDA_PPID                 .495, &
        _PTDA_PROCTYPE             .496, &
        _PTDA_CURRTCB              .497, &
        _PTDA_CURRTSD              .498, &
        _PTDA_SIGNATURE            .499, &
                                         &
# hole
                                         &
        DOSIOPENL                  .541, &
        DOSIPROTECTOPENL           .542, &
        DOSISETFILESIZEL           .543, &
        DOSIPROTECTSETFILESIZEL    .544, &
        DosExceptionCallBack       .545, &
        THK32STRLEN                .546, &
        THK32_UNITHUNK             .547, &
        DosR3ExceptionDispatcher   .548, &
        DosLibiDisp                .549, &
        DosLibiDisp16              .550, &
        DosLibiDisp32              .551, &
        DOS16R3EXITADDR            .552, &
        Dos32R3ExitAddr            .553, &
        Dos32IRead                 .554, &
                                         &
        Dos32IWrite                .556, &
        DosCreateSpinLock          .557, &
        DosAcquireSpinLock         .558, &
        DosReleaseSpinLock         .559, &
        DosFreeSpinLock            .560, &
        Dos32PMR3WaitEventSem      .561, &
        Dos32PMR3PostEventSem      .562, &
        DosQueryThreadAffinity     .563, &
        DosSetThreadAffinity       .564, &
        DOSISETFILEINFO            .565, &
        DOSISETPATHINFO            .566, &
                                         &
        DOSIFINDNEXT               .569, &
                                         &
# ???        Dos16SetFileSizeL            .570
                                         &
        DosQueryResourceSize       .572, &
        DOS16QUERYRESOURCESIZE     .573, &
        DosPMSemWait               .574, &
        DosPMMuxSemWait            .575, &
        THK16_UNITHUNK             .576, &
        HT16_STARTUP               .577, &
        HT16_CLEANUP               .578, &
        HT32_STARTUP               .579, &
        DosInitializePorthole      .580, &
        HT32_CLEANUP               .581, &
        DosQueryHeaderInfo         .582, &
        DOS16INITIALIZEPORTHOLE    .583, &
        DOS16QUERYHEADERINFO       .584, &
        Mon32MonRead               .585, &
        DosQueryProcType           .586, &
        DOS16QUERYPROCTYPE         .587, &
        Mon32MonWrite              .588, &
        DOSISIGDISPATCH            .589, &
        Dos32PMPostEventSem        .590, &
        Dos32PMWaitEventSem        .591, &
        DosDllTermDisp             .592, &
        Dos32PMRequestMutexSem     .593, &
        DOSIRAISEEXCEPTION         .594, &
        Dos32PMWaitMuxWaitSem      .595, &
        DOSPM16SEMCHK              .596, &
        DOSIQUERYFHSTATE           .597, &
        DOSISETFHSTATE             .598, &
        DosLDTSel                  .599, &
        DosR3Frestor               .600, &
        DOSIFINDFIRST              .601, &
                                         &
# moncalls (???: no such ordinals imported by moncalls from eCS 2.2 beta II)
        DOS16MONOPEN               .606, &
        DOS16MONCLOSE              .607, &
        DOS16MONREAD               .608, &
        DOS16MONWRITE              .609, &
        DOS16MONREG                .610, &
                                         &
        Dos32IProtectWrite         .615, &
                                         &
        DOSIPROTECTSETFILEINFO     .617, &
        Dos32IProtectSetFileInfo   .618, &
        Dos32IProtectSetFHState    .619, &
        Dos32IProtectQueryFHState  .620, &
        DosProtectSetFilePtr       .621, &
        DOS16PROTECTCLOSE          .622, &
        DOS16PROTECTFILEIO         .623, &
        DOS16PROTECTFILELOCKS      .624, &
        DOSIPROTECTREAD            .625, &
        DOSIPROTECTWRITE           .626, &
        DOS16PROTECTNEWSIZE        .627, &
        DOS16PROTECTOPEN           .628, &
        DOS16PROTECTQFHANDSTATE    .629, &
        DOS16PROTECTSETFHANDSTATE  .630, &
        DOS16PROTECTQFILEINFO      .631, &
        DOS16PROTECTSETFILEINFO    .632, &
                                         &
        DOS16PROTECTCHGFILEPTR     .634, &
        DOS16PROTECTENUMATTRIBUTE  .635, &
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
        DOS16LDRDIRTYWORKER        .651, &
        Dos32IGetInfoBlocks        .652, &
#
        T32IGETINFOBLOCKS          .653, &
                                         &
        T32GETPROCESSORSTATUS      .656, &
        T32SETPROCESSORSTATUS      .657, &
        T32TESTPSD                 .658, &
        T32QUERYTHREADAFFINITY     .659, &
        T32SETTHREADAFFINITY       .660, &
# quecalls 32
        DosReadQueue               .661, &
        DosPurgeQueue              .662, &
        DosCloseQueue              .663, &
        DosQueryQueue              .664, &
        DosPeekQueue               .665, &
        DosWriteQueue              .666, &
        DosOpenQueue               .667, &
        DosCreateQueue             .668, &
# sesmgr 32
        DosStartSession            .669, &
        DosSelectSession           .670, &
        DosSetSession              .671, &
        DosStopSession             .672, &
        DosRegisterNotification    .673, &
# quecalls 16
        DOS16READQUEUE             .674, &
        DOS16PURGEQUEUE            .675, &
        DOS16CLOSEQUEUE            .676, &
        DOS16QUERYQUEUE            .677, &
        DOS16PEEKQUEUE             .678, &
        DOS16WRITEQUEUE            .679, &
        DOS16OPENQUEUE             .680, &
        DOS16CREATEQUEUE           .681, &
# os2char 16
        DOS16SMGETMEM              .682, &
        DOS16SMFREEMEM             .683, &
        DOS16SMGETSGCB             .684, &
        DOS16SMINITSGCB            .685, &
# sesmgr 16
        DOS16SMSGDOPOPUP           .686, &
        DOS16SMSWITCH              .687, &
        DOS16SMSERVEAPPREQ         .688, &
        DOS16GETTIMES              .689, &
        DOS16SMSETTITLE            .690, &
        DOS16SCRUNLOCK             .691, &
        DOS16SMDOAPPREQ            .692, &
        DOS16STOPSESSION           .693, &
        DOS16SELECTSESSION         .694, &
        DOS16SCRLOCK               .695, &
        DOS16SAVREDRAWWAIT         .696, &
        DOS16SAVREDRAWUNDO         .697, &
        DOS16SMSGENDPOPUP          .698, &
        DOS16SETSESSION            .699, &
        DOS16SETMNLOCKTIME         .700, &
        DOS16MODEUNDO              .701, &
        DOS16STARTSESSION          .702, &
        DOS16SMGETSTATUS           .703, &
        DOS16MODEWAIT              .704, &
        DOS16SMTERMINATE           .705, &
        DOS16SMGETAPPREQ           .706, &
        DOS16SMINITIALIZE          .707, &
        DOS16SMSTART               .708, &
        DOS16SMPARENTSWITCH        .709, &
        DOS16SMPAUSE               .710, &
        DOS16SMHDEINIT             .711, &
        DOS16SMPMPRESENT           .712, &
        DOS16SMREGISTERDD          .713, &
        DOS16SMNOTIFYDD            .714, &
        DOS16SMNOTIFYDD2           .715, &
        DOS16SMOPENDD              .716, &
        DOS16SMSETSESSIONTYPE      .717, &
# os2char
        OS2BaseInit                .718, &
# moucalls
        MOUGETPTRSHAPE             .719, &
        MOUSETPTRSHAPE             .720, &
        MOUGETNUMMICKEYS           .721, &
        MOUGETTHRESHOLD            .722, &
        MOUSHELLINIT               .723, &
        MOUGETSCALEFACT            .724, &
        MOUFLUSHQUE                .725, &
        MOUGETNUMBUTTONS           .726, &
        MOUCLOSE                   .727, &
        MOUSETTHRESHOLD            .728, &
        MOUSETSCALEFACT            .729, &
        MOUGETNUMQUEEL             .730, &
        MOUDEREGISTER              .731, &
        MOUGETEVENTMASK            .732, &
        MOUSETEVENTMASK            .733, &
        MOUOPEN                    .734, &
        MOUREMOVEPTR               .735, &
        MOUGETPTRPOS               .736, &
        MOUREADEVENTQUE            .737, &
        MOUSETPTRPOS               .738, &
        MOUGETDEVSTATUS            .739, &
        MOUSYNCH                   .740, &
        MOUREGISTER                .741, &
        MOUSETDEVSTATUS            .742, &
        MOUDRAWPTR                 .743, &
        MOUINITREAL                .744, &
# kbdcalls
        KBDSETCUSTXT               .745, &
        KBDINIT                    .746, &
        KBDGETCP                   .747, &
        KBDCHARIN                  .748, &
        KBDSETCP                   .749, &
        KBDLOADINSTANCE            .750, &
        KBDSYNCH                   .751, &
        KBDREGISTER                .752, &
        KBDSTRINGIN                .753, &
        KBDGETSTATUS               .754, &
        KBDSETSTATUS               .755, &
        KBDGETFOCUS                .756, &
        KBDFLUSHBUFFER             .757, &
        KBDXLATE                   .758, &
        KBDSWITCHFGND              .759, &
        KBDSHELLINIT               .760, &
        KBDCLOSE                   .761, &
        KBDFREEFOCUS               .762, &
        KBDFREE                    .763, &
        KBDDEREGISTER              .764, &
        KBDSETFGND                 .765, &
        KBDPEEK                    .766, &
        KBDOPEN                    .767, &
        KBDGETHWID                 .768, &
        KBDSETHWID                 .769, &
# viocalls
#        VIOENDPOPUP                .770, &
#        VIOGETPHYSBUF              .771, &
#        VIOGETANSI                 .772, &
#        VIOFREE                    .773, &
#        VIOSETANSI                 .774, &
#        VIODEREGISTER              .775, &
#        VIOSCROLLUP                .776, &
#        VIOPRTSC                   .777, &
#        VIOGETCURPOS               .778, &
#        VIOWRTCELLSTR              .779, &
#        VIOPOPUP                   .780, &
#        VIOSCROLLRT                .781, &
#        VIOWRTCHARSTR              .782, &
#        AVS_PRTSC                  .783, &
#        VIOSETCURPOS               .784, &
#        VIOSRFUNBLOCK              .785, &
#        VIOSRFBLOCK                .786, &
#        VIOSCRUNLOCK               .787, &
        VIOWRTTTY                  .788, &
#        VIOSAVE                    .789, &
#        VIOGETMODE                 .790, &
#        VIOSETMODE                 .791, &
#        VIOSCRLOCK                 .792, &
#        VIOREADCELLSTR             .793, &
#        VIOSAVREDRAWWAIT           .794, &
#        VIOWRTNATTR                .795, &
#        VIOGETCURTYPE              .796, &
#        VIOSAVREDRAWUNDO           .797, &
#        VIOGETFONT                 .798, &
#        VIOREADCHARSTR             .799, &
#        VIOGETBUF                  .800, &
#        VIOSETCURTYPE              .801, &
#        VIOSETFONT                 .802, &
#        VIOHETINIT                 .803, &
#        VIOMODEUNDO                .804, &
#        VIOSSWSWITCH               .805, &
#        VIOMODEWAIT                .806, &
#        AVS_PRTSCTOGGLE            .807, &
#        VIOGETCP                   .808, &
#        VIORESTORE                 .809, &
#        VIOSETCP                   .810, &
#        VIOSHOWBUF                 .811, &
#        VIOSCROLLLF                .812, &
#        VIOREGISTER                .813, &
#        VIOGETCONFIG               .814, &
#        VIOSCROLLDN                .815, &
#        VIOWRTCHARSTRATT           .816, &
#        VIOGETSTATE                .817, &
#        VIOPRTSCTOGGLE             .818, &
#        VIOSETSTATE                .819, &
#        VIOWRTNCELL                .820, &
#        VIOWRTNCHAR                .821, &
#        VIOSHELLINIT               .822, &
#        VIOASSOCIATE               .823, &
#        VIOCREATEPS                .824, &
#        VIODELETESETID             .825, &
#        VIOGETDEVICECELLSIZE       .826, &
#        VIOGETORG                  .827, &
#        VIOCREATELOGFONT           .828, &
#        VIODESTROYPS               .829, &
#        VIOQUERYSETIDS             .830, &
#        VIOSETORG                  .831, &
#        VIOQUERYFONTS              .832, &
#        VIOSETDEVICECELLSIZE       .833, &
#        VIOSHOWPS                  .834, &
#        VIOGETPSADDRESS            .835, &
#        VIOQUERYCONSOLE            .836, &
#        VIOREDRAWSIZE              .837, &
#        VIOGLOBALREG               .838, &
#        XVIOSETCASTATE             .839, &
#        XVIOCHECKCHARTYPE          .840, &
#        XVIODESTROYCA              .841, &
#        XVIOCREATECA               .842, &
#        VIOCHECKCHARTYPE           .843, &
#        XVIOGETCASTATE             .844, &
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
        MOUGETHOTKEY               .853, &
        MOUSETHOTKEY               .854, &
# sesmgr
        DOS16SMSYSINIT             .855, &
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
        _BVHINSTANCE                .865, &
#
        THK32ALIASMEM              .866, &
        THK32FREEALIAS             .867, &
        THK32ALLOCVARLEN           .868, &
        THK32HANDLEBOUNDARY        .869, &
        THK32HANDLESTRING          .870, &
        THK32DEALLOC               .871, &
        THK32XHNDLR                .872, &
#
        DosSetExtLIBPATH           .873, &
        DosQueryExtLIBPATH         .874, &
        DOSPM16SEMRST              .875, &
        DosSysCtl                  .876, &
        DosQueryThreadContext      .877, &
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
        DOS16FSRAMSEMREQUEST2      .991, &
        DOS16FSRAMSEMCLEAR2        .992, &
                                         &
# Needed for Java
        DosSystemService           .995, &
#
        DosListIO                  .996, &
        DosCreateThread2           .997, &
        DOS16SETEXTLIBPATH         .998, &
        DOS16QUERYEXTLIBPATH       .999, &
#
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

!include $(%ROOT)tools/mk/appsos2_cmd.mk

.c: $(MYDIR)legacy
.asm: $(ROOT)shared$(SEP)mou
.asm: $(ROOT)shared$(SEP)kbd
.asm: $(ROOT)shared$(SEP)bms
.asm: $(ROOT)shared$(SEP)bks
.asm: $(PATH)

$(PATH)nlsmsg.obj: nlsmsg.asm $(PATH)msg.inc

$(PATH)msg.inc: $(PATH)$(PROJ).txt
  $(verbose)$(SAY) MKMSGF   $^.
  $(verbose)$(MC) -Q -A -I$(MYDIR)$(PS)$(%OS2TK)$(SEP)inc $[@ $^@

$(PATH)$(PROJ).txt: $(PATH)$(PROJ).ext
  $(verbose)$(SAY) MSGEXTRT $^.
  $(verbose)$(ME) $< $^@

#$(PATH)$(PROJ).ext: $(PROJ)msg.asm
#  msgprof $(PROJ).asm > $(PROJ).ext

$(PATH)$(PROJ).ext: .symbolic
 @%create $^@
 @%append $^@ >$(PROJ).asm
 @%append $^@ <$(%ROOT)build$(SEP)bin$(SEP)os2$(SEP)system$(SEP)oso001.msg
 @%append $^@ DOS0317
 @%append $^@ DOS0318
 @%append $^@ DOS0319
 @%append $^@ DOS0320
 @%append $^@ DOS0321

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


# <_dixie_> lys 64-bit file size is available only via (another) 32-bit open function ...
# <_dixie_> moveton да, 16-битного DosOpenL походу нету :)
# <moveton> _dixie_ исчо чего не хватало. Хватит того, что DosOpenL с HMA не совместим. Тоже, можно сказать, ода 16 бит
# <_dixie_> moveton ЫЫ :) DOSIOPENL       proc near
# <moveton> _dixie_ а так-то Dos32OpenL - это враппер на DOS32POPENL, а он цанк на 16 бит DosIOpenL. Т.е. позвать это всё из 16 бит можно и напрямую, но всё таки оно internal
# <_dixie_> moveton ну, 541 ORD_DOSIOPENL - походу таки функция... 
# <moveton> _dixie_ тут в фениксе рефакторили. Есть камент "DosIOpenL (567) теперь экспортируется из ядра". Уже не скажу, как на бимерском. Но в любом случае, это internal. Т.е. позвать можно, но типа приложение такое будет очень еретическим. Как я виду использование, бимеры это вытащили, чтобы 16 бит CopyFile работал и с большими файлами

/* SCCSID = %w% %e% */
/**************************************************************************
 *
 * SOURCE FILE NAME =  DHCALLS.H
 *
 * DESCRIPTIVE NAME =  ADD/DM include file
 *                     C Function Prototypes services in DHCALLS.LIB
 *
 * Copyright : COPYRIGHT IBM CORPORATION, 1991, 1992
 *             LICENSED MATERIAL - PROGRAM PROPERTY OF IBM
 *             REFER TO COPYRIGHT INSTRUCTION FORM#G120-2083
 *             RESTRICTED MATERIALS OF IBM
 *             IBM CONFIDENTIAL
 *
 * VERSION = V2.0
 *
 * DATE
 *
 * DESCRIPTION :
 *
 * Purpose:    DHCALLS.LIB provides a C-Callable interface to OS/2
 *             Device Help (DevHelp) services.
 *
 *             DevHelp services provide memory/interrupt/semaphore
 *             management services for ADDs and Device Managers.
 *
 *
 * FUNCTIONS  :
 *
 *
 *
 * NOTES
 *
 *
 * STRUCTURES
 *
 * EXTERNAL REFERENCES
 *
 *
 *
 * EXTERNAL FUNCTIONS
 *
 * CHANGE ACTIVITY =
 *   DATE      FLAG        APAR   CHANGE DESCRIPTION
 *   --------  ----------  -----  --------------------------------------
 *   10/22/93  @V74979     74979  Change DevHelp return codes from BOOL
 *                                to USHORT
 *
 *   10/22/93  @V74979     74979  Fix prototype DevHelp_DeRegister
 *                                              DevHelp_VMSetMem
 *                                              DevHelp_Beep
 *                                              DevHelp_RegisterBeep
 *
 *   10/22/93  @V74979     74979  Add DEVICECLASS_* equates for
 *                                DevHelp_RegisterDeviceClass.
 *
 *   12/02/93  @V76282     76282  Add defines for flags on various
 *                                DevHelps based on review of PDD Book
 *
 *   12/02/93  @V76282     76282  Fix prototype DevHelp_MonWrite
 *                                              DevHelp_SchedClock
 *
 *   12/02/93  @V76282     76282  Add prototype DevHelp_RegisterTmrDD
 *                                              DevHelp_DynamicAPI
 *
 *   05/31/94  @V85056     85056  Add DDTABLE type for DevHelp_AttachDD
 *
 *   02/06/95  @V111573   111573  Add prototype DevHelp_LogEntry
 *
 ****************************************************************************/


/*
** DevHelp Library Calls
*/

typedef USHORT NEAR *NPUSHORT;
typedef VOID   NEAR *NPVOID;


/*
** ABIOS Related
*/

USHORT APIENTRY DevHelp_GetLIDEntry ( USHORT  DeviceType,
                                      USHORT  LIDIndex,
                                      USHORT  LIDType,
                                      PUSHORT LID);

USHORT APIENTRY DevHelp_FreeLIDEntry( USHORT LIDNumber );

USHORT APIENTRY DevHelp_ABIOSCall( USHORT Lid,
                                   NPBYTE ReqBlk,
                                   USHORT Entry_Type );

USHORT APIENTRY DevHelp_ABIOSCommonEntry( NPBYTE ReqBlk,
                                          USHORT Entry_Type );

USHORT APIENTRY DevHelp_ABIOSGetParms( USHORT Lid,
                                       NPBYTE ParmsBlk);

USHORT APIENTRY DevHelp_GetDeviceBlock( USHORT Lid,
                                        PPVOID DeviceBlockPtr );


/*
** Memory Management
*/

USHORT APIENTRY DevHelp_AllocGDTSelector( PSEL   Selectors,
                                          USHORT Count );

USHORT APIENTRY DevHelp_PhysToGDTSelector( ULONG  PhysAddr,
                                           USHORT Count,
                                           SEL    Selector );

USHORT APIENTRY DevHelp_PhysToGDTSel( ULONG PhysAddr,
                                      ULONG Count,
                                      SEL   Selector,
                                      UCHAR Access );

USHORT APIENTRY DevHelp_AllocPhys( ULONG  lSize,
                                   USHORT MemType,
                                   PULONG PhysAddr);

USHORT APIENTRY DevHelp_PhysToUVirt( ULONG  PhysAddr,
                                     USHORT Length,
                                     USHORT Flags,
                                     USHORT TagType,
                                     PVOID  SelOffset);

USHORT APIENTRY DevHelp_PhysToVirt( ULONG   PhysAddr,
                                    USHORT  usLength,
                                    PVOID   SelOffset,
                                    PUSHORT ModeFlag );

USHORT APIENTRY DevHelp_UnPhysToVirt( PUSHORT ModeFlag );

USHORT APIENTRY DevHelp_FreePhys( ULONG PhysAddr );

USHORT APIENTRY DevHelp_VirtToPhys( PVOID  SelOffset,
                                    PULONG PhysAddr );

USHORT APIENTRY DevHelp_Lock( SEL    Segment,
                              USHORT LockType,
                              USHORT WaitFlag,
                              PULONG LockHandle );

USHORT APIENTRY DevHelp_UnLock( ULONG LockHandle );

USHORT APIENTRY DevHelp_VerifyAccess( SEL    MemSelector,
                                      USHORT Length,
                                      USHORT MemOffset,
                                      UCHAR  AccessFlag);

/* DevHelp_AllocPhys */
#define MEMTYPE_ABOVE_1M 0
#define MEMTYPE_BELOW_1M 1

/* DevHelp_Lock */

#define LOCKTYPE_SHORT_ANYMEM 0x00
#define LOCKTYPE_LONG_ANYMEM  0x01
#define LOCKTYPE_LONG_HIGHMEM 0x03
#define LOCKTYPE_SHORT_VERIFY 0x04

/* DevHelp_PhysToUVirt */

#define SELTYPE_R3CODE  0
#define SELTYPE_R3DATA  1
#define SELTYPE_FREE    2
#define SELTYPE_R2CODE  3
#define SELTYPE_R2DATA  4
#define SELTYPE_R3VIDEO 5


/* DevHelp_VerifyAccess */

#define VERIFY_READONLY    0
#define VERIFY_READWRITE   1

/*
** Request Packet Management
*/

USHORT APIENTRY DevHelp_AllocReqPacket( USHORT    WaitFlag,
                                        PBYTE FAR *ReqPktAddr );

USHORT APIENTRY DevHelp_FreeReqPacket( PBYTE ReqPktAddr );

USHORT APIENTRY DevHelp_PullParticular( NPBYTE Queue,
                                        PBYTE  ReqPktAddr );

USHORT APIENTRY DevHelp_PullRequest( NPBYTE    Queue,
                                     PBYTE FAR *ReqPktAddr );

USHORT APIENTRY DevHelp_PushRequest( NPBYTE Queue,
                                     PBYTE  ReqPktAddr );

USHORT APIENTRY DevHelp_SortRequest( NPBYTE Queue,
                                     PBYTE  ReqPktAddr );


/* DevHelp_AllocRequestPacket */

#define WAIT_NOT_ALLOWED 0
#define WAIT_IS_ALLOWED  1

/*
** Character Queue Management
*/

USHORT APIENTRY DevHelp_QueueInit( NPBYTE Queue );

USHORT APIENTRY DevHelp_QueueRead( NPBYTE Queue,
                                   PBYTE  Char );

USHORT APIENTRY DevHelp_QueueWrite( NPBYTE Queue,
                                    UCHAR  Char );

USHORT APIENTRY DevHelp_QueueFlush( NPBYTE Queue );


/* DevHelp_QueueInit */

typedef struct _QUEUEHDR  {             /* DHQH */

  USHORT   QSize;
  USHORT   QChrOut;
  USHORT   QCount;
  BYTE     Queue[1];
} QUEUEHDR;

typedef QUEUEHDR FAR *PQUEUEHDR;                                     /*@V76282*/



/*
** Inter-Device Driver Communications & Kernel Functions
*/

USHORT APIENTRY DevHelp_ProtToReal();

USHORT APIENTRY DevHelp_RealToProt();

USHORT APIENTRY DevHelp_InternalError( PSZ    MsgText,
                                       USHORT MsgLength );

USHORT APIENTRY DevHelp_RAS( USHORT Major,
                             USHORT Minor,
                             USHORT Size,
                             PBYTE Data);

USHORT APIENTRY DevHelp_RegisterPerfCtrs( NPBYTE pDataBlock,
                                          NPBYTE pTextBlock,
                                          USHORT Flags);

USHORT APIENTRY DevHelp_AttachDD( NPSZ   DDName,
                                  NPBYTE IDCTable );

typedef struct _IDCTABLE {               /* DHID */                  /*@V85056*/
                                                                     /*@V85056*/
  USHORT        Reserved[3];                                         /*@V85056*/
  VOID          (FAR *ProtIDCEntry)(VOID);                           /*@V85056*/
  USHORT        ProtIDC_DS;                                          /*@V85056*/
                                                                     /*@V85056*/
} IDCTABLE;                                                          /*@V85056*/
                                                                     /*@V85056*/
typedef IDCTABLE NEAR *NPIDCTABLE;                                   /*@V85056*/


USHORT APIENTRY DevHelp_GetDOSVar( USHORT VarNumber,
                                   USHORT VarMember,
                                   PPVOID KernelVar );

#define DHGETDOSV_SYSINFOSEG            1
#define DHGETDOSV_LOCINFOSEG            2
#define DHGETDOSV_VECTORSDF             4
#define DHGETDOSV_VECTORREBOOT          5
#define DHGETDOSV_YIELDFLAG             7                            /*@V76282*/
#define DHGETDOSV_TCYIELDFLAG           8                            /*@V76282*/
#define DHGETDOSV_DOSCODEPAGE           11                           /*@V76282*/
#define DHGETDOSV_INTERRUPTLEV          13
#define DHGETDOSV_DEVICECLASSTABLE      14                           /*@V76282*/
#define DHGETDOSV_DMQSSELECTOR          15                           /*@V76282*/
#define DHGETDOSV_APMINFO               16                           /*@V76282*/

USHORT APIENTRY DevHelp_Save_Message( NPBYTE MsgTable );

typedef struct _MSGTABLE {              /* DHMT */

  USHORT   MsgId;                       /* Message Id #                  */
  USHORT   cMsgStrings;                 /* # of (%) substitution strings */
  PSZ      MsgStrings[1];               /* Substitution string pointers  */
} MSGTABLE;

typedef MSGTABLE NEAR *NPMSGTABLE;                                   /*@V76282*/


USHORT APIENTRY DevHelp_LogEntry (PVOID, USHORT);  /* @V111573 */


/*
** Interrupt/Thread Management
*/

USHORT APIENTRY DevHelp_RegisterStackUsage( PVOID StackUsageData );

USHORT APIENTRY DevHelp_SetIRQ( NPFN   IRQHandler,
                                USHORT IRQLevel,
                                USHORT SharedFlag );

USHORT APIENTRY DevHelp_UnSetIRQ( USHORT IRQLevel );

USHORT APIENTRY DevHelp_EOI( USHORT IRQLevel );

USHORT APIENTRY DevHelp_ProcBlock( ULONG  EventId,
                                   ULONG  WaitTime,
                                   USHORT IntWaitFlag );

USHORT APIENTRY DevHelp_ProcRun( ULONG   EventId,
                                 PUSHORT AwakeCount);

USHORT APIENTRY DevHelp_DevDone( PBYTE ReqPktAddr );

USHORT APIENTRY DevHelp_TCYield(void);

USHORT APIENTRY DevHelp_Yield(void);

USHORT APIENTRY DevHelp_VideoPause( USHORT OnOff );

/* DevHelp_RegisterStackUsage */

typedef struct _STACKUSAGEDATA  {       /* DHRS */

  USHORT  Size;
  USHORT  Flags;
  USHORT  IRQLevel;
  USHORT  CLIStack;
  USHORT  STIStack;
  USHORT  EOIStack;
  USHORT  NestingLevel;
} STACKUSAGEDATA;

/* DevHelp_Block */

#define WAIT_IS_INTERRUPTABLE      0
#define WAIT_IS_NOT_INTERRUPTABLE  1

#define WAIT_INTERRUPTED           0x8003
#define WAIT_TIMED_OUT             0x8001

/* DevHelp_VideoPause */

#define VIDEO_PAUSE_OFF            0
#define VIDEO_PAUSE_ON             1


/*
** Semaphore Management
*/

USHORT APIENTRY DevHelp_SemHandle( ULONG  SemKey,
                                   USHORT SemUseFlag,
                                   PULONG SemHandle );

USHORT APIENTRY DevHelp_SemClear( ULONG SemHandle );

USHORT APIENTRY DevHelp_SemRequest( ULONG SemHandle,
                                    ULONG SemTimeout );

USHORT APIENTRY DevHelp_SendEvent( USHORT EventType,
                                   USHORT Parm );

USHORT APIENTRY DevHelp_OpenEventSem( ULONG hEvent );

USHORT APIENTRY DevHelp_CloseEventSem( ULONG hEvent );

USHORT APIENTRY DevHelp_PostEventSem( ULONG hEvent );

USHORT APIENTRY DevHlp_ResetEventSem( ULONG hEvent,
                                      PULONG pNumPosts );

/* DevHelp_SemHandle */

#define SEMUSEFLAG_IN_USE       0
#define SEMUSEFLAG_NOT_IN_USE   1

/* DevHelp_SemHandle */

#define EVENT_MOUSEHOTKEY   0
#define EVENT_CTRLBREAK     1
#define EVENT_CTRLC         2
#define EVENT_CTRLNUMLOCK   3
#define EVENT_CTRLPRTSC     4
#define EVENT_SHIFTPRTSC    5
#define EVENT_KBDHOTKEY     6
#define EVENT_KBDREBOOT     7

/*
** Timer Management
*/

USHORT APIENTRY DevHelp_ResetTimer( NPFN TimerHandler );

USHORT APIENTRY DevHelp_SchedClock( PFN NEAR *SchedRoutineAddr );    /*@V76282*/

USHORT APIENTRY DevHelp_SetTimer( NPFN TimerHandler );

USHORT APIENTRY DevHelp_TickCount( NPFN   TimerHandler,
                                   USHORT TickCount );

USHORT APIENTRY DevHelp_RegisterTmrDD( NPFN   TimerEntry,            /*@V76282*/
                                       PULONG TmrRollover,           /*@V76282*/
                                       PULONG Tmr          );        /*@V76282*/



/*
** Real Mode Helpers
*/

USHORT APIENTRY DevHelp_ProtToReal();

USHORT APIENTRY DevHelp_RealToProt();

USHORT APIENTRY DevHelp_ROMCritSection( USHORT EnterExit);

USHORT APIENTRY DevHelp_SetROMVector( NPFN   IntHandler,
                                      USHORT INTNum,
                                      USHORT SaveDSLoc,
                                      PULONG LastHeader );



/*
** Monitors
*/

USHORT APIENTRY DevHelp_MonFlush( USHORT MonitorHandle );

USHORT APIENTRY DevHelp_Register( USHORT MonitorHandle,
                                  USHORT MonitorPID,
                                  PBYTE  InputBuffer,
                                  NPBYTE OutputBuffer,
                                  USHORT ChainFlag );

USHORT APIENTRY DevHelp_MonitorCreate( USHORT  MonitorHandle,
                                       PBYTE   FinalBuffer,
                                       NPFN    NotifyRtn,
                                       PUSHORT MonitorChainHandle);

USHORT APIENTRY DevHelp_DeRegister( USHORT  MonitorPID,              /*@V74979*/
                                    USHORT  MonitorHandle,           /*@V74979*/
                                    PUSHORT MonitorsLeft);

USHORT APIENTRY DevHelp_MonWrite( USHORT MonitorHandle,
                                  PBYTE  DataRecord,
                                  USHORT Count,
                                  ULONG  TimeStampMS,                /*@V76282*/
                                  USHORT WaitFlag );

/* DevHelp_Register */

#define CHAIN_AT_TOP    0
#define CHAIN_AT_BOTTOM 1



/*
** OS/2 2.x Only
**
** Note: These DeviceHelps are not available on OS/2 1.x systems
**
*/

/*
** 32-Bit Memory Management
*/

typedef ULONG   LIN;                /* 32-Bit Linear Addess              */
typedef ULONG   _far *PLIN;         /* 16:16 Ptr to 32-Bit Linear Addess */

USHORT APIENTRY DevHelp_VMLock( ULONG  Flags,
                                LIN    LinearAddr,
                                ULONG  Length,
                                LIN    pPagelist,
                                LIN    pLockHandle,
                                PULONG PageListCount );

USHORT APIENTRY DevHelp_VMUnLock( LIN pLockHandle );

USHORT APIENTRY DevHelp_VMAlloc( ULONG  Flags,
                                 ULONG  Size,
                                 ULONG  PhysAddr,
                                 PLIN   LinearAddr,
                                 PPVOID SelOffset );

USHORT APIENTRY DevHelp_VMFree( LIN LinearAddr );

USHORT APIENTRY DevHelp_VMProcessToGlobal( ULONG Flags,
                                           LIN   LinearAddr,
                                           ULONG Length,
                                           PLIN  GlobalLinearAddr );

USHORT APIENTRY DevHelp_VMGlobalToProcess( ULONG Flags,
                                           LIN   LinearAddr,
                                           ULONG Length,
                                           PLIN  ProcessLinearAddr );

USHORT APIENTRY DevHelp_VirtToLin( SEL   Selector,
                                   ULONG Offset,
                                   PLIN  LinearAddr );

USHORT APIENTRY DevHelp_LinToGDTSelector( SEL   Selector,
                                          LIN   LinearAddr,
                                          ULONG Size );

USHORT APIENTRY DevHelp_GetDescInfo( SEL Selector,
                                     PBYTE SelInfo );

USHORT APIENTRY DevHelp_PageListToLin( ULONG Size,
                                       LIN   pPageList,
                                       PLIN  LinearAddr );

USHORT APIENTRY DevHelp_LinToPageList( LIN    LinearAddr,
                                       ULONG  Size,
                                       LIN    pPageList,
                                       PULONG PageListCount );

USHORT APIENTRY DevHelp_PageListToGDTSelector( SEL    Selector,
                                               ULONG  Size,
                                               USHORT Access,
                                               LIN    pPageList );

USHORT APIENTRY DevHelp_VMSetMem( LIN LinearAddr,                    /*@V74979*/
                                  ULONG Size,                        /*@V74979*/
                                  ULONG Flags );                     /*@V74979*/

USHORT APIENTRY DevHelp_FreeGDTSelector( SEL Selector );


/* DevHelp_VMLock */

#define VMDHL_NOBLOCK           0x0001
#define VMDHL_CONTIGUOUS        0x0002
#define VMDHL_16M               0x0004
#define VMDHL_WRITE             0x0008
#define VMDHL_LONG              0x0010
#define VMDHL_VERIFY            0x0020

/* DevHelp_VMAlloc */                                                /*@V76282*/
                                                                     /*@V76282*/
#define VMDHA_16M               0x0001                               /*@V76282*/
#define VMDHA_FIXED             0x0002                               /*@V76282*/
#define VMDHA_SWAP              0x0004                               /*@V76282*/
#define VMDHA_CONTIG            0x0008                               /*@V76282*/
#define VMDHA_PHYS              0x0010                               /*@V76282*/
#define VMDHA_PROCESS           0x0020                               /*@V76282*/
#define VMDHA_SGSCONT           0x0040                               /*@V76282*/
#define VMDHA_RESERVE           0x0100                               /*@V76282*/
#define VMDHA_USEHIGHMEM        0x0800                               /*@V76282*/

/* DevHelp_VMGlobalToProcess */                                      /*@V76282*/
                                                                     /*@V76282*/
#define VMDHGP_WRITE            0x0001                               /*@V76282*/
#define VMDHGP_SELMAP           0x0002                               /*@V76282*/
#define VMDHGP_SGSCONTROL       0x0004                               /*@V76282*/
#define VMDHGP_4MEG             0x0008                               /*@V76282*/

/* DevHelp_VMProcessToGlobal */                                      /*@V76282*/
                                                                     /*@V76282*/
#define VMDHPG_READONLY         0x0000                               /*@V76282*/
#define VMDHPG_WRITE            0x0001                               /*@V76282*/

/* DevHelp_GetDescInfo */                                            /*@V76282*/
                                                                     /*@V76282*/
typedef struct _SELDESCINFO {           /* DHSI */                   /*@V76282*/
                                                                     /*@V76282*/
  UCHAR    Type;                                                     /*@V76282*/
  UCHAR    Granularity;                                              /*@V76282*/
  LIN      BaseAddr;                                                 /*@V76282*/
  ULONG    Limit;                                                    /*@V76282*/
                                                                     /*@V76282*/
} SELDESCINFO;                                                       /*@V76282*/
                                                                     /*@V76282*/
typedef SELDESCINFO FAR *PSELDESCINFO;                               /*@V76282*/
                                                                     /*@V76282*/
typedef struct _GATEDESCINFO {          /* DHGI */                   /*@V76282*/
                                                                     /*@V76282*/
  UCHAR    Type;                                                     /*@V76282*/
  UCHAR    ParmCount;                                                /*@V76282*/
  SEL      Selector;                                                 /*@V76282*/
  USHORT   Reserved_1;                                               /*@V76282*/
  ULONG    Offset;                                                   /*@V76282*/
                                                                     /*@V76282*/
} GATEDESCINFO;                                                      /*@V76282*/
                                                                     /*@V76282*/
typedef GATEDESCINFO FAR *PGATEDESCINFO;                             /*@V76282*/

/* DevHelp_PageListToGDTSelector */                                  /*@V76282*/
/* DevHelp_PageListToGDTSel      */                                  /*@V76282*/
                                                                     /*@V76282*/
#define GDTSEL_R3CODE           0x0000                               /*@V76282*/
#define GDTSEL_R3DATA           0x0001                               /*@V76282*/
#define GDTSEL_R2CODE           0x0003                               /*@V76282*/
#define GDTSEL_R2DATA           0x0004                               /*@V76282*/
#define GDTSEL_R0CODE           0x0005                               /*@V76282*/
#define GDTSEL_R0DATA           0x0006                               /*@V76282*/

/* GDTSEL_ADDR32 may be OR'd with above defines */                   /*@V76282*/
#define GDTSEL_ADDR32           0x0080                               /*@V76282*/

/* DevHelp_VMSetMem */                                               /*@V76282*/
                                                                     /*@V76282*/
#define VMDHS_DECOMMIT          0x0001                               /*@V76282*/
#define VMDHS_RESIDENT          0x0002                               /*@V76282*/
#define VMDHS_SWAP              0x0004                               /*@V76282*/

/* PageList structure */                                             /*@V76282*/
                                                                     /*@V76282*/
typedef struct _PAGELIST {      /* DHPL */                           /*@V76282*/
                                                                     /*@V76282*/
  ULONG    PhysAddr;                                                 /*@V76282*/
  ULONG    Size;                                                     /*@V76282*/
                                                                     /*@V76282*/
} PAGELIST;                                                          /*@V76282*/
                                                                     /*@V76282*/
typedef PAGELIST NEAR *NPPAGELIST;                                   /*@V76282*/
typedef PAGELIST FAR  *PPAGELIST;                                    /*@V76282*/


/*
** 32-Bit Context Hooks
*/

USHORT APIENTRY DevHelp_AllocateCtxHook( NPFN   HookHandler,
                                         PULONG HookHandle );

USHORT APIENTRY DevHelp_FreeCtxHook( ULONG HookHandle );

USHORT APIENTRY DevHelp_ArmCtxHook( ULONG HookData,
                                    ULONG HookHandle );


/*
** Adapter Device Drivers / Device Managers
*/

USHORT APIENTRY DevHelp_RegisterDeviceClass( NPSZ    DeviceString,
                                             PFN     DriverEP,
                                             USHORT  DeviceFlags,
                                             USHORT  DeviceClass,
                                             PUSHORT DeviceHandle);

USHORT APIENTRY DevHelp_CreateInt13VDM( PBYTE VDMInt13CtrlBlk );

/* DevHelp_RegisterDeviceClass */                                    /*@V74979*/
                                                                     /*@V74979*/
#define DEVICECLASS_ADDDM       1                                    /*@V74979*/
#define DEVICECLASS_MOUSE       2                                    /*@V74979*/

/*
** Miscellaneous
*/

USHORT APIENTRY DevHelp_RegisterBeep( PFN BeepHandler );             /*@V74979*/

USHORT APIENTRY DevHelp_Beep( USHORT Frequency,                      /*@V74979*/
                              USHORT DurationMS );                   /*@V74979*/

USHORT APIENTRY DevHelp_RegisterPDD( NPSZ PhysDevName,
                                     PFN  HandlerRoutine );

USHORT APIENTRY DevHelp_DynamicAPI( PVOID  RoutineAddress,           /*@V76282*/
                                    USHORT ParmCount,                /*@V76282*/
                                    USHORT Flags,                    /*@V76282*/
                                    PSEL   CallGateSel );            /*@V76282*/
                                                                     /*@V76282*/
                                                                     /*@V76282*/
/* DevHelp_DynamicAPI */                                             /*@V76282*/
#define DYNAPI_CALLGATE16       0x0001   /* 16:16 CallGate     */    /*@V76282*/
#define DYNAPI_CALLGATE32       0x0000   /*  0:32 CallGate     */    /*@V76282*/
                                                                     /*@V76282*/
#define DYNAPI_ROUTINE16        0x0002   /* 16:16 Routine Addr */    /*@V76282*/
#define DYNAPI_ROUTINE32        0x0000   /*  0:32 Routine Addr */    /*@V76282*/


/* File Systems DevHelp routines for BaseDev Init Time Only    */

typedef struct FOPEN {           /* DHFSO */
        PSZ   FileName;
        ULONG FileSize;
} FILEOPEN;


typedef struct FCLOSE {          /* DHFSC */
        USHORT Reserved;
} FILECLOSE;

typedef struct FREAD  {          /* DHFSR */
        PBYTE Buffer;
        ULONG ReadSize;
} FILEREAD;

typedef struct FREADAT {         /* DHFSA */
        PBYTE Buffer;
        ULONG ReadSize;
        ULONG StartPosition;
} FILEREADAT;

typedef union FILEIOOP {            /* DHFOP */
               struct FOPEN FileOpen;
               struct FCLOSE FileClose;
               struct FREAD FileRead;
               struct FREADAT FileReadAt;
} FILEIOOP;

typedef struct _DDFileIo {       /* DHFSIO */
        USHORT   Length;
        FILEIOOP Data;
} FILEIOINFO, FAR * PFILEIOINFO;

USHORT APIENTRY DevHelp_OpenFile( PFILEIOINFO pFileOpen);
USHORT APIENTRY DevHelp_CloseFile( PFILEIOINFO pFileClose );
USHORT APIENTRY DevHelp_ReadFile( PFILEIOINFO pFileRead );
USHORT APIENTRY DevHelp_ReadFileAt( PFILEIOINFO pFileReadAT );


/*
** DevHlp Error Codes  (from ABERROR.INC)
*/

#define MSG_MEMORY_ALLOCATION_FAILED    0x00
#define ERROR_LID_ALREADY_OWNED         0x01
#define ERROR_LID_DOES_NOT_EXIST        0x02
#define ERROR_ABIOS_NOT_PRESENT         0x03
#define ERROR_NOT_YOUR_LID              0x04
#define ERROR_INVALID_ENTRY_POINT       0x05



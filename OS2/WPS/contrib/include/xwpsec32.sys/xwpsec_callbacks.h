
/*
 *@@sourcefile xwpsec32.h:
 *      declarations for xwpsec32.sys.
 *
 */

#ifndef XWPSEC_TYPES_H
    #define XWPSEC_TYPES_H

    /* ******************************************************************
     *
     *   Misc definitions
     *
     ********************************************************************/

    #ifdef __IBMC__
        #define CallType _System  /* for IBMC */
    #elif (_MSC_VER >=0 )
        #define CallType _syscall /* for MSC  */
    #else
        #define CallType         /* for MASM */
    #endif

    #define VA_START(ap, last) ap = ((va_list)__StackToFlat(&last)) + __nextword(last)

    typedef ULONG LINADDR;
    typedef ULONG PHYSADDR;

    /* ******************************************************************
     *
     *   Utilities
     *
     ********************************************************************/

    void _sprintf(const char *pcszFormat, ...);

    int kernel_printf(const char *fmt, ...);

    PVOID utilAllocFixed(ULONG cb);

    VOID utilFreeFixed(PVOID pv,
                       ULONG cb);

    /* ******************************************************************
     *
     *   Security contexts
     *
     ********************************************************************/

    typedef USHORT IOCTLRET;

    VOID ctxtInit(VOID);

    #ifdef XWPTREE_INCLUDED

        /*
         *@@ XWPSECURITYCONTEXT:
         */

        typedef struct _XWPSECURITYCONTEXT
        {
            TREE    tree;           // ulKey has the pid
            ULONG   cbStruct;       // size of entire struct, including TREE
            LONG    cOpenFiles;     // currently open files (raised with each
                                    // successful OPEN_POST, decr'd with each CLOSE)
            XWPSECURITYCONTEXTCORE
                    ctxt;           // context core (xwpsecty.h)
        } XWPSECURITYCONTEXT, *PXWPSECURITYCONTEXT;

        extern PXWPSECURITYCONTEXT G_pContextCreateVDM;

        PXWPSECURITYCONTEXT ctxtCreate(USHORT pidNew,
                                       USHORT pidParent,
                                       ULONG cSubjects);

        PXWPSECURITYCONTEXT ctxtFind(USHORT pid);

        VOID ctxtFree(PXWPSECURITYCONTEXT pCtxt);

        VOID ctxtClearAll(VOID);

        PVOID ctxtLogEvent(PXWPSECURITYCONTEXT pContext,
                           ULONG ulEventCode,
                           ULONG cbData);

    #endif

    VOID ctxtStopLogging(VOID);

    #ifdef RING0API_HEADER_INCLUDED

        IOCTLRET ctxtFillLogBuf(PLOGBUF pLogBufR3,
                                ULONG blockid);

        int ctxtSendACLs(PRING0BUF pBufR3);

        ULONG ctxtQueryPermissions(PCSZ pcszResource,
                                   ULONG ulResourceLen,     // in: strlen(pcszResource)
                                   ULONG cSubjects,
                                   const HXSUBJECT *paSubjects);
    #endif

    /* ******************************************************************
     *
     *   Strategy implementation routines
     *
     ********************************************************************/

    // extern int sec32_write(PTR16 reqpkt);
    extern int sec32_close(PTR16 reqpkt);
    // extern int sec32_read(PTR16 reqpkt);
    extern int sec32_open(PTR16 reqpkt);
    extern int sec32_init_base(PTR16 reqpkt);
    extern int sec32_init_complete(PTR16 reqpkt);
    extern int sec32_invalid_command(PTR16 reqpkt);
    extern int sec32_ioctl(PTR16 reqpkt);
    extern int sec32_shutdown(PTR16 reqpkt);

    /* ******************************************************************
     *
     *   OS/2 Security definitions
     *
     ********************************************************************/

    // #define DEVHLP_SECURITY     0x44
    // #define DHSEC_GETEXPORT     0x48a78df8
    // #define DHSEC_SETIMPORT     0x73ae3627
    // #define DHSEC_GETINFO       0x33528882

    #define SEC_EXPORT_MAJOR_VERSION 0x0001
    #define SEC_EXPORT_MINOR_VERSION 0x0000
    #define SEC_IMPORT_MAJOR_VERSION 0x0001
    #define SEC_IMPORT_MINOR_VERSION 0x0000

    #pragma pack(2)

    typedef struct
    {
        PSZ     pszPath;      // well formed path
        ULONG   ulHandle;     // search handle
        ULONG   rc;           // rc user got from findfirst
        PUSHORT pResultCnt;  // count of found files
        USHORT  usReqCnt;     // count user requested
        USHORT  usLevel;      // search level
        USHORT  usBufSize;    // user buffer size
        USHORT  fPosition;    // use position information?
        PCHAR   pcBuffer;     // ptr to user buffer
        ULONG   Position;     // Position for restarting search
        PSZ     pszPosition;  // file to restart search with
    } FINDPARMS, *PFINDPARMS;

    #pragma pack()

    typedef struct SecImp_s
    {
        USHORT siVersionMajor;
        USHORT siVersionMinor;
        ULONG (* CallType OPEN_PRE)(PSZ pszPath,
                                    ULONG fsOpenFlags,
                                    ULONG fsOpenMode,
                                    ULONG SFN);
        ULONG (* CallType OPEN_POST)(PSZ pszPath,
                                     ULONG fsOpenFlags,
                                     ULONG fsOpenMode,
                                     ULONG SFN,
                                     ULONG Action,
                                     ULONG RC);
        ULONG (* CallType READ_PRE)(ULONG SFN,
                                    PUCHAR pBuffer,
                                    ULONG cbBuf);
        VOID  (* CallType READ_POST)(ULONG SFN,
                                     PUCHAR PBUFFER,
                                     ULONG CBBYTESREAD,
                                     ULONG RC);
        ULONG (* CallType WRITE_PRE)(ULONG SFN,
                                     PUCHAR pBuffer,
                                     ULONG cbBuf);
        VOID  (* CallType WRITE_POST)(ULONG SFN,
                                      PUCHAR PBUFFER,
                                      ULONG CBBUF,
                                      ULONG cbBytesWritten,
                                      ULONG RC);
        VOID  (* CallType CLOSE)(ULONG SFN);
        VOID  (* CallType CHGFILEPTR)(ULONG SFN,
                                      PLONG SeekOff,
                                      PUSHORT SeekType,
                                      PLONG Absolute,
                                      PLONG pLogical);
        ULONG (* CallType DELETE_PRE)(PSZ pszPath);
        VOID  (* CallType DELETE_POST)(PSZ pszPath,
                                       ULONG RC);
        ULONG (* CallType MOVE_PRE)(PSZ pszNewPath,
                                    PSZ pszOldPath);
        VOID  (* CallType MOVE_POST)(PSZ pszNewPath,
                                     PSZ pszOldPath,
                                     ULONG RC);
        ULONG (* CallType LOADEROPEN)(PSZ pszPath,
                                      ULONG SFN);
        ULONG (* CallType GETMODULE)(PSZ pszPath);
        ULONG (* CallType EXECPGM)(PSZ pszPath,
                                   PCHAR pchArgs);
        ULONG (* CallType FINDFIRST)(PFINDPARMS pParms);
        ULONG (* CallType CALLGATE16)(VOID);
        ULONG (* CallType CALLGATE32)(VOID);
        ULONG (* CallType SETFILESIZE)(ULONG SFN, PULONG pSize);
        ULONG (* CallType QUERYFILEINFO)(ULONG SFN,
                                         PUCHAR pBuffer,
                                         ULONG cbBuffer,
                                         ULONG InfoLevel);
        ULONG (* CallType MAKEDIR)(PSZ pszPath);
        ULONG (* CallType CHANGEDIR)(PSZ pszPath);
        ULONG (* CallType REMOVEDIR)(PSZ pszPath);
        ULONG (* CallType FINDNEXT)(PFINDPARMS pParms);
        ULONG (* CallType FINDFIRST3X)(ULONG ulSrchHandle,
                                       PSZ pszPath);
        VOID  (* CallType FINDCLOSE)(ULONG ulSearchHandle);
        ULONG (* CallType FINDFIRSTNEXT3X)(ULONG ulSrchHandle,
                                           PSZ pszFile);
        ULONG (* CallType FINDCLOSE3X)(ULONG ulSrchHandle);
        VOID  (* CallType EXECPGMPOST)(PSZ pszPath,
                                       PCHAR pchArgs,
                                       ULONG NewPID);
        ULONG (* CallType CREATEVDM)(PSZ pszProgram,
                                     PSZ pszArgs);
        VOID  (* CallType CREATEVDMPOST)(int rc);
        ULONG (* CallType SETDATETIME)(PDATETIME pDateTimeBuf);
        ULONG (* CallType SETFILEINFO)(ULONG SFN,
                                       PUCHAR pBuffer,
                                       ULONG cbBuffer,
                                       ULONG InfoLevel);
        ULONG (* CallType SETFILEMODE)(PSZ pszPath,
                                       PUSHORT pNewAttribute);
        ULONG (* CallType SETPATHINFO)(PSZ pszPathName,
                                       ULONG InfoLevel,
                                       PUCHAR pBuffer,
                                       ULONG cbBuffer,
                                       ULONG InfoFlags);
        ULONG (* CallType DEVIOCTL)(ULONG SFN,
                                    ULONG Category, /* Category 8 and 9 only.*/
                                    ULONG Function,
                                    PUCHAR pParmList,
                                    ULONG cbParmList,
                                    PUCHAR pDataArea,
                                    ULONG cbDataArea,
                                    ULONG PhysicalDiskNumber); /* Category 9 only */
        ULONG (* CallType TRUSTEDPATHCONTROL)(VOID);

        /*
         *  The following are all SCS (SES) API audit hooks.
         */
        VOID (* CallType STARTEVENT)(ULONG AuditRC,
                                     PVOID /* PSESSTARTEVENT */ pSESStartEvent);
        VOID (* CallType WAITEVENT)(ULONG AuditRC,
                                    PVOID /* PSESEVENT */ pSESEventInfo,
                                    ULONG ulTimeout);
        VOID (* CallType RETURNEVENTSTATUS)(ULONG AuditRC,
                                            PVOID /* PSESEVENT */ pSESEventInfo);
        VOID (* CallType REGISTERDAEMON)(ULONG AuditRC,
                                         ULONG ulDaemonID,
                                         ULONG ulEventList);
        VOID (* CallType RETURNWAITEVENT)(ULONG AuditRC,
                                          PVOID /* PSESEVENT */ pSESEventInfo,
                                          ULONG ulTimeout);
        VOID (* CallType CREATESUBJECTHANDLE)(ULONG AuditRC,
                                              PVOID /* PSUBJECTINFO */ pSubjectInfo);
        VOID (* CallType DELETESUBJECTHANDLE)(ULONG AuditRC,
                                              PVOID /* HSUBJECT */ SubjectHandle);
        VOID (* CallType SETSUBJECTHANDLE)(ULONG AuditRC,
                                           ULONG TargetSubject,
                                           PVOID /* HSUBJECT */ SubjectHandle);
        VOID (* CallType QUERYSUBJECTHANDLE)(ULONG AuditRC,
                                             PID pid,
                                             ULONG TargetSubject,
                                             PVOID /* HSUBJECT */ SubjectHandle);
        VOID (* CallType QUERYSUBJECTINFO)(ULONG AuditRC,
                                           PID pid,
                                           ULONG TargetSubject,
                                           PVOID /* PSUBJECTINFO */ pSubjectInfo);
        VOID (* CallType QUERYSUBJECTHANDLEINFO)(ULONG AuditRC,
                                                 PVOID /* HSUBJECT */ SubjectHandle,
                                                 PVOID /* PSUBJECTINFO */ pSubjectInfo);
        VOID (* CallType SETCONTEXTSTATUS)(ULONG AuditRC,
                                           ULONG ContextStatus);
        VOID (* CallType QUERYCONTEXTSTATUS)(ULONG AuditRC,
                                             PID pid,
                                             ULONG ContextStatus);
        VOID (* CallType SETSECURITYCONTEXT)(ULONG AuditRC,
                                             PVOID /* PSECURITYCONTEXT */ pSecurityContext);
        VOID (* CallType QUERYSECURITYCONTEXT)(ULONG AuditRC,
                                               PID pid,
                                               PVOID /* PSECURITYCONTEXT */ pSecurityContext);
        VOID (* CallType QUERYAUTHORITYID)(ULONG AuditRC,
                                           PUCHAR szAuthorityTag,
                                           ULONG AuthorityID);
        VOID (* CallType CREATEINSTANCEHANDLE)(ULONG AuditRC,
                                               PVOID /* HSUBJECT */ SubjectHandle);
        VOID (* CallType RESERVESUBJECTHANDLE)(ULONG AuditRC,
                                               ULONG TargetSubject);
        VOID (* CallType RELEASESUBJECTHANDLE)(ULONG AuditRC,
                                               ULONG TargetSubject,
                                               PVOID /* HSUBJECT */ SubjectHandle);
        VOID (* CallType QUERYPROCESSINFO)(ULONG AuditRC,
                                           ULONG ActionCode,
                                           PVOID /* HSUBJECT */ CUH,
                                           ULONG ProcessCount,
                                           PVOID ProcessBuf);
        VOID (* CallType KILLPROCESS)(ULONG AuditRC,
                                      PID idProcessID);
        VOID (* CallType INACTIVITYNOTIFY)(ULONG AuditRC,
                                           ULONG ulTimeout);
        VOID (* CallType CONTROLPROCESSCREATION)(ULONG AuditRC,
                                                 ULONG ulActionCode);
        VOID (* CallType RESETTHREADCONTEXT)(ULONG AuditRC,
                                             ULONG TargetConext);
        VOID (* CallType CREATEHANDLENOTIFY)(ULONG AuditRC,
                                             PVOID /* PSUBJECTINFO */ pSubjectInfo);
        VOID (* CallType DELETEHANDLENOTIFY)(ULONG AuditRC,
                                             PVOID /* HSUBJECT */ SubjectHandle);
        VOID (* CallType CONTROLKBDMONITORS)(ULONG AuditRC,
                                             ULONG ActionCode,
                                             ULONG Status);
        /*
         *  End of SCS (SES) API audit hooks.
         */
    } SecurityImport, *pSecurityImport;

    /* ******************************************************************
     *
     *   XWPSEC32.SYS security callouts
     *
     ********************************************************************/

    extern struct SecImp_s G_SecurityHooks;

    ULONG CallType OPEN_PRE(PSZ pszPath,
                            ULONG fsOpenFlags,
                            ULONG fsOpenMode,
                            ULONG SFN);

    ULONG CallType OPEN_POST(PSZ pszPath,
                             ULONG fsOpenFlags,
                             ULONG fsOpenMode,
                             ULONG SFN,
                             ULONG ActionTaken,
                             ULONG RC);

    ULONG CallType READ_PRE(ULONG SFN,
                            PUCHAR pBuffer,
                            ULONG cbBuf);

    VOID  CallType READ_POST(ULONG SFN,
                             PUCHAR PBUFFER,
                             ULONG CBBYTESREAD,
                             ULONG RC);

    ULONG CallType WRITE_PRE(ULONG SFN,
                             PUCHAR pBuffer,
                             ULONG cbBuf);

    VOID  CallType WRITE_POST(ULONG SFN,
                              PUCHAR PBUFFER,
                              ULONG CBBUF,
                              ULONG cbBytesWritten,
                              ULONG RC);

    VOID  CallType CLOSE(ULONG SFN);

    VOID  CallType CHGFILEPTR(ULONG SFN,
                              PLONG SeekOff,
                              PUSHORT SeekType,
                              PLONG Absolute,   // physical (FS)
                              PLONG pLogical);  // logical (app)

    ULONG CallType DELETE_PRE(PSZ pszPath);

    VOID  CallType DELETE_POST(PSZ pszPath,
                               ULONG RC);

    ULONG CallType MOVE_PRE(PSZ pszNewPath,
                            PSZ pszOldPath);

    VOID  CallType MOVE_POST(PSZ pszNewPath,
                             PSZ pszOldPath,
                             ULONG RC);

    ULONG CallType LOADEROPEN(PSZ pszPath,
                              ULONG SFN);

    ULONG CallType GETMODULE(PSZ pszPath);

    ULONG CallType EXECPGM(PSZ pszPath,
                           PCHAR pchArgs);

    ULONG CallType FINDFIRST(PFINDPARMS pParms);

    ULONG CallType CALLGATE16(VOID);

    ULONG CallType CALLGATE32(VOID);

    ULONG CallType SETFILESIZE(ULONG SFN,
                               PULONG pSize);

    ULONG CallType QUERYFILEINFO_POST(ULONG  SFN,
                                      PUCHAR pBuffer,
                                      ULONG  cbBuffer,
                                      ULONG  InfoLevel);

    ULONG CallType MAKEDIR(PSZ pszPath);

    ULONG CallType CHANGEDIR(PSZ pszPath);

    ULONG CallType REMOVEDIR(PSZ pszPath);

    ULONG CallType FINDNEXT(PFINDPARMS pParms);

    ULONG CallType FINDFIRST3X(ULONG ulSrchHandle,
                               PSZ pszPath);

    VOID  CallType FINDCLOSE(ULONG ulSearchHandle);

    ULONG CallType FINDFIRSTNEXT3X(ULONG ulSrchHandle,
                                   PSZ pszFile);

    ULONG CallType FINDCLOSE3X(ULONG ulSrchHandle);

    VOID  CallType EXECPGM_POST(PSZ pszPath,
                                PCHAR pchArgs,
                                ULONG NewPID);

    ULONG CallType CREATEVDM(PSZ pszProgram,
                             PSZ pszArgs);

    VOID  CallType CREATEVDM_POST(int rc);

    ULONG CallType SETDATETIME(PDATETIME pDateTimeBuf);

    ULONG CallType SETFILEINFO(ULONG  SFN,
                               PUCHAR pBuffer,
                               ULONG  cbBuffer,
                               ULONG  InfoLevel);

    ULONG CallType SETFILEMODE(PSZ     pszPath,
                               PUSHORT pNewAttribute);

    ULONG CallType SETPATHINFO(PSZ    pszPathName,
                               ULONG  InfoLevel,
                               PUCHAR pBuffer,
                               ULONG  cbBuffer,
                               ULONG  InfoFlags);

    ULONG CallType DEVIOCTL(ULONG  SFN,
                            ULONG  Category,
                            ULONG  Function,
                            PUCHAR pParmList,
                            ULONG  cbParmList,
                            PUCHAR pDataArea,
                            ULONG  cbDataArea,
                            ULONG  PhysicalDiskNumber);

    ULONG CallType TRUSTEDPATHCONTROL(VOID);

    VOID CallType AUDIT_STARTEVENT(ULONG AuditRC,
                                   PVOID /* PSESSTARTEVENT */ pSESStartEvent);

    VOID CallType AUDIT_WAITEVENT(ULONG AuditRC,
                                  PVOID /* PSESEVENT */ pSESEventInfo,
                                  ULONG ulTimeout);

    VOID CallType AUDIT_RETURNEVENTSTATUS(ULONG AuditRC,
                                          PVOID /* PSESEVENT */ pSESEventInfo);

    VOID CallType AUDIT_REGISTERDAEMON(ULONG AuditRC,
                                       ULONG ulDaemonID,
                                       ULONG ulEventList);

    VOID CallType AUDIT_RETURNWAITEVENT(ULONG AuditRC,
                                        PVOID /* PSESEVENT */ pSESEventInfo,
                                        ULONG ulTimeout);

    /* ******************************************************************
     *
     *   Global variables
     *
     ********************************************************************/

    extern ULONG    G_pidShell;         // sec32_data.c

    extern struct InfoSegGDT
                        *G_pGDT;        // sec32_contexts.c
    extern struct InfoSegLDT
                        *G_pLDT;        // sec32_contexts.c

    extern BYTE     G_bLog;             // sec32_contexts.c
                #define LOG_INACTIVE        0
                #define LOG_ACTIVE          1
                #define LOG_ERROR           2

    // stuff elsewhere
    extern CHAR     G_szScratchBuf[];

    extern USHORT   G_rcUnknownContext; // default RC for unknown security contexts

    /*
    struct InfoSegGDT {

    // Time (offset 0x00)

    unsigned long   SIS_BigTime;    // Time from 1-1-1970 in seconds
    unsigned long   SIS_MsCount;    // Freerunning milliseconds counter
    unsigned char   SIS_HrsTime;    // Hours
    unsigned char   SIS_MinTime;    // Minutes
    unsigned char   SIS_SecTime;    // Seconds
    unsigned char   SIS_HunTime;    // Hundredths of seconds
    unsigned short  SIS_TimeZone;   // Timezone in min from GMT (Set to EST)
    unsigned short  SIS_ClkIntrvl;  // Timer interval (units=0.0001 secs)

    // Date (offset 0x10)

    unsigned char   SIS_DayDate;    // Day-of-month (1-31)
    unsigned char   SIS_MonDate;    // Month (1-12)
    unsigned short  SIS_YrsDate;    // Year (>= 1980)
    unsigned char   SIS_DOWDate;    // Day-of-week (1-1-80 = Tues = 3)

    // Version (offset 0x15)

    unsigned char   SIS_VerMajor;   // Major version number
    unsigned char   SIS_VerMinor;   // Minor version number
    unsigned char   SIS_RevLettr;   // Revision letter

    // System Status (offset 0x18)

    // XLATOFF
    #ifdef  OLDVER
    unsigned char   CurScrnGrp;     // Fgnd screen group #
    #else
    // XLATON
    unsigned char   SIS_CurScrnGrp; // Fgnd screen group #
    // XLATOFF
    #endif
    // XLATON
    unsigned char   SIS_MaxScrnGrp; // Maximum number of screen groups
    unsigned char   SIS_HugeShfCnt; // Shift count for huge segments
    unsigned char   SIS_ProtMdOnly; // Protect-mode-only indicator
    unsigned short  SIS_FgndPID;    // Foreground process ID

    // Scheduler Parms (offset 0x1E)

    unsigned char   SIS_Dynamic;    // Dynamic variation flag (1=enabled)
    unsigned char   SIS_MaxWait;    // Maxwait (seconds)
    unsigned short  SIS_MinSlice;   // Minimum timeslice (milliseconds)
    unsigned short  SIS_MaxSlice;   // Maximum timeslice (milliseconds)

    // Boot Drive (offset 0x24)

    unsigned short  SIS_BootDrv;    // Drive from which system was booted

    // RAS Major Event Code Table (offset 0x26)

    unsigned char   SIS_mec_table[32]; // Table of RAS Major Event Codes (MECs)

    // Additional Session Data (offset 0x46)

    unsigned char   SIS_MaxVioWinSG;  // Max. no. of VIO windowable SG's
    unsigned char   SIS_MaxPresMgrSG; // Max. no. of Presentation Manager SG's

    // Error logging Information (offset 0x48)

    unsigned short  SIS_SysLog;     // Error Logging Status

    // Additional RAS Information (offset 0x4A)

    unsigned short  SIS_MMIOBase;   // Memory mapped I/O selector
    unsigned long   SIS_MMIOAddr;   // Memory mapped I/O address

    // Additional 2.0 Data (offset 0x50)

    unsigned char   SIS_MaxVDMs;      // Max. no. of Virtual DOS machines
    unsigned char   SIS_Reserved;
    };


    struct InfoSegLDT
    {
        unsigned short  LIS_CurProcID;  // Current process ID
        unsigned short  LIS_ParProcID;  // Process ID of parent
        unsigned short  LIS_CurThrdPri; // Current thread priority
        unsigned short  LIS_CurThrdID;  // Current thread ID
        unsigned short  LIS_CurScrnGrp; // Screengroup
        unsigned char   LIS_ProcStatus; // Process status bits
        unsigned char   LIS_fillbyte1;  // filler byte
        unsigned short  LIS_Fgnd;       // Current process is in foreground
        unsigned char   LIS_ProcType;   // Current process type
        unsigned char   LIS_fillbyte2;  // filler byte

        unsigned short  LIS_AX;         // @@V1 Environment selector
        unsigned short  LIS_BX;         // @@V1 Offset of command line start
        unsigned short  LIS_CX;         // @@V1 Length of Data Segment
        unsigned short  LIS_DX;         // @@V1 STACKSIZE from the .EXE file
        unsigned short  LIS_SI;         // @@V1 HEAPSIZE  from the .EXE file
        unsigned short  LIS_DI;         // @@V1 Module handle of the application
        unsigned short  LIS_DS;         // @@V1 Data Segment Handle of application

        unsigned short  LIS_PackSel;    // First tiled selector in this EXE
        unsigned short  LIS_PackShrSel; // First selector above shared arena
        unsigned short  LIS_PackPckSel; // First selector above packed arena
    };
    */

#endif


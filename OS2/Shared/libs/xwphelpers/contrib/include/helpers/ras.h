
/*
 *@@sourcefile ras.h:
 *      "Reliability, Availability, Serviceability" (RAS)
 *      prototypes. In other words, undocumented OS/2 functions
 *      for problem determination (system dumps, process dumps,
 *      tracing, etc.).
 *
 *      This file has been taken directly from the OS/2 Debugging
 *      Handbook, V0.7a.
 *
 *@@added V0.9.0
 */

#ifndef RAS_HEADER_INCLUDED
    #define RAS_HEADER_INCLUDED

    /* definitions for DosDumpProcess */
    #define DDP_DISABLEPROCDUMP     0x00000000L     /* disable process dumps */
    #define DDP_ENABLEPROCDUMP      0x00000001L     /* enable process dumps */
    #define DDP_PERFORMPROCDUMP     0x00000002L     /* perform process dump */

    /* definitions for DosSuppressPopUps */
    #define SPU_DISABLESUPPRESSION  0x00000000L     /* disable popup suppression */
    #define SPU_ENABLESUPPRESSION   0x00000001L     /* enable popup suppression */

    /* definitions for DosQueryRASInfo Index */
    #define SIS_MMIOADDR            0
    #define SIS_MEC_TABLE           1
    #define SIS_SYS_LOG             2
    #define LF_LOGENABLE    0x0001          /* Logging enabled */
    #define LF_LOGAVAILABLE 0x0002          /* Logging available */

    APIRET  APIENTRY        DosQueryRASInfo(ULONG Index, PPVOID Addr);

    APIRET  APIENTRY        DosForceSystemDump(ULONG reserved);

    APIRET  APIENTRY        DosDumpProcess(ULONG Flag, ULONG Drive, PID Pid);

    APIRET  APIENTRY        DosSuppressPopUps(ULONG Flag, ULONG Drive);

    APIRET16  APIENTRY16    DosSysTrace(USHORT Majorcode, USHORT Length,
                                        USHORT Minorcode, PCHAR pData);

    APIRET16 APIENTRY16 DosGetSTDA(SEL, SHORT, SHORT );

    /* 32-bit Logging Facility Function Prototypes                   */

    /*--------------------------------------------*/
    /* Logging Defines                            */
    /*--------------------------------------------*/
    #define ERRLOG_SERVICE        1L
    #define ERRLOG_VERSION        1

    /*--------------------------------------------*/
    /* LogRecord status bits                      */
    /*--------------------------------------------*/
    #define LF_BIT_PROCNAME     0x0001L
    #define LF_BIT_ORIGIN_256   0x0002L
    #define LF_BIT_DATETIME     0x0004L
    #define LF_BIT_SUSPEND      0x0008L
    #define LF_BIT_RESUME       0x0010L
    #define LF_BIT_REDIRECT     0x0020L
    #define LF_BIT_GETSTATUS    0x0040L
    #define LF_BIT_REGISTER     0x0080L
    #define LF_BIT_REMOTE_FAIL  0x0100L

    /*--------------------------------------------*/
    /*    Log Entry Record Header for 2.X         */
    /*    This is format used by 2.0 device       */
    /*     drivers and callers of LogAddEntries   */
    /*--------------------------------------------*/
    typedef struct LogRecord
    {
      USHORT   len ;                   /* this record length(includes len field)*/
      USHORT   rec_id ;                /* record id                             */
      ULONG    status ;                /* record status bits(see LF_BIT_)       */
      UCHAR    qualifier[4] ;          /* qualifier tag                         */
      ULONG    reserved ;
      ULONG    time ;                  /* hours minutes seconds hundreds        */
      ULONG    date ;                  /* day month (USHORT)year                */
      UCHAR    data[1] ;               /* begin of variable data that includes: */
                                   /* Originator(256 bytes if LF_BIT_ORIGIN_256)*/
                                   /* else 8 bytes long                         */
                                   /* Processname(260 bytes) only if status     */
                                   /* LF_BIT_PROCNMAME set                      */
                                   /* FormatDLLName(12 bytes)                   */
                                   /* Variable data                             */
    } LOGRECORD ;
    /* typedef LOGRECORD far *PLOGREC ; */

    /*----------------------------------------------*/
    /* Format of buffer sent to LogAddEntries       */
    /*----------------------------------------------*/
    typedef struct LogEntryRec
    {
      USHORT    version ;                            /* this version is 1       */
      USHORT    count   ;                /* number of log records in this buffer*/
      LOGRECORD logrec  ;                            /* repeated count times    */
    } LOGENTRYREC ;
    /* typedef LOGENTRYREC far *PLOGENTRYREC ; */

    /*--------------------------------------------------*/
    /* Logging facility Function prototypes             */
    /*--------------------------------------------------*/
    APIRET APIENTRY LogOpen( PHFILE phf );

    APIRET APIENTRY LogClose( HFILE hf );

    APIRET APIENTRY  LogAddEntries( HFILE hf, ULONG ulService, PVOID pLogEntries );


    /* 16-bit Logging Facility Function Prototypes                   */


    APIRET16 APIENTRY16     DosLogRegister(PUSHORT LogHandle,
                                           PVOID LogRegList,
                                           PUSHORT RequestID);


    APIRET16 APIENTRY16     DosLogEntry(USHORT Function,
                                        PVOID LogData);

    APIRET16 APIENTRY16     DosLogRead(USHORT LogHandle,
                                       USHORT Length,
                                       PVOID LogBuffer,
                                       PUSHORT ReadSize);

#endif



/*
 *@@sourcefile sec32_callbacks.c:
 *      SES kernel hook code.
 *
 *      See strat_init_base.c for an introduction to the driver
 *      structure in general.
 *
 *      This file contains the table of security callbacks which
 *      are passed to the OS/2 kernel at system bootup. This is
 *      done by calling DevHlp32_Security in sec32_init_base().
 *
 *      After that call, the OS/2 kernel calls the specified
 *      callbacks before certain API calls. This allows us to
 *      intercept API calls like DosOpen.
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
 *      Based on the MWDD32.SYS example sources,
 *      Copyright (C) 1995, 1996, 1997  Matthieu Willm (willm@ibm.net).
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include "xwpsec32.sys\types.h"
#include "xwpsec32.sys\StackToFlat.h"
#include "xwpsec32.sys\devhlp32.h"

#include "xwpsec32.sys\xwpsec_callbacks.h"

struct SecImp_s G_SecurityHooks =
{
   SEC_IMPORT_MAJOR_VERSION,        // USHORT siVersionMajor;
   SEC_IMPORT_MINOR_VERSION,        // USHORT siVersionMinor;

   OPEN_PRE,            // ULONG (* CallType OPEN_PRE) (PSZ pszPath,
                        //                              ULONG fsOpenFlags,
                        //                              ULONG fsOpenMode,
                        //                              ULONG SFN);
                        //      lets ISS authorize DosOpen

   OPEN_POST,           // ULONG (* CallType OPEN_POST)(PSZ pszPath,
                        //                              ULONG fsOpenFlags,
                        //                              ULONG fsOpenMode,
                        //                              ULONG SFN,
                        //                              ULONG ActionTaken,
                        //                              ULONG RC);
                        //      notifies ISS of DosOpen completion

   0,                   // ULONG (* CallType READ_PRE) (ULONG SFN,
                        //                              PUCHAR pBuffer,
                        //                              ULONG cbBuf);
                        //      lets ISS authorize DosRead

   0,                   // VOID  (* CallType READ_POST)(ULONG SFN,
                        //                              PUCHAR PBUFFER,
                        //                              ULONG CBBYTESREAD,
                        //                              ULONG RC);
                        //      notifies ISS of DosRead completion;
                        //      ISS can modify (decrypt?) data

   0,                   // ULONG (* CallType WRITE_PRE)(ULONG SFN,
                        //                              PUCHAR pBuffer,
                        //                              ULONG cbBuf);
                        //      lets ISS authorize DosWrite

   0,                   // VOID  (* CallType WRITE_POST)(ULONG SFN,
                        //                               PUCHAR PBUFFER,
                        //                               ULONG CBBUF,
                        //                               ULONG cbBytesWritten,
                        //                               ULONG RC);
                        //      notifies ISS of DosWrite completion;
                        //      ISS can restore data

   CLOSE,               // VOID  (* CallType CLOSE)(ULONG SFN);
                        //      notifies ISS of DosClose completion

   0,                   // VOID  (* CallType CHGFILEPTR)(ULONG SFN,
                        //                               PLONG  SeekOff,
                        //                               PUSHORT SeekType,
                        //                               PLONG Absolute, // physical (FS)
                        //                               PLONG pLogical); // logical (app)
                        //      notifies ISS of DosSetFilePtr processing;
                        //      ISS may modify values;
                        //      this way ISS may embed ACL data in the files

   DELETE_PRE,          // ULONG (* CallType DELETE_PRE) (PSZ pszPath);
                        //      lets ISS authorize DosDelete

   DELETE_POST,         // VOID  (* CallType DELETE_POST)(PSZ pszPath,
                        //                                ULONG RC);
                        //      notifies ISS of DosDelete completion

   MOVE_PRE,            // ULONG (* CallType MOVE_PRE)  (PSZ pszNewPath,
                        //                               PSZ pszOldPath);
                        //      lets ISS authorize DosMove;
                        //      will only get called when source and dest
                        //      are on same volume

   MOVE_POST,           // VOID  (* CallType MOVE_POST) (PSZ pszNewPath,
                        //                               PSZ pszOldPath,
                        //                               ULONG RC);
                        //      notifies ISS of DosMove completion

   LOADEROPEN,          // ULONG (* CallType LOADEROPEN)(PSZ pszPath,
                        //                               ULONG SFN);
                        //      lets ISS authorize kernel loader open...
                        //      is this called after OPENPRE?

   GETMODULE,           // ULONG (* CallType GETMODULE) (PSZ pszPath);
                        //      called whenever a process tries to load a
                        //      DLL or EXE into its address space...
                        //      is this called after OPENPRE?

   EXECPGM,             // ULONG (* CallType EXECPGM)   (PSZ pszPath,
                        //                               PCHAR pchArgs);
                        //      lets ISS authorize DosExecPgm;
                        //      all executables boil down to this function,
                        //      so this gets called for really all exec calls
                        //      (including DosStartSession, WinStartApp)
                        //      (see also: EXECPGMPOST below)

   FINDFIRST,           // ULONG (* CallType FINDFIRST) (PFINDPARMS pParms);
                        //          typedef struct {
                        //              PSZ    pszPath;      // well formed path
                        //              ULONG  ulHandle;     // search handle
                        //              ULONG  rc;           // rc user got from findfirst
                        //              PUSHORT pResultCnt;  // count of found files
                        //              USHORT usReqCnt;     // count user requested
                        //              USHORT usLevel;      // search level
                        //              USHORT usBufSize;    // user buffer size
                        //              USHORT fPosition;    // use position information?
                        //              PCHAR  pcBuffer;     // ptr to user buffer
                        //              ULONG  Position;     // Position for restarting search
                        //              PSZ    pszPosition;  // file to restart search with
                        //          } FINDPARMS, *PFINDPARMS;
                        //      lets ISS authorize DosFindFirst; this gets
                        //      called only after internal FindFirst has
                        //      completed successfully

   0,                   // ULONG (* CallType CALLGATE16)(VOID);

   0,                   // ULONG (* CallType CALLGATE32)(VOID);

   0,                   // ULONG (* CallType SETFILESIZE)(ULONG SFN,
                        //                                PULONG pSize);
                        //      lets ISS authorize DosSetFileSize

   0,                   // ULONG (* CallType QUERYFILEINFO_POST)(ULONG  SFN,
                        //                                       PUCHAR pBuffer,
                        //                                       ULONG  cbBuffer,
                        //                                       ULONG  InfoLevel);
                        //      lets ISS authorize DosQueryFileInfo;
                        //      ISS may also modify parameters; this is a post-call!

   MAKEDIR,             // ULONG (* CallType MAKEDIR)   (PSZ pszPath);
                        //      lets ISS authorize DosCreateDir

   CHANGEDIR,           // ULONG (* CallType CHANGEDIR) (PSZ pszPath);
                        //      lets ISS authorize DosSetCurrentDir

   REMOVEDIR,           // ULONG (* CallType REMOVEDIR) (PSZ pszPath);
                        //      lets ISS authorize DosDeleteDir

   0,                   // ULONG (* CallType FINDNEXT) (PFINDPARMS pParms);
                        //      lets ISS authorize DosFindNext; this gets
                        //      called only after internal FindNext has
                        //      completed successfully

   FINDFIRST3X,         // ULONG (* CallType FINDFIRST3X) (ULONG ulSrchHandle,
                        //                                 PSZ pszPath);  //DGE02
                        //      lets ISS authorize DOS SearchFirst;
                        //      has fully qualified name with metacharacters

   0,                   // VOID  (* CallType FINDCLOSE) (ULONG ulSearchHandle);              //DGE02
                        //      notifies ISS of DosFindClose completion
                        //      (search finished)

   0,                   // ULONG (* CallType FINDFIRSTNEXT3X) (ULONG ulSrchHandle,
                        //                                     PSZ pszFile);//DGE02
                        //      called from DOS SearchFirst and SearchNext

   0,                   // ULONG (* CallType FINDCLOSE3X) (ULONG ulSrchHandle);              //DGE02

   EXECPGM_POST,        // VOID  (* CallType EXECPGM_POST) (PSZ pszPath,
                        //                                  PCHAR pchArgs,
                        //                                  ULONG NewPID);
                        //      notifies ISS of DosExecPgm completion
                        //      (see also: EXECPGM)

   CREATEVDM,           // ULONG (* CallType CREATEVDM)   (PSZ pszProgram,
                        //                                 PSZ pszArgs);
                        //      lets ISS authorize creation of a VDM

   CREATEVDM_POST,      // VOID  (* CallType CREATEVDM_POST) (int rc);
                        //      notifies ISS of completion of creation of a VDM

   0,                   // ULONG (* CallType SETDATETIME) (PDATETIME pDateTimeBuf);
                        //      lets ISS authorize DosSetDateTime

   0,                   // ULONG (* CallType SETFILEINFO) (ULONG  SFN,
                        //                                 PUCHAR pBuffer,
                        //                                 ULONG  cbBuffer,
                        //                                 ULONG  InfoLevel);
                        //      lets ISS authorize DosSetFileInfo;
                        //      ISS may also modify parameters; this is a pre call

   0,                   // ULONG (* CallType SETFILEMODE) (PSZ     pszPath,
                        //                                 PUSHORT pNewAttribute);
                        //      called in DosSetFileMode; let ISS validate and/or
                        //      modify parameters; this is a pre call

   0,                   // ULONG (* CallType SETPATHINFO) (PSZ    pszPathName,
                        //                                 ULONG  InfoLevel,
                        //                                 PUCHAR pBuffer,
                        //                                 ULONG  cbBuffer,
                        //                                 ULONG  InfoFlags);
                        //      called in DosSetPathInfo; let ISS validate and/or
                        //      modify parameters; this is a pre call

   0,                   // ULONG (* CallType DEVIOCTL) (ULONG  SFN,
                        //                              ULONG  Category, /* Category 8 and 9 only.*/
                        //                              ULONG  Function,
                        //                              PUCHAR pParmList,
                        //                              ULONG  cbParmList,
                        //                              PUCHAR pDataArea,
                        //                              ULONG  cbDataArea,
                        //                              ULONG  PhysicalDiskNumber); /* Category 9 only */

   TRUSTEDPATHCONTROL,  // ULONG (* CallType TRUSTEDPATHCONTROL) (VOID);
                        //      notification of Ctrl-Alt-Del

   /*
    *  The following are all SCS (SES) API audit hooks.
    */

   0, // AUDIT_STARTEVENT,    // VOID (* CallType STARTEVENT) (ULONG AuditRC,
                        //                               PSESSTARTEVENT pSESStartEvent);
   0, // AUDIT_WAITEVENT,     // VOID (* CallType WAITEVENT) (ULONG AuditRC,
                        //                              PSESEVENT pSESEventInfo,
                        //                              ULONG ulTimeout);
   0, // AUDIT_RETURNEVENTSTATUS,
                        // VOID (* CallType RETURNEVENTSTATUS) (ULONG AuditRC,
                        //                                      PSESEVENT pSESEventInfo);
   0, // AUDIT_REGISTERDAEMON,
                        // VOID (* CallType REGISTERDAEMON) (ULONG AuditRC,
                        //                                  ULONG ulDaemonID,
                        //                                    ULONG ulEventList);
   0, // AUDIT_RETURNWAITEVENT,
                        // VOID (* CallType RETURNWAITEVENT) (ULONG AuditRC,
                        //                                    PSESEVENT pSESEventInfo,
                        //                                    ULONG ulTimeout);
   0,                   // VOID (* CallType CREATESUBJECTHANDLE) (ULONG AuditRC,
                        //                                        PSUBJECTINFO pSubjectInfo);
   0,                   // VOID (* CallType DELETESUBJECTHANDLE) (ULONG AuditRC,
                        //                                        HSUBJECT SubjectHandle);
   0,                   // VOID (* CallType SETSUBJECTHANDLE) (ULONG AuditRC,
                        //                                     ULONG TargetSubject,
                        //                                     HSUBJECT SubjectHandle);
   0,                   // VOID (* CallType QUERYSUBJECTHANDLE) (ULONG AuditRC,
                        //                                       PID pid,
                        //                                       ULONG TargetSubject,
                        //                                       HSUBJECT SubjectHandle);
   0,                   // VOID (* CallType QUERYSUBJECTINFO) (ULONG AuditRC,
                        //                                     PID pid,
                        //                                     ULONG TargetSubject,
                        //                                     PSUBJECTINFO pSubjectInfo);
   0,                   // VOID (* CallType QUERYSUBJECTHANDLEINFO) (ULONG AuditRC,
                        //                                           HSUBJECT SubjectHandle,
                        //                                           PSUBJECTINFO pSubjectInfo);
   0,                   // VOID (* CallType SETCONTEXTSTATUS) (ULONG AuditRC,
                        //                                     ULONG ContextStatus);
   0,                   // VOID (* CallType QUERYCONTEXTSTATUS) (ULONG AuditRC,
                        //                                       PID pid,
                        //                                       ULONG ContextStatus);
   0,                   // VOID (* CallType SETSECURITYCONTEXT) (ULONG AuditRC,
                        //                                       PSECURITYCONTEXT pSecurityContext);
   0,                   // VOID (* CallType QUERYSECURITYCONTEXT) (ULONG AuditRC,
                        //                                         PID pid,
                        //                                         PSECURITYCONTEXT pSecurityContext);
   0,                   // VOID (* CallType QUERYAUTHORITYID) (ULONG  AuditRC,
                        //                                     PUCHAR szAuthorityTag,
                        //                                     ULONG  AuthorityID);
   0,                   // VOID (* CallType CREATEINSTANCEHANDLE) (ULONG AuditRC,
                        //                                         HSUBJECT SubjectHandle);
   0,                   // VOID (* CallType RESERVESUBJECTHANDLE) (ULONG AuditRC,
                        //                                         ULONG TargetSubject);
   0,                   // VOID (* CallType RELEASESUBJECTHANDLE) (ULONG AuditRC,
                        //                                         ULONG TargetSubject,
                        //                                         HSUBJECT SubjectHandle);
   0,                   // VOID (* CallType QUERYPROCESSINFO) (ULONG    AuditRC,
                        //                                     ULONG    ActionCode,
                        //                                     HSUBJECT CUH,
                        //                                     ULONG    ProcessCount,
                        //                                     PVOID    ProcessBuf);
   0,                   // VOID (* CallType KILLPROCESS) (ULONG AuditRC,
                        //                                PID idProcessID);
   0,                   // VOID (* CallType INACTIVITYNOTIFY) (ULONG AuditRC,
                        //                                     ULONG ulTimeout);
   0,                   // VOID (* CallType CONTROLPROCESSCREATION) (ULONG AuditRC,
                        //                                           ULONG ulActionCode);
   0,                   // VOID (* CallType RESETTHREADCONTEXT) (ULONG AuditRC,
                        //                                       ULONG TargetConext);
   0,                   // VOID (* CallType CREATEHANDLENOTIFY) (ULONG        AuditRC,
                        //                                       PSUBJECTINFO pSubjectInfo);
   0,                   // VOID (* CallType DELETEHANDLENOTIFY) (ULONG    AuditRC,
                        //                                       HSUBJECT SubjectHandle);
   0                    // VOID (* CallType CONTROLKBDMONITORS) (ULONG  AuditRC,
                        //                                       ULONG  ActionCode,
                        //                                       ULONG  Status);

   /*
    *  End of SCS (SES) API audit hooks.
    */
};


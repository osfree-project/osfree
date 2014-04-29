#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <uconv.h>

#define INCL_DOSDEVIOCTL
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_VIO
#include <os2.h>
#include "portable.h"
#include "fat32def.h"

#define TIME_FACTOR 1

#define SHAREMEM     "\\SHAREMEM\\CACHEF32"

int (* CALLCONV pUniCreateUconvObject)(UniChar * code_set, UconvObject * uobj);
int (* CALLCONV pUniUconvToUcs)(
             UconvObject uobj,         /* I  - Uconv object handle         */
             void    * * inbuf,        /* IO - Input buffer                */
             size_t    * inbytes,      /* IO - Input buffer size (bytes)   */
             UniChar * * outbuf,       /* IO - Output buffer size          */
             size_t    * outchars,     /* IO - Output size (chars)         */
             size_t    * subst  );     /* IO - Substitution count          */

HMODULE hModLang;

PRIVATE VOID Handler(INT iSignal);
PRIVATE VOID InitProg(INT iArgc, PSZ rgArgv[]);
PRIVATE PSZ GetFSName(PSZ pszDevice);
PRIVATE VOID APIENTRY LWThread(ULONG ulArg);
PRIVATE VOID APIENTRY EMThread(ULONG ulArg);
PRIVATE BOOL IsDiskClean(PSZ pszDrive);
PRIVATE BOOL DoCheckDisk(BOOL fDoCheck);
PRIVATE BOOL ChkDsk(ULONG ulBootDisk, PSZ pszDrive);
PRIVATE BOOL StartMe(PSZ pszPath);
PRIVATE ULONG GetFAT32Drives(VOID);
PRIVATE BOOL IsDiskFat32(PSZ pszDisk);
PRIVATE VOID ShowRASectors(VOID);
PRIVATE BOOL SetRASectors(PSZ pszArg);
PRIVATE BOOL LoadTranslateTable(VOID);
PRIVATE void WriteLogMessage(PSZ pszMessage);

static PSZ rgPriority[]=
{
"No change",
"Idle time",
"Regular",
"Time critical",
"Foreground server"
};

static F32PARMS  f32Parms;
static BOOL      fActive = FALSE;
static BOOL      fLoadDeamon = TRUE;
static PLWOPTS   pOptions = NULL;
static BOOL      fForeGround;
static ULONG     ulDriveMap = 0;

/******************************************************************
*
******************************************************************/
INT main(INT iArgc, PSZ rgArgv[])
{
APIRET rc;
ULONG ulParmSize;
ULONG ulDataSize;
BYTE  bPrevPrio;

   DoCheckDisk(FALSE);


   InitProg(iArgc, rgArgv);
   if (fActive)
      DosExit(EXIT_PROCESS, 0);

   if (fForeGround)
      {
      DoCheckDisk(TRUE);
      if (!f32Parms.usCacheSize)
         printf("Cache size has been set to zero, lazy writer will not be started!\n");
      else if (fLoadDeamon)
         StartMe(rgArgv[0]);
      DosExit(EXIT_PROCESS, 0);
      }

   rc = DosFSCtl(NULL, 0, &ulDataSize,
                 NULL, 0, &ulParmSize,
      FAT32_STOPLW, "FAT32", -1, FSCTL_FSDNAME);

   rc = DosFSCtl(NULL, 0, &ulDataSize,
                 NULL, 0, &ulParmSize,
      FAT32_STARTLW, "FAT32", -1, FSCTL_FSDNAME);
   if (rc)
      {
      printf("Starting LW failed, rc = %d\n", rc);
      exit(1);
      }

   signal(SIGBREAK, Handler);
   signal(SIGTERM, Handler);
   signal(SIGINT, Handler);

   rc = DosCreateThread(&pOptions->ulEMTID, EMThread, 0L, 0, 8196);
   if (rc)
      printf("DosCreateThread failed, rc = %d\n", rc);

   rc = DosCreateThread(&pOptions->ulLWTID, LWThread, 0L, 0, 8196);
   if (rc)
      printf("DosCreateThread failed, rc = %d\n", rc);

   ulParmSize = sizeof f32Parms;
   rc = DosFSCtl(
      NULL, 0, &ulDataSize,
      (PVOID)&f32Parms, ulParmSize, &ulParmSize,
      FAT32_SETPARMS, "FAT32", -1, FSCTL_FSDNAME);

   bPrevPrio = pOptions->bLWPrio;
   while (!pOptions->fTerminate)
      {
      if (bPrevPrio != pOptions->bLWPrio)
         {
         DosSetPriority(PRTYS_THREAD,
            pOptions->bLWPrio, 0, pOptions->ulLWTID);
         bPrevPrio = pOptions->bLWPrio;
         }
      DosSleep(5000);
      }

   DosExit(EXIT_PROCESS, 0);

   return 0;
}

/******************************************************************
*
******************************************************************/
VOID APIENTRY LWThread(ULONG ulArg)
{
APIRET rc;
ULONG  ulParmSize;
ULONG  ulDataSize;

   rc = DosSetPriority(PRTYS_THREAD,
      pOptions->bLWPrio, 0, 0);

   ulParmSize = sizeof (LWOPTS);
   ulDataSize = 0;

   rc = DosFSCtl(NULL, 0, &ulDataSize,
      (PVOID)pOptions, ulParmSize, &ulParmSize,
      FAT32_DOLW, "FAT32", -1, FSCTL_FSDNAME);

   rc = rc;

   pOptions->fTerminate = TRUE;
}

VOID APIENTRY EMThread(ULONG ulArg)
{
APIRET rc;
ULONG  ulParmSize;
ULONG  ulDataSize;

#if 1
   rc = DosSetPriority(PRTYS_THREAD,
      PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, 0);
#else
   rc = DosSetPriority(PRTYS_THREAD,
      PRTYC_FOREGROUNDSERVER, PRTYD_MAXIMUM, 0);
#endif

   ulParmSize = sizeof (LWOPTS);
   ulDataSize = 0;

   rc = DosFSCtl(NULL, 0, &ulDataSize,
      (PVOID)pOptions, ulParmSize, &ulParmSize,
      FAT32_EMERGTHREAD, "FAT32", -1, FSCTL_FSDNAME);
   if (rc)
      printf("EMThread: rc = %u\n", rc);
}

/******************************************************************
*
******************************************************************/
VOID Handler(INT iSignal)
{
   printf("Signal %d was received\n", iSignal);
   exit(1);
}

/******************************************************************
*
******************************************************************/
VOID InitProg(INT iArgc, PSZ rgArgv[])
{
APIRET    rc;
BYTE      szTranslate[256];
INT       iArg;
ULONG     ulLVB;
USHORT    uscbLVB;
ULONG     ulDataSize;
ULONG     ulParmSize;
BOOL      fSetParms = FALSE;
ULONG     ulParm;


   memset(szTranslate, 0, sizeof szTranslate);

   /*
      Determine if we run in the foreground
   */
   rc = VioGetBuf(&ulLVB, &uscbLVB, (HVIO)0);
   if (rc)
      fForeGround = FALSE;
   else
      fForeGround = TRUE;

   if (fForeGround)
      printf("FAT32 cache helper version %s.\n", FAT32_VERSION);
   else
      WriteLogMessage("FAT32 task detached");

   rc = DosGetNamedSharedMem((PVOID *)&pOptions, SHAREMEM, PAG_READ|PAG_WRITE);
   if (!rc)
      {
      fActive = TRUE;
      WriteLogMessage("Shared memory found!");
      }
   else
      {
      rc = DosAllocSharedMem((PVOID *)&pOptions, SHAREMEM, sizeof (LWOPTS), PAG_COMMIT|PAG_READ|PAG_WRITE);
      if (rc)
         DosExit(EXIT_PROCESS, 1);
      memset(pOptions, 0, sizeof pOptions);
      pOptions->bLWPrio = PRTYC_IDLETIME;
      WriteLogMessage("Shared memory allocated!");
      }

   ulDataSize = sizeof f32Parms;
   rc = DosFSCtl(
      (PVOID)&f32Parms, ulDataSize, &ulDataSize,
      NULL, 0, &ulParmSize,
      FAT32_GETPARMS, "FAT32", -1, FSCTL_FSDNAME);
   if (rc)
      {
      printf("DosFSCtl, FAT32_GETPARMS failed, rc = %d\n", rc);
      DosExit(EXIT_PROCESS, 1);
      }
   if (strcmp(f32Parms.szVersion, FAT32_VERSION))
      {
      printf("ERROR: FAT32 version (%s) differs from CACHEF32 version (%s)\n", f32Parms.szVersion, FAT32_VERSION);
      DosExit(EXIT_PROCESS, 1);
      }

   for (iArg = 1; iArg < iArgc; iArg++)
      {
      strupr(rgArgv[iArg]);
      if (rgArgv[iArg][0] == '/' || rgArgv[iArg][0] == '-')
         {
         switch (rgArgv[iArg][1])
            {
            case '?' :
               printf("USAGE: CACHEF32 [options]\n");
               printf("/Q (Quit)\n");
               printf("/N do NOT load lazy write deamon.\n");
               printf("/D:diskidle in millisecs.\n");
               printf("/B:bufferidle in millisecs.\n");
               printf("/M:maxage in millisecs.\n");
               printf("/R:d:,n sets read ahead sector count for drive d: to n.\n");
               printf("/FS use short file names internally.\n");
               printf("/FL use long file names internally.\n");
               printf("/L:on|off set lazy writing on or off.\n");
               printf("/P:1|2|3|4 Set priority of Lazy writer\n");
               DosExit(EXIT_PROCESS, 0);
               break;

            case 'P':
               if (rgArgv[iArg][2] != ':')
                  {
                  printf("Missing : after /P\n");
                  DosExit(EXIT_PROCESS, 1);
                  }
               if (rgArgv[iArg][3] < '1' ||
                   rgArgv[iArg][3] > '4')
                  {
                  printf("Lazy write priority should be from 1 to 4!\n");
                  DosExit(EXIT_PROCESS, 1);
                  }
               pOptions->bLWPrio = rgArgv[iArg][3] - '0';
               break;


            case 'N':
               fLoadDeamon = FALSE;
               break;

            case 'T':
               printf("The /T option is no longer supported.\n");
               printf("Please read the documentation.\n");
               break;

            case 'Q' :
               if (fActive)
                  {
                  if (pOptions->fTerminate)
                     printf("Terminate request already set!\n");
                  pOptions->fTerminate = TRUE;
                  printf("Terminating CACHEF32.EXE...\n");
                  DosExit(EXIT_PROCESS, 0);
                  }
               printf("/Q is invalid, CACHEF32 is not running!\n");
               DosExit(EXIT_PROCESS, 1);
               break;
            case 'D':
               if (rgArgv[iArg][2] != ':')
                  {
                  printf("ERROR: missing : in %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               ulParm = atol(&rgArgv[iArg][3]);
               if (!ulParm)
                  {
                  printf("ERROR: Invalid value in %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               f32Parms.ulDiskIdle = ulParm / TIME_FACTOR;
               fSetParms = TRUE;
               break;

            case 'B':
               if (rgArgv[iArg][2] != ':')
                  {
                  printf("ERROR: missing : in %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               ulParm = atol(&rgArgv[iArg][3]);
               if (!ulParm)
                  {
                  printf("ERROR: Invalid value in %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               f32Parms.ulBufferIdle = ulParm / TIME_FACTOR;
               fSetParms = TRUE;
               break;

            case 'M':
               if (rgArgv[iArg][2] != ':')
                  {
                  printf("ERROR: missing : in %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               ulParm = atol(&rgArgv[iArg][3]);
               if (!ulParm)
                  {
                  printf("ERROR: Invalid value in %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               f32Parms.ulMaxAge = ulParm / TIME_FACTOR;
               fSetParms = TRUE;
               break;

            case 'R':
               if (rgArgv[iArg][2] != ':')
                  {
                  printf("ERROR: missing : in %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               SetRASectors(&rgArgv[iArg][3]);
               break;
            case 'F':
               if (rgArgv[iArg][2] == 'S')
                  f32Parms.fUseShortNames = TRUE;
               else if (rgArgv[iArg][2] == 'L')
                  f32Parms.fUseShortNames = FALSE;
               else
                  {
                  printf("ERROR: Unknown option %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               fSetParms = TRUE;
               break;

            case 'L':
               if (!stricmp(&rgArgv[iArg][2], ":ON"))
                  {
                  rc = DosFSCtl(NULL, 0, NULL,
                              NULL, 0, NULL,
                     FAT32_STARTLW, "FAT32", -1, FSCTL_FSDNAME);
                  if (rc)
                     printf("Warning: Lazy writing is already active or cachesize is 0!\n");
                  }
               else if (!stricmp(&rgArgv[iArg][2], ":OFF"))
                  {
                  rc = DosFSCtl(NULL, 0, NULL,
                              NULL, 0, NULL,
                     FAT32_STOPLW, "FAT32", -1, FSCTL_FSDNAME);
                  if (rc)
                     printf("Warning: Lazy writing is not active!\n");
                  }
               else
                  {
                  printf("ERROR: Unknown option %s\n", rgArgv[iArg]);
                  DosExit(EXIT_PROCESS, 1);
                  }
               break;

            default :
               printf("ERROR: Unknown option %s\n", rgArgv[iArg]);
               DosExit(EXIT_PROCESS, 1);
               break;
            }

         }
      }

   if (LoadTranslateTable())
      fSetParms = TRUE;

   if (fSetParms)
      {
      if (f32Parms.ulDiskIdle < f32Parms.ulBufferIdle)
         {
         printf("DISKIDLE must be greater than BUFFERIDLE\n");
         DosExit(EXIT_PROCESS, 1);
         }

      ulParmSize = sizeof f32Parms;
      rc = DosFSCtl(
         NULL, 0, &ulDataSize,
         (PVOID)&f32Parms, ulParmSize, &ulParmSize,
         FAT32_SETPARMS, "FAT32", -1, FSCTL_FSDNAME);
      if (rc)
         {
         printf("DosFSCtl FAT32_SETPARMS, failed, rc = %d\n", rc);
         DosExit(EXIT_PROCESS, 1);
         }
      }

   ulDriveMap = GetFAT32Drives();
   if (!fActive)
      {
      if (!ulDriveMap)
         {
         printf("FAT32: No FAT32 partitions found, aborting...\n");
         DosExit(EXIT_PROCESS, 1);
         }
      }



   /*
      Query parms
   */

   if (fActive || !f32Parms.usCacheSize)
      {
      if (fActive)
         {
         printf("CACHEF32 is already running.\n");
         printf("Current priority is %s.\n", rgPriority[pOptions->bLWPrio]);
         }

      if (!f32Parms.fLW)
         printf("LAZY WRITING is NOT active!\n\n");
      else
         {
         printf("\n");
         printf("DISKIDLE  : %lu milliseconds.\n", f32Parms.ulDiskIdle * TIME_FACTOR);
         printf("BUFFERIDLE: %lu milliseconds.\n", f32Parms.ulBufferIdle * TIME_FACTOR);
         printf("MAXAGE    : %lu milliseconds.\n", f32Parms.ulMaxAge * TIME_FACTOR);
         }

      printf("\n");
      ShowRASectors();
      printf("\n");
      printf("CACHE has space for %u sectors\n", f32Parms.usCacheSize);
      printf("CACHE contains %u sectors\n", f32Parms.usCacheUsed);
      printf("There are %u dirty sectors in cache.\n", f32Parms.usDirtySectors);
      if (f32Parms.usPendingFlush > 0)
         printf("%u sectors are in pending flush state.\n", f32Parms.usPendingFlush);
      printf("The cache hits ratio is %3d%%.\n",
         f32Parms.ulTotalHits * 100 / f32Parms.ulTotalReads);
      if (f32Parms.fUseShortNames)
         {
         printf("Internally, short names are used.\n");
         printf("All files are visible in DOS sessions.\n");
         }
      else
         {
         printf("Internally, long names are used.\n");
         printf("Files and directories with long names are hidden for DOS.\n");
         }
      printf("FAT32.IFS has currently %u GDT segments allocated.\n",
         f32Parms.usSegmentsAllocated);
      }

   return;
}

VOID ShowRASectors(VOID)
{
USHORT usIndex;
APIRET rc;
HFILE hDisk;
ULONG ulAction;
USHORT usRASectors;
ULONG  ulDataSize;

      for (usIndex = 0; usIndex < 26; usIndex++)
         {
         ULONG Mask = 0x0001 << usIndex;
         BYTE szDisk[3];

         if (!(ulDriveMap & Mask))
            continue;
         szDisk[0] = (BYTE)('A' + usIndex);
         szDisk[1] = ':';
         szDisk[2] = 0;


         rc = DosOpen(szDisk,
            &hDisk,
            &ulAction,                          /* action taken */
            0L,                                 /* new size     */
            0L,                                 /* attributes   */
            OPEN_ACTION_OPEN_IF_EXISTS,         /* open flags   */
            OPEN_ACCESS_READONLY |              /* open mode    */
            OPEN_SHARE_DENYNONE |
            OPEN_FLAGS_DASD,
            NULL);                              /* ea data      */

         ulDataSize = sizeof usRASectors;
         usRASectors = FALSE;
         rc = DosDevIOCtl(hDisk,
            IOCTL_FAT32,
            FAT32_QUERYRASECTORS,
            NULL, 0, NULL,
            (PVOID)&usRASectors, ulDataSize, &ulDataSize);
         if (rc)
            printf("DosDevIOCtl, FAT_QUERYRASECTORS for drive %s failed, rc = %d\n",
               szDisk, rc);

         DosClose(hDisk);
         if (!rc)
            printf("Read-Ahead sector count for drive %s is %u.\n",
               szDisk, usRASectors);
         }

}

BOOL SetRASectors(PSZ pszArg)
{
APIRET rc;
HFILE hDisk;
ULONG ulAction;
USHORT usRASectors;
ULONG  ulDataSize;
BYTE   szDisk[3];

   if (pszArg[1] != ':')
      {
      printf("Invalid argument for /R option.\n");
      DosExit(EXIT_PROCESS, 1);
      }
   memset(szDisk, 0, sizeof szDisk);
   memcpy(szDisk, pszArg, 2);
   strupr(szDisk);
   pszArg+=2;
   if (*pszArg !=',')
      {
      printf("Comma missing in /R:d:,n\n");
      DosExit(EXIT_PROCESS, 1);
      }
   pszArg++;
   usRASectors = atoi(pszArg);

   rc = DosOpen(szDisk,
      &hDisk,
      &ulAction,                          /* action taken */
      0L,                                 /* new size     */
      0L,                                 /* attributes   */
      OPEN_ACTION_OPEN_IF_EXISTS,         /* open flags   */
      OPEN_ACCESS_READONLY |              /* open mode    */
      OPEN_SHARE_DENYNONE |
      OPEN_FLAGS_DASD,
      NULL);                              /* ea data      */

   if (rc)
      {
      printf("Cannot access drive %s, rc = %d\n",
         szDisk, rc);
      DosExit(EXIT_PROCESS, 1);
      }

   ulDataSize = sizeof usRASectors;
   rc = DosDevIOCtl(hDisk,
      IOCTL_FAT32,
      FAT32_SETRASECTORS,
      (PVOID)&usRASectors, ulDataSize, &ulDataSize,
      NULL, 0, NULL);
   if (rc)
      {
      printf("DosDevIOCtl, FAT_SETRASECTORS for drive %s failed, rc = %d\n",
         szDisk, rc);
      DosExit(EXIT_PROCESS, 1);
      }

   DosClose(hDisk);
   return TRUE;
}

/******************************************************************
*
******************************************************************/
PSZ GetFSName(PSZ pszDevice)
{
static BYTE Buffer[200];
ULONG ulBufferSize;
PFSQBUFFER2 fsqBuf = (PFSQBUFFER2)Buffer;
APIRET rc;
  
   ulBufferSize = sizeof Buffer;

   DosError(0);
   rc = DosQueryFSAttach(pszDevice,
      1L,
      FSAIL_QUERYNAME,
      fsqBuf,
      &ulBufferSize);
   DosError(1);
   if (rc)
      return "";
   return fsqBuf->szName + fsqBuf->cbName + 1;
}

/******************************************************************
*
******************************************************************/
BOOL DoCheckDisk(BOOL fDoCheck)
{
ULONG ulCurDisk;
USHORT usIndex;
ULONG ulDummy;

      DosQueryCurrentDisk(&ulCurDisk, &ulDummy);
      ulDriveMap = GetFAT32Drives();

      for (usIndex = 0; usIndex < 26; usIndex++)
         {
         ULONG Mask = 0x0001 << usIndex;
         BYTE szDisk[3];

         if (!(ulDriveMap & Mask))
            continue;
         szDisk[0] = (BYTE)('A' + usIndex);
         szDisk[1] = ':';
         szDisk[2] = 0;

         if (!IsDiskClean(szDisk) && fDoCheck)
            if (!ChkDsk(ulCurDisk, szDisk))
               return FALSE;
         }

      return TRUE;
}


/******************************************************************
*
******************************************************************/
BOOL ChkDsk(ULONG ulBootDisk, PSZ pszDisk)
{
APIRET rc;
static BYTE szObjName[255];
static BYTE szProgram[255] = "X:\\OS2\\CHKDSK.COM";
static BYTE szArguments[512];
RESULTCODES Res;


   szProgram[0] = (BYTE)(ulBootDisk + '@');
   memset(szArguments, 0, sizeof szArguments);
   strcpy(szArguments, szProgram);
   sprintf(szArguments + strlen(szArguments) + 1,
      "%s /F /C", pszDisk);
      

   rc = DosExecPgm(szObjName, sizeof szObjName,
      EXEC_SYNC,
      szArguments,
      NULL,
      &Res,
      szProgram);
   if (rc)
      {
      printf("DosExecPgm Failed, rc = %d\n", rc);
      return FALSE;
      }
   return TRUE;   
}

/******************************************************************
*
******************************************************************/
BOOL IsDiskClean(PSZ pszDisk)
{
APIRET rc;
HFILE hDisk;
ULONG ulAction;
USHORT fClean;
ULONG  ulDataSize;

   rc = DosOpen(pszDisk,
      &hDisk,
      &ulAction,                          /* action taken */
      0L,                                 /* new size     */
      0L,                                 /* attributes   */
      OPEN_ACTION_OPEN_IF_EXISTS,         /* open flags   */
      OPEN_ACCESS_READONLY |              /* open mode    */
        OPEN_SHARE_DENYNONE |
        OPEN_FLAGS_DASD,
      NULL);                              /* ea data      */

   ulDataSize = sizeof fClean;
   fClean = FALSE;
   rc = DosDevIOCtl(hDisk,
      IOCTL_FAT32,
      FAT32_GETVOLCLEAN,
      NULL, 0, NULL,
      (PVOID)&fClean, ulDataSize, &ulDataSize);
   if (rc)
      printf("DosDevIOCtl, FAT_GETVOLCLEAN failed, rc = %d\n", rc);

   DosClose(hDisk);
   if (rc)
      return FALSE;

//   if (!fClean)
//      printf("FAT32: Drive %s was improperly stopped.\n", pszDisk);


   return fClean;
}

/******************************************************************
*
******************************************************************/
BOOL StartMe(PSZ pszPath)
{
APIRET rc;
static BYTE szObjName[255];
static BYTE szArguments[512];
RESULTCODES Res;

   memset(szArguments, 0, sizeof szArguments);
   strcpy(szArguments, pszPath);
   sprintf(szArguments + strlen(szArguments) + 1,
      " /P:%u", pOptions->bLWPrio);

   rc = DosExecPgm(szObjName, sizeof szObjName,
      EXEC_BACKGROUND,
      szArguments,
      NULL,
      &Res,
      pszPath);
   if (rc)
      {
      printf("FAT32: unable to start deamon (%s)\n. rc = %d for %s\n",
         szArguments, rc, szObjName);
      return FALSE;
      }
   printf("FAT32: Lazy write daemon started.\n");
   return TRUE;   
}


ULONG GetFAT32Drives(VOID)
{
USHORT usIndex;
ULONG ulCurDisk;

   if (ulDriveMap)
      return ulDriveMap;

   if (DosQueryCurrentDisk(&ulCurDisk, &ulDriveMap))
      {
      ulDriveMap = 0L;
      return 0L;
      }

   for (usIndex = 0; usIndex < 26; usIndex++)
      {
      ULONG Mask = 1L << usIndex;
      BYTE szDisk[3];

      if (!(ulDriveMap & Mask))
         continue;

      /*
         Skip A: and B:
      */

      if (usIndex < 2)
         {
         ulDriveMap &= ~Mask;
         continue;
         }

      szDisk[0] = (BYTE)('A' + usIndex);
      szDisk[1] = ':';
      szDisk[2] = 0;
      if (!IsDiskFat32(szDisk))
         ulDriveMap &= ~Mask;
      }
   return ulDriveMap;
}

BOOL IsDiskFat32(PSZ pszDisk)
{
   strupr(pszDisk);
   if (!stricmp(GetFSName(pszDisk), "FAT32"))
      {
      return TRUE;
      }
   return FALSE;
}

BOOL LoadTranslateTable(VOID)
{
APIRET rc;
ULONG ulParmSize;
BYTE   rgData[256];
PBYTE  pIn;
USHORT rgTranslate[256];
PUSHORT pOut;
UconvObject  uconv_object = NULL;
INT iIndex;
size_t       in_bytes_left;
size_t       uni_chars_left;
size_t       num_subs;
ULONG rgCP[3];
ULONG cbCP;

   rc = DosLoadModule(rgData, sizeof rgData, "UCONV.DLL", &hModLang);
   if (rc)
      {
      printf("No NLS support found (%s does not load).\n", rgData);
      printf("No UNICODE translate table loaded!\n");
      return TRUE;
      }
   rc = DosQueryProcAddr(hModLang, 0L,
      "UniCreateUconvObject", (PFN *)&pUniCreateUconvObject);
   if (rc)
      {
      printf("ERROR: Could not find address of UniCreateUconvObject.\n");
      return FALSE;
      }
   rc = DosQueryProcAddr(hModLang, 0L,
      "UniUconvToUcs", (PFN *)&pUniUconvToUcs);
   if (rc)
      {
      printf("ERROR: Could not find address of UniUconvToUcs.\n");
      return FALSE;
      }

   rc = DosQueryCp(sizeof rgCP, rgCP, &cbCP);
   if (f32Parms.ulCurCP == rgCP[0])
      return FALSE;

   if (f32Parms.ulCurCP)
      {
      BYTE chChar;
      printf("Loaded unicode translate table is for CP %lu\n", f32Parms.ulCurCP);
      printf("Current CP is %lu\n", rgCP[0]);
      printf("Would you like to reload the translate table for this CP [Y/N]? ");
      fflush(stdout);

      for (;;)
         {
         chChar = getch();
         switch (chChar)
            {
            case 'y':
            case 'Y':
               chChar = 'Y';
               break;
            case 'n':
            case 'N':
               chChar = 'N';
               break;
            default :
               DosBeep(660, 10);
               continue;
            }
         printf("%c\n", chChar);
         break;
         }
      if (chChar == 'N')
         return FALSE;
      }

   for (iIndex = 0; iIndex < 256; iIndex++)
      rgData[iIndex] = iIndex;

   rc = pUniCreateUconvObject((UniChar *)L"", &uconv_object);
   if (rc != ULS_SUCCESS)
      {
      printf("UniCreateUconvObject error: return code = %u\n", rc);
      return FALSE;
      }

   pIn  = rgData;
   in_bytes_left = sizeof rgData;
   pOut = rgTranslate;
   uni_chars_left = sizeof rgTranslate / sizeof (USHORT);

   rc = pUniUconvToUcs(uconv_object,
      (PVOID *)&pIn,
      &in_bytes_left,
      &pOut,
      &uni_chars_left,
      &num_subs);

   if (rc != ULS_SUCCESS)
      {
      printf("UniUconvToUcs failed, rc = %u\n", rc);
      return FALSE;
      }


   ulParmSize = sizeof rgTranslate;
   rc = DosFSCtl(NULL, 0, NULL,
               rgTranslate, ulParmSize, &ulParmSize,
               FAT32_SETTRANSTABLE, "FAT32", -1, FSCTL_FSDNAME);
   if (rc)
      {
      printf("Unable to set translate table for current Codepage.\n");
      return FALSE;
      }

   f32Parms.ulCurCP = rgCP[0];
   printf("Unicode translate table for CP %lu loaded.\n", rgCP[0]);
   DosFreeModule(hModLang);
   return TRUE;
}

void WriteLogMessage(PSZ pszMessage)
{
FILE *fp;

   return;

   fp = fopen("\\CACHEF32.LOG", "a");
   fprintf(fp, "%s\n", pszMessage);
   fclose(fp);
}

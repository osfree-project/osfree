#include "kal.h"

APIRET APIENTRY
DosQuerySysInfo(ULONG iStart, ULONG iLast,
                PVOID pBuf, ULONG cbBuf)
{
  APIRET rc = NO_ERROR;
  ULONG *pul;
  ULONG i;

  log("%s enter\n", __FUNCTION__);
  log("iStart=%lu\n", iStart);
  log("iLast=%lu\n", iLast);
  log("cbBuf=%lu\n", cbBuf);

  // for now
  //goto DOSQUERYSYSINFO_EXIT;
  //

  if(iStart>iLast || pBuf==0)
  {
    rc = 87; //invalid parameter
    goto DOSQUERYSYSINFO_EXIT;
  }

  if(iLast>26)
  {
    rc = 87; //invalid parameter
    goto DOSQUERYSYSINFO_EXIT;
  }

  if((iLast-iStart+1)*sizeof(ULONG) < cbBuf)
  {
    rc = 111; //buffer overflow
    goto DOSQUERYSYSINFO_EXIT;
  }

  pul = (ULONG*)pBuf;
  for(i=iStart; i<=iLast; i++,pul++) {
    switch(i) {
      case QSV_MAX_PATH_LENGTH:
        //ascii not unicode is assumed
        *pul = 260;
        break;
      case QSV_MAX_TEXT_SESSIONS:
        //NT has no limits
        *pul = 256;
        break;
      case QSV_MAX_PM_SESSIONS:
        //no PM support yet
        *pul = 0;
        break;
      case QSV_MAX_VDM_SESSIONS:
        //NT has no limits
        *pul = 256;
        break;
      case QSV_BOOT_DRIVE:
        {
        *pul = 0;
        }
        break;
      case QSV_DYN_PRI_VARIATION:
        *pul = 1;
        break;
      case QSV_MAX_WAIT:
        //no way to query this under NT
        *pul = 3;
        break;
      case QSV_MIN_SLICE:
        *pul = 32;
        break;
      case QSV_MAX_SLICE:
        *pul = 1000;
        break;
      case QSV_PAGE_SIZE:
        {
        *pul = 4096;
        }
        break;
      case QSV_VERSION_MAJOR:
        {
          *pul = 20;
        }
        break;
      case QSV_VERSION_MINOR:
        {
          *pul = 45;
        }
        break;
      case QSV_VERSION_REVISION:
        *pul = 0;
        break;
      case QSV_MS_COUNT:
        *pul = 1;
        break;
      case QSV_TIME_LOW:
        {
           *pul = 1;
        }
        break;
      case QSV_TIME_HIGH:
        {
           *pul = 0;
        }
        break;
      case QSV_TOTPHYSMEM:
        {
           *pul = 100;
        }
        break;
      case QSV_TOTRESMEM:
        {
          *pul = 1024*1024;
        }
        break;
      case QSV_TOTAVAILMEM:
        {
           *pul = 1023*1024;
        }
        break;
      case QSV_MAXPRMEM:
        *pul = 1024*1024*1024;
        break;
      case QSV_MAXSHMEM:
        *pul = 64*1024*1024;
        break;
      case QSV_TIMER_INTERVAL:
        *pul = 33;
        break;
      case QSV_MAX_COMP_LENGTH:
        *pul = 256;
        break;
      case QSV_FOREGROUND_FS_SESSION:
        *pul = 1;
        break;
      case QSV_FOREGROUND_PROCESS:
        *pul = 0; //N/A
        break;
      case 26: //QSV_NUMPROCESSORS
        {
          *pul = 1;
        }
        break;
    }
  }

DOSQUERYSYSINFO_EXIT:
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

#if 0
    1   QSV_MAX_PATH_LENGTH     Maximum length of a path name (bytes). Use this when allocating filename buffers.       All
    2   QSV_MAX_TEXT_SESSIONS   Maximum number of text sessions.        All
    3   QSV_MAX_PM_SESSIONS     Maximum number of PM sessions.  All
    4   QSV_MAX_VDM_SESSIONS    Maximum number of DOS sessions.         All
    5   QSV_BOOT_DRIVE  Boot drive (1 = A:, 2 = B:, 3 = C, and so on).  All
    6   QSV_DYN_PRI_VARIATION   Absolute(= 0)/Dynamic(= 1) priority.    All
    7   QSV_MAX_WAIT    Maximum wait time (seconds).    All
    8   QSV_MIN_SLICE   Minimum time slice allowed (milliseconds).      All
    9   QSV_MAX_SLICE   Maximum time slice allowed (milliseconds).      All
    10  QSV_PAGE_SIZE   Memory page size (bytes). Default 4096 bytes.   All
    11  QSV_VERSION_MAJOR       Major version number.   All
    12  QSV_VERSION_MINOR       Minor version number.   All
    13  QSV_VERSION_REVISION    Revision letter.        All
    14  QSV_MS_COUNT    Value of a 32-bit, free-running counter (milliseconds). Zero at boot time.      All
    15  QSV_TIME_LOW    Low-order 32 bits of the time since January 1, 1980 (seconds).  All
    16  QSV_TIME_HIGH   High-order 32 bits of the time since January 1, 1980 (seconds).         All
    17  QSV_TOTPHYSMEM  Total number of bytes of physical memory.       All
    18  QSV_TOTRESMEM   Total number of bytes of system-resident memory.        All
    19  QSV_TOTAVAILMEM         Maximum number of bytes available for all processes in the system RIGHT NOW.    All
    20  QSV_MAXPRMEM    Maximum number of bytes available for this process RIGHT NOW.   All
    21  QSV_MAXSHMEM    Maximum number of shareable bytes available RIGHT NOW.  All
    22  QSV_TIMER_INTERVAL      Timer interval (1/10 milliseconds).     All
    23  QSV_MAX_COMP_LENGTH     Maximum length of one component in a path name (bytes).         All
    24  QSV_FOREGROUND_FS_SESSION       Session ID of the current foreground full screen session. (any PM, VIO or Win-DOS session would be ID = 1).     Warp
    25  QSV_FOREGROUND_PROCESS  Process ID of the current foreground process.   Warp
    26  QSV_NUMPROCESSORS       Number of processors in the computer.   Warp
#endif

#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <os2.h>

#include "extboot.h"
#include "portable.h"
#include "fat32def.h"

/* globals */
ULONG autocheck_mask = 0;              /* global to be set by parse args for */
                                       /* drives to be autochecked           */
ULONG force_mask = 0;                  /* global to be set by parse args for */
                                       /* drives to be autochecked regardless*/
                                       /* of dirty status                    */
VOID InitMessage(PSZ pszMessage);

/*
 * NAME:        autocheck()
 *
 * FUNCTION:    invoke Autocheck for FAT32 drives
 *
 *
 * PARAMETERS:  cachef32.exe parameters
 *
 * RETURN:      void
 */

void _cdecl autocheck(char *args)
{

  PSZ        ModuleName      = "UFAT32";               /* name of utility dll*/
  PSZ        ProgramName     = "FAT32CHK.EXE";         /* special exe        */
  PSZ        ProgramName2    = "CACHEF32.EXE";         /* special exe        */
  UCHAR      LoadError[64];                            /* for dosloadmodule  */
  HMODULE    ModuleHandle    = 0;           /* handle od ufat32.dll          */
  PFN        ModuleAddr      = 0;           /* addr of chkdsk                */
  ULONG      ModuleType      = 0;
  UCHAR Arg_Buf[256];                       /* argument buffer               */
  USHORT env,cmd;
  RESULTCODES Result;
  USHORT len = 0;                           /* temp var for filling in buffer*/
  ULONG i;                                  /* loop count                    */
  char far * temp;

  char     drive[4];                        /* drive name ie C:              */
  HFILE    drive_handle;                    /* file handle of drive          */
  USHORT   action;                          /* return alue from dosopen      */
  char     boot_sector[512];                /* data area for boot sector     */
  ULONG    rc =0;
  USHORT   bytes_read;                      /* bytes read by dosread         */
  BOOTSECT *boot_rec;                       /* extended boot structure       */

  /* get the current environment to be passed in on execpgm */
  DosGetEnv(&env, &cmd);

  for (i = 0; i <= 31; i++)  /* loop for max drives allowed */
  {
    if ((autocheck_mask & (1UL << i)) == 0) /* mask not on for this drive */
    {
      continue;  /* go to next drive */
    }

    /* set up the drive name to be used in DosOpen */
    drive[0] = (char)('a' + i);
    drive[1] = ':';
    drive[2] = 0;

    /* open the drive */
    rc = DosOpen(drive, &drive_handle, &action, 0L, 0, 1, 0x8020, 0L);

    /* if the open fails then just skip this drive */
    if (rc)
    {
      continue;
    }

    /* read sector 0 the boot sector of this drive */
    rc = DosRead(drive_handle, boot_sector, 512, &bytes_read);

    /* close the drive */
    DosClose(drive_handle);

    /* if the read fails then just skip this drive */
    if (rc)
    {
      continue;
    }

    /* check the system_id in the boot sector to be sure it is JFS */
    /* if it is not then we will just skip this drive              */
    /* due to link problems we can't use memcmp for this           */

    boot_rec = (BOOTSECT *) boot_sector;
    if (boot_rec->FileSystem[0] != 'F' ||
        boot_rec->FileSystem[1] != 'A' ||
        boot_rec->FileSystem[2] != 'T' ||
        boot_rec->FileSystem[3] != '3' ||
        boot_rec->FileSystem[4] != '2' ||
        boot_rec->FileSystem[5] != ' ' ||
        boot_rec->FileSystem[6] != ' ' ||
        boot_rec->FileSystem[7] != ' ')
    {
      continue;
    }

    /* write something to the screen.  This will prevent bvhvga.dll from */
    /* unloading when chkdsk exits. */
    DosPutMessage(0, 1, " ");

    len = 0;
    temp = ProgramName;

    while (*temp != 0)       /* copy prog name as first arg */
    {
      Arg_Buf[len] = *temp;
      len++;
      temp++;
    }

    Arg_Buf[len] = '\0'; /* special null after first arg, all others use blank */
    len++;

    Arg_Buf[len] = (char)('a' + i); /* parm 2 is the drive letter to be checked */
    len++;
    Arg_Buf[len] = ':';
    len++;
    Arg_Buf[len] = ' ';
    len++;

    Arg_Buf[len] = '/';     /* parm 3 is /F for fix it                  */
    len++;
    Arg_Buf[len] = 'F';
    len++;
    Arg_Buf[len] = ' ';
    len++;

    Arg_Buf[len] = '/';     /* parm 4 is /A for autocheck               */
    len++;
    Arg_Buf[len] = 'A';
    len++;
    Arg_Buf[len] = ' ';
    len++;

    /* if the force mask is set for this drive ommit the /C, otherwise */
    /* add it as parm 5 */
    if ((force_mask & (1<< i)) == 0)
    {
      Arg_Buf[len] = '/';
      len++;
      Arg_Buf[len] = 'C';
      len++;
    }

    /* terminate the arg buf with a double null */
    Arg_Buf[len]='\0';
    len++;
    Arg_Buf[len]='\0';

    /* call exec PGM to execute chkdsk on this drive */
    rc = DosExecPgm(LoadError, sizeof(LoadError), 0, /* EXEC_SYNC */
                      Arg_Buf, MAKEP(env,0), &Result, ProgramName);

  } /* end for */

#if 0
  if (! args)
    return;

  temp = ProgramName2;
  len  = 0;

  while (*temp)        /* copy prog name as first arg */
  {
    Arg_Buf[len] = *temp;
    len++;
    temp++;
  }

  Arg_Buf[len] = '\0'; /* special null after first arg, all others use blank */
  len++;

  while (*args)        /* copy prog name as first arg */
  {
    Arg_Buf[len] = *args;
    len++;
    args++;
  }

  /* terminate the arg buf with a double null */
  Arg_Buf[len]='\0';
  len++;
  Arg_Buf[len]='\0';

  /* call exec PGM to execute chkdsk on this drive */
  rc = DosExecPgm(LoadError, sizeof(LoadError), 0, /* EXEC_SYNC */
                  Arg_Buf, MAKEP(env,0), &Result, ProgramName2);
#endif

  return;
}

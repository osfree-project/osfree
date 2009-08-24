/* $Id: fs_ioctl.c,v 1.1 2000/04/21 10:58:03 ktk Exp $ */

static char *SCCSID = "@(#)1.4  7/30/98 14:09:40 src/jfs/ifs/fs_ioctl.c, sysjfs, w45.fs32, 990417.1";
/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
*/

#define INCL_DOSDEVICES
#define INCL_DOSFILEMGR
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <os2.h>
#include <fsd.h>
#include <fsh.h>
#include <stdlib.h>
#include <stdio.h>
#include <extboot.h>

/* globals */
ULONG _cdecl autocheck_mask;    /* global to be set by parse args for */
                                /* drives to be autochecked           */
ULONG _cdecl force_mask;        /* global to be set by parse args for */
                                /* drives to be autochecked regardless*/
                                /* of dirty status                    */


int far pascal
FS_IOCTL (struct sffsi far * psffsi,
          struct sffsd far * psffsd,
          USHORT             cat,
          USHORT             func,
          PCHAR              pParm,
          USHORT             lenMaxParm,
          PUSHORT            plenInOutParm,
          PCHAR              pData,
          USHORT             lenMaxData,
          PUSHORT            plenInOutData)
{
        struct vpfsi far   *pVPBfsi;
        struct vpfsd far   *pVPBfsd;
        USHORT             usRet;

        /* get volume parameters */
        usRet = FSH_GETVOLPARM (psffsi->sfi_hVPB,
                                (struct vpfsi far * far *) &pVPBfsi,
                                (struct vpfsd far * far *) &pVPBfsd);

        if (usRet == 0)
            usRet = FSH_DOVOLIO2 (pVPBfsi->vpi_hDEV,
                                  psffsi->sfi_selfsfn,
                                  cat,
                                  func,
                                  pParm,
                                  lenMaxParm,
                                  pData,
                                  lenMaxData);

        /* Set output lengths as best we can. */

        if (plenInOutData != NULL) {
           *plenInOutData = lenMaxData;
        } /* endif */

        if (plenInOutParm != NULL) {
           *plenInOutParm = lenMaxParm;
        } /* endif */

        return (usRet);
}


/*
 * NAME:        autocheck()
 *
 * FUNCTION:    invoke Autocheck for JFS drives
 *
 *      called by fs_init16 during init processing
 *
 * PARAMETERS:   none
 *
 * RETURN:      void
 */

void _cdecl autocheck()
{

  PSZ        ModuleName      = "UJFS";                 /* name of utility dll*/
  PSZ        ProgramName     = "\\OS2\\JFSCHK32.EXE";  /* special exe        */
  UCHAR      LoadError[64];                            /* for dosloadmodule  */
  HMODULE    ModuleHandle    = 0;           /* handle od ujfs.dll            */
  PFN        ModuleAddr      = 0;           /* addr of chkdsk                */
  ULONG      ModuleType      = 0;
  UCHAR Arg_Buf[256];                       /* argument buffer               */
  USHORT env,cmd;
  RESULTCODES Result;
  USHORT len = 0;                           /* temp var for filling in buffer*/
  USHORT i;                                 /* loop count                    */
  char far * temp;

  char     drive[4];                        /* drive name ie C:              */
  HFILE    drive_handle;                    /* file handle of drive          */
  USHORT   action;                          /* return alue from dosopen      */
  char     boot_sector[512];                /* data area for boot sector     */
  ULONG    rc =0;
  USHORT   bytes_read;                      /* bytes read by dosread         */
  struct Extended_Boot *boot_rec;           /* extended boot structure       */


  /* get the current environment to be passed in on execpgm */
  DosGetEnv(&env,&cmd);

  for (i = 0; i <= 31; i++)  /* loop for max drives allowed */
  {
    if ((autocheck_mask & (1L << i)) == 0) /* mask not on for this drive */
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

    boot_rec = (struct Extended_Boot *) boot_sector;
    if (boot_rec->Boot_System_ID[0] != 'J' ||
        boot_rec->Boot_System_ID[1] != 'F' ||
        boot_rec->Boot_System_ID[2] != 'S' ||
        boot_rec->Boot_System_ID[3] != ' ' ||
        boot_rec->Boot_System_ID[4] != ' ' ||
        boot_rec->Boot_System_ID[5] != ' ' ||
        boot_rec->Boot_System_ID[6] != ' ' ||
        boot_rec->Boot_System_ID[7] != ' ')
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
  return;
}


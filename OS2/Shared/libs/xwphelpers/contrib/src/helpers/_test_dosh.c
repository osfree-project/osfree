
#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSESMGR
#define INCL_DOSQUEUES
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define INCL_KBD
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\standards.h"

#pragma hdrstop

int main (int argc, char *argv[])
{
    APIRET  arc;

    BYTE    cCDs,
            bFirstCD;

    ULONG   ul;

            printf("Drive checker ("__DATE__")\n");
            printf("    type   fs      remot fixed parrm bootd       media audio eas   longn\n");

    for (ul = 1;
         ul <= 26;
         ul++)
    {
        XDISKINFO xdi;
        arc = doshGetDriveInfo(ul,
                               0, // DRVFL_TOUCHFLOPPIES,
                               &xdi);

        printf(" %c: ", xdi.cDriveLetter, ul);

        if (!xdi.fPresent)
            printf("not present\n");
        else
        {
            if (arc)
                printf("error %d (IsFixedDisk: %d, QueryDiskParams %d, QueryMedia %d)\n",
                        arc,
                        xdi.arcIsFixedDisk,
                        xdi.arcQueryDiskParams,
                        xdi.arcQueryMedia);
            else
            {
                ULONG   aulFlags[] =
                    {
                        DFL_REMOTE,
                        DFL_FIXED,
                        DFL_PARTITIONABLEREMOVEABLE,
                        DFL_BOOTDRIVE,
                        0,
                        DFL_MEDIA_PRESENT,
                        DFL_AUDIO_CD,
                        DFL_SUPPORTS_EAS,
                        DFL_SUPPORTS_LONGNAMES
                    };
                ULONG ul2;

                PCSZ pcsz = NULL;

                switch (xdi.bType)
                {
                    case DRVTYPE_HARDDISK:  pcsz = "HDISK ";    break;
                    case DRVTYPE_FLOPPY:    pcsz = "FLOPPY"; break;
                    case DRVTYPE_TAPE:      pcsz = "TAPE  "; break;
                    case DRVTYPE_VDISK:     pcsz = "VDISK "; break;
                    case DRVTYPE_CDROM:     pcsz = "CDROM "; break;
                    case DRVTYPE_LAN:       pcsz = "LAN   "; break;
                    case DRVTYPE_PRT:
                                            pcsz = "PRTREM"; break;
                    default:
                                            printf("bType=%d, BPB.bDevType=%d",
                                                    xdi.bType,
                                                    xdi.bpb.bDeviceType);
                                            printf("\n           ");
                }

                if (pcsz)
                    printf("%s ", pcsz);

                if (xdi.lFileSystem < 0)
                    // negative means error
                    printf("E%3d    ", xdi.lFileSystem); // , xdi.bFileSystem);
                else
                    printf("%7s ", xdi.szFileSystem); // , xdi.bFileSystem);

                for (ul2 = 0;
                     ul2 < ARRAYITEMCOUNT(aulFlags);
                     ul2++)
                {
                    if (xdi.flDevice & aulFlags[ul2])
                        printf("  X   ");
                    else
                        if (    (xdi.arcOpenLongnames)
                             && (aulFlags[ul2] == DFL_SUPPORTS_LONGNAMES)
                           )
                            printf(" E%03d ", xdi.arcOpenLongnames);
                        else
                            printf("  -   ");
                }
                printf("\n");
            }
        }
    }

    if (!(arc = doshQueryCDDrives(&cCDs,
                                  &bFirstCD)))
    {
        printf("First CD-ROM drive is %c:\n", bFirstCD);
        printf("%d CD-ROM drive(s) present\n", cCDs);
    }
    else
        printf("doshQueryCDDrives returned %d\n", arc);

    {
        #pragma pack(1)
        struct
        {
            BYTE    ci;
            USHORT  usDrive;
        } parms;
        #pragma pack()

        USHORT fs = 123;

        parms.ci = 0;
        parms.usDrive = bFirstCD - 'A';

        if (!(arc = doshDevIOCtl((HFILE)-1,
                                 IOCTL_DISK,
                                 DSK_GETLOCKSTATUS,
                                 &parms, sizeof(parms),
                                 &fs, sizeof(fs))))
        {
            printf("IOCTL_DISK DSK_GETLOCKSTATUS returned 0x%lX\n", fs);
            switch (fs & 0x03)
            {
                case 0:
                    printf("  Lock/Unlock/Eject/Status functions not supported\n");
                break;

                case 1:
                    printf("  Drive locked; Lock/Unlock/Eject functions supported\n");
                break;

                case 2:
                    printf("  Drive unlocked; Lock/Unlock/Eject functions supported\n");
                break;

                case 3:
                    printf("  Lock Status not supported; Lock/Unlock/Eject functions supported\n");
                break;
            }

            if (fs & 0x04)
                printf("  Media in drive\n");
            else
                printf("  No media in drive\n");

        }
        else
            printf("ioctl DSK_GETLOCKSTATUS returned %d\n", arc);
    }

    {
        HFILE hf;
        ULONG dummy;
        CHAR    szDrive[] = "C:";
        szDrive[0] = bFirstCD;
        if (arc = DosOpen(szDrive,   // "C:", "D:", ...
                          &hf,
                          &dummy,
                          0,
                          FILE_NORMAL,
                          // OPEN_ACTION_FAIL_IF_NEW
                                 OPEN_ACTION_OPEN_IF_EXISTS,
                          OPEN_FLAGS_DASD
                                 | OPEN_FLAGS_FAIL_ON_ERROR
                                       // ^^^ if this flag is not set, we get the white
                                       // hard-error box
                                 | OPEN_FLAGS_NOINHERIT     // V0.9.6 (2000-11-25) [pr]
                      //            | OPEN_ACCESS_READONLY  // V0.9.13 (2001-06-14) [umoeller]
                                 | OPEN_SHARE_DENYNONE,
                          NULL))
            printf("DosOpen(\"%s\") returned %d\n",
                   szDrive,
                   arc);
        else
        {
            ULONG fl;
            if (!(arc = doshQueryCDStatus(hf, &fl)))
            {
                #define FLAG(f) f, # f
                struct
                {
                    ULONG fl;
                    PCSZ pcszFl;
                } aFlags[] =
                    {
                        FLAG(CDFL_DOOROPEN),
                        FLAG(CDFL_DOORLOCKED),
                        FLAG(CDFL_COOKEDANDRAW),
                        FLAG(CDFL_READWRITE),
                        FLAG(CDFL_DATAANDAUDIO),
                        FLAG(CDFL_ISO9660INTERLEAVE   ),
                        FLAG(CDFL_PREFETCHSUPPORT),
                        FLAG(CDFL_AUDIOCHANNELMANIP),
                        FLAG(CDFL_MINUTESECONDADDR),
                        FLAG(CDFL_MODE2SUPPORT),
                        FLAG(CDFL_DISKPRESENT),
                        FLAG(CDFL_PLAYINGAUDIO),
                        FLAG(CDFL_CDDA),
                    };
                ULONG ul;

                printf("IOCTL_CDROMDISK CDROMDISK_DEVICESTATUS returned 0x%lX\n", fl);

                for (ul = 0;
                     ul < ARRAYITEMCOUNT(aFlags);
                     ++ul)
                {
                    if (fl & aFlags[ul].fl)
                        printf("    + ");
                    else
                        printf("    - ");
                    printf("%s\n", aFlags[ul].pcszFl);
                }
            }
            else
                printf("IOCTL_CDROMDISK CDROMDISK_DEVICESTATUS returned error 0x%lX (%d)\n",
                        arc, arc);

            DosClose(hf);
        }
    }
}


/*
 *@@sourcefile apmh.c:
 *      contains helpers for accessing APM.SYS (Advanced Power Management).
 *
 *      These functions have been moved to this file with V0.9.14
 *      and were previously used in XWorkplace code.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes:
 *      --  apmh*   APM helper functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\apmh.h"
 *@@added V0.9.14 (2001-08-01) [umoeller]
 */

/*
 *      Copyright (C) 1998-2001 Ulrich M”ller.
 *      This file is part of the "XWorkplace helpers" source package.
 *      This is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS
#include <os2.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers/apmh.h"
#include "helpers/standards.h"

/*
 *@@category: Helpers\Control program helpers\Advanced Power Management
 *      See apmh.c.
 */

/* ******************************************************************
 *
 *   APM monitor
 *
 ********************************************************************/

/*
 *@@ apmhIOCtl:
 *      shortcut for DosDevIOCtl on the APM.SYS driver.
 */

APIRET apmhIOCtl(HFILE hfAPMSys,
                 ULONG ulFunction,
                 PVOID pvParamPck,
                 ULONG cbParamPck)
{
    APIRET          arc;
    APMGIO_DPKT     DataPacket;
    ULONG           ulRetSize = sizeof(DataPacket);
    DataPacket.ReturnCode = GIOERR_PowerNoError;
    if (!(arc = DosDevIOCtl(hfAPMSys,
                            APMGIO_Category,
                            ulFunction,
                            pvParamPck, cbParamPck, &cbParamPck,
                            &DataPacket, sizeof(DataPacket), &ulRetSize)))
        if (DataPacket.ReturnCode)
            arc = DataPacket.ReturnCode + 10000;

    return arc;
}

/*
 *@@ apmhOpen:
 *      opens the APM.SYS driver and creates
 *      an APM structure in *ppApm.
 *
 *      In the APM structure, the version fields are
 *      set to the version flags returned from the
 *      BIOS and APM.SYS itself.
 */

APIRET apmhOpen(PAPM *ppApm)
{
    // open APM.SYS
    APIRET arc;
    HFILE hfAPMSys = NULLHANDLE;
    ULONG ulAction;

    if (!(arc = DosOpen("\\DEV\\APM$",
                        &hfAPMSys,
                        &ulAction,
                        0,
                        FILE_NORMAL,
                        OPEN_ACTION_OPEN_IF_EXISTS,
                        OPEN_FLAGS_FAIL_ON_ERROR
                             | OPEN_SHARE_DENYNONE
                             | OPEN_ACCESS_READWRITE,
                        NULL)))
    {
        // query version of APM-BIOS and APM driver
        GETPOWERINFO    getpowerinfo;
        memset(&getpowerinfo, 0, sizeof(getpowerinfo));
        getpowerinfo.usParmLength = sizeof(getpowerinfo);

        if (!(arc = apmhIOCtl(hfAPMSys,
                              POWER_GETPOWERINFO,
                              &getpowerinfo,
                              getpowerinfo.usParmLength)))
        {
            PAPM papm;
            if (!(papm = NEW(APM)))
                arc = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                ZERO(papm);

                papm->hfAPMSys = hfAPMSys;

                // swap lower-byte(major vers.) to higher-byte(minor vers.)
                papm->usBIOSVersion =     (getpowerinfo.usBIOSVersion & 0xff) << 8
                                        | (getpowerinfo.usBIOSVersion >> 8);
                papm->usDriverVersion =   (getpowerinfo.usDriverVersion & 0xff) << 8
                                        | (getpowerinfo.usDriverVersion >> 8);

                // set general APM version to the lower of the two
                papm->usLowestAPMVersion = (papm->usBIOSVersion < papm->usDriverVersion)
                                                ? papm->usBIOSVersion
                                                : papm->usDriverVersion;

                *ppApm = papm;
            }
        }
    }

    if ((arc) && (hfAPMSys))
        DosClose(hfAPMSys);

    return arc;
}

/*
 *@@ apmhReadStatus:
 *      reads in the current battery status.
 *
 *      After this, the status fields in APM
 *      are valid (if NO_ERROR is returned).
 *      If the values changed since the previous
 *      call, *pfChanged is set to TRUE; FALSE
 *      otherwise.
 *
 *@@changed V0.9.19 (2002-05-28) [umoeller]: added fUsingAC
 */

APIRET apmhReadStatus(PAPM pApm,        // in: APM structure created by apmhOpen
                      PBOOL pfChanged)  // out: values changed (ptr can be NULL)
{
    APIRET  arc = NO_ERROR;
    BOOL    fChanged = FALSE;

    if ((pApm) && (pApm->hfAPMSys))
    {
        APMGIO_QSTATUS_PPKT  PowerStatus;
        PowerStatus.ParmLength = sizeof(PowerStatus);

        if (!(arc = apmhIOCtl(pApm->hfAPMSys,
                              APMGIO_QueryStatus,
                              &PowerStatus,
                              PowerStatus.ParmLength)))
        {
            if (    (pApm->fAlreadyRead)
                 || (pApm->bBatteryStatus != PowerStatus.BatteryStatus)
                 || (pApm->bBatteryLife != PowerStatus.BatteryLife)
                 || (pApm->fUsingAC != PowerStatus.ACStatus)
               )
            {
                pApm->bBatteryStatus = PowerStatus.BatteryStatus;
                pApm->bBatteryLife = PowerStatus.BatteryLife;
                pApm->fUsingAC = PowerStatus.ACStatus;

                pApm->fAlreadyRead = FALSE;
                fChanged = TRUE;
            }
        }

        if (pfChanged)
            *pfChanged = fChanged;
    }
    else
        arc = ERROR_INVALID_PARAMETER;

    return arc;
}

/*
 *@@ apmhClose:
 *      closes the APM device driver and frees
 *      the APM structure. *ppApm is set to NULL.
 */

VOID apmhClose(PAPM *ppApm)
{
    if (ppApm && *ppApm)
    {
        PAPM pApm = *ppApm;
        if (pApm->hfAPMSys)
            DosClose(pApm->hfAPMSys);
        free(pApm);
        *ppApm = NULL;
    }
}

/*
 *@@ apmhHasBattery:
 *      quick'n'dirty helper which returns TRUE only
 *      if APM is supported on the system and the
 *      system actually has a battery (i.e. is a laptop).
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 */

BOOL apmhHasBattery(VOID)
{
    BOOL brc = FALSE;

    PAPM p = NULL;
    if (!apmhOpen(&p))
    {
        if (!apmhReadStatus(p, NULL))
            brc = (p->bBatteryStatus != 0xFF);

        apmhClose(&p);
    }

    return brc;
}

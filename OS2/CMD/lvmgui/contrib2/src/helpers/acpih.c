
/*
 *@@sourcefile apcih.c:
 *      contains helpers for accessing ACPI.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes:
 *      --  acpih*   ACPI helper functions
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\acpih.h"
 *@@added V1.0.5 (2006-06-26) [pr]
 */

/*
 *      Copyright (C) 2006-2014 Paul Ratcliffe.
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

#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\apmh.h"               // @@added V1.0.9 (2012-02-20) [slevine]
#include "helpers\acpih.h"
#include "helpers\standards.h"

/* ******************************************************************
 *
 *   Globals
 *
 ********************************************************************/

HMODULE       G_hmodACPI = NULLHANDLE;
ULONG         G_ulCount = 0;
// @@added V1.0.9 (2012-02-20) [slevine]: additional ACPI support, code from David Azarewicz
ACPI_HANDLE   G_ahAC = 0;
#define MAX_BATTERY_COUNT 4
ACPI_HANDLE   G_ahBat[MAX_BATTERY_COUNT];
ULONG         G_uiBatteryCount = 0;
ULONG         G_uiAlreadyWalked = 0;

ACPISTARTAPI  *pAcpiStartApi = NULL;
ACPIENDAPI    *pAcpiEndApi = NULL;
ACPIGOTOSLEEP *pAcpiGoToSleep = NULL;

ACPITKGETOBJECTINFOALLOC *pAcpiTkGetObjectInfoAlloc = NULL;
ACPITKGETHANDLE *pAcpiTkGetHandle = NULL;
ACPITKOSFREE *pAcpiTkOsFree = NULL;
ACPITKWALKNAMESPACE *pAcpiTkWalkNamespace = NULL;
ACPITKEVALUATEOBJECT *pAcpiTkEvaluateObject = NULL;
// @@added V1.0.9 (2012-12-10) [slevine]: additional ACPI support
ACPITKPREPARETOSLEEP *pAcpiTkPrepareToSleep = NULL;

/*
 *@@category: Helpers\Control program helpers\ACPI
 *      See acpih.c.
 */

/*
 *@@ acpihOpen:
 *      resolves the ACPI entrypoints and loads the ACPI DLL.
 *
 *@@changed V1.0.10 (2014-08-30) [dazarewicz]: Call ACPI start API func.
 */

APIRET acpihOpen(ACPI_API_HANDLE *phACPI)
{
    APIRET arc = NO_ERROR;

    if (!G_hmodACPI)
    {
        if (!(arc = DosLoadModule(NULL,
                                  0,
                                  "ACPI32",
                                  &G_hmodACPI)))
        {
            arc = DosQueryProcAddr(G_hmodACPI,
                                   ORD_ACPISTARTAPI,
                                   NULL,
                                   (PFN *) &pAcpiStartApi);
            if (!arc)
                arc = DosQueryProcAddr(G_hmodACPI,
                                       ORD_ACPIENDAPI,
                                       NULL,
                                       (PFN *) &pAcpiEndApi);

            if (!arc)
                arc = DosQueryProcAddr(G_hmodACPI,
                                       ORD_ACPIGOTOSLEEP,
                                       NULL,
                                       (PFN *) &pAcpiGoToSleep);
            if (arc)
            {
                DosFreeModule(G_hmodACPI);
                G_hmodACPI = NULLHANDLE;
                pAcpiStartApi = NULL;
                pAcpiEndApi = NULL;
                pAcpiGoToSleep = NULL;
                return(arc);
            }

            // @@added V1.0.9 (2012-02-20) [slevine]: additional ACPI support, code from David Azarewicz
            DosQueryProcAddr(G_hmodACPI, ORD_ACPITKGETOBJECTINFOALLOC,
                             NULL, (PFN *) &pAcpiTkGetObjectInfoAlloc);
            DosQueryProcAddr(G_hmodACPI, ORD_ACPITKGETHANDLE,
                             NULL, (PFN *) &pAcpiTkGetHandle);
            DosQueryProcAddr(G_hmodACPI, ORD_ACPITKOSFREE,
                             NULL, (PFN *) &pAcpiTkOsFree);
            DosQueryProcAddr(G_hmodACPI, ORD_ACPITKWALKNAMESPACE,
                             NULL, (PFN *) &pAcpiTkWalkNamespace);
            DosQueryProcAddr(G_hmodACPI, ORD_ACPITKEVALUATEOBJECT,
                             NULL, (PFN *) &pAcpiTkEvaluateObject);
            // @@added V1.0.9 (2012-12-10) [slevine]: additional ACPI support
            DosQueryProcAddr(G_hmodACPI, ORD_ACPITKPREPARETOSLEEP,
                             NULL, (PFN *) &pAcpiTkPrepareToSleep);
        }
    }

    if (arc)
        return(arc);
    else
    {
        G_ulCount++;

        return(pAcpiStartApi(phACPI));
    }
}

/*
 *@@ acpihPrepareToSleep:
 *      Prepares the system to sleep or power off
 *
 *@@added V1.0.10 (2014-08-30) [dazarewicz]
 */

VOID acpihPrepareToSleep(UCHAR ucState)
{
    // @@added V1.0.9 (2012-12-10) [slevine]: use AcpiTkPrepareToSleep rather than workaround
    /* This function does not exist in older versions of acpi
     * As a result the shutdown attempt will usually hang because
     * the required code has not been committed into memory.
     */
    if (pAcpiTkPrepareToSleep)
        pAcpiTkPrepareToSleep(ucState);
}

/*
 *@@ acpihClose:
 *      unloads the ACPI DLL.
 */

VOID acpihClose(ACPI_API_HANDLE *phACPI)
{
    if (pAcpiEndApi)
    {
        pAcpiEndApi(phACPI);
        G_ulCount--;
    }

    if (!G_ulCount)
    {
        DosFreeModule(G_hmodACPI);
        G_hmodACPI = NULLHANDLE;
        pAcpiStartApi = NULL;
        pAcpiEndApi = NULL;
        pAcpiGoToSleep = NULL;
        // @@added V1.0.9 (2012-12-10) [slevine]: additional ACPI support
        pAcpiTkPrepareToSleep = NULL;
    }
}

/*
 *@@ acpihGoToSleep:
 *      changes the Power State.
 */

APIRET acpihGoToSleep(ACPI_API_HANDLE *phACPI, UCHAR ucState)
{
    if (pAcpiGoToSleep)
        return(pAcpiGoToSleep(phACPI, ucState));
    else
        return(ERROR_PROTECTION_VIOLATION);
}


/**
 *@@ AcpiCallbackWidget:
 *      ACPI callback helper for battery and power status queries.
 *      Code provided by David Azarewicz
 *@@added V1.0.9 (2012-02-20) [slevine]: code from David Azarewicz
 *@@changed V1.0.10 (2014-08-30) [dazarewicz]: release resources correctly
 */

ACPI_STATUS APIENTRY AcpiCallbackWidget( ACPI_HANDLE ObjHandle, UINT32 NestingLevel, void *Context, void **ReturnValue )
{
    ACPI_DEVICE_INFO *pDevInfo = NULL;

    if (pAcpiTkGetObjectInfoAlloc( ObjHandle, &pDevInfo ) != AE_OK)
        return AE_OK;

    do
    {
        if (pDevInfo->Type != ACPI_TYPE_DEVICE)
            break;

        if (!(pDevInfo->Valid & ACPI_VALID_HID))
            break;

        if (!pDevInfo->HardwareId.String)
            break;

        if (strncmp(pDevInfo->HardwareId.String, "ACPI0003", 8) == 0)
        { /* AC Power */
            if (pAcpiTkGetHandle(ObjHandle, "_PSR", &G_ahAC))
                G_ahAC = 0;

            break;
        }

        if (strncmp(pDevInfo->HardwareId.String, "PNP0C0A", 7) == 0)
        { /* Smart battery */
            if (G_uiBatteryCount < MAX_BATTERY_COUNT)
                G_ahBat[G_uiBatteryCount++] = ObjHandle;

            break;
        }
    } while (0);

    if (pDevInfo)
        pAcpiTkOsFree(pDevInfo);

    return AE_OK;
}

/**
 *@@ acpihGetPowerStatus:
 *      Returns power and battery status in caller provided buffers.
 *      Returns zero if success, non-zero if fail.
 *      Code provided by David Azarewicz
 *@@added V1.0.9 (2012-02-20) [slevine]: code from David Azarewicz
 *@@changed V1.0.10 (2014-08-30) [dazarewicz]: tidies
 */

APIRET acpihGetPowerStatus(PAPM pApm, PBOOL pfChanged)
{
    ACPI_STATUS Status;
    ACPI_BUFFER Result;
    ACPI_OBJECT *Obj, Object[20];
    UINT32 uiI;
    ULONG ulTmp, BRemaining, LastFull;
    BOOL fChanged;

    /* Make sure all the functions we need have valid pointers.
     * @@added V1.0.9 (2012-02-25) [dazarewicz]: additional ACPI support
     */
    if (   (pAcpiTkWalkNamespace == NULL)
        || (pAcpiTkGetObjectInfoAlloc == NULL)
        || (pAcpiTkGetHandle == NULL)
        || (pAcpiTkOsFree == NULL)
        || (pAcpiTkEvaluateObject == NULL)
        || (pApm == NULL)
       )
        return 1;

    if (!G_uiAlreadyWalked)
    {
        Status = pAcpiTkWalkNamespace(ACPI_TYPE_DEVICE, ACPI_ROOT_OBJECT,
                                      ACPI_UINT32_MAX, AcpiCallbackWidget,
                                      pApm, NULL);
        G_uiAlreadyWalked = 1;
    }

    fChanged = FALSE;

    // VAC 3.08 long long compatibility support
#ifdef INCL_LONGLONG // VAC 3.6.5 - compiler supports long long
#define OBJECT_VALUE(index)  (Object[index].Integer.Value)
#define OBJ_VALUE(index)  (Obj[index].Integer.Value)
#else // VAC 3.08 - compiler does not support long long
#define OBJECT_VALUE(index)  (Object[index].Integer.Value.ulLo)
#define OBJ_VALUE(index)  (Obj[index].Integer.Value.ulLo)
#endif

    if (G_ahAC)
    {
        // Have _PSR
        Result.Length = sizeof(Object);
        Result.Pointer = Object;
        Status = pAcpiTkEvaluateObject(G_ahAC, NULL, NULL, &Result);
        if (Status != AE_OK)
            ulTmp = 2;                  // assume on backup power
        else if (Object[0].Type != ACPI_TYPE_INTEGER)
            ulTmp = 2;                  // assume on backup power
            else
                ulTmp = (UINT32)OBJECT_VALUE(0);

        if (pApm->fUsingAC != (BYTE) ulTmp)
        {
            pApm->fUsingAC = (BYTE) ulTmp;
            fChanged = TRUE;
        }
    }

    for (uiI=0; uiI < G_uiBatteryCount; uiI++)
    {
        if (G_ahBat[uiI] == 0)
            continue;

        Result.Length = sizeof(Object);
        Result.Pointer = Object;
        // Get battery info
        Status = pAcpiTkEvaluateObject(G_ahBat[uiI], "_BIF", NULL, &Result);
        if (Status != AE_OK)
        {
            G_ahBat[uiI] = 0;
            continue;
        }

        Obj = Result.Pointer;
        Obj = (ACPI_OBJECT *)Obj[0].Package.Elements;   // Battery info package
        LastFull = (UINT32)OBJ_VALUE(2);

        Result.Length = sizeof(Object);
        Result.Pointer = Object;
        // Get battery status
        Status = pAcpiTkEvaluateObject(G_ahBat[uiI], "_BST", NULL, &Result);
        if (Status != AE_OK)
        {
            G_ahBat[uiI] = 0;
            continue;
        }

        Obj = Result.Pointer;
        Obj = (ACPI_OBJECT *)Obj[0].Package.Elements;   // Battery status package
        BRemaining = (UINT32)OBJ_VALUE(2);

        // If battery units are mWh or mAh
        // If not, it is a percentage
        if (    (LastFull != 0xffffffff)
             && (BRemaining != 0xffffffff)
           )
        {
            if (BRemaining > (LastFull >> 1)) // > 50% is high. < 50% is low
                ulTmp = 1; // High
            else
                ulTmp = 2; // Low

            if (OBJ_VALUE(0) & 4)
                ulTmp = 2; // Critical

            // If battery charging - it can't be critical
            if (OBJ_VALUE(0) & 2)
                ulTmp = 3; // Charging

            if (pApm->bBatteryStatus != ulTmp)
            {
                pApm->bBatteryStatus = (BYTE)ulTmp;
                fChanged = TRUE;
            }

            ulTmp = (BRemaining*100) / LastFull;
            if (ulTmp > 100)
                ulTmp = 100;

            if (pApm->bBatteryLife != ulTmp)
            {
                pApm->bBatteryLife = (BYTE) ulTmp;
                fChanged = TRUE;
            }
        }
    }

    if (pfChanged)
        *pfChanged = fChanged;

    pApm->fAlreadyRead = FALSE;
    return 0;

#undef OBJECT_VALUE
#undef OBJ_VALUE
}

/*
 *@@ acpihHasBattery:
 *      quick'n'dirty helper which returns TRUE only
 *      if ACPI is supported on the system and the
 *      system actually has a battery (i.e. is a laptop).
 *      Code provided by David Azarewicz.
 * @@added V1.0.9 (2012-02-20) [slevine]: code from David Azarewicz
 */
BOOL acpihHasBattery(VOID)
{
    BOOL brc = FALSE;
    ACPI_API_HANDLE hACPI;
    APM Apm;

    if (!acpihOpen(&hACPI))
    {
        Apm.bBatteryStatus = 0xff;
        if (!acpihGetPowerStatus(&Apm, NULL))
            brc = (Apm.bBatteryStatus != 0xFF);

        acpihClose(&hACPI);
    }

    return brc;
}


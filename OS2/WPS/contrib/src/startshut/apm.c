
/*
 *@@sourcefile apm.c:
 *      this contains the XFolder APM and ACPI interfaces for automatically
 *      turning the computer off after shutdown has completed.
 *      This is mostly used in the context of XShutdown (shutdown.c).
 *
 *      Massive thanks go out to ARAKAWA Atsushi (arakaw@ibm.net)
 *      for filling this in, and to Roman Stangl (rstangl@vnet.ibm.com)
 *      for finding out all the APM stuff in the first place.
 *
 *@@header "startshut\apm.h"
 */

/*
 *      Copyright (C) 1998 ARAKAWA Atsushi.
 *      Copyright (C) 1997-2003 Ulrich M”ller.
 *      Copyright (C) 2006-2014 Paul Ratcliffe.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC
#define INCL_DOSSPINLOCK
#include <os2.h>

// C library headers
#include <stdio.h>
#include <string.h>

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\apmh.h"               // Advanced Power Management helpers
#include "helpers\acpih.h"              // ACPI helpers

// XWorkplace implementation headers
#include "startshut\apm.h"            // APM power-off for XShutdown

static PAPM            G_pApm;
static ULONG           G_ulAPMStat = APM_UNKNOWN;
static USHORT          G_usAPMVersion = 0;
static CHAR            G_szAPMVersion[10];

static ACPI_API_HANDLE G_hACPI;
static ULONG           G_ulACPIStat = APM_UNKNOWN;
static USHORT          G_ulACPIVersionMajor = 0, G_ulACPIVersionMinor = 0;
static CHAR            G_szACPIVersion[10];

/*
 *@@  apmQueryVersion:
 *      This function returns a PSZ to a static APM
 *      version string, e.g. "1.2".
 *
 *      <B>Usage:</B> any time, esp. on the shutdown
 *      notebook page.
 *
 */

PSZ apmQueryVersion(VOID)
{
    if (G_ulAPMStat == APM_UNKNOWN)
        apmPowerOffSupported();

    sprintf(G_szAPMVersion, "%d.%d", G_usAPMVersion>>8, G_usAPMVersion & 0xff);
    return G_szAPMVersion;
}

/*
 *@@  apmPowerOffSupported:
 *      This function returns TRUE if the computer
 *      supports the APM power-off function.
 *
 *      <B>Usage:</B> any time, especially
 *      by the "XShutdown" page in the Desktop's
 *      settings notebook to determine if the "APM power
 *      off" checkbox will be enabled or not.
 *
 *@@changed V0.9.9 (2001-02-28) [umoeller]: now allowing power-off with APM 1.1 also
 *@@changed V1.0.5 (2006-06-26) [pr]: rewritten to use Helpers
 */

BOOL apmPowerOffSupported(VOID)
{
    if (G_ulAPMStat == APM_UNKNOWN)
    {
        APIRET         arc;

        if ((arc = apmhOpen(&G_pApm)) == NO_ERROR)
        {
            G_usAPMVersion = G_pApm->usLowestAPMVersion;
            apmhClose(&G_pApm);
            // check APM version whether power-off is supported
            if (G_usAPMVersion >= 0x101)  // version 1.1 or above
                G_ulAPMStat = APM_OK;
            else
                G_ulAPMStat = APM_IGNORE;
        }
        else
        {
            // APM.SYS is not loaded
            G_ulAPMStat = APM_IGNORE;
            G_usAPMVersion = 0;
        }
    }

    return (G_ulAPMStat == APM_OK);
}

/*
 *@@ apmPreparePowerOff:
 *      This function is called _once_ by XShutdown while
 *      the system is being shut down, if apmPowerOffSupported
 *      above returned TRUE. This call happens
 *      after all windows have been closed, but
 *      before DosShutdown is called, so you may
 *      open the APM.SYS device driver here and
 *      do other preparations you might need.
 *
 *      After this, you must return _one_ of the
 *      following flags:
 *      --  APM_OK:  go ahead with XShutdown and call
 *                   apmDoPowerOff later.
 *      --  APM_IGNORE: go ahead with XShutdown and do
 *                   _not_ call apmDoPowerOff later;
 *                   this will lead to the normal
 *                   "Press Ctrl+Alt+Del" window.
 *      --  APM_CANCEL: cancel shutdown and present the
 *                   error message which you must
 *                   then copy to pszError, which
 *                   points to a buffer 500 bytes in
 *                   size.
 *
 *      ORed with _one_ or _none_ of the following:
 *      --  APM_DOSSHUTDOWN_0:
 *                   set this flag if XFolder should
 *                   call DosShutdown(0); this is _not_
 *                   recommended, because this would block
 *                   any subsequent DosDevIOCtl calls.
 *      --  APM_DOSSHUTDOWN_1:
 *                   the same for DosShutdown(1), which is
 *                   recommended.
 *
 *      If you return APM_OK only without either
 *      APM_DOSSHUTDOWN_0 or APM_DOSSHUTDOWN_1,
 *      XFolder will call apmDoPowerOff later,
 *      but without having called DosShutdown.
 *      You MUST do this yourself then.
 *      This will however prevent XFolder from
 *      presenting the proper informational windows
 *      to the user.
 *
 *      The buffer that pszError points to is only
 *      evaluated if you return APM_CANCEL.
 *
 *@@changed V1.0.5 (2006-06-26) [pr]: rewritten to use Helpers
 *@@changed V1.0.9 (2010-07-18) [pr]: ignore Enable errors for broken Virtualbox @@fixes 1185
 */

ULONG apmPreparePowerOff(PSZ pszError)      // in: error message
{
    APIRET         arc;

    if ((arc = apmhOpen(&G_pApm)) == NO_ERROR)
    {
        SENDPOWEREVENT  sendpowerevent;

        // enable APM feature
        memset(&sendpowerevent, 0, sizeof(sendpowerevent));
        sendpowerevent.usSubID = POWER_SUBID_ENABLE_APM;
        if ((arc = apmhIOCtl(G_pApm->hfAPMSys,
                        POWER_SENDPOWEREVENT,
                        &sendpowerevent,
                        sizeof(sendpowerevent))) != NO_ERROR)
        {
            strcpy(pszError, "Cannot enable APM.");
            // return APM_CANCEL;
        }
    }
    else
    {
        strcpy(pszError, "Cannot open APM driver.");
        return APM_CANCEL;
    }

    return APM_OK | APM_DOSSHUTDOWN_1;
}

/*
 *@@ apmDoPowerOff:
 *      If apmPreparePowerOff returned with the APM_OK flag set,
 *      XFolder calls this function after it is
 *      done with XShutdown. In this function,
 *      you should call the APM function which
 *      turns off the computer's power. If you
 *      have not specified one of the APM_DOSSHUTDOWN
 *      flags in apmPreparePowerOff, you must call DosShutdown
 *      yourself here.
 *
 *      <P><B>Usage:</B>
 *      only once after XShutdown.
 *
 *      <P><B>Parameters:</B>
 *      none.
 *      </LL>
 *
 *@@changed V0.9.2 (2000-03-04) [umoeller]: added "APM delay" support
 *@@changed V1.0.5 (2006-06-26) [pr]: rewritten to use Helpers
 */

VOID apmDoPowerOff(VOID)
{
    SENDPOWEREVENT  sendpowerevent;

    memset(&sendpowerevent, 0, sizeof(sendpowerevent));
    sendpowerevent.usSubID = POWER_SUBID_SET_POWER_STATE;
    sendpowerevent.usData1 = POWER_DEVID_ALL_DEVICES;
    sendpowerevent.usData2 = POWER_STATE_OFF;
    apmhIOCtl(G_pApm->hfAPMSys,
              POWER_SENDPOWEREVENT,
              &sendpowerevent,
              sizeof(sendpowerevent));
    apmhClose(&G_pApm);
}

/*
 *@@  acpiQueryVersion:
 *      This function returns a PSZ to a static ACPI
 *      version string, e.g. "2.09".
 *
 *      <B>Usage:</B> any time, esp. on the shutdown
 *      notebook page.
 *
 *@@added V1.0.5 (2006-06-26) [pr]
 */

PSZ acpiQueryVersion(VOID)
{
    if (G_ulACPIStat == APM_UNKNOWN)
        acpiPowerOffSupported();

    sprintf(G_szACPIVersion, "%d.%02d", G_ulACPIVersionMajor, G_ulACPIVersionMinor);
    return G_szACPIVersion;
}

/*
 *@@  acpiPowerOffSupported:
 *      This function returns TRUE if the computer
 *      supports the ACPI power-off function.
 *
 *      <B>Usage:</B> any time, especially
 *      by the "XShutdown" page in the Desktop's
 *      settings notebook to determine if the "ACPI power
 *      off" radiobutton will be enabled or not.
 *
 *@@added V1.0.5 (2006-06-26) [pr]
 */

BOOL acpiPowerOffSupported(VOID)
{
    if (G_ulACPIStat == APM_UNKNOWN)
    {
        APIRET         arc;

        arc = acpihOpen(&G_hACPI);
        if (arc == NO_ERROR)
        {
            G_ulACPIVersionMajor = G_hACPI.PSD.Major;
            G_ulACPIVersionMinor = G_hACPI.PSD.Minor;
            G_ulACPIStat = APM_OK;
            acpihClose(&G_hACPI);
        }
        else
        {
            G_ulACPIStat = APM_IGNORE;
            G_ulACPIVersionMajor = G_ulACPIVersionMinor = 0;
        }
    }

    return (G_ulACPIStat == APM_OK);
}

/*
 *@@ acpiPreparePowerOff:
 *      This function is called _once_ by XShutdown while
 *      the system is being shut down, if acpiPowerOffSupported
 *      above returned TRUE. This call happens
 *      after all windows have been closed, but
 *      before DosShutdown is called.
 *
 *      After this, you must return _one_ of the
 *      following flags:
 *      --  APM_OK:  go ahead with XShutdown and call
 *                   acpiDoPowerOff later.
 *      --  APM_IGNORE: go ahead with XShutdown and do
 *                   _not_ call acpiDoPowerOff later;
 *                   this will lead to the normal
 *                   "Press Ctrl+Alt+Del" window.
 *      --  APM_CANCEL: cancel shutdown and present the
 *                   error message which you must
 *                   then copy to pszError, which
 *                   points to a buffer 500 bytes in
 *                   size.
 *
 *      ORed with _one_ or _none_ of the following:
 *      --  APM_DOSSHUTDOWN_0:
 *                   set this flag if XFolder should
 *                   call DosShutdown(0); this is
 *                   recommended for ACPI, because it
 *                   ensures that the file system caches
 *                   are flushed to disk.
 *      --  APM_DOSSHUTDOWN_1:
 *                   the same for DosShutdown(1), which is
 *                   not recommended for ACPI as it is sure
 *                   to leave the file systems dirty.
 *
 *      If you return APM_OK only without either
 *      APM_DOSSHUTDOWN_0 or APM_DOSSHUTDOWN_1,
 *      XFolder will call acpiDoPowerOff later,
 *      but without having called DosShutdown.
 *      You MUST do this yourself then.
 *      This will however prevent XFolder from
 *      presenting the proper informational windows
 *      to the user.
 *
 *      The buffer that pszError points to is only
 *      evaluated if you return APM_CANCEL.
 *
 *@@added V1.0.5 (2006-06-26) [pr]
 *@@changed V1.0.8 (2007-04-13) [pr]: use APM_DOSSHUTDOWN_0 @@fixes 726
 *@@changed V1.0.10 (2014-08-30) [dazarewicz]: Call ACPI prepare to sleep func.
 */

ULONG acpiPreparePowerOff(PSZ pszError)      // in: error message
{
    APIRET         arc;

    if ((arc = acpihOpen(&G_hACPI)) != NO_ERROR)
    {
        strcpy(pszError, "Cannot open ACPI driver.");
        return APM_CANCEL;
    }

    acpihPrepareToSleep(ACPI_STATE_S5);
    return APM_OK | APM_DOSSHUTDOWN_0;
}

/*
 *@@ acpiDoPowerOff:
 *      If acpiPreparePowerOff returned with the APM_OK flag set,
 *      XFolder calls this function after it is
 *      done with XShutdown. In this function,
 *      you should call the ACPI function which
 *      turns off the computer's power. If you
 *      have not specified one of the APM_DOSSHUTDOWN
 *      flags in acpiPreparePowerOff, you must call DosShutdown
 *      yourself here.
 *
 *      <P><B>Usage:</B>
 *      only once after XShutdown.
 *
 *      <P><B>Parameters:</B>
 *      none.
 *      </LL>
 *
 *@@added V1.0.5 (2006-06-26) [pr]
 *@@changed V1.0.10 (2012-04-27) [pr]: turn off all CPUs apart from CPU 1
 */

VOID acpiDoPowerOff(VOID)
{
     ULONG i, ulCpuCount = 1;

     /* Set all CPUs to OffLine, except CPU 1 */
     DosQuerySysInfo(QSV_NUMPROCESSORS, QSV_NUMPROCESSORS, &ulCpuCount, sizeof(ulCpuCount));
     for (i = 2; i <= ulCpuCount; i++)
         DosSetProcessorStatus(i, PROC_OFFLINE);

    DosSleep(250);  // @@changed V1.0.9 (2009-05-29) [shl]: ensure system really has time to go idle
    acpihGoToSleep(&G_hACPI, ACPI_STATE_S5);
    acpihClose(&G_hACPI);
}


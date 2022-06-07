
/*
 *@@sourcefile lan.c:
 *      LAN helpers.
 *
 *      Usage: All OS/2 programs.
 *
 *      Function prefixes (new with V0.81):
 *      --  nls*        LAN helpers
 *
 *      This file is new with 0.9.16.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@header "helpers\nls.h"
 *@@added V0.9.16 (2001-10-19) [umoeller]
 */

/*
 *      Copyright (C) 2001 Ulrich M”ller.
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

#define INCL_DOS
#include <os2.h>

// #define PURE_32
// #include <neterr.h>
// #include <netcons.h>
// #include <server.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "setup.h"                      // code generation and debugging options

#include "helpers\dosh.h"
#include "helpers\lan.h"
#include "helpers\standards.h"

#pragma hdrstop

/*
 *@@category: Helpers\Network helpers
 *      See lan.c.
 */

/* ******************************************************************
 *
 *   Prototypes
 *
 ********************************************************************/

#define API32_FUNCTION APIRET APIENTRY

typedef API32_FUNCTION NET32WKSTAGETINFO(PCSZ pszServer,
                                         unsigned long ulLevel,
                                         PBYTE pbBuffer,
                                         unsigned long ulBuffer,
                                         unsigned long *pulTotalAvail);

typedef API32_FUNCTION NET32SERVERENUM2(PCSZ pszServer,
                                        unsigned long ulLevel,
                                        PBYTE pbBuffer,
                                        unsigned long cbBuffer,
                                        unsigned long *pcEntriesRead,
                                        unsigned long *pcTotalAvail,
                                        unsigned long flServerType,
                                        PBYTE pszDomain);

typedef API32_FUNCTION NET32SERVICESTATUS(PCSZ pbBuffer,
                                          unsigned long ulBuffer);

typedef API32_FUNCTION NET32SERVICEGETINFO(PCSZ pszServer,
                                           PCSZ pszService,
                                           unsigned long ulLevel,
                                           PBYTE pbBuffer,
                                           unsigned long ulBuffer,
                                           unsigned long *pulTotalAvail);

typedef API32_FUNCTION NET32SERVICECONTROL(PCSZ pszServer,
                                          PCSZ pszService,
                                          unsigned short usOpcode,
                                          unsigned short usArg,
                                          PBYTE pbBuffer,
                                          unsigned long ulBuffer);

typedef API32_FUNCTION NET32SERVICEINSTALL(PCSZ pszServer,
                                           PCSZ pszService,
                                           PCSZ pszCmdArgs,
                                           PBYTE pbBuffer,
                                           unsigned long ulBuffer);

/* ******************************************************************
 *
 *   Globals
 *
 ********************************************************************/

HMODULE         hmodLan = NULLHANDLE;

NET32WKSTAGETINFO *pNet32WkstaGetInfo = NULL;
NET32SERVERENUM2 *pNet32ServerEnum2 = NULL;
NET32SERVICESTATUS *pNet32ServiceStatus = NULL;
NET32SERVICEGETINFO *pNet32ServiceGetInfo = NULL;
NET32SERVICECONTROL *pNet32ServiceControl = NULL;
NET32SERVICEINSTALL *pNet32ServiceInstall = NULL;

RESOLVEFUNCTION NetResolves[] =
    {
        "Net32WkstaGetInfo", (PFN*)&pNet32WkstaGetInfo,
        "Net32ServerEnum2", (PFN*)&pNet32ServerEnum2,
        "Net32ServiceStatus", (PFN*)&pNet32ServiceStatus,
        "Net32ServiceGetInfo", (PFN*)&pNet32ServiceGetInfo,
        "Net32ServiceControl", (PFN*)&pNet32ServiceControl,
        "Net32ServiceInstall", (PFN*)&pNet32ServiceInstall,
    };

STATIC ULONG    G_ulNetsResolved = -1;      // -1 == not init'd
                                            // 0  == success
                                            // APIRET == failure

/*
 *@@ lanInit:
 *      initializes the LAN server interface.
 *
 *      Returns:
 *
 *      --  NO_ERROR: functions resolved successfully,
 *          engine is ready.
 *
 *      or the error codes from doshResolveImports.
 */

APIRET lanInit(VOID)
{
    if (G_ulNetsResolved == -1)
    {
        G_ulNetsResolved = doshResolveImports("NETAPI32",   // \MUGLIB\DLL
                                              &hmodLan,
                                              NetResolves,
                                              ARRAYITEMCOUNT(NetResolves));
    }

    return G_ulNetsResolved;
}

/*
 *@@ lanQueryServers:
 *      returns an array of SERVER structures describing
 *      the servers which are available from this computer.
 *
 *      With PEER, this should return the peers, assuming
 *      that remote IBMLAN.INI's do not contain "srvhidden = yes".
 *
 *      If NO_ERROR is returned, the caller should free()
 *      the returned array
 */

APIRET lanQueryServers(PSERVER *paServers,      // out: array of SERVER structs
                       ULONG *pcServers)        // out: array item count (NOT array size)
{
    APIRET arc;

    if (!(arc = lanInit()))
    {
        ULONG   ulEntriesRead = 0,
                cTotalAvail = 0;
        PSERVER pBuf;
        ULONG cb = 4096;            // set this fixed, can't get it to work otherwise
        if (pBuf = (PSERVER)doshMalloc(cb,
                                       &arc))
        {
            if (!(arc = pNet32ServerEnum2(NULL,
                                          1,          // ulLevel
                                          (PBYTE)pBuf,       // pbBuffer
                                          cb,          // cbBuffer,
                                          &ulEntriesRead, // *pcEntriesRead,
                                          &cTotalAvail,   // *pcTotalAvail,
                                          SV_TYPE_ALL,    // all servers
                                          NULL)))      // pszDomain == all domains
            {
                *pcServers = ulEntriesRead;
                *paServers = pBuf;
            }
            else
                free(pBuf);
        }
    }

    printf(__FUNCTION__ ": arc %d\n", arc);

    return arc;
}

/*
 *@@ lanServiceGetInfo:
 *      queries the given service.
 *
 *      If NO_ERROR is returned,
 +
 +          SERVICEBUF.svci2_status & SERVICE_INSTALL_STATE
 +
 *      can be queried for the service state, which should
 *      be one of the following:
 *
 *      --  SERVICE_UNINSTALLED: not running. For the
 *          REQUESTER service, this is only a theoretical
 *          value because without it, NERR_WkstaNotStarted
 *          (2138) is returned.
 *
 *      --  SERVICE_INSTALL_PENDING: start in progress
 *
 *      --  SERVICE_UNINSTALL_PENDING: stop in progress
 *
 *      --  SERVICE_INSTALLED: running
 *
 *      Alternatively, call lanServiceControl with the
 *      SERVICE_CTRL_INTERROGATE code, which actually asks the
 *      service.
 *
 *      Returns, among others:
 *
 *      --  NO_ERROR
 *
 *      --  NERR_WkstaNotStarted (2138): requester is not
 *          running.
 *
 *      --  NERR_ServiceNotInstalled: requested service is
 *          not running.
 *
 *@@added V1.0.0 (2002-09-24) [umoeller]
 */

APIRET lanServiceGetInfo(PCSZ pcszServiceName,
                         PSERVICEBUF2 pBuf)
{
    APIRET  arc;

    if (!(arc = lanInit()))
    {
        ULONG   ulBytesAvail = 0;
        arc = pNet32ServiceGetInfo(NULL,
                                   pcszServiceName,
                                   2,        // level
                                   (PBYTE)pBuf,
                                   sizeof(SERVICEBUF2),
                                   &ulBytesAvail);

    }

    return arc;
}
/*
 *@@ lanServiceInstall:
 *      starts the given service. The service name
 *      must be fully qualified so you cannot
 *      abbreviate "requester" with "req", for
 *      example (as valid with the NET START command).
 *
 *      The name of the service is found in the IBMLAN.INI file.
 *      The executable file name of the service is matched to a
 *      corresponding entry in the Services section of the
 *      IBMLAN.INI file. Any relative file path name supplied
 *      for a service is assumed to be relative to the LAN
 *      Server root directory (\IBMLAN).
 *
 *      #Net32ServiceInstall supports a cmdargs argument, which
 *      is presently always passed as NULL with this implementation.
 *
 *      Returns, among others:
 *
 *      --  NO_ERROR
 *
 *      --  NERR_ServiceInstalled (2182): the service
 *          has already been started.
 *
 *      --  NERR_BadServiceName (2185): invalid service
 *          name.
 *
 *@@added V1.0.0 (2002-09-24) [umoeller]
 */

APIRET lanServiceInstall(PCSZ pcszServiceName,
                         PSERVICEBUF2 pBuf2)        // out: service data
{
    APIRET  arc;

    if (!(arc = lanInit()))
    {
        SERVICEBUF2 buf2;

        if (!(arc = pNet32ServiceInstall(NULL,
                                         pcszServiceName,
                                         NULL,
                                         (PBYTE)pBuf2,
                                         sizeof(SERVICEBUF2))))
            ;
    }

    return arc;
}

/*
 *@@ lanServiceControl:
 *      queries, pauses, resumes, or stops the given service. This
 *      has the functionality of the NET STOP command.
 *
 *      opcode must be one of:
 *
 *      --  SERVICE_CTRL_INTERROGATE (0): interrogate service status.
 *          This is similar to running lanServiceGetInfo, except
 *          that this one actually asks the service for its status,
 *          while lanServiceGetInfo simply dumps the status last
 *          posted.
 *
 *      --  SERVICE_CTRL_PAUSE (1): pause service.
 *
 *      --  SERVICE_CTRL_CONTINUE (2): continue service.
 *
 *      --  SERVICE_CTRL_UNINSTALL (3): stop service.
 *
 *@@added V1.0.0 (2002-09-24) [umoeller]
 */

APIRET lanServiceControl(PCSZ pcszServiceName,
                         ULONG opcode,
                         PSERVICEBUF2 pBuf2)        // out: service data
{
    APIRET  arc;

    if (!(arc = lanInit()))
    {
       if (!(arc = pNet32ServiceControl(NULL,
                                        pcszServiceName,
                                        opcode,
                                        0,
                                        (PBYTE)pBuf2,
                                        sizeof(SERVICEBUF2))))
            ;

    }

    return arc;
}

#ifdef __LAN_TEST__

int main(void)
{
    ULONG ul;

    PSERVER paServers;
    ULONG cServers;
    lanQueryServers(&paServers, &cServers);

    for (ul = 0;
         ul < cServers;
         ul++)
    {
        printf("Server %d: \\\\%s (%s)\n",
               ul,
               paServers[ul].achServerName,
               paServers[ul].pszComment);
    }

    return 0;
}

#endif


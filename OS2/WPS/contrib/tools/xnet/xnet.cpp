
/*
 *  xnet.c:
 *      source code for xnet.exe.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#include <os2.h>

// #define PURE_32
// #include <neterr.h>
// #include <netcons.h>
// #include <server.h>

#include "setup.h"
#include "bldlevel.h"

#include "helpers\dosh.h"
#include "helpers\lan.h"

VOID DumpFlags(PSERVICEBUF2 pBuf2)
{
    #define DUMPFLAG(f) printf("%s: %d\n", # f, !!(pBuf2->svci2_status & f))

    DUMPFLAG(SERVICE_UNINSTALLED);
    DUMPFLAG(SERVICE_INSTALL_PENDING);
    DUMPFLAG(SERVICE_UNINSTALL_PENDING);
    DUMPFLAG(SERVICE_INSTALLED);
    DUMPFLAG(SERVICE_ACTIVE);
    DUMPFLAG(SERVICE_CONTINUE_PENDING);
    DUMPFLAG(SERVICE_PAUSE_PENDING);
    DUMPFLAG(SERVICE_PAUSED);
    DUMPFLAG(SERVICE_NOT_UNINSTALLABLE);
    DUMPFLAG(SERVICE_UNINSTALLABLE);
    DUMPFLAG(SERVICE_NOT_PAUSABLE);
    DUMPFLAG(SERVICE_PAUSABLE);
    DUMPFLAG(SERVICE_REDIR_DISK_PAUSED);
    DUMPFLAG(SERVICE_REDIR_PRINT_PAUSED);
    DUMPFLAG(SERVICE_REDIR_COMM_PAUSED);
}

/*
 *@@ ProcessStart:
 *
 */

APIRET ProcessStart(PCSZ pcszService)
{
    APIRET  arc = NO_ERROR;

    printf("Starting %s...\n", pcszService);

    SERVICEBUF2 buf2;

    if (arc = lanServiceInstall(pcszService, &buf2))
        printf("Error %d starting service \"%s\"\n",
               arc,
               pcszService);
    else
    {
        printf("Service \"%s\" was successfully started (PID 0x%lX)\n",
               buf2.svci2_name,
               buf2.svci2_pid);
    }

    return arc;
}

/*
 *@@ ProcessQuery:
 *
 */

APIRET ProcessQuery(PCSZ pcszService)
{
    APIRET  arc = NO_ERROR;

    SERVICEBUF2 buf2;

    // if (arc = lanServiceGetInfo(pcszService, &buf2))
    if (arc = lanServiceControl(pcszService,
                                SERVICE_CTRL_INTERROGATE,
                                &buf2))
        printf("Error %d getting info for service \"%s\"\n",
               arc,
               pcszService);
    else
    {
        PCSZ    pcszInstallState;
        switch (buf2.svci2_status & SERVICE_INSTALL_STATE)
        {
            case SERVICE_UNINSTALLED:
                pcszInstallState = "not running";
            break;

            case SERVICE_INSTALL_PENDING:
                pcszInstallState = "start in progress";
            break;

            case SERVICE_UNINSTALL_PENDING:
                pcszInstallState = "stop in progress";
            break;

            case SERVICE_INSTALLED:
                pcszInstallState = "running";
            break;
        }

        printf("Service \"%s\" (PID 0x%lX): %s\n",
               buf2.svci2_name,
               buf2.svci2_pid,
               pcszInstallState);
    }

    return arc;
}

/*
 *@@ ProcessStop:
 *
 */

APIRET ProcessStop(PCSZ pcszService)
{
    APIRET  arc = NO_ERROR;

    printf("Stopping %s...\n", pcszService);

    SERVICEBUF2 buf2;

    if (arc = lanServiceControl(pcszService,
                                SERVICE_CTRL_UNINSTALL,
                                &buf2))
        printf("Error %d getting info for service \"%s\"\n",
               arc,
               pcszService);
    else
    {
        printf("Service \"%s\" was successfully stopped (PID 0x%lX)\n",
               buf2.svci2_name,
               buf2.svci2_pid);
    }

    return arc;
}

/*
 *@@ main:
 *
 */

int main(int argc, char* argv[])
{
    APIRET  arc = NO_ERROR;

    if (argc < 2)
    {
        printf("xnet V" BLDLEVEL_VERSION " built " __DATE__ "\n"
               "(C) 2002 Ulrich Moeller\n"
               "Usage: xnet <mode> <args>\n"
               "-- xnet start <service> [<service>...]\n"
               "-- xnet stop <service> [<service>...]\n"
               "-- xnet query <service> [<service>...]\n");
        exit(-1);
    }

    int i = 1;

    enum enCommandMode
    {
        CMODE_NONE,
        CMODE_START,
        CMODE_STOP,
        CMODE_QUERY
    } cm = CMODE_NONE;

    while (i < argc)
    {
        PCSZ    pcszArgThis = argv[i];

        if (!stricmp(pcszArgThis, "START"))
            cm = CMODE_START;
        else if (!stricmp(pcszArgThis, "STOP"))
            cm = CMODE_STOP;
        else if (!stricmp(pcszArgThis, "QUERY"))
            cm = CMODE_QUERY;
        else
            // additional parameters depending on command mode
            switch (cm)
            {
                case CMODE_START:
                    arc = ProcessStart(pcszArgThis);
                break;

                case CMODE_STOP:
                    arc = ProcessStop(pcszArgThis);
                break;

                case CMODE_QUERY:
                    arc = ProcessQuery(pcszArgThis);
                break;

                default:
                    printf("Unknown command \"%s\".\n", pcszArgThis);
                    arc = -1;
                break;
            }

        if (arc)
            break;

        ++i;
    }

    if (arc)
    {
        PSZ pszError;
        if (pszError = doshQuerySysErrorMsg(arc))
        {
            printf("%s\n", pszError);
            free(pszError);
        }
        else
            printf("Unknown error %d occurred.\n", arc);
    }

    return arc;
}

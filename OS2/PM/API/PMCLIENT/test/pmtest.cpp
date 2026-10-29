/* pmtest.cpp */
/* Minimal FreePM test client.

   Statically linked against PMCLIENT.LIB. PMCLIENT.DLL is loaded
   automatically by the OS at process start; its DLL entry point
   establishes the connection to the server. If the server is not
   reachable, PMCLIENT.DLL fails to load and PMTEST.EXE does not
   start (standard OS/2 / Win32 behaviour).

   Sends a few commands through PmcSendCommand and prints the
   32-bit replies. With the stub PMSERVER handler the reply is the
   command code itself. */

#include <stdio.h>
#include "pmclient.h"

int main(int argc, char *argv[])
{
    APIRET rc;

    (void)argc;
    (void)argv;

    printf("PMTEST: starting\n");
    fflush(stdout);

    printf("PMTEST: PmcSendCommand(0x100, 0x42)\n");
    rc = PmcSendCommand(0x100, 0x42);
    printf("PMTEST:   reply = 0x%lx\n", (unsigned long)rc);

    printf("PMTEST: PmcSendCommand(0x200, 0x07)\n");
    rc = PmcSendCommand(0x200, 0x07);
    printf("PMTEST:   reply = 0x%lx\n", (unsigned long)rc);

    printf("PMTEST: PmcSendCommand(0x300, 0x00)\n");
    rc = PmcSendCommand(0x300, 0x00);
    printf("PMTEST:   reply = 0x%lx\n", (unsigned long)rc);

    printf("PMTEST: done\n");
    return 0;
}

/***************************************************************************
 *      Created 2005  eCo Software                                         *
 *                                                                         *
 *      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is        *
 *      sample code created by eCo Software. This sample code is not part  *
 *      of any standard or eCo Software product and is provided to you     *
 *      solely for the purpose of assisting you in the development of your *
 *      applications.  The code is provided "AS IS", without               *
 *      warranty of any kind. eCo Software shall not be liable for any     *
 *      damages arising out of your use of the sample code, even if they   *
 *      have been advised of the possibility of such damages.              *
 *-------------------------------------------------------------------------*/
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_BASE
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSSPINLOCK
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acpi.h>
#include <acpiapi.h>

void
main(
    int  argc,
    char **argv)
{
    APIRET rc;
    ACPI_API_HANDLE Hdl;
    ACPI_TK_VERSION ApiBuffer;

    if (AcpiTkValidateVersion(ACPI_TK_VERSION_MAJOR, ACPI_TK_VERSION_MINOR)) {
        printf("This program is not compatible with the version of ACPI that is installed.\n");

        /* display the version information */
        ApiBuffer.Size = sizeof(ACPI_TK_VERSION);
        if (AcpiTkGetVersion(&ApiBuffer)) exit(1);
        if (ApiBuffer.Size != sizeof(ACPI_TK_VERSION)) exit(1);

        printf("The version of ACPI installed is %u.%u\n", ApiBuffer.PSD.Major, ApiBuffer.PSD.Minor);
        printf("The API version is %u.%u, but version %u.%u is needed.\n",
            ApiBuffer.Api.Major, ApiBuffer.Api.Minor, ACPI_TK_VERSION_MAJOR, ACPI_TK_VERSION_MINOR);
    }

    rc = AcpiTkStartApi(&Hdl);
    if (rc)
    {
        printf("acpi start failed rc=%d 0x%x\n",rc,rc);
	    exit(1);
    }

    AcpiTkPrepareToSleep(ACPI_STATE_S5);
    rc = DosShutdown(0L);
    DosSleep(3000); /* Wait allows shutdown to complete */
    rc = AcpiTkGoToSleep(&Hdl, ACPI_STATE_S5);
    printf("AcpiTkGoToSleep failed rc = %d 0x%x\n", rc, rc);
}

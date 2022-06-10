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
#define INCL_DOSPROCESS
#include <os2.h>
#include "acpi.h"
#include "acpiapi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void main(
    void)
{
    APIRET          rc;
    ACPI_API_HANDLE Hdl;
    ULONG           State;
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
        printf("Error start acpi rc=%d 0x%x\n",rc,rc);
        return;
    }

    printf("Start rc=%d Wait Press BUTTON  10000ms\n",rc);
    rc = AcpiTkWaitPressButton(&Hdl,SET_POWERBUTTON,10000,&State);

    printf("Wait Power rc=%d State=%d\n",rc,State);
    printf("Start rc=%d Wait Press SLEEP  10000ms\n",rc);
    rc = AcpiTkWaitPressButton(&Hdl,SET_SLEEPBUTTON,10000,&State);

    printf("Wait Sleep rc=%d State=%d\n",rc,State);
    rc = AcpiTkEndApi(&Hdl);
    printf("End rc=%d\n",rc);
}

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

void
main(
   int  argc,
   char **argv)
{
    APIRET                rc;
    ACPI_API_HANDLE       Hdl;
    UINT32                Number,Function;
    ACPI_PHYSICAL_ADDRESS Address;
    ACPI_INTEGER          Value = 0;
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
        printf("Error open ACPI API rc=%d\n",rc);
        return;
    }
    if (argc < 4)
    {
        printf("Usage: EmbeddedRW NumEC Function Address [Value]\n");
        rc = AcpiTkEndApi(&Hdl);
        return;
    }

    Number   = atoi( argv[1] );
    Function = atoi( argv[2] );
    Address  = atoi( argv[3] );
    if (Function)
        Value = atoi( argv[4] );
    rc = AcpiTkRWEmbedded(&Hdl,Number,Function,Address, &Value);

    if (Function)
        printf ("Write to address 0x%x value 0x%x rc=%d\n",Address,(UINT32)Value,rc);
    else
        printf ("Read from address 0x%x value 0x%x rc=%d\n",Address,(UINT32)Value,rc);

    rc = AcpiTkEndApi(&Hdl);
    rc = AcpiTkRWEmbedded(&Hdl,0,0,0x31, &Value);
    exit(0);
}

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
#include <os2.h>
#include <acpi.h>
#include <acpiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
main(
    void)
{
    APIRET          rc;
    ACPI_API_HANDLE Hdl;
    OS2_ACPI_STATISTICS Stat;
    PAcpiIRQ        IRQ;
    ULONG           i, Num;
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
        printf("Error open ACPI API rc=%d - 0x%x\n",rc,rc);
        exit(1);
    }

    Stat.Command = ACPI_STATISTICS_GETSIZEIRQ;
    rc = AcpiTkGetStatistics (&Hdl, &Stat);
    if (rc)
    {
        printf("Error open Stat rc=%d - %x\n",rc,rc);
        exit(1);
    }

    Num = Stat.Number;
    printf("Use %d IRQs\n",Stat.Number);
    if (Num)
    {
       IRQ = Stat.Data = malloc(Num * sizeof(AcpiIRQ) );
       Stat.Command    = ACPI_STATISTICS_GETIRQ;
       rc = AcpiTkGetStatistics(&Hdl,&Stat);
       if (rc)
       {
           printf("Error open ACPI API rc=%d\n",rc);
           exit(1);
       }
       for (i = 0; i < Stat.Number; i++)
       {
            if (IRQ[i].Count)
            {
                printf("IRQ%2.2d called %lld\n",i,IRQ[i].Count);
            }
       }
    }

    rc = AcpiTkEndApi(&Hdl);
}

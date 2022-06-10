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
#include <time.h>

void
main(
    void)
{
    ACPI_STATUS     Status;
    APIRET          rc;
    ACPI_API_HANDLE Hdl;
    ACPI_HANDLE     NotifyHandle;
    UINT32          NotifyNumber;
    ACPI_BUFFER     Buffer;
    char            achName[5];
    time_t          ltime;
    struct tm       *tp;
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
        printf("Error open ACPI rc=%d\n",rc);
        return;
    }

    while (1)
    {
        NotifyHandle = 0; /* request any handle */
        NotifyNumber = 0; /* request any number */
        rc = AcpiTkWaitNotify(&Hdl,&NotifyHandle, &NotifyNumber, ACPI_ALL_NOTIFY, -1);
        if (rc)
        {
            printf("Error wait rc = %d\n",rc);
            break;
        }

        memset(achName, 0, 5);
        Buffer.Length  = 5;
        Buffer.Pointer = achName;
        if (NotifyHandle)
        {
            time(&ltime);
            tp = localtime (&ltime);
            AcpiTkGetName(NotifyHandle, ACPI_SINGLE_NAME, &Buffer);
            printf ("%02i:%02i:%02i ", tp->tm_hour, tp->tm_min, tp->tm_sec);
            printf("Device \"%s\" ",achName);
            printf("Event Number: 0x%x\n",NotifyNumber);
        }
    }
}

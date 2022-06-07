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
#include <acpi.h>
#include <accommon.h>
#include <acpiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MICROSECONS_IN_DAY   ((UINT64)24*(UINT64)60*(UINT64)60*(UINT64)1000*(UINT64)1000)
#define MICROSECONS_IN_HOUR  ((UINT64)60*(UINT64)60*(UINT64)1000*(UINT64)1000)
#define MICROSECONS_IN_MIN   ((UINT64)60*(UINT64)1000*(UINT64)1000)
#define MICROSECONS_IN_SEC   ((UINT64)1000*(UINT64)1000)

void main(
    void)
{

    ACPI_STATUS     Status;
    UINT32          Ticks, Resolution, MaxTickLimit;
    UINT64          Calls, TimesInCall, TimeAll;
    APIRET          rc;
    ACPI_API_HANDLE Hdl;
    UINT64          Day, Hour, Min, Sec, Msec, MkSec;
    UINT64          TimeElapsed;
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

    Status=AcpiTkGetTimerResolution(&Resolution);
    if (Status != AE_OK)
    {
        printf("AcpiTkGetTimerResolution Status=%x\n",Status);
        return;
    }

    rc = AcpiTkGetUpTime(&Hdl,&Calls,&Ticks);

    printf("Tics %d\n",Ticks);

    MaxTickLimit = (UINT32)(  (1 << ((UINT8)Resolution-1) ));
    TimeElapsed  = (Ticks & (MaxTickLimit-1) ); //0x7fffffff
    TimeAll      = ((((Calls << ((UINT8) Resolution-1)) + TimeElapsed )) * (UINT64)1000000) / (UINT64)PM_TIMER_FREQUENCY_SEC;

    Day   = TimeAll / MICROSECONS_IN_DAY;
    Hour  = (TimeAll - Day * MICROSECONS_IN_DAY)/MICROSECONS_IN_HOUR;
    Min   = (TimeAll - Day * MICROSECONS_IN_DAY - Hour * MICROSECONS_IN_HOUR)/MICROSECONS_IN_MIN;
    MkSec = (TimeAll - Day * MICROSECONS_IN_DAY - Hour * MICROSECONS_IN_HOUR - Min * MICROSECONS_IN_MIN);
    Sec   = MkSec/MICROSECONS_IN_SEC;
    MkSec = MkSec - Sec * MICROSECONS_IN_SEC;

    if (Day) printf("Uptime: %lldd %lldh %lldm %llds %3lldms",Day, Hour, Min,Sec, MkSec/1000);
    else if (Hour)
             printf("Uptime: %lldh %lldm %llds %3lldms",     Hour, Min,Sec, MkSec/1000);
         else
             printf("Uptime: %lldm %llds %3lldms ",          Min,Sec, MkSec/1000);

 rc = AcpiTkEndApi(&Hdl);

}

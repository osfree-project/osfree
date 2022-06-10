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

void main(
    void)
{
    ACPI_STATUS Status;
    UINT32 StartTicks, EndTicks, Resolution, TimeElapsed, DeltaTicks, TimeElapsed2;
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

    Status = AcpiTkGetTimerResolution(&Resolution);
    printf("Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));
    printf("Timer\'s resolution %d bit\n",Resolution);

    Status = AcpiTkGetTimer(&StartTicks);
    printf("Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));
    printf("Start ticks - %d\n Call DosSleep(1000)...\n",StartTicks);
    DosSleep(1000);

    Status = AcpiTkGetTimer(&EndTicks);
    printf("Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));
    printf("End ticks - %d\n",EndTicks);
    /*
     * Compute Tick Delta:
     * Handle (max one) timer rollovers on 24-bit versus 32-bit timers.
     */
    if (StartTicks < EndTicks)
    {
        DeltaTicks = EndTicks - StartTicks;
    }
    else
        if (StartTicks > EndTicks)
        {
            if (Resolution == 24)
            {            /* 24-bit Timer */
                DeltaTicks = (((0x00FFFFFF - StartTicks) + EndTicks) & 0x00FFFFFF);
            }
            else
            {  /* 32-bit Timer */
               DeltaTicks = (0xFFFFFFFF - StartTicks) + EndTicks;
            }
        }
        else /* StartTicks == EndTicks */
        {
            TimeElapsed  = 0;
            TimeElapsed2 = 0;
        }
    /*
     * Compute Duration (Requires a 64-bit multiply and divide):
     *
     * TimeElapsed = (DeltaTicks * 1000000) / PM_TIMER_FREQUENCY_SEC;
     */
    TimeElapsed  = ( ((UINT64) DeltaTicks) * 1000000LL) / PM_TIMER_FREQUENCY_SEC;
    printf("Time elapsed - %d mks\n",TimeElapsed);

}

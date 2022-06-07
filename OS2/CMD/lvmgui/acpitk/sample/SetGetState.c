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

PROCESSOR   Processor[64];
ACPI_OBJECT Object[100];

ACPI_STATUS
GetProcessor(
    ACPI_HANDLE ObjHandle,
    UINT32 NestingLevel,
    void *Context,
    void **ReturnValue)
{
    ACPI_STATUS       Status;
    ACPI_BUFFER       Buffer, Results;
    ACPI_OBJECT       Obj, *Pss, *pPss;
    char              DevName[5];
    UINT32            a, i, j, Count, Stage, Cnt;

    Buffer.Length  = 5;
    Buffer.Pointer = DevName;
    AcpiTkGetName(ObjHandle, ACPI_SINGLE_NAME, &Buffer);
    printf("==================================\n");
    printf("Processor [");
    for (i = 0; i < 4; i++)
         printf("%c",DevName[i]);
    printf("]\n");

    Results.Length  = sizeof(Obj);
    Results.Pointer = &Obj;
    Status          = AcpiTkEvaluateObject(ObjHandle,"_PPC",NULL,&Results);
    if (Status != AE_OK)
    {
        printf("Not supported or not CPU present\n");
        printf("==================================\n");
        return AE_OK;
    }
    printf("Now use %d state\n",(UINT32)Obj.Integer.Value);

    Results.Length  = sizeof(Object);
    Results.Pointer = &Object;
    Status          = AcpiTkEvaluateObject(ObjHandle,"_PSS",NULL,&Results);
    printf("Status:%d Support %d power state\n",Status,Object[0].Package.Count);
    Count = Object[0].Package.Count;
    pPss  = (ACPI_OBJECT *)Object[0].Package.Elements;
    for (j = 0; j < Count; j++)
    {
         printf("-------Power State %d ----------\n",j);
         Pss = (ACPI_OBJECT *)pPss->Package.Elements;
         printf("Frequence:          %d Mhz\n",Pss->Integer.Value);
         Pss++;
         printf("Power:              %d mWatt\n",Pss->Integer.Value);
         Pss++;
         printf("Bus Master latency: %d uSec\n",Pss->Integer.Value);
         pPss++;
    }

    printf("==================================\n");
    return Status;

}

void
AcpiFindProcessor(
    void)
{
    ACPI_STATUS       Status;

    printf("Searching processors\n");
    Status=AcpiTkWalkNamespace(ACPI_TYPE_PROCESSOR, // All device
                             ACPI_ROOT_OBJECT,    // Start object
                             ACPI_UINT32_MAX,     // Max depth
                             GetProcessor,        // Callback
                             NULL, NULL);         // Callback param and poitner to return value

}

void
main(
    int  argc,
    char **argv)
{
    APIRET          rc;
    ACPI_API_HANDLE Hdl;
    UINT32          State, CpuId, i;
    UINT8           Width;
    MPAFFINITY      af;
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
        exit(1);
    }

    if (argc < 2)
    {
        AcpiFindProcessor();                                         // Finding processors
        rc = AcpiTkEndApi(&Hdl);
        exit(0);
    }
    else if (argc < 3)
         {
             printf("Usage: GetSet [ProcID State]\n");
             exit(0);
         }

    CpuId = atoi(argv[1]);
    State = atoi(argv[2]);
    rc    = AcpiTkSetPowerState(&Hdl,CpuId,State);
    if (rc == 0)
        printf("State set successfully\n");
    else
        printf("State set error rc=%d\n",rc);

   rc=AcpiTkEndApi(&Hdl);
   exit(0);
}


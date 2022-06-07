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

PROCESSOR Processor[64];
UINT32    CPUFinding = 0;

void AcpiFindProcessor(void);

void
main(
    int  argc,
    char **argv)
{
    APIRET          rc;
    ACPI_API_HANDLE Hdl;
    UINT32          Perfomance, CpuId, i;
    UINT8           Width;
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

    memset (Processor, 0, sizeof (PROCESSOR) * 64);
    rc = AcpiTkStartApi(&Hdl);
    if (rc)
    {
        printf("Error open ACPI API rc=%d\n",rc);
        exit(1);
    }

   AcpiFindProcessor();                                         // Finding processors
   if (argc < 2)
   {
       for (i = 0; i < 64; i++)
       {
            if (Processor[i].PblkAddress == 0) continue;
            printf(" CPU  number %d\n",i);
            printf(" ProcId:     0x%x\n",Processor[i].ProcessorId);
            printf(" P_BLK:      0x%x\n",Processor[i].PblkAddress);
            printf(" P_BLK len:  0x%x\n",Processor[i].PblkLength);
            rc = AcpiTkGetThrottling(&Hdl,i,&Perfomance,&Width);
            if (rc)
            {
                printf("CPU%d Error get throttling rc=%d\n",i,rc);
                continue;
            }
            if (Width)
            {
                printf("Min set is %d\n",(100 >> Width) + 1);
            }
            if (Perfomance)
            {
                printf("CPU%d Perfomance=%d %% \n",i,Perfomance);
            }
            else
            {
                printf("CPU%d throttling is disabled\n",i);
            }
            printf("------------------------------------\n");
       }
       rc = AcpiTkEndApi(&Hdl);
       exit(0);
    }
    else if (argc < 3)
         {
             printf("Usage: GetSet [CPUNumber Perfomance]\n Pefomance 0...100 %%\n");
             exit(0);
         }

    CpuId      = atoi(argv[1]);
    Perfomance = atoi(argv[2]);
    rc         = AcpiTkSetThrottling(&Hdl,CpuId,Perfomance);
    if (rc == 0)
        printf("Throttling to %d set succesfull\n",CpuId);
    else
        printf("Throttling set error rc=%d\n",rc);

    rc = AcpiTkEndApi(&Hdl);
    exit(rc);
}

ACPI_STATUS
GetProcessor(
    ACPI_HANDLE ObjHandle,
    UINT32      NestingLevel,
    void        *Context,
    void        **ReturnValue)
{
    ACPI_STATUS Status;
    ACPI_BUFFER Buffer, Results;
    ACPI_OBJECT Obj;
    char        DevName[5];
    UINT32      i;

    Buffer.Length  = 5;
    Buffer.Pointer = DevName;
    AcpiTkGetName(ObjHandle, ACPI_SINGLE_NAME, &Buffer);
    printf("Processor [");
    for (i = 0; i < 4; i++)
    {
         printf("%c",DevName[i]);
    }
    printf("]  ");

    Results.Length  = sizeof(Obj);
    Results.Pointer = &Obj;
    Status          = AcpiTkEvaluateObject(ObjHandle,NULL,NULL,&Results);
    if (Status != AE_OK)                   return Status;
    if (Obj.Type != ACPI_TYPE_PROCESSOR)   return Status;

    printf ("id:%d ", Obj.Processor.ProcId);
    if (Obj.Processor.PblkAddress)
    {
        printf("Possible for throttle setting");
    }
    printf("\n------------------------------------\n");
    Processor[Obj.Processor.ProcId].PblkAddress = Obj.Processor.PblkAddress;
    Processor[Obj.Processor.ProcId].PblkLength  = Obj.Processor.PblkLength;
    Processor[Obj.Processor.ProcId].ProcessorId = Obj.Processor.ProcId;
    CPUFinding++;

    return Status;
}
void
AcpiFindProcessor(
    void)
{
    ACPI_STATUS       Status;
    printf("Searching processors\n");
    printf("------------------------------------\n");
    Status = AcpiTkWalkNamespace(ACPI_TYPE_PROCESSOR, // All device
                               ACPI_ROOT_OBJECT,    // Start object
                               ACPI_UINT32_MAX,     // Max depth
                               GetProcessor,        // Callback
                               NULL, NULL);         // Callback param and poitner to return value

}

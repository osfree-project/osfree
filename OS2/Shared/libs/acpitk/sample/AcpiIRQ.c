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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acpi.h>
#include <accommon.h>
#include <acpiapi.h>

/*
 *  Show ACPI tree with resource
 *  Pavel Shtemenko 14.08.2005
 */
struct _INTx_
      {
      ULONG INTA;
      ULONG INTB;
      ULONG INTC;
      ULONG INTD;
      };
void          AcpiWalkNameSpaceTst(ULONG PIC);
UCHAR         IntPin[] = {'A','B','C','D','E'};
ACPI_OBJECT   Object[1000];

ACPI_STATUS
IRQPrintResourceInfo(
    ACPI_RESOURCE *Resource,
    void          *Context)
{
    ULONG i;
    switch (Resource->Type)
    {
            case ACPI_RESOURCE_TYPE_IRQ:
                 printf("used ");
                 for (i=0; i < Resource->Data.Irq.InterruptCount; i++)
	               printf(" IRQ%-2.d ", Resource->Data.Irq.Interrupts[i]);
                 if (Resource->Data.Irq.InterruptCount)
                 {
                     printf("triggerred by %s, ",Resource->Data.Irq.Triggering == ACPI_LEVEL_SENSITIVE? "Level":"Edge");
                     printf("polarity %s",Resource->Data.Irq.Polarity == ACPI_ACTIVE_LOW? "Low":"High");
                     //if (Resource->Data.Irq.Sharable) printf(", Sharable");
                 }
                 break;
            case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
                 printf("PrCm %d",Resource->Data.ExtendedIrq.ProducerConsumer);
                 for (i=0; i < Resource->Data.ExtendedIrq.InterruptCount; i++)
	               printf(" IRQ%d ", Resource->Data.ExtendedIrq.Interrupts[i]);
                 if (Resource->Data.ExtendedIrq.InterruptCount)
                 {
                     printf("triggerred by %s, ",Resource->Data.ExtendedIrq.Triggering == ACPI_LEVEL_SENSITIVE? "Level":"Edge");
                     printf("polarity %s",Resource->Data.ExtendedIrq.Polarity == ACPI_ACTIVE_LOW? "Low":"High");
                     //if (Resource->Data.ExtendedIrq.Sharable) printf(", Sharable");
                 }
                 printf(" SourceIdx %d",Resource->Data.ExtendedIrq.ResourceSource.Index);
                 if (Resource->Data.ExtendedIrq.ResourceSource.StringLength)
                     printf("%s",Resource->Data.ExtendedIrq.ResourceSource.StringPtr);
                 break;
            default:
                 break;
    }

    return AE_OK;
}

void
main(
    ULONG argc,
    char  *argv[])
{
    ULONG PIC;
    ACPI_TK_VERSION ApiBuffer;

    if (argc < 2 || (PIC=atoi(argv[1])) > 1)
    {
        printf("Usage: %s 0/1\n  Where:\n\t0 - PIC mode\n\t1 - APIC mode\n",argv[0]);
        exit(0);
    }
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
    AcpiWalkNameSpaceTst(PIC);
}

ULONG
PrintPackage(
    ULONG       Count,
    ACPI_OBJECT *oObj)
{
    ULONG       j, i, Cnt;
    ACPI_OBJECT *Obj;
    char        DevName[5];
    ACPI_BUFFER Buffer;

    printf("==================================================\n");
    Obj = (ACPI_OBJECT *)oObj->Package.Elements;
    for (Cnt=0,j = 0; j < Count; j++,Cnt++ ,oObj++)
    {
         Obj = (ACPI_OBJECT *)oObj->Package.Elements;
         printf("   Device  0x%4.4x ", ( Obj->Integer.Value >> 16));
         if ((Obj->Integer.Value & 0xffff) == 0xffff)
            printf("Function ALL ");
         else
            printf("Function 0x%4.4x ",Obj->Integer.Value);
         Obj++;
         printf("   INT%c ",IntPin[Obj->Integer.Value]);
         Obj++;
         printf("   Route to ");
         if (Obj->Reference.Handle)
         {
             Buffer.Length  = 5;
             Buffer.Pointer = DevName;
             AcpiTkGetName(Obj->Reference.Handle, ACPI_SINGLE_NAME, &Buffer);
             printf(" Device [");
             for (i=0; i < 4; i++)
                  printf("%c",DevName[i]);
             printf("] ");
             AcpiTkWalkResources(Obj->Reference.Handle, METHOD_NAME__CRS, IRQPrintResourceInfo, NULL);
             printf("\n");
             Obj++;
         }
         else
         {
            Obj++;
            printf(" IRQ%d\n",Obj->Integer.Value);
         }
         Obj++;
         fflush(stdout);
    }

    printf("==================================================\n");
    return Cnt;
}
/*
 * CallBack function for print object info and walk resource info
 */
ACPI_STATUS
internal_PrintObjectInfo(
    ACPI_HANDLE ObjHandle,
    UINT32 NestingLevel,
    void *Context,
    void **ReturnValue)
{
    int              i;
    ACPI_BUFFER      Buffer;
    ACPI_DEVICE_INFO *DevInfo = NULL;
    char             achName[5];
    ACPI_BUFFER      Results;
    ACPI_OBJECT      Obj, *oObj;
    ACPI_STATUS      Status;

    Results.Length  = sizeof(Object);
    Results.Pointer = &Object;

    if (NestingLevel > 1)
    {
        for (i=0; i < NestingLevel-1 ; i++)
             printf("   ");
    }
    printf("|\n");

    if (NestingLevel > 1)
    {
        for (i=0; i<NestingLevel-1 ; i++)
             printf("   ");
    }

    printf("+*");
    memset(achName, 0, 5);
    Buffer.Length  = 5;
    Buffer.Pointer = achName;
    AcpiTkGetName(ObjHandle, ACPI_SINGLE_NAME, &Buffer);
    printf("%s", achName);   fflush(stdout);

    AcpiTkGetObjectInfoAlloc3(ObjHandle, &DevInfo);
    printf(" T:[%s]", AcpiTkUtGetTypeName(DevInfo->Type));
    if (DevInfo->Valid & ACPI_VALID_HID)
        printf(" HID:[%.9s]", &(DevInfo->HardwareId.String));
    if (DevInfo->Valid & ACPI_VALID_CID)
        printf(" CID:[%s]", &(DevInfo->HardwareId.String));
    if (DevInfo->Valid & ACPI_VALID_UID)
        printf(" UID:[%.9s]", &(DevInfo->UniqueId.String));
    if (DevInfo->Valid & ACPI_VALID_ADR)
        printf(" ADR:[0x%x]", DevInfo->Address);
    if (DevInfo->Valid & ACPI_VALID_SXDS)
    {
        printf(" SxD State:[0x%02x:%02x:%02x:%02x]",
                DevInfo->HighestDstates[0],
                DevInfo->HighestDstates[1],
                DevInfo->HighestDstates[2],
                DevInfo->HighestDstates[3]);
    }
    printf("\n");
    fflush(stdout);

    Status = AcpiTkEvaluateObject(ObjHandle,METHOD_NAME__PRT,NULL,&Results);
    if (Status == AE_OK)
    {
        oObj=(ACPI_OBJECT *)Object[0].Package.Elements;
        i += PrintPackage((ULONG)Object[0].Package.Count,oObj);
    }

    return AE_OK;
}
/*
 * Main programm for walk ACPI tree
*/
void
AcpiWalkNameSpaceTst(
    ULONG PIC)
{
    ACPI_STATUS      Status;
    ULONG            Lev;
    ACPI_OBJECT      Obj;
    ACPI_OBJECT_LIST Params;

    Params.Count      = 1;
    Params.Pointer    = &Obj;
    Obj.Type          = ACPI_TYPE_INTEGER;
    Obj.Integer.Value = PIC;

    Status = AcpiTkEvaluateObject(NULL,"\\_PIC",&Params,NULL);
    if (Status != AE_OK)
    {
        printf("Error turn on APIC, status %x\n",Status);
    }

    printf("Show ACPI IRQ routing\n");
    printf("@\n");
    Status = AcpiTkWalkNamespace(ACPI_TYPE_DEVICE, // All object types
                               ACPI_ROOT_OBJECT, // Start object
                               ACPI_UINT32_MAX,  // Max depth
                               internal_PrintObjectInfo,  // Callback
                               &Lev, NULL);  // Callback param and poitner to return value
    printf("Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));
}

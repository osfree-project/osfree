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
#include <accommon.h>
#include <acpiapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void AcpiWalkNameSpaceTst(void);
/*
 *  Show ACPI tree with resource
 *  Pavel Shtemenko 14.08.2005
 */
void
main(
    void)
{
    ACPI_TK_VERSION ApiBuffer;

    printf("******** ACPI Tree test Programm *********\n");

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

    AcpiWalkNameSpaceTst();

}
/*
 * CallBack function for print resorce for _PRS ans CRS method (if possible)
 * WARNING! In any acpi table it can generate trap. For remove - need check -
 * _PRS and _CRS method would be exist.
 */
ACPI_STATUS
internal_PrintResourceInfo(
    ACPI_RESOURCE *Resource,
    void          *Context)
{
    UINT32 NestingLevel = (UINT32) Context;
    int i;

    for (i = 0; i < NestingLevel + 2; i++)
         printf(" ");
    printf(">");
    fflush(stdout);

    switch (Resource->Type)
    {
            case ACPI_RESOURCE_TYPE_IRQ:
                 printf("Trigger by %s, ",Resource->Data.Irq.Triggering == ACPI_LEVEL_SENSITIVE? "Level":"Edge");
                 printf("Polarity %s",Resource->Data.Irq.Polarity == ACPI_ACTIVE_LOW? "Low":"High");
                 if (Resource->Data.Irq.Sharable)
                     printf(", Sharable");
                 for (i = 0; i < Resource->Data.Irq.InterruptCount; i++)
	               printf(" IRQ-%d", Resource->Data.Irq.Interrupts[i]);
                 break;
            case ACPI_RESOURCE_TYPE_DMA:
                 for (i = 0; i < Resource->Data.Dma.ChannelCount; i++)
	               printf(" DMA-%d", Resource->Data.Dma.Channels[i]);
                 break;
            case ACPI_RESOURCE_TYPE_START_DEPENDENT:
                 printf(" DPF-S: CP:0x%x PR:0x%x",Resource->Data.StartDpf.CompatibilityPriority,Resource->Data.StartDpf.PerformanceRobustness);
                 break;
            case ACPI_RESOURCE_TYPE_END_DEPENDENT:
                 printf(" DPF-E: CP:0x%x PR:0x%x",Resource->Data.StartDpf.CompatibilityPriority,Resource->Data.StartDpf.PerformanceRobustness);
                 break;
            case ACPI_RESOURCE_TYPE_IO:
                 printf(" IO:[%d 0x%x - 0x%x Al:%d Len:%d]",
	                  Resource->Data.Io.IoDecode,
	                  Resource->Data.Io.Minimum,
                         Resource->Data.Io.Maximum,
	                  Resource->Data.Io.Alignment,
	                  Resource->Data.Io.AddressLength);
                 break;
            case ACPI_RESOURCE_TYPE_FIXED_IO:
                 printf(" FIXED-IO:[0x%x Len:%d]",
	                  Resource->Data.FixedIo.Address,
	                  Resource->Data.FixedIo.AddressLength);
                 break;
            case ACPI_RESOURCE_TYPE_VENDOR:
                 printf(" VENDOR");
                 break;
            case ACPI_RESOURCE_TYPE_END_TAG:
                 printf(" ENDTAG");
                 break;
            case ACPI_RESOURCE_TYPE_MEMORY24:
                 printf(" MEM24:[%d 0x%x - 0x%x Al:%d Len:%d]",
                         Resource->Data.Memory24.WriteProtect,
                         Resource->Data.Memory24.Minimum,
                         Resource->Data.Memory24.Maximum,
                         Resource->Data.Memory24.Alignment,
                         Resource->Data.Memory24.AddressLength);
                 break;
            case ACPI_RESOURCE_TYPE_MEMORY32:
                 printf(" MEM32:[%d 0x%x - 0x%x Al:%d Len:%d]",
                         Resource->Data.Memory32.WriteProtect,
                         Resource->Data.Memory32.Minimum,
                         Resource->Data.Memory32.Maximum,
                         Resource->Data.Memory32.Alignment,
                         Resource->Data.Memory32.AddressLength);
                 break;
            case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
                 printf(" FIXED-MEM32:[%d 0x%x Len:%d]",
                 Resource->Data.FixedMemory32.WriteProtect,
                 Resource->Data.FixedMemory32.Address,
                 Resource->Data.FixedMemory32.AddressLength);
                 break;
            case ACPI_RESOURCE_TYPE_ADDRESS16:
                 printf(" ADDRESS16:[%d]",
                         Resource->Data.Address16.ResourceType);
                 break;
            case ACPI_RESOURCE_TYPE_ADDRESS32:
                 printf(" ADDRESS32:[%d]",
                         Resource->Data.Address32.ResourceType);
                 break;
            case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
                 printf(" PrCm %d",Resource->Data.ExtendedIrq.ProducerConsumer);
                 for (i = 0; i < Resource->Data.ExtendedIrq.InterruptCount; i++)
                   printf(" IRQ%d ", Resource->Data.ExtendedIrq.Interrupts[i]);
                 if (Resource->Data.ExtendedIrq.InterruptCount)
                 {
                     printf("triggerred by %s, ",Resource->Data.ExtendedIrq.Triggering & ACPI_LEVEL_SENSITIVE? "Level":"Edge");
                     printf("polarity %s",Resource->Data.ExtendedIrq.Polarity & ACPI_ACTIVE_LOW? "Low":"High");
                     if (Resource->Data.ExtendedIrq.Sharable)
                         printf(", Sharable");
                 }
                 printf(" SourceIdx %d",Resource->Data.ExtendedIrq.ResourceSource.Index);
                 if (Resource->Data.ExtendedIrq.ResourceSource.StringLength)
                     printf("%s",Resource->Data.ExtendedIrq.ResourceSource.StringPtr);
                 break;
            case ACPI_RESOURCE_TYPE_ADDRESS64:
            default:
                 printf(" (Unknown resource type: %d)", Resource->Type);
                 break;
      }
      printf("\n");
      return AE_OK;
}

struct PrtLev
            {
             UINT32 CurLevel;
             UINT32 PrevLevel;
            } Lev = {0,0};
void
PrintOffs(
    ACPI_DEVICE_INFO *a)
{
    printf("Name 0x%x Type:0x%x ParCnt:0x%x Valid:0x%x Flags:0x%x\n",
             (UINT32)&a->Name - (UINT32)&a->InfoSize,
             (UINT32)&a->Type - (UINT32)&a->InfoSize,
             (UINT32)&a->ParamCount - (UINT32)&a->InfoSize,
             (UINT32)&a->Valid - (UINT32)&a->InfoSize,
             (UINT32)&a->Flags - (UINT32)&a->InfoSize);
    printf("DS 0x%x LDS:0x%x Addr:0x%x HID:0x%x UID:0x%x CID:0x%x\n",
            (UINT32)a->HighestDstates - (UINT32)&a->InfoSize,
            (UINT32)a->LowestDstates - (UINT32)&a->InfoSize,
            (UINT32)&a->Address - (UINT32)&a->InfoSize,
            (UINT32)&a->HardwareId - (UINT32)&a->InfoSize,
            (UINT32)&a->UniqueId - (UINT32)&a->InfoSize,
            (UINT32)&a->CompatibleIdList - (UINT32)&a->InfoSize);
    fflush(stdout);
}
/*
 * CallBack function for print object info and walk resource info
 */
ACPI_STATUS
internal_PrintObjectInfo(
    ACPI_HANDLE ObjHandle,
    UINT32      NestingLevel,
    void        *Context,
    void        **ReturnValue)
{
    int              i;
    ACPI_BUFFER      Buffer;
    ACPI_DEVICE_INFO *DevInfo = NULL;
    char             achName[5];
    ACPI_STATUS      Status;

    if (NestingLevel > 1)
    {
        for (i = 0; i < NestingLevel-1 ; i++)
             printf("   ");
    }
    printf("|\n");

    if (NestingLevel > 1)
    {
        for (i = 0; i<NestingLevel-1 ; i++)
             printf("   ");
    }
    printf("+*");

    memset(achName, 0, 5);
    Buffer.Length  = 5;
    Buffer.Pointer = achName;
    Status         = AcpiTkGetName(ObjHandle, ACPI_SINGLE_NAME, &Buffer);
    printf("%s", achName);   fflush(stdout);

//    printf(" 0x%x ", DevInfo);   fflush(stdout);
    Status         = AcpiTkGetObjectInfoAlloc3(ObjHandle, &DevInfo);
//    printf(" END ");   fflush(stdout);
    if (Status != AE_OK)
        printf(" AcpiTkGetObjectInfoAlloc Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));

//    PrintOffs(DevInfo);
//     printf(" T0x%x S:0x%x  P:0x%x", DevInfo, DevInfo->InfoSize, DevInfo->HardwareId.String);   fflush(stdout);
     printf(" T:[%s]", AcpiTkUtGetTypeName(DevInfo->Type));
     if (DevInfo->Valid & ACPI_VALID_HID)
     {
         printf(" HID:[%s]", DevInfo->HardwareId.String);
     }
     if (DevInfo->Valid & ACPI_VALID_CID)
     {
         printf(" CID:[");
         for (i = 0; i < DevInfo->CompatibleIdList.Count; i++)
         {
              printf(" %s", DevInfo->CompatibleIdList.Ids[i].String);
         }
         printf("]");
     }
     if (DevInfo->Valid & ACPI_VALID_UID)
         printf(" UID:[%.9s]", DevInfo->UniqueId.String);
     if (DevInfo->Valid & ACPI_VALID_ADR)
         printf(" ADR:[0x%x]", (UINT32)DevInfo->Address);
     if (DevInfo->Valid & ACPI_VALID_SXDS)
     {
         printf(" SxD State:[0x%02x:%02x:%02x:%02x]",
                 DevInfo->HighestDstates[0],
                 DevInfo->HighestDstates[1],
                 DevInfo->HighestDstates[2],
                 DevInfo->HighestDstates[3]);
     }
     if (DevInfo->Valid & ACPI_VALID_SXWS)
     {
         printf(" SxD State:[0x%02x:%02x:%02x:%02x]",
                 DevInfo->LowestDstates[0],
                 DevInfo->LowestDstates[1],
                 DevInfo->LowestDstates[2],
                 DevInfo->LowestDstates[3]);
    }
    printf("\n");
    fflush(stdout);

    if (DevInfo->Type == ACPI_TYPE_DEVICE)
    {
        printf(" Current state:\n");
        AcpiTkWalkResources(ObjHandle,
                          METHOD_NAME__CRS,
                          internal_PrintResourceInfo,
                          (void *) NestingLevel);
        printf(" Possible State:\n");
        AcpiTkWalkResources(ObjHandle,
                          METHOD_NAME__PRS,
                          internal_PrintResourceInfo,
                          (void *) NestingLevel);
    }

    if (DevInfo->Type == ACPI_TYPE_THERMAL)
    {
         printf(" Thermal State:\n");
         AcpiTkWalkResources(ObjHandle,
                           "_TMP",
                           internal_PrintResourceInfo,
                           (void *) NestingLevel);
    }

    if (DevInfo)
    {
        AcpiTkOsFree(DevInfo);
    }
    return AE_OK;

}
/*
 * Main program for walk ACPI tree
*/
void
AcpiWalkNameSpaceTst(
    void)
{
    ACPI_STATUS Status;

    printf("Test AcpiTkWalkNameSpace\n");
    printf("@\n");
    Status=AcpiTkWalkNamespace(ACPI_TYPE_ANY, // All object types
                             ACPI_ROOT_OBJECT, // Start object
                             ACPI_UINT32_MAX, // Max depth
                             internal_PrintObjectInfo,  // Callback
                             &Lev, NULL);  // Callback param and poitner to return value
    printf("Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));
    printf("Size %d\n",sizeof(ACPI_DEVICE_INFO));

}

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
ACPI_OBJECT       Object[20];
char              DevBuf[4098];

void
PrintTMP(
    ACPI_HANDLE ObjHandle)
{
    ACPI_OBJECT  *Obj;
    ACPI_BUFFER  Results;
    UINT32       CurTemp;
    ACPI_STATUS  Status;

    Results.Length  = sizeof(Object);
    Results.Pointer = Object;

    Status = AcpiTkEvaluateObject(ObjHandle,"_TMP",NULL,&Results);
    if (Status == AE_OK)
    {
        CurTemp = Object[0].Integer.Value;
        printf("   Current temperature: %d C ",(Object[0].Integer.Value - 2732)/10 );
        printf("( %d K * 10)\n",CurTemp );
    }
    Status = AcpiTkEvaluateObject(ObjHandle,"_CRT",NULL,&Results);
    if (Status == AE_OK)
    {
        CurTemp = Object[0].Integer.Value;
        printf("   Critical temperature: %d C ",(Object[0].Integer.Value - 2732)/10 );
        printf("( %d K * 10)\n",CurTemp );
    }

    Status = AcpiTkEvaluateObject(ObjHandle,"_TC1",NULL,&Results);
    if (Status == AE_OK)
        printf("   Thermal constant1: %d\n",Object[0].Integer.Value);
    Status = AcpiTkEvaluateObject(ObjHandle,"_TC2",NULL,&Results);
    if (Status == AE_OK)
        printf("   Thermal constant2: %d\n",Object[0].Integer.Value);
    Status = AcpiTkEvaluateObject(ObjHandle,"_TSP",NULL,&Results);
    if (Status == AE_OK)
        printf("   Thermal period: %d sec * 10\n",Object[0].Integer.Value);

}

void
PrintPSL(
    ACPI_HANDLE ObjHandle)
{
    ACPI_OBJECT *Obj;
    ACPI_BUFFER Buffer, Results;
    ACPI_STATUS Status;
    UINT32      i, o;
    char        DevName[5];

    Results.Length  = sizeof(Object);
    Results.Pointer = Object;

    Status = AcpiTkEvaluateObject(ObjHandle,"_PSL",NULL,&Results);
    if (Status == AE_OK)
    {
        Obj = (ACPI_OBJECT *)Object[0].Package.Elements;
        printf("   Passive List has %d elements\n",Object[0].Package.Count);
        for (o = 0; o < Object[0].Package.Count; o++)
        {
             if (Obj[0].Reference.Handle)
             {
                 Buffer.Length  = 5;
                 Buffer.Pointer = DevName;
                 AcpiTkGetName(Obj[o].Reference.Handle, ACPI_SINGLE_NAME, &Buffer);
                 printf("          Device [");
                 for (i = 0; i < 4; i++)
                 {
                      printf("%c",DevName[i]);
                 }
                 printf("]\n");
             }
        }
   }
/*************************************************************************/
    Results.Length  = sizeof(Object);
    Results.Pointer = Object;

    Status = AcpiTkEvaluateObject(ObjHandle,"_AL0",NULL,&Results);
    if (Status == AE_OK)
    {
        Obj = (ACPI_OBJECT *)Object[0].Package.Elements;
        printf("   Active List has %d elements\n",Object[0].Package.Count);
        if (Obj[0].Reference.Handle)
        {
            Buffer.Length  = 5;
            Buffer.Pointer = DevName;
            AcpiTkGetName(Obj[0].Reference.Handle, ACPI_SINGLE_NAME, &Buffer);
            printf("          Device [");
            for (i = 0; i < 4; i++)
                 printf("%c",DevName[i]);
            printf("]\n");
        }
    }
/*****************************************************************************/
    Status = AcpiTkEvaluateObject(ObjHandle,"_PSV",NULL,&Results);
    if (Status == AE_OK)
    {
        i = Object[0].Integer.Value;
        printf("   Passive cooling temperature: %d C ",(Object[0].Integer.Value - 2732)/10 );
        printf("( %d K * 10 )\n",i );
    }
/*****************************************************************************/
    Status = AcpiTkEvaluateObject(ObjHandle,"_AC0",NULL,&Results);
    if (Status == AE_OK)
    {
        i = Object[0].Integer.Value;
        printf("   Active cooling temperature: %d C ",(Object[0].Integer.Value - 2732)/10 );
        printf("( %d K * 10 )\n",i );
    }

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
    int i;
    ACPI_BUFFER         Buffer, Results;
    ACPI_DEVICE_INFO    *DevInfo = NULL;
    char                achName[5];
    ACPI_OBJECT         Obj,*oObj;
    ACPI_STATUS         Status;
    ULONG Lev;
    double Life;

    Results.Length  = sizeof(Object);
    Results.Pointer = &Object;

    AcpiTkGetObjectInfoAlloc3(ObjHandle,&DevInfo);

    Buffer.Length  = 5;
    Buffer.Pointer = achName;
    AcpiTkGetName(ObjHandle, ACPI_SINGLE_NAME, &Buffer);
    printf("Thermal zone \"%s\"\n",achName);

    PrintTMP(ObjHandle);
    PrintPSL(ObjHandle);

    if (DevInfo)
    {
       AcpiTkOsFree(DevInfo);
    }
    return AE_OK;
}
/*
 * Main programm for walk ACPI tree
*/
void
main(
    void)
{
    ACPI_STATUS Status;
    ULONG       Lev;
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

    printf("Show Thermal\n");
    Status = AcpiTkWalkNamespace(ACPI_TYPE_THERMAL,  // All object types
                               ACPI_ROOT_OBJECT,   // Start object
                               ACPI_UINT32_MAX,    // Max depth
                               internal_PrintObjectInfo,  // Callback
                               &Lev, NULL);        // Callback param and poitner to return value
     printf("Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));
}

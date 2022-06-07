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

void AcpiWalkNameSpaceTst(void);

UINT32       PowerUnit = 2;
char         *PowerString[] = { "mWh","mAh","Unknown"};
char         *PowerString2[] = { "mW","mA","Unknown"};

UINT32       BRate=0;
UINT32       BRemaining=0;
UINT32       LastFull=0;
/*
 *  Show ACPI battery
 *  Pavel Shtemenko 14.08.2005
 */

ACPI_OBJECT       Object[200];
UINT32            CheckState = 0;

/**
* Print all field from _BST method
*
* @param  ObjHandle ACPI_HANDLE to battery object
* @return None
*
*/
void
PrintBST(
    ACPI_HANDLE ObjHandle)
{
    ACPI_OBJECT  *Obj;
    ACPI_BUFFER  Results;
    ACPI_STATUS  Status;

    Results.Length  = sizeof(Object);
    Results.Pointer = Object;

    Status = AcpiTkEvaluateObject(ObjHandle,"_BST",NULL,&Results);
    if (Status != AE_OK)
    {
        printf("Error evaluate _BST, status %x\n",Status);
        return;
    }

    printf("=========== Battery Status ===============\n");
    Obj = (ACPI_OBJECT *)Object[0].Package.Elements;

    if ((UINT32)Obj[0].Integer.Value & 1) printf("Battery is discharging\n");
    if ((UINT32)Obj[0].Integer.Value & 2) printf("Battery is charging\n");
    if ((UINT32)Obj[0].Integer.Value & 4) printf("Battery is low critical energy state\n");

    if (Obj[1].Integer.Value != 0xffffffff)
    {
        if (!BRate)
        {
            BRate = (UINT32)Obj[1].Integer.Value;
        }
        printf("%d Battery present rate %d %s\n",Obj[1].Type,(UINT32)Obj[1].Integer.Value,PowerString2[PowerUnit]);
    }
    else
    {
        printf("Unknown battery present rate\n");
    }

    if (Obj[2].Integer.Value != 0xffffffff)
    {
        if (!BRemaining)
        {
            BRemaining = (UINT32)Obj[2].Integer.Value;
        }
        printf("%d Battery remaining capacity %d %s\n",Obj[2].Type,(UINT32)Obj[2].Integer.Value,PowerString[PowerUnit]);
    }
    else
    {
        printf("Unknown battery remaining capacity\n");
    }

    if (Obj[3].Integer.Value != 0xffffffff)
    {
        printf("%d Battery present voltage %d mV \n",Obj[3].Type,(UINT32)Obj[3].Integer.Value);
    }
    else
    {
        printf("Unknown battery remaining capacity\n");
    }

}

/**
* Print all field from _BIF method
*
* @param  ObjHandle ACPI_HANDLE to battery object
* @return None
*
*/
void
PrintBIF(
    ACPI_HANDLE ObjHandle)
{
    ACPI_OBJECT  *Obj;
    ACPI_BUFFER  Results;
    ACPI_STATUS  Status;
    UINT32       i;

    Results.Length  = sizeof(Object);
    Results.Pointer = Object;

    Status = AcpiTkEvaluateObject(ObjHandle,"_BIF",NULL,&Results);
    if (Status != AE_OK)
    {
        printf("Error evaluate _BIF, status %x\n",Status);
        return;
    }

    printf("=========== Battery design ===============\n");
    Obj = (ACPI_OBJECT *)Object[0].Package.Elements;

    PowerUnit = (UINT32)Obj[0].Integer.Value;
    printf("Power Unit: capacity is %s , charge/discharge rate is %s\n",PowerString[PowerUnit],PowerString2[PowerUnit]);
    if ((UINT32)Obj[1].Integer.Value != 0xffffffff )
    {
        printf("%d Design capacity %d %s\n",Obj[1].Type,(UINT32)Obj[1].Integer.Value,PowerString[PowerUnit]);
    }
    else
    {
        printf("Unknown design capacity\n");
    }

    if ((UINT32)Obj[2].Integer.Value != 0xffffffff)
    {
        if (!LastFull)
        {
            LastFull = (UINT32)Obj[2].Integer.Value;
        }
        printf("%d Last full charge capacity %d %s\n",Obj[2].Type,(UINT32)Obj[2].Integer.Value, PowerString[PowerUnit]);
    }
    else
    {
        printf("Unknown last full charge capacity\n");
    }

    printf("%d Battery technology %x\n",Obj[3].Type,(UINT32)Obj[3].Integer.Value);

    if ((UINT32)Obj[4].Integer.Value != 0xffffffff)
    {
        printf("%d Design voltage %d mV\n",Obj[4].Type,(UINT32)Obj[4].Integer.Value);
    }
    else
    {
        printf("Unknown design voltage\n");
    }

    if ((UINT32)Obj[5].Integer.Value != 0xffffffff)
    {
        printf("%d Design capacity of warning %d %s\n",Obj[5].Type,(UINT32)Obj[5].Integer.Value,PowerString[PowerUnit]);
    }
    else
    {
        printf("Unknown design capacity of warning\n");
    }

    if ((UINT32)Obj[6].Integer.Value != 0xffffffff)
    {
        printf("%d Design capacity of low %d %s\n",Obj[6].Type,(UINT32)Obj[6].Integer.Value,PowerString[PowerUnit]);
    }
    else
    {
        printf("Unknown design capacity of low\n");
    }

    if ((UINT32)Obj[7].Integer.Value != 0xffffffff)
    {
        printf("%d Baterry granularity between low and warning %u %s\n",Obj[7].Type,(UINT32)Obj[7].Integer.Value,PowerString[PowerUnit]);
    }
    else
    {
        printf("Unknown Baterry granularity between low and warning\n");
    }

    if ((UINT32)Obj[8].Integer.Value != 0xffffffff)
    {
        printf("%d Baterry granularity between warning and high %u %s\n",Obj[8].Type,(UINT32)Obj[8].Integer.Value,PowerString[PowerUnit]);
    }
    else
    {
        printf("Unknown Baterry granularity between warning and high\n");
    }

    if (Obj[9].Buffer.Pointer)
    {
        printf("%d ModelNumber: \"%s\" %d bytes \"",Obj[9].Type,Obj[9].Buffer.Pointer,Obj[9].Buffer.Length);
        for (i = 0; i < Obj[9].Buffer.Length; i++)
        {
            printf("%2.2x",Obj[9].Buffer.Pointer[i]);
        }
        printf("\"\n");
    }

    if (Obj[10].Buffer.Pointer)
    {
        printf("%d SerialNumber: \"%s\" %d bytes %x\n",Obj[10].Type,Obj[10].Buffer.Pointer,Obj[10].Buffer.Length,Obj[10].Buffer.Pointer[0]);
    }

    if (Obj[11].Buffer.Pointer)
    {
        printf("%d Battery type: \"%s\"%d bytes\n",Obj[11].Type,Obj[11].Buffer.Pointer,Obj[11].Buffer.Length);
    }

    if (Obj[12].Buffer.Pointer)
    {
        printf("%d OEM info: \"%s\" %d\n",Obj[12].Type,Obj[12].Buffer.Pointer,Obj[12].Buffer.Length);
    }

}

UINT32 ACStatus=0;
ACPI_HANDLE ACHandle = 0;

/**
* Get current AC status
* @return 0 - disconnected, 1 - connected or 1 - unknown
*/
ULONG
GetACStatus(
    void)
{

    ACPI_BUFFER       Buffer;
    ACPI_STATUS       Status;
    ACPI_OBJECT       Obj;

    if ( ACHandle == 0)
    {
        return 1;
    }

    Buffer.Length = sizeof(ACPI_OBJECT);
    Buffer.Pointer = &Obj;
    Status = AcpiTkEvaluateObject(ACHandle,"_PSR",NULL,&Buffer);
    if (Status == AE_OK)
    {
        if (Obj.Type == ACPI_TYPE_INTEGER)
        {
            printf ("AC status is: %x -",(UINT32)Obj.Integer.Value);
            switch ((UINT32)Obj.Integer.Value)
            {
                  case 0:
                       printf ("disconnected\n");
                       break;
                  case 1:
                       printf ("connected\n");
                       break;
                  default:
                       printf ("unknown\n");
                       break;
            }
            return (UINT32)Obj.Integer.Value;
        }
        else
        {
            return 1;
        }
     }

     return 1;
}

/**
 * CallBack function for print object info and walk resource info
 *
 * @param  ObjHandle
 * @param  NestingLevel
 * @param  Context
 * @param  ReturnValue
 * @return ACPI status
 */
ACPI_STATUS
internal_PrintObjectInfo(
    ACPI_HANDLE ObjHandle,
    UINT32 NestingLevel,
    void *Context,
    void **ReturnValue)
{
    int i;
    ACPI_BUFFER Buffer;
    ACPI_DEVICE_INFO  *DevInfo = NULL;
    char achName[255];
    ACPI_BUFFER       Results;
    ACPI_OBJECT       Obj,*oObj;
    ACPI_STATUS       Status;
    ULONG Lev;
    int Life2;


    BRate      = 0;
    BRemaining = 0;
    LastFull   = 0;

    AcpiTkGetObjectInfoAlloc3(ObjHandle, &DevInfo);
//            Buffer.Length  = 5;
//            Buffer.Pointer = achName;
//            AcpiTkGetName(ObjHandle, ACPI_SINGLE_NAME, &Buffer);
//            printf("look %s 0x%x 0x%x\n",achName, DevInfo, DevInfo->HardwareId.String);

    if ((DevInfo->Valid & ACPI_VALID_HID) && DevInfo->HardwareId.String)
    {
        if (strncmp (DevInfo->HardwareId.String,"ACPI0003",8) == 0 )
        {
            Buffer.Length  = 255;
            Buffer.Pointer = achName;
            AcpiTkGetName(ObjHandle, ACPI_FULL_PATHNAME, &Buffer);
            printf("Found AC \"%s\"\n",achName);
            ACHandle=ObjHandle;
            ACStatus = GetACStatus();
        }
        if (strncmp ((char *)DevInfo->HardwareId.String,"PNP0C0A",7) == 0)
        {
            memset(achName, 0, 255);
            Buffer.Length = 255;
            Buffer.Pointer = achName;
            AcpiTkGetName(ObjHandle, ACPI_FULL_PATHNAME, &Buffer);
            printf("Found Smart Battery \"%s\"\n",achName);
            // Check _STA
            Results.Length  = sizeof(Object);
            Results.Pointer = Object;
            Status = AcpiTkEvaluateObject(ObjHandle,"_STA",NULL,&Results);
            if (Status == AE_OK)
            {
                printf("Battery status 0x%x\n",(UINT32)Object[0].Integer.Value);
            }
            // Bit 4 mean "battery is present"
            if (CheckState && !((UINT32)Object[0].Integer.Value & 0x10))
            {
                return AE_OK;
            }
            PrintBIF(ObjHandle);
            PrintBST(ObjHandle);
            if (BRate && BRemaining )
            {
                Life2 = (BRemaining * 60) / BRate;
                printf("Remaining Battery Life: %d min\n", Life2 );
            }
            if (LastFull && BRemaining)
            {
                printf("Remaining Battery Percentage: %d %%\n",  (BRemaining * 100) / LastFull );
            }
        }
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
void main(
     int  argc,
     char *argv[])
{
    ACPI_STATUS       Status;
    ULONG             Lev;
    ACPI_TK_VERSION ApiBuffer;

    if (argc > 1 && ( (*argv[1] & 0x5F) == 'S')  )
    {
       CheckState = 1;
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

    printf("Show ACPI battery state\n");
    Status = AcpiTkWalkNamespace(ACPI_TYPE_ANY,           // All object types
                               ACPI_ROOT_OBJECT,          // Start object
                               ACPI_UINT32_MAX,           // Max depth
                               internal_PrintObjectInfo,  // Callback in
                               &Lev, NULL);               // Callback param and poitner to return value

    printf("Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));
}

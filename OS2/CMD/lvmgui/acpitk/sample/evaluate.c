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

ACPI_OBJECT   Object[1000];
char          ExecPath[]="\\_PR_.CPU1._CST";

/**
* Print ACPI package
* @Count Count elements in this package
* @oObj  Pointer to package
* @ret   Count printed elements
*/
ULONG
PrintPackage(
    ULONG        Count,
    ACPI_OBJECT  *oObj)
{
    ULONG       j,i,Cnt;
    ACPI_OBJECT *Obj;
    char        DevName[5];
    ACPI_BUFFER Buffer;

    for (Cnt=0,j=0; j < Count; j++,oObj++,Cnt++)
    {
         switch (oObj->Type)
         {
                 case ACPI_TYPE_INTEGER:
                      printf("   I:[%d] value: 0x%x\n",j,oObj->Integer.Value);
                      fflush(stdout);
                      break;
                 case ACPI_TYPE_STRING:
                      printf("   S:[%d] value: 0x%x %s\n",j,oObj->String.Length,oObj->String.Pointer);
                      break;
                 case ACPI_TYPE_PACKAGE:
                      Obj=(ACPI_OBJECT *)oObj->Package.Elements;
                      printf("P:[%d] Package elements: 0x%x addr:%x\n",j,oObj->Package.Count,oObj->Package.Elements);
                      fflush(stdout);
                      Cnt += PrintPackage((ULONG)oObj->Package.Count,Obj);
                      break;
                 default:
                      printf("   U:[%d] Unknown 0x%x Fill:%x Handle:%x\n",j,oObj->Type,oObj->Reference.ActualType,oObj->Reference.Handle);
                      if (oObj->Reference.Handle)
                      {
                          Buffer.Length = 5;
                          Buffer.Pointer = DevName;
                          AcpiTkGetName(oObj->Reference.Handle, ACPI_SINGLE_NAME, &Buffer);
                          printf("          Device [");
                          for (i=0; i < 4; i++)
                               printf("%c",DevName[i]);
                           printf("]\n");
                      }
                      fflush(stdout);
                      break;
         }
    }

    return Cnt;

}
/**
*  Main program for evaluate
*/
void
main(
    int  argc,
    char *argv[])
{
    ACPI_BUFFER       Results;
    ACPI_OBJECT       Obj,*oObj;
    ACPI_STATUS       Status;
    ACPI_OBJECT_LIST  Params;
    ULONG             i,j,ii;
    ACPI_TK_VERSION ApiBuffer;

    if (argc < 2)
    {
        printf("Usage evaluate string\n");
        exit(1);
    }

    printf("Test AcpiTkEvaluateObject\n");

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

    Params.Count      = 1;
    Params.Pointer    = Object;
    Obj.Type          = ACPI_TYPE_INTEGER;
    Obj.Integer.Value = 3;
    Results.Length  = sizeof(Object);
    Results.Pointer = Object;


    printf("Calling AcpiTkEvaluateObject(0,\"%s\",%x,%x)\n",argv[1],&Params,&Results);
    fflush(stdout);
    //************** Calling ACPI function from PSD *********************
    Status = AcpiTkEvaluateObject(NULL,argv[1],NULL,&Results);
    printf("Status 0x%x %d - %s\n",Status,Status,AcpiTkStatusToStr(Status));
    printf("Result len:%d sizeof %d\n",Results.Length,sizeof(Object));

    for (i=0; i < Results.Length/sizeof(Obj); i++)
    {
         switch (Object[i].Type)
         {
                 case ACPI_TYPE_PROCESSOR:
                      printf("Processor:\n");
                      printf("[%d] Type:       0x%x\n",i,Object[i].Processor.Type);
                      printf("[%d] ProcId:     0x%x\n",i,Object[i].Processor.ProcId);
                      printf("[%d] P_BLK:      0x%x\n",i,Object[i].Processor.PblkAddress);
                      printf("[%d] P_BLK len:  0x%x\n",i,Object[i].Processor.PblkLength);
                      break;
                 case ACPI_TYPE_INTEGER:
                      printf("I:[%d] value: 0x%x\n",i,Object[i].Integer.Value);
                      break;
                 case ACPI_TYPE_STRING:
                      printf("S:[%d] value: 0x%x %s\n",i,Object[i].String.Length,Object[i].String.Pointer);
                      break;
                 case ACPI_TYPE_PACKAGE:
                      oObj=(ACPI_OBJECT *)Object[i].Package.Elements;
                      printf("P:[%d] Package elements: 0x%x addr:%x\n",i,Object[i].Package.Count,Object[i].Package.Elements);
                      printf("--------------------------------------------------\n");
                      fflush(stdout);
                      i += PrintPackage((ULONG)Object[i].Package.Count,oObj);
                      break;
                 default:
                      printf("[%d] Unknown 0x%x\n",i,Object[i].Type);
                      break;
         }
    }

    fflush(stdout);

}

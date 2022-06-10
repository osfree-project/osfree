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

/*
 * Main program for get ACPI table
*/
void
main(
    int  argc,
    char *argv[])
{
    ACPI_STATUS       Status;
    ACPI_TABLE_HEADER Table;
    ACPI_TABLE_HEADER *Tbl = NULL;
    FILE              *OutTbl = NULL;
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

    if (argc < 2)
    {
        printf ( "Usage: GetTable TableName\n");
        return;
    }

    printf("Look table \"%s\"\n",argv[1]);
    Status = AcpiTkGetTableHeader (argv[1],0,&Table);
    if (Status != AE_OK)
    {
        printf ( "Error call ACPI CA %x - %s\n",Status,AcpiTkStatusToStr(Status));
        return;
    }

    printf ("Signature          : %4.4s\n",Table.Signature);
    printf ("Length             : %d bytes\n",Table.Length);     /* Length of table in bytes, including this header */
    printf ("Revision           : %d\n",Table.Revision);         /* ACPI Specification minor version # */
    printf ("Checksum           ; 0x%x\n",Table.Checksum);       /* To make sum of entire table == 0 */
    printf ("OemId              : %6.6s\n",Table.OemId);         /* ASCII OEM identification */
    printf ("Table OemId        : %8.8s\n",Table.OemTableId);    /* ASCII OEM table identification */
    printf ("OemRevision        : 0x%x\n",Table.OemRevision);    /* OEM revision number */
    printf ("AslCompilerId      : %4.4s\n",Table.AslCompilerId); /* ASCII ASL compiler vendor ID */
    printf ("AslCompilerRevision: 0x%x\n",Table.AslCompilerId);  /* ASL compiler version */

    Tbl    = malloc ( Table.Length );
    Status = AcpiTkGetTable (argv[1],0,&Tbl);
    printf ("Status:%x addr %x\n",Status,Tbl);
    printf ("Signature          : %4.4s\n",Tbl->Signature);

    if ((OutTbl = fopen ( "table.dat","wb")) != NULL)
    {
        fwrite(Tbl,Table.Length,1,OutTbl);
        fclose(OutTbl);
    }
    else
        printf ("Error write table\n");
}

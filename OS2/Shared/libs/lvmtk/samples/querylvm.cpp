/* -------------------------------------------------------------------------- *
 * QUERYLVM.CPP                                                               *
 *                                                                            *
 * Simple program to query volumes, drives & partitions using the LVM Engine. *
 * By Alex Taylor, alex@eddie.cis.uoguelph.ca                                 *
 * Some of this code is derived from lvmtest.cpp by John Martin Alfredsson.   *
 *                                                                            *
 * Syntax:                                                                    *
 *      querylvm [options]                                                    *
 *      where available options are                                           *
 *      <none> : print volume information                                     *
 *          -v : print volume information (verbose)                           *
 *          -d : print disk and partition information                         *
 *                                                                            *
 * Returns:                                                                   *
 *      0 on normal exit                                                      *
 *      LVM Engine error-code (positive integer) on error                     *
 *                                                                            *
 *                                                                            *
 * LICENSE:                                                                   *
 *                                                                            *
 *   This program is free software;  you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   This program is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                *
 *   the GNU General Public License for more details.                         *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with this program;  if not, write to the Free Software             *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  *
 *                                                                            *
 * -------------------------------------------------------------------------- */

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvm_intr.h"


void ListVolumes( int verbosity );
void ListDisks( void );
int PartitionsByDrive( ADDRESS hdrv );
void PrintHelp( void );

/* ------------------------------------------------------------------------- */
int main( int argc, char *argv[] )
{
    CARDINAL32 Error_Code = 1;

                    /* Open the LVM engine */
    Open_LVM_Engine( TRUE, &Error_Code );
    if ( Error_Code != LVM_ENGINE_NO_ERROR ) {
        printf("ERROR\nOpen_LVM_Engine returned %d\n", Error_Code );
        return ( Error_Code );
    }

    if ( argc > 1 ) {
        if ( strncmp( argv[1], "-v", 2 ) == 0 ) ListVolumes( 1 );
        else if ( strncmp( argv[1], "-d", 2 ) == 0 ) ListDisks();
        else if ( strncmp( argv[1], "-?", 2 ) == 0 ) PrintHelp();
        else { printf("Unrecognized option: %s\n", argv[1] ); PrintHelp(); }
    } else ListVolumes( 0 );

    Close_LVM_Engine();
    return ( 0 );
}



/* ------------------------------------------------------------------------- *
 * ListVolumes                                                               *
 *    Prints a list of all defined volumes.                                  *
 *                                                                           *
 * Parameters:                                                               *
 *    int verbosity - controls the detail level of the output.               *
 *                     0 = omit 'Device Type' and 'Number of Partitions'.    *
 *                    >0 = show all details.                                 *
 *                                                                           *
 * Returns: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ListVolumes( int verbosity ) {
    CARDINAL32 Error_Code = 1;
    Volume_Control_Array vca;
    Volume_Information_Record volinfo;
    char dletter;
    char devtype[ 8 ],
         status[ 12 ],
         voltype[ 14 ];
    int index;
    long vsize;

                    /* Get the array of volume handles */
    vca = Get_Volume_Control_Data( &Error_Code );
    if ( Error_Code != LVM_ENGINE_NO_ERROR )
        printf("ERROR\nGet_Volume_Control_Data returned %d\n", Error_Code );
    else {
        if ( verbosity > 0 ) {
            printf("====================================================================================\n");
            printf("LOGICAL VOLUME SUMMARY\n");
            printf("                Volume  FileSys  Size/MB  Parts   Device  Volume Type    Flags\n");
            printf("------------------------------------------------------------------------------------\n");
        } else {
            printf("====================================================================\n");
            printf("LOGICAL VOLUME SUMMARY\n");
            printf("                Volume  FileSys  Size/MB  Volume Type    Flags\n");
            printf("--------------------------------------------------------------------\n");
        }
                        /* Now get the volume information for each volume */
        for ( index = 0; index < vca.Count; index++ ) {
            volinfo = Get_Volume_Information( vca.Volume_Control_Data[ index ].Volume_Handle,
                                              &Error_Code                                     );
            if (Error_Code != 0)
                printf("ERROR\nGet_Volume_Information returned %d\n", Error_Code );
            else {
                switch( vca.Volume_Control_Data[ index ].Device_Type ) {
                    case 0:  strcpy( devtype, "HDD"); break;
                    case 1:  strcpy( devtype, "PRM"); break;
                    case 2:  strcpy( devtype, "CD-ROM"); break;
                    case 3:  strcpy( devtype, "Network"); break;
                    default: strcpy( devtype, "Unknown"); break;
                }
                switch ( volinfo.Status ) {
                    case 1:  strcpy( status, "Bootable"); break;
                    case 2:  strcpy( status, "Startable"); break;
                    case 3:  strcpy( status, "Installable"); break;
                    default: strcpy( status, "None"); break;
                }
                if ( vca.Volume_Control_Data[ index ].Compatibility_Volume == TRUE )
                   strcpy( voltype, "Compatibility");
                else strcpy( voltype, "LVM");
                vsize = volinfo.Volume_Size / 2048;
                if ( volinfo.Current_Drive_Letter == 0 ) {
                    dletter = '-';
                } else dletter = volinfo.Current_Drive_Letter;
                printf("%c %20s  %7s  %7d  ", dletter,
                                              volinfo.Volume_Name,
                                              volinfo.File_System_Name,
                                              vsize                        );
                if ( verbosity > 0 )
                    printf("%5d %8s  ", volinfo.Partition_Count, devtype );
                printf("%13s  %s\n", voltype, status );
            }
        }
        if ( verbosity > 0 )
            printf("====================================================================================\n");
        else printf("====================================================================\n");
        Free_Engine_Memory( vca.Volume_Control_Data );
    }
}


/* ------------------------------------------------------------------------- *
 * ListDisks                                                                 *
 *    Prints a list of physical disks and their partitions.                  *
 *                                                                           *
 * Parameters: N/A                                                           *
 *                                                                           *
 * Returns: N/A                                                              *
 * ------------------------------------------------------------------------- */
void ListDisks( void ) {
    CARDINAL32 Error_Code = 1;
    Drive_Control_Array dca;
    Drive_Control_Record dcr;
    Drive_Information_Record drvinfo;
    int index;
    long dsize;

    dca = Get_Drive_Control_Data( &Error_Code );
    if ( Error_Code != LVM_ENGINE_NO_ERROR )
        printf("ERROR\nGet_Drive_Control_Data returned %d\n", Error_Code );
    else {
        printf("===============================================================================\n");
        printf("PHYSICAL DISK SUMMARY\n");
        printf(" #  Disk/Partition Name  Size/MB  P.Type   OS                Volume  Status\n");
        for ( index = 0; index < dca.Count; index++ ) {
            printf("-------------------------------------------------------------------------------\n");
            dcr = dca.Drive_Control_Data[ index ];
            drvinfo = Get_Drive_Status( dcr.Drive_Handle, &Error_Code );
            if (Error_Code != 0)
                printf("ERROR\nGet_Drive_Status returned %d\n", Error_Code );
            else {
                dsize = ( dcr.Drive_Size ) / 2048;
                printf("%2d %20s  %7d\n", dcr.Drive_Number,
                                          drvinfo.Drive_Name,
                                          dsize               );
                PartitionsByDrive( dcr.Drive_Handle );
            }
        }
        printf("===============================================================================\n");
        Free_Engine_Memory( dca.Drive_Control_Data );
    }
}



/* ------------------------------------------------------------------------- *
 * PartitionsByDrive                                                         *
 *    Lists all partitions that exist on the given disk.                     *
 *                                                                           *
 * Parameters:                                                               *
 *    ADDRESS hdrv - handle of the disk to be listed.                        *
 *                                                                           *
 * Returns: int                                                              *
 *    Number of partitions found.                                            *
 * ------------------------------------------------------------------------- */
int PartitionsByDrive( ADDRESS hdrv ) {
    CARDINAL32 Error_Code = 1;
    Partition_Information_Array pia;
    Partition_Information_Record partinfo;
    int idx;
    long psize;
    char ptype[ 8 ],
         pstatus[ 11 ];

    pia = Get_Partitions( hdrv, &Error_Code );
    if ( Error_Code != LVM_ENGINE_NO_ERROR ) {
        printf("ERROR\nGet_Partitions returned %d\n", Error_Code );
        return ( -1 );
    }
    else {
        for ( idx = 0; idx < pia.Count; idx++ ) {
            partinfo = pia.Partition_Array[ idx ];
            if ( partinfo.Primary_Partition ) strcpy( ptype, "Primary");
            else strcpy( ptype, "Logical");
            switch ( partinfo.Partition_Status ) {
                case 0:  strcpy( pstatus, "Free Space"); break;
                case 1:  strcpy( pstatus, "In Use"); break;
                case 2:  strcpy( pstatus, "Available"); break;
                default: strcpy( pstatus, "Unknown"); break;
            }
            psize = partinfo.Usable_Partition_Size / 2048; /* convert sectors to MB */
            printf("   %20s  %7d %8s  %2x  %20s  %s\n", partinfo.Partition_Name,
                                                        psize,
                                                        ptype,
                                                        partinfo.OS_Flag,
                                                        partinfo.Volume_Name,
                                                        pstatus                  );
        }
        Free_Engine_Memory( pia.Partition_Array );
        return ( pia.Count );
    }

}


/* ------------------------------------------------------------------------- *
 * PrintHelp                                                                 *
 *    Prints the command-line syntax help.                                   *
 *                                                                           *
 * Parameters: N/A                                                           *
 *                                                                           *
 * Returns: N/A                                                              *
 * ------------------------------------------------------------------------- */
void PrintHelp( void ) {
    printf("Syntax:\n\tquerylvm [options]\n");
    printf("\nwhere available options are\n");
    printf("  <none> : print volume information\n");
    printf("      -v : print volume information (verbose)\n");
    printf("      -d : print disk and partition information\n\n");
}


/*****************************************************************************
*   This program is free software;  you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY;  without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
*   the GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program;  if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*****************************************************************************/

// lvmtest.cpp, program to test the LVM library
//
// By John Martin Alfredsson, jma@jmast.se
//

// Includes files for OS/2
//-------------------------
#include <os2.h>

// Generic includes
//------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <process.h>
#include <conio.h>
#include <io.h>

#include "lvm_intr.h"


int main(int argv, char *argc[])
    {
    CARDINAL32			Error_Code = 99999;
    Volume_Control_Array	vca;
    Volume_Information_Record	vir;
    int				iCounter;


    Open_LVM_Engine(TRUE, &Error_Code);
    if (Error_Code != 0)
       {
       printf("Open_LVM_Engine Error !!\n");
       return (1);
       }

    vca = Get_Volume_Control_Data(&Error_Code);
    if (Error_Code != 0)
       {
       printf("Get_Volume_Control_Data Error !!\n");
       return (1);
       }

    for (iCounter = 0; iCounter < vca.Count; iCounter++)
        {
        printf("--------------------------------------\n");
        vir = Get_Volume_Information(vca.Volume_Control_Data[iCounter].Volume_Handle, &Error_Code);
        printf("Volname      : [%c:] %s\n", vir.Current_Drive_Letter, vir.Volume_Name);
        printf("FileSystem   : %s\n", vir.File_System_Name);

        if (vir.Status == 0)
           printf("Status       : None\n");

        if (vir.Status == 1)
           printf("Status       : Bootable\n");

        if (vir.Status == 2)
           printf("Status       : Startable\n");

        if (vir.Status == 3)
           printf("Status       : Installable\n");


        if (vca.Volume_Control_Data[iCounter].Compatibility_Volume == TRUE)
           printf("Volume type  : Compatibility Volume\n");
        else
           printf("Volume type  : LVM Volume\n");
        }

    printf("--------------------------------------\n");

    Free_Engine_Memory(vca.Volume_Control_Data);
    Close_LVM_Engine();
    }



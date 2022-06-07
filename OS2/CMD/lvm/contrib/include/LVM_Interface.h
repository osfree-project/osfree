/*****************************************************************************
 *
 * IBM CONFIDENTIAL
 *
 * OCO Source Materials
 *
 * (c) Copyright IBM Corp. 1998
 *
 * The source code for this program is not published or otherwise divested of
 * its tradesecrets, irrespective of what has been deposited with the U.S.
 * Copyright Office.
 *
 * Header: LVM_Interface.H
 *
 * Functions: void                         Open_LVM_Engine
 *            void                         Close_LVM_Engine
 *            Drive_Control_Array          Get_Drive_Control_Data
 *            Drive_Information_Record     Get_Drive_Status
 *            Partition_Information_Array  Get_Partitions
 *            Partition_Information_Record Get_Partition_Information
 *            void                         Create_Partition
 *            void                         Delete_Partition
 *            void                         Set_Active_Flag
 *            void                         Set_OS_Flag
 *            Volume_Control_Array         Get_Volume_Control_Data
 *            Volume_Information_Record    Get_Volume_Information
 *            void                         Create_Volume
 *            void                         Hide_Volume
 *            void                         Expand_Volume
 *            void                         Assign_Drive_Letter
 *            void                         Set_Installable
 *            void                         Set_Name
 *            CARDINAL32                   Get_Valid_Options
 *            BOOLEAN                      Boot_Manager_Is_Installed
 *            void                         Add_To_Boot_Manager
 *            void                         Remove_From_Boot_Manager
 *            Boot_Manager_Menu            Get_Boot_Manager_Menu
 *            void                         Install_Boot_Manager
 *            void                         Remove_Boot_Manager
 *            void                         Set_Boot_Manager_Options
 *            void                         Get_Boot_Manager_Options
 *            void                         New_MBR
 *            CARDINAL32                   Get_Available_Drive_Letters
 *            BOOLEAN                      Reboot_Required
 *            void                         Set_Reboot_Flag
 *            BOOLEAN                      Get_Reboot_Flag
 *            void                         Set_Min_Install_Size
 *            void                         Start_Logging
 *            void                         Stop_Logging
 *            void                         Export_Configuration
 *
 * Description:  This module defines the interface to LVM.DLL, which is the
 *               engine that performs all of the disk partitioning/volume
 *               creation work.
 *
 * Notes:
 *
 * Date      Name          Description
 * ---------------------------------------------------------------------------
 * 04/09/98  B. Rafanello  Original author
 * 10/16/98  B. Rafanello  Original contents moved to lvm_intr.h in release C.PC
 *
 *****************************************************************************/

#define NEED_BYTE_DEFINED
#include <lvm_intr.h>

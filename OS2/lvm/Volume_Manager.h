/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
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
 *
 * Module: Volume_Manager.h
 */

/*
 * Change History:
 *
 */

/*
 * Functions: The following functions are actually declared in
 *            LVM_INTERFACE.H but are implemented in Volume_Manager.C:
 *
 *
 * Description: Since the LVM Engine Interface is very
 *              large, it became undesirable to put all of the code
 *              to implement it in a single file.  Instead, the
 *              implementation of the LVM Engine Interface was divided
 *              among several C files which would have access to a
 *              core set of data.  Engine.H and Engine.C define,
 *              allocate, and initialize that core set of data.
 *              Volume_Manager.H and Volume_Manager.C implement
 *              the volume management functions found in the
 *              LVM Engine Interface.
 *
 *
 *    Partition_Manager.C                                     Volume_Manager.C
 *            \                                               /
 *              \  |-------------------------------------|  /
 *                \|                                     |/
 *                 |        Engine.H and Engine.C        |
 *                /|                                     |\
 *              /  |-------------------------------------|  \
 *            /                                               \
 *    BootManager.C                                          Handle_Manager.C
 *
 * Notes: LVM Drive Letter Assignment Tables (DLA_Tables) appear on the
 *        last sector of each track containing a valid MBR or EBR.  Since
 *        partitions must be track aligned, any track containing an MBR or
 *        EBR will be almost all empty sectors.  We will grab the last
 *        of these empty sectors for our DLT_Tables.
 *
 */

#ifndef MANAGE_VOLUMES

#define MANAGE_VOLUMES 1

#define NEED_BYTE_DEFINED
#include "lvm_gbls.h"
#include "lvm_intr.h"


BOOLEAN Initialize_Volume_Manager( CARDINAL32 * Error_Code );  /* Returns TRUE if successful, FALSE otherwise. */

void Close_Volume_Manager( void );

void Discover_Volumes( CARDINAL32 * Error_Code );

void Commit_Volume_Changes( CARDINAL32 * Error_Code );

CARDINAL32 Get_Volume_Size( ADDRESS Handle, CARDINAL32 * Error_Code);

CARDINAL32 Get_Volume_Options(Volume_Data * VolumeRecord, CARDINAL32 * Error_Code);

/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 Reconcile_Drive_Letters( BOOLEAN Update_NON_LVM_Volumes_Only, CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_PartitionsOn_Volume                          */
/*                                                                   */
/*   Descriptive Name: Returns an array of partitions associated     */
/*                     with the Volume specified by Handle.          */
/*                                                                   */
/*   Input:Volume_Data * Volume_Information : The Volume_Data for the*/
/*                                            Volume of interest.    */
/*         CARDINAL32 * Error_Code : The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns a structure.  The structure has   */
/*           two components: an array of partition information       */
/*           records and the number of entries in the array.  The    */
/*           array will contain a partition information record for   */
/*           each partition which is part of the specified volume.   */
/*           If no errors occur, then *Error_Code will be 0.  If an  */
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling: Any memory allocated for the return value will  */
/*                   be freed.  The Partition_Information_Array      */
/*                   returned by this function will contain a NULL   */
/*                   pointer for Partition_Array, and have a Count of*/
/*                   0.  *Error_Code will be non-zero.               */
/*                                                                   */
/*                   If Handle is invalid, a trap is likely.         */
/*                                                                   */
/*   Side Effects:  Memory will be allocated to hold the array       */
/*                  returned by this function.                       */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Partition_Information_Records pointed  */
/*           to by Partition_Array pointer in the                    */
/*           Partition_Information_Array structure returned by this  */
/*           function.  The caller should free this memory when they */
/*           are done using it.                                      */
/*                                                                   */
/*********************************************************************/
Partition_Information_Array Get_Partitions_On_Volume( Volume_Data * Volume_Information, CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Convert_Fake_Volumes_On_PRM_To_Real_Volumes      */
/*                                                                   */
/*   Descriptive Name:  Converts any "fake" volumes created to       */
/*                      represent partitions on a PRM into real      */
/*                      compatibility volumes.  NOTE:  This will     */
/*                      not work for "fake" volumes created to       */
/*                      represent blank media or big floppy          */
/*                      formatted media!                             */
/*                                                                   */
/*   Input: CARDINAL32 DriveIndex - The index into the drive array   */
/*                                  for the entry representing the   */
/*                                  PRM whose "fake" volumes are to  */
/*                                  be converted to real volumes.    */
/*          CARDINAL32 * Error_Code - The address of a variable in   */
/*                                    which to place an error return */
/*                                    code.                          */
/*                                                                   */
/*   Output:  *Error_Code is set to LVM_ENGINE_NO_ERROR is this      */
/*            function completes successfully.  Any other value is an*/
/*            error code being returned due to an error being        */
/*            encountered.                                           */
/*                                                                   */
/*   Error Handling:  The only errors which can occur would be due   */
/*                    to corrupted data structures.  Thus, any error */
/*                    encountered by this function is an LVM Engine  */
/*                    internal error!                                */
/*                                                                   */
/*   Side Effects:  The status of the "fake" volumes on the          */
/*                  designated PRMs is altered.  If a commit         */
/*                  operation is done at some point after this       */
/*                  function completes, then the DLAT tables on the  */
/*                  PRM will be updated to reflect the new status.   */
/*                                                                   */
/*   Notes:  This function should not be used with "fake" volumes    */
/*           that were created to represent the entire PRM, such as  */
/*           those created to represent big floppy formatted media,  */
/*           or blank media (no partition table, and not big floppy  */
/*           formatted).                                             */
/*                                                                   */
/*********************************************************************/
void Convert_Fake_Volumes_On_PRM_To_Real_Volumes( CARDINAL32 DriveIndex, CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Volume_Features                              */
/*                                                                   */
/*   Descriptive Name: Returns the feature ID information for each of*/
/*                     the features that are installed on the        */
/*                     volume.                                       */
/*                                                                   */
/*   Input: Volume_Data * VolumeRecord - The volume to use.          */
/*                                                                   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: If successful, a Feature_Information_Array structure is */
/*           returned with a non-zero Count.  Also, *Error_Code will */
/*           be set to LVM_ENGINE_NO_ERROR.  If an error occurs,     */
/*           then the Count field in the structure will be 0 and     */
/*           (*Error_Code) will contain a non-zero error code.       */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  Memory is allocated using the LVM Engine's memory*/
/*                  manager for the array of Feature_ID_Data items   */
/*                  being returned.                                  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Feature_Information_Array _System Get_Volume_Features( Volume_Data * VolumeRecord, CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Issue_Volume_Feature_Command                     */
/*                                                                   */
/*   Descriptive Name: Issues a feature specific command to all of   */
/*                     features on a volume.                         */
/*                                                                   */
/*   Input: Volume_Data * VolumeRecord - The volume to use.          */
/*          CARDINAL32 Feature_ID - The numeric ID assigned to the   */
/*                                  feature which is to receive the  */
/*                                  command being issued.            */
/*          ADDRESS InputBuffer - A buffer containing the command and*/
/*                                any necessary information for the  */
/*                                feature to process the command.    */
/*          CARDINAL32 InputSize - The number of bytes in the        */
/*                                 InputBuffer.                      */
/*          ADDRESS * OutputBuffer - The address of a variable used  */
/*                                   to hold the location of the     */
/*                                   output buffer created by the    */
/*                                   feature in response to the      */
/*                                   command in InputBuffer.         */
/*          CARDINAL32 * OutputSize - The address of a variable used */
/*                                    to hold the number of bytes in */
/*                                    *OutputBuffer.                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: If successful, a Feature_Information_Array structure is */
/*           returned with a non-zero Count.  Also, *Error_Code will */
/*           be set to LVM_ENGINE_NO_ERROR.  If an error occurs,     */
/*           then the Count field in the structure will be 0 and     */
/*           (*Error_Code) will contain a non-zero error code.       */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  Memory is allocated using the LVM Engine's memory*/
/*                  manager for the array of Feature_ID_Data items   */
/*                  being returned.                                  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
 void _System Issue_Volume_Feature_Command( Volume_Data * VolumeRecord,
                                            CARDINAL32 Feature_ID,
                                            ADDRESS InputBuffer,
                                            CARDINAL32 InputSize,
                                            ADDRESS * OutputBuffer,
                                            CARDINAL32 * OutputSize,
                                            CARDINAL32 * Error_Code );



#endif

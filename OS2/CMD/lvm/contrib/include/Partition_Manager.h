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
 * Module: Partition_Manager.h
 */

/*
 * Change History:
 *
 */

/*
 * Functions: The following functions are actually declared in
 *            LVM_INTERFACE.H but are implemented in Partition_Manager.C:
 *
 *
 * Description: Since the LVM Engine Interface is very
 *              large, it became undesirable to put all of the code
 *              to implement it in a single file.  Instead, the
 *              implementation of the LVM Engine Interface was divided
 *              among several C files which would have access to a
 *              core set of data.  Engine.H and Engine.C define,
 *              allocate, and initialize that core set of data.
 *              Partition_Manager.H and Partition_Manager.C implement
 *              the partition management functions found in the
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
 *            /                    |                          \
 *    BootManager.C             Logging.C                    Handle_Manager.C
 *
 * Notes: LVM Drive Letter Assignment Tables (DLA_Tables) appear on the
 *        last sector of each track containing a valid MBR or EBR.  Since
 *        partitions must be track aligned, any track containing an MBR or
 *        EBR will be almost all empty sectors.  We will grab the last
 *        of these empty sectors for our DLT_Tables.
 *
 * Notes: This module makes use of DLIST.
 *
 */


#ifndef MANAGE_PARTITIONS

#define MANAGE_PARTITIONS 1

/*--------------------------------------------------
 * Type definitions
 --------------------------------------------------*/

typedef struct _Partition_Data_To_Update {
                                           BOOLEAN  Update_Drive_Letter;
                                           char     New_Drive_Letter;
                                           BOOLEAN  Update_Volume_Name;
                                           char *   New_Name;
                                           BOOLEAN  Update_Boot_Manager_Status;
                                           BOOLEAN  On_Menu;
                                           BOOLEAN  Update_Volume_Spanning;
                                           BOOLEAN  Spanned_Volume;
                                         } Partition_Data_To_Update;


/*--------------------------------------------------
 * Functions Available.
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: Initialize_Partition_Manager                     */
/*                                                                   */
/*   Descriptive Name: Initializes the Partition Manager for use.    */
/*                     This function must be called before any other */
/*                     functions in this module are used.  Also, this*/
/*                     function assumes that the DriveArray has      */
/*                     already been created and initialized!         */
/*                                                                   */
/*   Input: BOOLEAN Ignore_CHS - Tells the partition manager to skip */
/*                               all partition record CHS consistency*/
/*                               checks.  This will allow drives with*/
/*                               CHS vs. (size, offset) address      */
/*                               problems to be processed as long as */
/*                               there are no other errors in the    */
/*                               drive's partitioning.               */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be set to LVM_ENGINE_NO_ERROR if this  */
/*           function succeeds.  *Error_Code will contain an error   */
/*           code otherwise.                                         */
/*                                                                   */
/*   Error Handling: This function will return an error code if an   */
/*                   error is detected.  It will restore the state   */
/*                   of the DriveArray to what it was before this    */
/*                   function is called.                             */
/*                                                                   */
/*   Side Effects: Entries in the DriveArray will have their         */
/*                 Partitions lists modified.                        */
/*                                                                   */
/*   Notes: The DriveArray must be created and fully initialized     */
/*          before this function is called.  The Handle Manager must */
/*          also be Initialized prior to calling this function.      */
/*                                                                   */
/*********************************************************************/
void Initialize_Partition_Manager( BOOLEAN Ignore_CHS, CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Close_Partition_Manager                          */
/*                                                                   */
/*   Descriptive Name: This function closes the Partition Manager    */
/*                     and frees all memory that it is responsible   */
/*                     for managing.                                 */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects: All memory controlled by this module is freed.    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Close_Partition_Manager( void );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Discover_Partitions                              */
/*                                                                   */
/*   Descriptive Name: This function walks the DriveArray, and, for  */
/*                     drive with an entry in the DriveArray, it     */
/*                     reads the partitioning information for the    */
/*                     drive and converts it into a Partition List.  */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If successful, the *ErrorCode will be                   */
/*              LVM_ENGINE_NO_ERROR and each drive in the DriveArray */
/*              will have its Partitions list filled in.             */
/*           If an unrecoverable error is encountered, then          */
/*              *Error_Code will contain an error code.              */
/*                                                                   */
/*   Error Handling: This function will not return an error unless   */
/*                   none of the drives in the DriveArray could be   */
/*                   accessed successfully.  In any case, the        */
/*                   IO_Error and Corrupt fields in each entry in    */
/*                   the drive array will be set based upon whether  */
/*                   the drive could be accessed and whether the     */
/*                   partitioning information found (if any) was     */
/*                   valid.                                          */
/*                                                                   */
/*   Side Effects: Each disk drive with an entry in the DiskArray    */
/*                 will have its partitioning information read.      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Discover_Partitions( CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Commit_Partition_Changes                         */
/*                                                                   */
/*   Descriptive Name: This function writes out the partitioning     */
/*                     information for any drive which has had its   */
/*                     partitioning information altered in any way.  */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If successful, then *Error_Code will be                 */
/*              LVM_ENGINE_NO_ERROR and the affected disk drives will*/
/*              have had their partitioning information updated.     */
/*                                                                   */
/*   Error Handling: If there is an I/O error, the DriveArray entry  */
/*                   for the partition experiencing the error will be*/
/*                   set to TRUE.                                    */
/*                                                                   */
/*   Side Effects: If there is an I/O error, the DriveArray entry    */
/*                 for the partition experiencing the error will be  */
/*                 set to TRUE.                                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Commit_Partition_Changes( CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_PartitionsOn_Drive                           */
/*                                                                   */
/*   Descriptive Name: Returns an array of partitions associated     */
/*                     with the drive specified by DriveArrayIndex.  */
/*                                                                   */
/*   Input:CARDINAL32 DriveArrayIndex : The index into the drive     */
/*                                      array for the drive to use.  */
/*         CARDINAL32 * Error_Code - The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns a structure.  The structure has   */
/*           two components: an array of partition information       */
/*           records and the number of entries in the array.  The    */
/*           array will contain a partition information record for   */
/*           each partition and block of free space on the drive.    */
/*           If no errors occur, then *Error_Code will be 0.  If an  */
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling: Any memory allocated for the return value will  */
/*                   be freed.  The Partition_Information_Array      */
/*                   returned by this function will contain a NULL   */
/*                   pointer for Partition_Array, and have a Count of*/
/*                   0.  *Error_Code will be non-zero.               */
/*                                                                   */
/*                   If DriveArrayIndex is invalid, a trap is likely.*/
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
Partition_Information_Array Get_Partitions_On_Drive( CARDINAL32 DriveArrayIndex, CARDINAL32 * Error_Code );


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
void Set_Partition_Information_Record( Partition_Information_Record * Partition_Information, Partition_Data * PartitionRecord);


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
CARDINAL32 Get_Partition_Options(Partition_Data * PartitionRecord, CARDINAL32 * Error);


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
void Convert_To_CHS(LBA Starting_Sector, CARDINAL32 Index, CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector );


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
void Convert_CHS_To_Partition_Table_Format( CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector );


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
void _System Update_Partitions_Volume_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);


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
void _System Convert_To_CHS_With_No_Checking(LBA Starting_Sector, CARDINAL32 Index, CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code );


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
void _System Convert_CHS_To_Partition_Table_Format_With_Checking( CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:  None.                                            */
/*   Notes:  None.                                                   */
/*********************************************************************/
//void _System Convert_Partition_Table_Format_To_CHS_With_Checking( CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code );


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
void _System Convert_CHS_To_LBA(CARDINAL32 Drive_Index, CARDINAL32 Cylinder, CARDINAL32 Head, CARDINAL32 Sector, LBA * Starting_Sector, CARDINAL32 * Error_Code);


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partition_Features                           */
/*                                                                   */
/*   Descriptive Name: Returns the feature ID information for each of*/
/*                     the features that are installed on the        */
/*                     partition.                                    */
/*                                                                   */
/*   Input: Partition_Data * PartitionRecord - The partition to use. */
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
Feature_Information_Array _System Get_Partition_Features( Partition_Data * PartitionRecord, CARDINAL32 * Error_Code );


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
void _System Update_Partitions_Parent_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);


#endif

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
 * Module: CRC.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: CARDINAL32 CalculateCRC
 *            void       Build_CRC_Table
 *
 * Description: The functions in this module provide a means of calculating
 *              the 32 bit CRC for a block of data.  Build_CRC_Table must
 *              be called to initialize this module.  CalculateCRC must
 *              NOT be used until after Build_CRC_Table has been called.
 *              Once Build_CRC_Table has been called, CalculateCRC can
 *              be used to calculate the CRC of the data residing in a
 *              user specified buffer.
 *
 * Notes: Build_CRC_Table only needs to be called once.  Once the internal
 *        CRC table has been built, there is no need to build it again.
 *
 */

#define NEED_BYTE_DEFINED
#include "lvm_gbls.h" /* CARDINAL32, BYTE, ADDRESS */
#include "logging.h"  /* FUNCTION_ENTRY, FUNCTION_EXIT */


/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
#define CRC_POLYNOMIAL     0xEDB88320L


/*--------------------------------------------------
 * There are no Private Type Definitions
 --------------------------------------------------*/


/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static CARDINAL32 CRC_Table[ 256 ];                     /* Used by the CalculateCRC function. */

static BOOLEAN CRC_Table_Built=FALSE;  /* TRUE=table is built, FALSE=table not built yet */


/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Build_CRC_Table                                  */
/*                                                                   */
/*   Descriptive Name: This module implements the CRC function using */
/*                     a table driven method.  The required table    */
/*                     must be setup before the CalculateCRC function*/
/*                     can be used.  This table only needs to be set */
/*                     up once.  This function sets up the CRC table */
/*                     needed by CalculateCRC.                       */
/*                                                                   */
/*   Input: None                                                     */
/*                                                                   */
/*   Output: None                                                    */
/*                                                                   */
/*   Error Handling: N/A                                             */
/*                                                                   */
/*   Side Effects:  The internal CRC Table is initialized.           */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Build_CRC_Table( void )
{
    CARDINAL32 i;
    CARDINAL32 j;
    CARDINAL32 CRC;

    for ( i = 0; i <= 255 ; i++ )
    {

      CRC = i;

      for ( j = 8 ; j > 0; j-- )
      {

         if ( CRC & 1 )
            CRC = ( CRC >> 1 ) ^ CRC_POLYNOMIAL;
          else
            CRC >>= 1;
      }

      CRC_Table[ i ] = CRC;

    }
    CRC_Table_Built = TRUE;
}

/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: CalculateCRC                                     */
/*                                                                   */
/*   Descriptive Name: This function calculates the CRC value for    */
/*                     the data in the buffer specified by Buffer.   */
/*                                                                   */
/*   Input: CARDINAL32   CRC : This is the starting CRC.  If you are */
/*                             starting a new CRC calculation, then  */
/*                             this should be set to INITIAL_CRC.  If*/
/*                             you are continuing a CRC calculation  */
/*                             (i.e. all of the data did not fit in  */
/*                             the buffer so you could not calculate */
/*                             the CRC in a single operation), then  */
/*                             this is the CRC output by the last    */
/*                             CalculateCRC call.                    */
/*                                                                   */
/*   Output: The CRC for the data in the buffer, based upon the value*/
/*           of the input parameter CRC.                             */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _System CalculateCRC( CARDINAL32 CRC, ADDRESS Buffer, CARDINAL32 BufferSize)
{

  BYTE   *   Current_Byte;
  CARDINAL32 Temp1;
  CARDINAL32 Temp2;
  CARDINAL32 I;             /* Used to walk the buffer. */

//  FUNCTION_ENTRY(__FUNCTION__);

  /* Establish access to the buffer on a byte by byte basis. */
  Current_Byte = ( BYTE *) Buffer;

  /* Make sure the CRC table is available */
  if (CRC_Table_Built==FALSE)  Build_CRC_Table();

  /* Process each byte in the buffer. */
  for ( I = 0; I < BufferSize; I++ )
  {

    Temp1 = (CRC >> 8) & 0x00FFFFFFL;
    Temp2 = CRC_Table[ ( CRC ^ (CARDINAL32) *Current_Byte ) & (CARDINAL32) 0xff ];
    Current_Byte++;
    CRC = Temp1 ^ Temp2;

  }

//  FUNCTION_EXIT(__FUNCTION__);
  return( CRC );
}

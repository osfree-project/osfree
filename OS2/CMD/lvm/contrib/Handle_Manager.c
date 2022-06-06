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
 * Module: Handle_Manager.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: BOOLEAN Initizlize_Handle_Manager
 *            ADDRESS Create_Handle
 *            void    Destroy_Handle
 *            void    Destroy_All_Handles
 *            void    Translate_Handle
 *
 * Description: This module provides a uniform way for creating a handle
 *              and associating it with something.
 *
 * Notes: This module makes use of DLIST.
 *
 */

#include <stdlib.h>           /* NULL */
#include "gbltypes.h"         /* ADDRESS, CARDINAL32 */
#include "dlist.h"            /* TAG, InsertObject, GetObject, GetTag, GoToSpecifiedItem, GoToEndOfList, CreateList, GetHandle, DeleteItem */
#include "Handle_Manager.h"   /* Included to ensure that Handle_Manager.C and Handle_Manager.H are consistent. */

#ifdef DEBUG

#ifdef PARANOID

#include <assert.h>

#endif

#endif

/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
 #define HANDLE_MASK  0xFC257107



/*--------------------------------------------------
 * There are no private Type definitions
 --------------------------------------------------*/



/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
DLIST  Handles = (DLIST) NULL;           /* Used to create, track, translate, and destroy handles. */


/*--------------------------------------------------
 * There are no private functions.
 --------------------------------------------------*/


/*--------------------------------------------------
 * There are no public global variables
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: Initialize_Handle_Manager                        */
/*                                                                   */
/*   Descriptive Name: Initializes the Handle Manager for use.       */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: If successful, then the function return value will be   */
/*              TRUE.                                                */
/*           If there is a failure, the function return value will   */
/*              be FALSE.                                            */
/*                                                                   */
/*   Error Handling: The function return value will be FALSE if an   */
/*                   error occurs.                                   */
/*                                                                   */
/*   Side Effects: Memory will be allocated for internal structures. */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN Initialize_Handle_Manager(void)
{

  /* Has the Handle Manager already been initialized? */
  if ( Handles != NULL )
  {

    /* The Handle Manager has already been initialized!  Nothing to do. */
    return TRUE;

  }

  /* Create the Handles list. */
  Handles = CreateList();

  /* Did we succeed? */
  if ( Handles == NULL )
  {

    /* We could not create the Handles list.  We are probably out of memory! */

    /* Report the error. */
    return FALSE;

  }

  /* The Handle Manager has been initialized. */

  /* Indicate success. */
  return TRUE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Create_Handle                                    */
/*                                                                   */
/*   Descriptive Name: Takes an object, its TAG, and associates a    */
/*                     Handle with them.                             */
/*                                                                   */
/*   Input: ADDRESS Object : The address of the object to be         */
/*                           associated with a handle.               */
/*          TAG ObjectTag : The TAG value of Object.  TAG values are */
/*                          explained in the DLIST module.           */
/*          CARDINAL32 ObjectSize : The size of the item whose       */
/*                                  address is given in Object.      */
/*          CARDINAL32 * Error_Code : This is the address of a       */
/*                                    variable which is to hold any  */
/*                                    error codes generated by this  */
/*                                    function.                      */
/*                                                                   */
/*   Output: If successful, then the function return value will be   */
/*              a non-zero Handle and *Error_Code will be            */
/*              LVM_ENGINE_NO_ERROR.                                 */
/*           If there is a failure, *Error will contain a non-zero   */
/*              error code and the function return value will be 0.  */
/*                                                                   */
/*   Error Handling: *Error_Code will be non-zero if an error occurs.*/
/*                                                                   */
/*   Side Effects: A new handle may be created.                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System Create_Handle( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, CARDINAL32 * Error_Code )
{

  ADDRESS ReturnValue = NULL;    /* Used to hold the Handle that will be returned by this function. */

  /* Has the Handles list been created yet? */
  if ( Handles == NULL )
  {

    /* There is no handle list, which means that this module has not been initialized yet! */
    *Error_Code = HANDLE_MANAGER_NOT_INITIALIZED;

    return NULL;

  }

  /* Since the Handles list exists, lets add the object and get a handle for it. */
  ReturnValue = InsertObject(Handles,ObjectSize,Object,ObjectTag,NULL, AppendToList, FALSE, Error_Code);

  /* Was the item successfully added? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Set *Error_Code to a value defined for the Handle Manager. */
    if ( *Error_Code != DLIST_OUT_OF_MEMORY )
    {

#ifdef DEBUG

#ifdef PARANOID

      assert(0);

#endif

#endif

      *Error_Code = HANDLE_MANAGER_INTERNAL_ERROR;

    }
    else
    {

      *Error_Code = HANDLE_MANAGER_OUT_OF_MEMORY;

    }

    return NULL;

  }

  /* Indicate success and return! */
  *Error_Code = HANDLE_MANAGER_NO_ERROR;

  return ( ADDRESS ) ( ( CARDINAL32 ) ReturnValue ^ ( CARDINAL32 ) HANDLE_MASK );

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Destroy_Handle                                   */
/*                                                                   */
/*   Descriptive Name: Removes the association between an object and */
/*                     a Handle.  The Handle is thereby eliminated.  */
/*                                                                   */
/*   Input: ADDRESS Handle : The handle to be eliminated.            */
/*          CARDINAL32 * Error_Code : This is the address of a       */
/*                                    variable which is to hold any  */
/*                                    error codes generated by this  */
/*                                    function.                      */
/*                                                                   */
/*   Output: If successful, then *Error_Code will be                 */
/*              LVM_ENGINE_NO_ERROR.                                 */
/*           If there is a failure, *Error will contain a non-zero   */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling: *Error_Code will be non-zero if an error occurs.*/
/*                                                                   */
/*   Side Effects: A handle may be eliminated.                       */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Destroy_Handle( ADDRESS Handle, CARDINAL32 * Error_Code )
{

  /* Has the Handles list been created yet? */
  if ( Handles == NULL )
  {

    /* There is no handle list, which means that this module has not been initialized yet! */
    *Error_Code = HANDLE_MANAGER_NOT_INITIALIZED;

    return;

  }


  /* Unmask Handle. */
  Handle = ( ADDRESS ) ( ( CARDINAL32 ) Handle ^ ( CARDINAL32 ) HANDLE_MASK);

  /* Each handle represents an item in the Handles list.  */

  /* Delete this item from the list, and its handle will no longer be valid. */
  DeleteItem(Handles,FALSE,Handle,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert(*Error_Code == DLIST_SUCCESS);

#else

  /* Was there an error. */
  if ( *Error_Code != DLIST_SUCCESS )
  {

      *Error_Code = HANDLE_MANAGER_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Destroy_All_Handles                              */
/*                                                                   */
/*   Descriptive Name: Eliminates all existing handles being tracked */
/*                     by the Handle Manager.                        */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code : This is the address of a       */
/*                                    variable which is to hold any  */
/*                                    error codes generated by this  */
/*                                    function.                      */
/*                                                                   */
/*   Output: If successful, then *Error_Code will be                 */
/*              HANDLE_MANAGER_NO_ERROR.                             */
/*           If there is a failure, *Error will contain a non-zero   */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling: *Error_Code will be non-zero if an error occurs.*/
/*                                                                   */
/*   Side Effects: All handles may be eliminated.                    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Destroy_All_Handles( CARDINAL32 * Error_Code )
{

  /* Has the Handles list been created yet? */
  if ( Handles == NULL )
  {

    /* There is no handle list, which means that this module has not been initialized yet! */
    *Error_Code = HANDLE_MANAGER_NOT_INITIALIZED;

    return;

  }

  /* Now delete all entries in the Handle list. */
  DeleteAllItems( Handles, FALSE, Error_Code );

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* We have an internal error! */
    *Error_Code = HANDLE_MANAGER_INTERNAL_ERROR;

  }
  else
  {

    /* Indicate success. */
    *Error_Code = HANDLE_MANAGER_NO_ERROR;

  }

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Translate_Handle                                 */
/*                                                                   */
/*   Descriptive Name: Returns a pointer to, and the TAG value of,   */
/*                     the object associated with a handle.          */
/*                                                                   */
/*   Input: ADDRESS Handle : The handle to translate.                */
/*          ADDRESS * Object : The location of a pointer into which  */
/*                             the address of the object associated  */
/*                             with Handle is to be placed.          */
/*          TAG * ObjectTag : The address of a variable to hold the  */
/*                            TAG value of the object associated     */
/*                            with Handle.  TAG values are explained */
/*                            in the DLIST module.                   */
/*          CARDINAL32 * Error_Code : This is the address of a       */
/*                                    variable which is to hold any  */
/*                                    error codes generated by this  */
/*                                    function.                      */
/*                                                                   */
/*   Output: If successful, then *Object will be set to the address  */
/*              of the object associated with Handle, *ObjectTag will*/
/*              contain the TAG of the object associated with Handle,*/
/*              and *Error_Code will be LVM_ENGINE_NO_ERROR.         */
/*           If there is a failure, *Error will contain a non-zero   */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling: *Error_Code will be non-zero if a recoverable   */
/*                   error occurs.  If Handle is invalid, then a     */
/*                   trap or exception may occur.                    */
/*                                                                   */
/*   Side Effects: If Handle is invalid, a trap or exception may     */
/*                 occur.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Translate_Handle( ADDRESS Handle, ADDRESS * Object, TAG * ObjectTag, CARDINAL32 * Error_Code )
{

  CARDINAL32   ItemSize;      /* Used with the GetTag function. */


  /* To translate a handle, we need to get the object associated with the handle from the Handles list.  To do that, we need to :

      1.  Make the item associated with Handle the current item in the Handles list.
      2.  Get the ItemSize and TAG
      3.  Get the Item itself.

  */


  /* Has the Handles list been created yet? */
  if ( Handles == NULL )
  {

    /* There is no handle list, which means that this module has not been initialized yet! */
    *Error_Code = HANDLE_MANAGER_NOT_INITIALIZED;

    return;

  }

  /* Unmask Handle. */
  Handle = ( ADDRESS ) ( ( CARDINAL32 ) Handle ^ ( CARDINAL32 ) HANDLE_MASK);

  /* Before we can get the item, we must get its size and TAG. */
  *ObjectTag = GetTag(Handles, Handle, &ItemSize, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert(*Error_Code == DLIST_SUCCESS);

#else

  /* Was there an error? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We have some kind of internal error here as this should not have failed! */
    *Error_Code = HANDLE_MANAGER_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Now we can use the TAG and ItemSize we just got to retrieve the current item. */
  *Object = GetObject(Handles, ItemSize, *ObjectTag, Handle, FALSE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert(*Error_Code == DLIST_SUCCESS);

#else

  /* Was there an error? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We have some kind of internal error here as this should not have failed! */
    *Error_Code = HANDLE_MANAGER_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Signal success. */
  *Error_Code = HANDLE_MANAGER_NO_ERROR;

  return;

}



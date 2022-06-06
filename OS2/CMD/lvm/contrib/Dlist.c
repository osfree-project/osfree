/*
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
 * Module: Dlist.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: DLIST       CreateList
 *            void        AppendItem
 *            void        AppendObject
 *            void        InsertItem
 *            void        InsertObject
 *            void        GetItem
 *            void        GetItem_By_Handle
 *            void        GetNextItem
 *            void        GetPreviousItem
 *            ADDRESS     GetObject
 *            ADDRESS     GetObject_By_Handle
 *            ADDRESS     GetNextObject
 *            ADDRESS     GetPreviousObject
 *            void        ExtractItem
 *            void        ExtractItem_By_Handle
 *            ADDRESS     ExtractObject
 *            ADDRESS     ExtractObject_By_Handle
 *            void        ReplaceItem
 *            void        ReplaceItem_By_Handle
 *            ADDRESS     ReplaceObject
 *            ADDRESS     ReplaceObject_By_Handle
 *            void        DeleteItem
 *            void        DeleteItem_By_Handle
 *            TAG         GetTag
 *            ADDRESS     GetHandle
 *            CARDINAL32  GetListSize
 *            BOOLEAN     ListEmpty
 *            BOOLEAN     AtEndOfList
 *            BOOLEAN     AtStartOfList
 *            void        DestroyList
 *            void        NextItem
 *            void        PreviousItem
 *            void        GoToStartOfList
 *            void        GoToEndOfList
 *            void        GoToSpecifiedItem
 *            void        SortList
 *            void        ForEachItem
 *            void        PruneList
 *            void        AppendList
 *
 * Description:  This module implements a simple, generic, doubly linked list.
 *               Data objects of any type can be placed into a linked list
 *               created by this module.  Futhermore, data objects of different
 *               types may be placed into the same linked list.
 *
 * Notes:  SEE THE INITIAL COMMENT IN DLIST.H!
 *
 */

#include <stdlib.h>   /* free */
#include <string.h>   /* memcpy */
#include "dlist.h"    /* Import dlist.h so that the compiler can check the
                         consistency of the declarations in dlist.h against
                         those in this module.                              */
#ifdef USE_POOLMAN

  #include "poolman.h"  /* POOL, CreatePool, AllocateFromPool, DeallocateToPool, DestroyPool, SmartMalloc, SmartFree */

#endif

#ifdef DEBUG

  #ifdef PARANOID

    #include <assert.h>

  #endif

#endif


/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/

/* Each list contains a Verify field.  Before any operations are performed on
   the list, this module checks the Verify field to see if the VerifyValue
   is in the field.  If the VerifyValue is not found in the Verify field,
   the operation is aborted.                                                 */
#define VerifyValue 39646966L


/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/


/* A list has the following structure:

                                   DLIST         An item of type DLIST is a pointer
                                    |            to the following structure:
                                    |
                                    V
                          ----------------------
                          |                    |
                          |    ControlNode     |
                          ----------------------
                         /           /          \
         Pointer to the /           /            \
         LinkNode of   /           /              \  Pointer to the LinkNode of
         the first    /           /                \ the last item in the list.
         item in the /           / Pointer to the   \
         list.      /           /  LinkNode of the   \
                   /           /   Current Item in    \             NOTE:  All LinkNodes
                  /           /    the list.           \                   have a pointer
                 V           V                          V                  to the ControlNode.
        -------------      -------------                -------------
NULL <--|  LinkNode | <--> | LinkNode  |<-->  ...  <--> |  LinkNode |--> NULL
        -------------      -------------                -------------
             |                   |                            |
             |                   |                            |
             V                   V                            V
           Data                Data                         Data


NOTES:  The ControlNode does the bookeeping for things which affect the
        entire list.  It tracks the number of items in the list, where the
        first, current, and last items in the list reside.  It also holds
        the Verify field which is used to see if a pointer passed to us
        really does point to a list created by this module.

        Each data item placed in the list gets its own LinkNode.  The
        LinkNode tracks where in memory the data item is, how big the
        data item is, what item tag the user gave that data item, and
        where the LinkNodes for the previous and next items in the list
        are located.  By tracking this information here, the user does
        not have to worry about placing fields for this information
        inside of his data in order for his data items to be compatible
        with this module.  Thus, the operation of the list module is
        decoupled from the data that is placed in the list.

*/

struct LinkNodeRecord
{
  ADDRESS                   DataLocation;        /* Where the data associated with this LinkNode is */
  CARDINAL32                DataSize;            /* The size of the data associated with this LinkNode. */
  TAG                       DataTag;             /* The item tag the user gave to the data. */
  struct MasterListRecord * ControlNodeLocation; /* The control node of the list containing this
                                                    item.                                         */
  struct LinkNodeRecord *   NextLinkNode;        /* The LinkNode of the next item in the list. */
  struct LinkNodeRecord *   PreviousLinkNode;    /* The LinkNode of the item preceeding this one in the list. */
};

typedef struct LinkNodeRecord LinkNode;

struct MasterListRecord
{
  CARDINAL32      ItemCount;             /* The number of items in the list. */
  LinkNode *      StartOfList;           /* The address of the LinkNode of the first item in the list. */
  LinkNode *      EndOfList;             /* The address of the LinkNode of the last item in the list. */
  LinkNode *      CurrentItem;           /* The address of the LinkNode of the current item in the list. */
#ifdef USE_POOLMAN
  POOL            NodePool;              /* The pool of LinkNodes for this DLIST. */
#endif
  CARDINAL32      Verify;                /* A field to contain the VerifyValue which marks this as a list created by this module. */
};

typedef struct MasterListRecord ControlNode;


/*--------------------------------------------------
 Private global variables.
--------------------------------------------------*/
BOOLEAN  ErrorsFound = FALSE; /* Used to track whether or not errors have
                                 been found.  Can be used with a memory access
                                 breakpoint to stop program execution when
                                 an error is detected so that the type of
                                 error can be seen.                              */


/*--------------------------------------------------
 There are no private functions.
--------------------------------------------------*/



/*--------------------------------------------------
 There are no public global variables.
--------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

#ifdef USE_POOLMAN


/*********************************************************************/
/*                                                                   */
/*   Function Name:  CreateList                                      */
/*                                                                   */
/*   Descriptive Name: This function allocates and initializes the   */
/*                     data structures associated with a list and    */
/*                     then returns a pointer to these structures.   */
/*                                                                   */
/*   Input: CARDINAL32 InitialPoolSize - Each List gets a pool of    */
/*                                     link nodes.  When items are   */
/*                                     added to the List, a link node*/
/*                                     is removed from the pool.     */
/*                                     When an item is removed from  */
/*                                     the List, the link node used  */
/*                                     for that item is returned to  */
/*                                     the pool.  InitialPoolSize is */
/*                                     the number of link nodes to   */
/*                                     place in the pool when the    */
/*                                     pool is created.              */
/*          CARDINAL32 MaximumPoolSize - When the pool runs out of   */
/*                                     link nodes, new nodes are     */
/*                                     allocated by the pool.  When  */
/*                                     these links start being       */
/*                                     returned to the pool, the pool*/
/*                                     will grow.  This parameter    */
/*                                     puts a limit on how big the   */
/*                                     pool may grow to.  Once the   */
/*                                     pool reaches this size, any   */
/*                                     link nodes being returned to  */
/*                                     the pool will be deallocated. */
/*          CARDINAL32 PoolIncrement - When the pool runs out of link*/
/*                                   nodes and more are required,    */
/*                                   the pool will allocate one or   */
/*                                   more link nodes.  This tells the*/
/*                                   pool how many link nodes to     */
/*                                   allocate at one time.           */
/*                                                                   */
/*   Output: If Success : The function return value will be non-NULL */
/*                                                                   */
/*           If Failure : The function return value will be NULL.    */
/*                                                                   */
/*   Error Handling:  The function will only fail if it can not      */
/*                    allocate enough memory to create the new list  */
/*                    and its associated pool of link nodes.         */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
DLIST _System CreateList(CARDINAL32 InitialPoolSize,
                         CARDINAL32 MaximumPoolSize,
                         CARDINAL32 PoolIncrement)
{

  /* This function returns a DLIST.  A variable of type DLIST is really a
     pointer to to a ControlNode which has been typecast to (void *).
     We will therefore declare a variable of type (ControlNode *) to
     work with while in this function and then, if successful, typecast
     this variable to DLIST for the return value.                           */
  ControlNode * ListData;

  /* We must now initialize the data structures for the DLIST we are
     creating.  We will start by allocating the memory.                    */
  ListData = (ControlNode *) SmartMalloc(sizeof(ControlNode));
  if (ListData == NULL)
  {

    return NULL;
  }

  /* Now that we have the memory, lets initialize the fields in the master record. */
  ListData->ItemCount = 0;         /* No items in the list. */
  ListData->StartOfList = NULL;    /* Since the list is empty, there is no first item */
  ListData->EndOfList = NULL;      /* Since the list is empty, there is no last item */
  ListData->CurrentItem = NULL;    /* Since the list is empty, there is no current item */

  /* Create the pool of link nodes for this list. */
  ListData->NodePool = CreatePool(sizeof(LinkNode),InitialPoolSize, MaximumPoolSize, PoolIncrement,FALSE);

  if ( ListData->NodePool != NULL )
  {

    /* The DLIST has been successfully created and is ready to use. */
  #ifdef DEBUG

    ListData->Verify = VerifyValue;  /* Initialize the Verify field so that this list will recognized as being valid. */

  #endif


  }
  else
  {

    /* We failed to create the NodePool!  We will
       destroy this DLIST and return to caller.                           */
  #ifdef USE_POOLMAN
    SmartFree(ListData);
  #else
    free(ListData);
  #endif

    return NULL;

  }


  /* Set the return value. */
  return (DLIST) ListData;

}


#else


/*********************************************************************/
/*                                                                   */
/*   Function Name:  CreateList                                      */
/*                                                                   */
/*   Descriptive Name: This function allocates and initializes the   */
/*                     data structures associated with a list and    */
/*                     then returns a pointer to these structures.   */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: If Success : The function return value will be non-NULL */
/*                                                                   */
/*           If Failure : The function return value will be NULL.    */
/*                                                                   */
/*   Error Handling:  The function will only fail if it can not      */
/*                    allocate enough memory to create the new list. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
DLIST _System CreateList( void )
{

  /* This function returns a DLIST.  A variable of type DLIST is really a
     pointer to to a ControlNode which has been typecast to (void *).
     We will therefore declare a variable of type (ControlNode *) to
     work with while in this function and then, if successful, typecast
     this variable to DLIST for the return value.                           */
  ControlNode * ListData;

  /* We must now initialize the data structures for the DLIST we are
     creating.  We will start by allocating the memory.                    */
  ListData = (ControlNode *) malloc(sizeof(ControlNode));
  if (ListData == NULL)
  {

    return NULL;
  }

  /* Now that we have the memory, lets initialize the fields in the master record. */
  ListData->ItemCount = 0;         /* No items in the list. */
  ListData->StartOfList = NULL;    /* Since the list is empty, there is no first item */
  ListData->EndOfList = NULL;      /* Since the list is empty, there is no last item */
  ListData->CurrentItem = NULL;    /* Since the list is empty, there is no current item */

  #ifdef DEBUG

  ListData->Verify = VerifyValue;  /* Initialize the Verify field so that this list will recognized as being valid. */

  #endif

  /* Set the return value. */
  return (DLIST) ListData;

}


#endif


/*********************************************************************/
/*                                                                   */
/*   Function Name: InsertItem                                       */
/*                                                                   */
/*   Descriptive Name:  This function inserts an item into a DLIST.  */
/*                      The item can be placed either before or      */
/*                      after the current item in the DLIST.         */
/*                                                                   */
/*   Input:  DLIST          ListToAddTo : The list to which the      */
/*                                        data item is to be         */
/*                                        added.                     */
/*           CARDINAL32    ItemSize : The size of the data item, in  */
/*                                    bytes.                         */
/*           ADDRESS       ItemLocation : The address of the data    */
/*                                        to append to the list      */
/*           TAG           ItemTag : The item tag to associate with  */
/*                                   item being appended to the list */
/*           ADDRESS TargetHandle : The item in ListToAddTo which    */
/*                                   is used to determine where      */
/*                                   the item being transferred will */
/*                                   be placed.  If this is NULL,    */
/*                                   then the current item in        */
/*                                   ListToAddTo will be used.       */
/*           Insertion_Modes InsertMode : This indicates where,      */
/*                                   relative to the item in         */
/*                                   ListToAddTo specified by        */
/*                                   Target_Handle, the item being   */
/*                                   inserted can be placed.         */
/*           BOOLEAN MakeCurrent : If TRUE, the item being inserted  */
/*                                 into ListToAddTo becomes the      */
/*                                 current item in ListToAddTo.      */
/*           CARDINAL32 *  Error : The address of a variable to hold */
/*                                 the error return code.            */
/*                                                                   */
/*   Output:  If the operation is successful, then *Error will be    */
/*            set to 0 and the function return value will be the     */
/*            handle for the item that was appended to the list.     */
/*            If the operation fails, then *Error will contain an    */
/*            error code and the function return value will be NULL. */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                       ListToAddTo does not point to a valid       */
/*                           list                                    */
/*                       ItemSize is 0                               */
/*                       ItemLocation is NULL                        */
/*                       The memory required to hold a copy of the   */
/*                           item can not be allocated.              */
/*                       The memory required to create a LINK NODE   */
/*                           can not be allocated.                   */
/*                       TargetHandle is invalid or is for an item   */
/*                           in another list.                        */
/*                   If this routine fails, an error code is returned*/
/*                   and any memory allocated by this function is    */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  The item to add is copied to the heap to                */
/*           avoid possible conflicts with the usage of              */
/*           local variables in functions which process              */
/*           DLISTs.  However, a pointer to a local variable         */
/*           should not be appended to the DLIST.                    */
/*                                                                   */
/*           It is assumed that TargetHandle is valid, or is at least*/
/*           the address of an accessible block of storage.  If      */
/*           TargetHandle is invalid, or is not the address of an    */
/*           accessible block of storage, then a trap or exception   */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System InsertItem ( DLIST           ListToAddTo,
                             CARDINAL32      ItemSize,
                             ADDRESS         ItemLocation,
                             TAG             ItemTag,
                             ADDRESS         TargetHandle,
                             Insertion_Modes Insert_Mode,
                             BOOLEAN         MakeCurrent,
                             CARDINAL32 *    Error)

{

  ADDRESS         Buffer;     /* Used during the allocation of space on the heap to hold the item being added to the list. */
  ADDRESS         Handle;     /* Used to capture the handle of the item being inserted. */

  /* Check the size and location of the item to add to the list. */
  if ( ItemLocation == NULL )
  {

    *Error = DLIST_BAD_ITEM_POINTER;
    return NULL;
  }

  if ( ItemSize == 0)
  {
    *Error = DLIST_ITEM_SIZE_ZERO;
    return NULL;

  }

  /* Allocate memory to hold the item being added to the list. */
#ifdef USE_POOLMAN
  Buffer = SmartMalloc(ItemSize);
#else
  Buffer = malloc(ItemSize);
#endif

  /* Did we get the memory we needed? */
  if (Buffer == NULL)
  {

    *Error = DLIST_OUT_OF_MEMORY;
    return NULL;

  }

  /* Now we must copy the data to its new home on the heap. */
  memcpy(Buffer,ItemLocation,ItemSize);

  /* Now add the item to the list. */
  Handle = InsertObject(ListToAddTo, ItemSize, Buffer, ItemTag, TargetHandle, Insert_Mode, MakeCurrent, Error);

  if ( *Error != DLIST_SUCCESS )
  {

    /* Since we could not add the item to the list, delete the buffer. */
    free(Buffer);
    return NULL;

  }

  return Handle;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: InsertObject                                     */
/*                                                                   */
/*   Descriptive Name:  This function inserts an object into a DLIST.*/
/*                      The object can be inserted before or after   */
/*                      the current item in the list.                */
/*                                                                   */
/*   Input:  DLIST          ListToAddTo : The list to which the      */
/*                                          data object is to be     */
/*                                          inserted.                */
/*           CARDINAL32    ItemSize : The size of the data item, in  */
/*                                    bytes.                         */
/*           ADDRESS       ItemLocation : The address of the data    */
/*                                        to append to the list      */
/*           TAG           ItemTag : The item tag to associate with  */
/*                                   the item being appended to the  */
/*                                   list                            */
/*           ADDRESS TargetHandle : The item in ListToAddTo which    */
/*                                   is used to determine where      */
/*                                   the item being transferred will */
/*                                   be placed.  If this is NULL,    */
/*                                   then the current item in        */
/*                                   ListToAddTo will be used.       */
/*           Insertion_Modes Insert_Mode : This indicates where,     */
/*                                   relative to the item in         */
/*                                   ListToAddTo specified by        */
/*                                   Target_Handle, the item being   */
/*                                   inserted can be placed.         */
/*           BOOLEAN MakeCurrent : If TRUE, the item being inserted  */
/*                                 into ListToAddTo becomes the      */
/*                                 current item in ListToAddTo.      */
/*           CARDINAL32 *  Error : The address of a variable to hold */
/*                                 the error return code.            */
/*                                                                   */
/*   Output:  If the operation is successful, then *Error will be    */
/*            set to 0 and the function return value will be the     */
/*            handle for the item that was appended to the list.     */
/*            If the operation fails, then *Error will contain an    */
/*            error code and the function return value will be NULL. */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                       ListToAddTo does not point to a valid       */
/*                           list                                    */
/*                       ItemSize is 0                               */
/*                       ItemLocation is NULL                        */
/*                       The memory required for a LINK NODE can not */
/*                           be allocated.                           */
/*                       TargetHandle is invalid or is for an item   */
/*                           in another list.                        */
/*                   If this routine fails, an error code is returned*/
/*                   and any memory allocated by this function is    */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  The item to insert is NOT copied to the heap.  Instead, */
/*           the location of the item is stored in the list.  This   */
/*           is the major difference between InsertObject and        */
/*           InsertItem.  InsertItem allocates memory on the heap,   */
/*           copies the item to the memory it allocated, and stores  */
/*           the address of the memory it allocated in the list.     */
/*           InsertObject stores the address provided by the user.   */
/*                                                                   */
/*           It is assumed that TargetHandle is valid, or is at least*/
/*           the address of an accessible block of storage.  If      */
/*           TargetHandle is invalid, or is not the address of an    */
/*           accessible block of storage, then a trap or exception   */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System InsertObject ( DLIST           ListToAddTo,
                               CARDINAL32      ItemSize,
                               ADDRESS         ItemLocation,
                               TAG             ItemTag,
                               ADDRESS         TargetHandle,
                               Insertion_Modes Insert_Mode,
                               BOOLEAN         MakeCurrent,
                               CARDINAL32 *    Error)
{

  /* Since ListToAddTo is of type DLIST, we can not use it without having
     to type cast it each time.  To avoid all of the type casting, we
     will declare a local variable of type ControlNode * and then
     initialize it once using ListToAddTo.  This way we just do the
     cast once.                                                            */

  ControlNode *      ListData;

  LinkNode *         NewNode;         /* Used to create the LinkNode for the new item. */
  LinkNode *         CurrentNode;     /* Used to hold the reference point for the insertion. */
  LinkNode *         PreviousNode;    /* Used to point to the item prior to CurrentNode in the list while
                                         the new item is being inserted. */
  LinkNode *         NextNode;        /* Used to point to the item after CurrentNode in the list while
                                         the new item is being inserted.                                   */


  /* We will assume that ListToAppendTo points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                              */
  ListData = (ControlNode *) ListToAddTo;

  /* Has the user specified a specific item in the list as a reference point for this insertion? */
  if ( TargetHandle != NULL )
  {

    /* Since the user has specified a reference point for this insertion, set up to use it. */
    CurrentNode = (LinkNode *) TargetHandle;

  }
  else
  {

    /* The user did not specify a reference point, so use the current item in the list as the reference point. */
    CurrentNode = ListData->CurrentItem;

  }

#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToAddTo) )
  {
    *Error = DLIST_CORRUPTED;
    return NULL;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return NULL;
  }

  #endif

  /* Since the list is valid, we must now see if the TargetHandle is valid.  We
     will assume that, if the TargetHandle is not NULL, it points to a LinkNode.
     If the ControlNodeLocation field of the LinkNode points to the
     ControlNode for the list we are working with, then the LinkNode is in
     the list and can therefore be used safely.

      At this point, CurrentNode has been set equal to TargetHandle if TargetHandle
      is not NULL.  If TargetHandle is NULL, then CurrentNode was set to the current
      item in the list.                                                              .*/
  if ( TargetHandle != NULL  )
  {

    /* Is CurrentNode part of this list? */
    if ( CurrentNode->ControlNodeLocation != ListData )
    {

      /* The handle either did not point to a ControlNode or it pointed to the wrong ControlNode! */
      *Error = DLIST_BAD_HANDLE;
      return NULL;

    }

  }

  /* We must check the insertion mode. */
  if ( Insert_Mode > AppendToList )
  {

    *Error = DLIST_INVALID_INSERTION_MODE;
    return NULL;

  }

  /* Lets check the item being added to the DLIST. */
  if (ItemLocation == NULL)
  {
    *Error = DLIST_BAD_ITEM_POINTER;
    return NULL;
  }

  if ( ItemSize == 0)
  {
    *Error = DLIST_ITEM_SIZE_ZERO;
    return NULL;
  }

#endif

  /* Since both the list and item are valid, lets make a LinkNode. */
#ifdef USE_POOLMAN
  NewNode = (LinkNode *) AllocateFromPool(ListData->NodePool);
#else
  NewNode = (LinkNode *) malloc( sizeof(LinkNode) );
#endif

  /* Did we get the memory? */
  if (NewNode == NULL)
  {
    *Error = DLIST_OUT_OF_MEMORY;
    return NULL;
  }

  /* Now that all memory has been allocated, lets finish initializing the LinkNode. */
  NewNode->DataSize = ItemSize;
  NewNode->DataLocation = ItemLocation;
  NewNode->DataTag = ItemTag;
  NewNode->NextLinkNode = NULL;
  NewNode->PreviousLinkNode = NULL;
  NewNode->ControlNodeLocation = ListData;     /* Initialize the link to the control node
                                                  of the list containing this link node.   */

  /* Now we can add the node to the list. */

  /* Is the list empty?  If so, then the Insertion_Mode does not matter! */
  if (ListData->CurrentItem == NULL)
  {
    /* The List is empty.  This will be the first (and only) item in the list.
       Also, since this will be the only item in the list, it automatically
       becomes the current item.                                               */
    ListData->EndOfList = NewNode;
    ListData->StartOfList = NewNode;
    ListData->CurrentItem = NewNode;
  }
  else
  {
    /* The list was not empty.  */

    /* Now lets insert the item according to the specified Insert_Mode. */
    switch ( Insert_Mode )
    {

      case InsertAtStart: /* Get the first item in the list. */
                          CurrentNode = ListData->StartOfList;

                          /* Now insert NewNode before CurrentNode. */
                          NewNode->NextLinkNode = CurrentNode;
                          CurrentNode->PreviousLinkNode = NewNode;

                          /* Now update the ControlNode. */
                          ListData->StartOfList = NewNode;

                          break;
      case InsertBefore:  /* CurrentNode already points to the Item we are to insert NewNode before. */

                          /* Is CurrentNode the first item in the list? */
                          if ( ListData->StartOfList != CurrentNode )
                          {

                            /* Since CurrentNode is not the first item in the list, we need the node prior to CurrentNode
                               so we can adjust its link fields.                                                           */
                            PreviousNode = CurrentNode->PreviousLinkNode;

                            /* Now make PreviousLinkNode point to NewNode and vice versa. */
                            PreviousNode->NextLinkNode = NewNode;
                            NewNode->PreviousLinkNode = PreviousNode;

                          }
                          else
                          {

                            /* Since CurrentNode is the first item in the list, that means that NewNode will be
                               the first item in the list after it is inserted.  Update the ControlNode for this
                               list to reflect that NewNode will be the first item in the list.                     */
                            ListData->StartOfList = NewNode;

                          }

                          /* Now make NewNode point to CurrentNode and vice versa. */
                          NewNode->NextLinkNode = CurrentNode;
                          CurrentNode->PreviousLinkNode = NewNode;

                          break;
      case InsertAfter:   /* CurrentNode already points to the Item we are to insert NewNode after. */

                          /* Is CurrentNode the last item in the list? */
                          if ( ListData->EndOfList != CurrentNode )
                          {

                            /* Since CurrentNode is not the last item in the list, we need the node after to CurrentNode
                               so we can adjust its link fields.                                                           */
                            NextNode = CurrentNode->NextLinkNode;

                            /* Now make NextLinkNode point to NewNode and vice versa. */
                            NextNode->PreviousLinkNode = NewNode;
                            NewNode->NextLinkNode = NextNode;

                          }
                          else
                          {

                            /* Since CurrentNode is the last item in the list, that means that NewNode will be
                               the last item in the list after it is inserted.  Update the ControlNode for this
                               list to reflect that NewNode will be the last item in the list.                     */
                            ListData->EndOfList = NewNode;

                          }

                          /* Now make NewNode point to CurrentNode and vice versa. */
                          CurrentNode->NextLinkNode = NewNode;
                          NewNode->PreviousLinkNode = CurrentNode;

                          break;
      case AppendToList:  /* Get the last item in the list. */
                          CurrentNode = ListData->EndOfList;

                          /* Now insert NewNode after CurrentNode. */
                          CurrentNode->NextLinkNode = NewNode;
                          NewNode->PreviousLinkNode = CurrentNode;

                          /* Now update the ControlNode. */
                          ListData->EndOfList = NewNode;

                          break;
      default :
                NewNode->ControlNodeLocation = NULL;
                free(NewNode->DataLocation);
                free(NewNode);
                *Error = DLIST_INVALID_INSERTION_MODE;
                return NULL;

    }

  }

  /* Adjust the count of the number of items in the list. */
  ListData->ItemCount++;

  /* Should the new node become the current item in the list? */
  if ( MakeCurrent )
  {

    /* Adjust the control node so that NewNode becomes the current item in the list. */
    ListData->CurrentItem = NewNode;

  }

#ifdef PARANOID

  assert (CheckListIntegrity( ListToAddTo ) );

#endif

  /* All done.  Signal successful operation. */
  *Error = DLIST_SUCCESS;

  return NewNode;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  DeleteItem                                      */
/*                                                                   */
/*   Descriptive Name:  This function removes the specified item from*/
/*                      the list and optionally frees the memory     */
/*                      associated with it.                          */
/*                                                                   */
/*   Input:  DLIST       ListToDeleteFrom : The list whose current   */
/*                                         item is to be deleted.    */
/*           BOOLEAN    FreeMemory : If TRUE, then the memory        */
/*                                   associated with the current     */
/*                                   item will be freed.  If FALSE   */
/*                                   then the current item will be   */
/*                                   removed from the list but its   */
/*                                   memory will not be freed.       */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToDeleteFrom, or NULL.  If      */
/*                            NULL is used, then the current item    */
/*                            in ListToDeleteFrom will be deleted.   */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                 the error return code.            */
/*                                                                   */
/*   Output:  If the operation is successful, then *Error will be    */
/*            set to 0.  If the operation fails, then *Error will    */
/*            contain an error code.                                 */
/*                                                                   */
/*   Error Handling: This function will fail if ListToDeleteFrom is  */
/*                   not a valid list, or if ListToDeleteFrom is     */
/*                   empty, or if Handle is invalid.                 */
/*                   If this routine fails, an error code is returned*/
/*                   in *Error.                                      */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  Items in a list can be accessed in two ways:  A copy of */
/*           the item can be obtained using GetItem and its related  */
/*           calls, or a pointer to the item can be obtained using   */
/*           GetObject and its related calls.  If you have a copy of */
/*           the data and wish to remove the item from the list, set */
/*           FreeMemory to TRUE.  This will remove the item from the */
/*           list and deallocate the memory used to hold it.  If you */
/*           have a pointer to the item in the list (from one of the */
/*           GetObject style functions) and wish to remove the item  */
/*           from the list, set FreeMemory to FALSE.  This removes   */
/*           the item from the list without freeing its memory, so   */
/*           that the pointer obtained with the GetObject style      */
/*           functions is still useable.                             */
/*                                                                   */
/*           It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list, unless the handle specified belongs   */
/*           to the current item in the list, in which case this     */
/*           function behaves the same as DeleteItem.                */
/*                                                                   */
/*********************************************************************/
void _System DeleteItem (DLIST        ListToDeleteFrom,
                         BOOLEAN      FreeMemory,
                         ADDRESS      Handle,
                         CARDINAL32 * Error)
{
  /* Since ListToDeleteFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToDeleteFrom.  This way we just do the
     cast once.                                                            */

  ControlNode *      ListData;

  LinkNode *         CurrentLinkNode;    /* Used to point to the item being deleted. */
  LinkNode *         NextLinkNode;       /* Used to point to the item immediately after
                                            the one being deleted so that its fields can
                                            be updated.                                  */
  LinkNode *         PreviousLinkNode;   /* Used to point to the item immediately before
                                            the one being deleted so that its fields can
                                            be updated.                                  */


  /* We will assume that ListToDeleteFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToDeleteFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToDeleteFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return;
  }

  /* We must find the link node that corresponds to the handle, if the handle is valid. */
  CurrentLinkNode = (LinkNode *) Handle;

  /* We will assume that, if the Handle is not NULL, it points to a LinkNode.
     If the ControlNodeLocation field of the LinkNode points to the
     ControlNode for ListToDeleteFrom, then the LinkNode is in ListToDeleteFrom
     and we can proceed to delete the item.                                        */
  if ( CurrentLinkNode != NULL )
  {

    /* Does the LinkNode corresponding to the handle point to the ControlNode for this list? */
    if (CurrentLinkNode->ControlNodeLocation != ListData)
    {
      /* The handle did not point to a LinkNode or the LinkNode it pointed to was not
         in ListToDeleteFrom. */
      *Error = DLIST_BAD_HANDLE;
      return;
    }

  }
  else
  {

    /* Since Handle was NULL, we will use the current item in the list. */
    CurrentLinkNode = ListData->CurrentItem;

  }

  /* Find the next and previous list nodes in the source list. */
  PreviousLinkNode = CurrentLinkNode->PreviousLinkNode;
  NextLinkNode = CurrentLinkNode->NextLinkNode;

  /* Take the current node out of the source list. */
  if (PreviousLinkNode != NULL)
  {
    /* The current item was not the first item in the list. */

    /* Remove the current item from the list. */
    PreviousLinkNode->NextLinkNode = NextLinkNode;

  }

  if ( NextLinkNode != NULL )
  {

    /* The current item was not the last item in the list. */
    NextLinkNode->PreviousLinkNode = PreviousLinkNode;

  }

  /* Was the current link node the first item in the list? */
  if ( ListData->StartOfList == CurrentLinkNode )
    ListData->StartOfList = NextLinkNode;

  /* Was the current link node the last item in the list?*/
  if ( ListData->EndOfList == CurrentLinkNode )
    ListData->EndOfList = PreviousLinkNode;

  /* Was the node being extracted the current item in the list? */
  if ( ListData->CurrentItem == CurrentLinkNode )
  {

    /* The current item in the list will be the item which follows the
       item we are extracting.  If the item being extracted is the last
       item in the list, then the current item becomes the item immediately
       before the item being extracted.  If there are no items before or
       after the item being extracted, then the list is empty!                */
    if ( NextLinkNode != NULL )
      ListData->CurrentItem = NextLinkNode;
    else
      if ( PreviousLinkNode != NULL )
      ListData->CurrentItem = PreviousLinkNode;
    else
      ListData->CurrentItem = NULL;

  }

  /* Adjust the count of items in the list. */
  ListData->ItemCount = ListData->ItemCount - 1;

  /* Now we must free the memory associated with the current node. */
  if ( FreeMemory )
  {
    /* Free the memory associated with the actual item stored in the list. */
#ifdef USE_POOLMAN
    SmartFree(CurrentLinkNode->DataLocation);
#else
    free(CurrentLinkNode->DataLocation);
#endif
  }

  /* Free the memory associated with the control structures used to manage items in the list. */
  CurrentLinkNode->ControlNodeLocation = NULL;
#ifdef USE_POOLMAN
  DeallocateToPool(ListData->NodePool,CurrentLinkNode);    /* Return LinkNode to the Node Pool. */
#else
  free(CurrentLinkNode);
#endif

#ifdef PARANOID

  assert (CheckListIntegrity( ListToDeleteFrom ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  DeleteAllItems                                  */
/*                                                                   */
/*   Descriptive Name:  This function deletes all of the items in the*/
/*                      specified list and optionally frees the      */
/*                      memory associated with each item deleted.    */
/*                                                                   */
/*   Input:  DLIST       ListToDeleteFrom : The list whose items     */
/*                                          are to be deleted.       */
/*           BOOLEAN    FreeMemory : If TRUE, then the memory        */
/*                                   associated with each item in the*/
/*                                   list will be freed.  If FALSE   */
/*                                   then the each item will be      */
/*                                   removed from the list but its   */
/*                                   memory will not be freed.       */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                 the error return code.            */
/*                                                                   */
/*   Output:  If the operation is successful, then *Error will be    */
/*            set to 0.  If the operation fails, then *Error will    */
/*            contain an error code.                                 */
/*                                                                   */
/*   Error Handling: This function will fail if ListToDeleteFrom is  */
/*                   not a valid list, or if ListToDeleteFrom is     */
/*                   empty.                                          */
/*                   If this routine fails, an error code is returned*/
/*                   in *Error.                                      */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  Items in a list can be accessed in two ways:  A copy of */
/*           the item can be obtained using GetItem and its related  */
/*           calls, or a pointer to the item can be obtained using   */
/*           GetObject and its related calls.  If you have a copy of */
/*           the data and wish to remove the item from the list, set */
/*           FreeMemory to TRUE.  This will remove the item from the */
/*           list and deallocate the memory used to hold it.  If you */
/*           have a pointer to the item in the list (from one of the */
/*           GetObject style functions) and wish to remove the item  */
/*           from the list, set FreeMemory to FALSE.  This removes   */
/*           the item from the list without freeing its memory, so   */
/*           that the pointer obtained with the GetObject style      */
/*           functions is still useable.                             */
/*                                                                   */
/*           It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void _System DeleteAllItems (DLIST        ListToDeleteFrom,
                             BOOLEAN      FreeMemory,
                             CARDINAL32 * Error)
{

  /* Since ListToDeleteFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToDeleteFrom.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode;  /* This is used to walk through the
                                          linked list of LinkNodes.        */


  /* We will assume that ListToDeleteFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                 */
  ListData = (ControlNode *) (ListToDeleteFrom);


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToDeleteFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /*--------------------------------------------------
     To empty a DLIST, we must traverse the linked
     list of LinkNodes and dispose of each LinkNode,
     as well as the data item associated with each
     LinkNode.
  --------------------------------------------------*/

  /* Loop to dispose of the Listnodes. */
  while (ListData->ItemCount > 0)
  {
    CurrentLinkNode = ListData->StartOfList;                /* Get the first DLIST node. */
    ListData->StartOfList = CurrentLinkNode->NextLinkNode;  /* Remove that DLIST node from the DLIST. */
    ListData->ItemCount--;                                  /* Decrement the number of items in the list or we will never leave the loop! */
    if ( (CurrentLinkNode->DataLocation != NULL) && FreeMemory )
    {

#ifdef USE_POOLMAN
      SmartFree(CurrentLinkNode->DataLocation);                /* Free the heap memory used to store the data for the DLIST node. */
#else
      free(CurrentLinkNode->DataLocation);
#endif

    }

    CurrentLinkNode->ControlNodeLocation = NULL;

#ifdef USE_POOLMAN
    DeallocateToPool(ListData->NodePool,CurrentLinkNode);   /* Return LinkNode to the Node Pool. */
#else
    free(CurrentLinkNode);
#endif
  }

  /* Since there are no items in the list, set the CurrentItem and EndOfList pointers to NULL. */
  ListData->CurrentItem = NULL;
  ListData->EndOfList = NULL;

#ifdef PARANOID

  assert (CheckListIntegrity( ListToDeleteFrom ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetItem                                         */
/*                                                                   */
/*   Descriptive Name:  This function copies the specified item in   */
/*                      the list to a buffer provided by the caller. */
/*                                                                   */
/*   Input:  DLIST   ListToGetItemFrom : The list whose current item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL32 ItemSize : What the caller thinks the size of*/
/*                               the current item is.                */
/*           ADDRESS     ItemLocation : This is the location of the  */
/*                                      buffer into which the current*/
/*                                      item is to be copied.        */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToGetItemFrom, or NULL.  If     */
/*                            NULL, then the current item in the list*/
/*                            will be used.                          */
/*           BOOLEAN MakeCurrent : If TRUE, the item to get will     */
/*                                 become the current item in the    */
/*                                 list.                             */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                              the error return code.               */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The buffer at ItemLocation will contain a copy of */
/*                    the current item from ListToGetItemFrom.       */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                                                                   */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemLocation is NULL                      */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle corresponding to the current item in the   */
/*                 list.                                             */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list.                                       */
/*                                                                   */
/*********************************************************************/
void _System GetItem( DLIST          ListToGetItemFrom,
                      CARDINAL32     ItemSize,
                      ADDRESS        ItemLocation,
                      TAG            ItemTag,
                      ADDRESS        Handle,
                      BOOLEAN        MakeCurrent,
                      CARDINAL32 *   Error)
{
  /* Since ListToGetItemFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToGetItemFrom.  This way we just do the
     cast once.                                                            */

  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode;  /* Used to point to the LinkNode of the item we are going to return. */
  void *             NotNeeded;        /* Needed to avoid certain compiler warnings. */



  /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */

  ListData = (ControlNode *) ListToGetItemFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetItemFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

  /* Lets check the pointer to the location of where we are to put the data. */
  if (ItemLocation == NULL)
  {
    *Error = DLIST_BAD_ITEM_POINTER;
    return;
  }

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return;
  }

  /* Were we given a handle? */
  if ( Handle != NULL )
  {

    /* Lets check the handle we were given.*/
    CurrentLinkNode = (LinkNode*) Handle;

    /* Is the handle valid? */
    if ( CurrentLinkNode->ControlNodeLocation != ListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return;

    }

  }
  else
  {

    /* Since no handle was given, use the current item in the list. */
    CurrentLinkNode = ListData->CurrentItem;

  }

  /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
  if (CurrentLinkNode->DataTag != ItemTag)
  {
    *Error = DLIST_ITEM_TAG_WRONG;
    return;
  }

  /* Check for an item size mismatch. */
  if (CurrentLinkNode->DataSize != ItemSize)
  {
    *Error = DLIST_ITEM_SIZE_WRONG;
    return;
  }

  /* Since everything checks out, lets transfer the data. */
  NotNeeded = (void *) memcpy(ItemLocation,CurrentLinkNode->DataLocation,ItemSize);

  /* Did the user want this item to become the current item? */
  if ( MakeCurrent )
  {

    /* Make this item the current item in the list. */
    ListData->CurrentItem = CurrentLinkNode;

  }

#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetItemFrom ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetNextItem                                     */
/*                                                                   */
/*   Descriptive Name:  This function advances the current item      */
/*                      pointer and then copies the current item in  */
/*                      the list to a buffer provided by the caller. */
/*                                                                   */
/*   Input:  DLIST   ListToGetItemFrom : The list whose current item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL32 ItemSize : What the caller thinks the size of*/
/*                               the current item is.                */
/*           ADDRESS     ItemLocation : This is the location of the  */
/*                                      buffer into which the current*/
/*                                      item is to be copied.        */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                              the error return code.               */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The buffer at ItemLocation will contain a copy of */
/*                    the current item from ListToGetItemFrom.       */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                 The current item pointer will NOT be advanced.    */
/*                     The current item in the list will be the same */
/*                     as before the call to this function.          */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemLocation is NULL                      */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         The current item in the list before this  */
/*                             function is called is the last item   */
/*                             item in the list.                     */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void _System GetNextItem( DLIST           ListToGetItemFrom,
                          CARDINAL32     ItemSize,
                          ADDRESS        ItemLocation,
                          TAG            ItemTag,
                          CARDINAL32 *   Error)
{
  /* Since ListToGetItemFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToAdvance.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item while we access its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */
  LinkNode *         OriginalCurrentLinkNode; /* This is used to hold the value
                                                 of the Current Item pointer
                                                 as it was upon entry to this
                                                 function.  If there is an
                                                 error, the Current Item
                                                 pointer will be reset to the
                                                 value stored in this variable. */
  void *             NotNeeded;       /* Needed to avoid certain compiler warnings. */



  /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToGetItemFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetItemFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

  /* Lets check the pointer to the location of where we are to put the data. */
  if (ItemLocation == NULL)
  {
    *Error = DLIST_BAD_ITEM_POINTER;
    return;
  }

#endif

  /* Check for empty list. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return;
  }

  /* Check for end of list. */
  if (ListData->CurrentItem == ListData->EndOfList)
  {
    *Error = DLIST_END_OF_LIST;
    return;
  }

  /* Save the current item pointer so that we can restore it should something
     go wrong later.                                                          */
  OriginalCurrentLinkNode = ListData->CurrentItem;

  /* Advance the current item pointer. */
  CurrentLinkNode = ListData->CurrentItem;
  ListData->CurrentItem = CurrentLinkNode->NextLinkNode;

  /* Lets get the current node from the DLIST. */
  CurrentLinkNode = ListData->CurrentItem;

  /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
  if (CurrentLinkNode->DataTag != ItemTag)
  {
    *Error =DLIST_ITEM_TAG_WRONG;
    ListData->CurrentItem = OriginalCurrentLinkNode;
    return;
  }

  /* Check for an item size mismatch. */
  if (CurrentLinkNode->DataSize != ItemSize)
  {
    *Error = DLIST_ITEM_SIZE_WRONG;
    ListData->CurrentItem = OriginalCurrentLinkNode;
    return;
  }

  /* Since everything checks out, lets transfer the data. */
  NotNeeded = (void *) memcpy(ItemLocation,CurrentLinkNode->DataLocation,ItemSize);

#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetItemFrom ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetPreviousItem                                 */
/*                                                                   */
/*   Descriptive Name:  This function makes the previous item in the */
/*                      list the current item in the list and then   */
/*                      copies that item to a buffer provided by the */
/*                      user.                                        */
/*                                                                   */
/*   Input:  DLIST   ListToGetItemFrom : The list whose current item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL32 ItemSize : What the caller thinks the size of*/
/*                                 the current item is.              */
/*           ADDRESS    ItemLocation : This is the location of the   */
/*                                      buffer into which the current*/
/*                                      item is to be copied.        */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The buffer at ItemLocation will contain a copy of */
/*                    the current item from ListToGetItemFrom.       */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                 The current item pointer will NOT be advanced.    */
/*                     The current item in the list will be the same */
/*                     as before the call to this function.          */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemLocation is NULL                      */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         The current item in the list before this  */
/*                             function is called is the last item   */
/*                             item in the list.                     */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void _System GetPreviousItem( DLIST          ListToGetItemFrom,
                              CARDINAL32     ItemSize,
                              ADDRESS        ItemLocation,
                              TAG            ItemTag,
                              CARDINAL32 *   Error)
{

  /* Since ListToGetItemFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToAdvance.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item while we access its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */
  LinkNode *         OriginalCurrentLinkNode; /* This is used to hold the value
                                                 of the Current Item pointer
                                                 as it was upon entry to this
                                                 function.  If there is an
                                                 error, the Current Item
                                                 pointer will be reset to the
                                                 value stored in this variable. */
  void *             NotNeeded;       /* Needed to avoid certain compiler warnings. */



  /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToGetItemFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetItemFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

  /* Lets check the pointer to the location of where we are to put the data. */
  if (ItemLocation == NULL)
  {
    *Error = DLIST_BAD_ITEM_POINTER;
    return;
  }

#endif

  /* Check for empty list. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return;
  }

  /* Check for beginning of list. */
  if (ListData->CurrentItem == ListData->StartOfList)
  {
    *Error = DLIST_ALREADY_AT_START;
    return;
  }

  /* Save the current item pointer so that we can restore it should something
     go wrong later.                                                          */
  OriginalCurrentLinkNode = ListData->CurrentItem;

  /* Position the current item pointer. */
  CurrentLinkNode = ListData->CurrentItem;
  ListData->CurrentItem = CurrentLinkNode->PreviousLinkNode;

  /* Lets get the current node from the DLIST. */
  CurrentLinkNode = ListData->CurrentItem;

  /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
  if (CurrentLinkNode->DataTag != ItemTag)
  {
    *Error =DLIST_ITEM_TAG_WRONG;
    ListData->CurrentItem = OriginalCurrentLinkNode;
    return;
  }

  /* Check for an item size mismatch. */
  if (CurrentLinkNode->DataSize != ItemSize)
  {
    *Error = DLIST_ITEM_SIZE_WRONG;
    ListData->CurrentItem = OriginalCurrentLinkNode;
    return;
  }

  /* Since everything checks out, lets transfer the data. */
  NotNeeded = (void *) memcpy(ItemLocation,CurrentLinkNode->DataLocation,ItemSize);

#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetItemFrom ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetObject                                       */
/*                                                                   */
/*   Descriptive Name:  This function returns the address of the data*/
/*                      associated with the specified item in the    */
/*                      list.                                        */
/*                                                                   */
/*   Input:  DLIST   ListToGetItemFrom : The list whose current item */
/*                                      is to have its address       */
/*                                      returned to the caller.      */
/*           CARDINAL32 ItemSize : What the caller thinks the size of*/
/*                               the current item is.                */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToGetItemFrom, or NULL.  If     */
/*                            NULL, then the current item in the list*/
/*           BOOLEAN MakeCurrent : If TRUE, the item to get will     */
/*                                 become the current item in the    */
/*                                 list.                             */
/*                            will be used.                          */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The function return value will be the address of  */
/*                 the data associated with the current item in the  */
/*                 list.                                             */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                 The function return value will be NULL.           */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user should not free the memory associated with     */
/*           the address returned by this function as the object is  */
/*           still in the list.                                      */
/*                                                                   */
/*           It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap may   */
/*           occur.                                                  */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle designating the current item in the list.  */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list.                                       */
/*                                                                   */
/*********************************************************************/
ADDRESS _System GetObject( DLIST          ListToGetItemFrom,
                           CARDINAL32     ItemSize,
                           TAG            ItemTag,
                           ADDRESS        Handle,
                           BOOLEAN        MakeCurrent,
                           CARDINAL32 *   Error)
{
  /* Since ListToGetItemFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToGetItemFrom.  This way we just do the
     cast once.                                                            */

  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode;  /* Used to point to the LinkNode of the item to return. */



  /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */

  ListData = (ControlNode *) ListToGetItemFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetItemFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return NULL;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return NULL;
  }

  #endif

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return NULL;
  }

  /* Were we given a handle? */
  if ( Handle != NULL )
  {

    /* Lets check the handle we were given.*/
    CurrentLinkNode = (LinkNode*) Handle;

    /* Is the handle valid? */
    if ( CurrentLinkNode->ControlNodeLocation != ListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return NULL;

    }

  }
  else
  {

    /* Since we were not given a handle, we will use the current item in the list for this operation. */
    CurrentLinkNode = ListData->CurrentItem;

  }

  /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
  if (CurrentLinkNode->DataTag != ItemTag)
  {
    *Error =DLIST_ITEM_TAG_WRONG;
    return NULL;
  }

  /* Check for an item size mismatch. */
  if (CurrentLinkNode->DataSize != ItemSize)
  {
    *Error = DLIST_ITEM_SIZE_WRONG;
    return NULL;
  }

  /* Does the user want this item made the current item in the list? */
  if ( MakeCurrent )
  {

    /* Make this item the current item in the list. */
    ListData->CurrentItem = CurrentLinkNode;

  }

#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetItemFrom ) );

#endif


  /* Since everything checks out, lets signal success and return the address of the data. */
  *Error = DLIST_SUCCESS;
  return CurrentLinkNode->DataLocation;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetNextObject                                   */
/*                                                                   */
/*   Descriptive Name:  This function advances the current item      */
/*                      pointer and then returns the address of the  */
/*                      data associated with the current item in the */
/*                      list.                                        */
/*                                                                   */
/*   Input:  DLIST   ListToGetItemFrom : The list whose current item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL32 ItemSize : What the caller thinks the size of*/
/*                               the current item is.                */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                              the error return code.               */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The function return value will be the address of  */
/*                 the data associated with the current item in the  */
/*                 list.                                             */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                 The function return value will be NULL.           */
/*                 The current item pointer will NOT be advanced.    */
/*                     The current item in the list will be the same */
/*                     as before the call to this function.          */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         The current item in the list before this  */
/*                             function is called is the last item   */
/*                             item in the list.                     */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption are violated, an exception or trap may  */
/*           occur.                                                  */
/*                                                                   */
/*********************************************************************/
ADDRESS _System GetNextObject( DLIST           ListToGetItemFrom,
                               CARDINAL32     ItemSize,
                               TAG            ItemTag,
                               CARDINAL32 *   Error)
{
  /* Since ListToGetItemFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToAdvance.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item while we access its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */
  LinkNode *         OriginalCurrentLinkNode; /* This is used to hold the value
                                                 of the Current Item pointer
                                                 as it was upon entry to this
                                                 function.  If there is an
                                                 error, the Current Item
                                                 pointer will be reset to the
                                                 value stored in this variable. */



  /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToGetItemFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetItemFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return NULL;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return NULL;
  }

  #endif

#endif

  /* Check for empty list. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return NULL;
  }

  /* Check for end of list. */
  if (ListData->CurrentItem == ListData->EndOfList)
  {
    *Error = DLIST_END_OF_LIST;
    return NULL;
  }

  /* Save the current item pointer so that we can restore it should something
     go wrong later.                                                          */
  OriginalCurrentLinkNode = ListData->CurrentItem;

  /* Advance the current item pointer. */
  CurrentLinkNode = ListData->CurrentItem;
  ListData->CurrentItem = CurrentLinkNode->NextLinkNode;

  /* Lets get the current node from the DLIST. */
  CurrentLinkNode = ListData->CurrentItem;

  /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
  if (CurrentLinkNode->DataTag != ItemTag)
  {
    *Error =DLIST_ITEM_TAG_WRONG;
    ListData->CurrentItem = OriginalCurrentLinkNode;
    return NULL;
  }

  /* Check for an item size mismatch. */
  if (CurrentLinkNode->DataSize != ItemSize)
  {
    *Error = DLIST_ITEM_SIZE_WRONG;
    ListData->CurrentItem = OriginalCurrentLinkNode;
    return NULL;
  }


#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetItemFrom ) );

#endif


  /* Since everything checks out, lets signal success and return the address of the data. */
  *Error = DLIST_SUCCESS;
  return CurrentLinkNode->DataLocation;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetPreviousObject                               */
/*                                                                   */
/*   Descriptive Name:  This function makes the previous item in the */
/*                      list the current item and then returns the   */
/*                      address of the data associated with the      */
/*                      current item in the list.                    */
/*                                                                   */
/*   Input:  DLIST   ListToGetItemFrom : The list whose current item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL32 ItemSize : What the caller thinks the size of*/
/*                                 the current item is.              */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The function return value will be the address of  */
/*                 the data associated with the current item in the  */
/*                 list.                                             */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                 The function return value will be NULL.           */
/*                 The current item pointer will NOT be advanced.    */
/*                     The current item in the list will be the same */
/*                     as before the call to this function.          */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         The current item in the list before this  */
/*                             function is called is the last item   */
/*                             item in the list.                     */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user should not free the memory associated with     */
/*           the address returned by this function as the object is  */
/*           still in the list.                                      */
/*                                                                   */
/*           It is assumed that Error contains a valid address. If   */
/*           this assumption are violated, an exception or trap may  */
/*           occur.                                                  */
/*                                                                   */
/*********************************************************************/
ADDRESS _System GetPreviousObject( DLIST          ListToGetItemFrom,
                                   CARDINAL32     ItemSize,
                                   TAG            ItemTag,
                                   CARDINAL32 *   Error)
{

  /* Since ListToGetItemFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToAdvance.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item while we access its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */
  LinkNode *         OriginalCurrentLinkNode; /* This is used to hold the value
                                                 of the Current Item pointer
                                                 as it was upon entry to this
                                                 function.  If there is an
                                                 error, the Current Item
                                                 pointer will be reset to the
                                                 value stored in this variable. */



  /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToGetItemFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetItemFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return NULL;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return NULL;
  }

  #endif

#endif

  /* Check for empty list. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return NULL;
  }

  /* Check for beginning of list. */
  if (ListData->CurrentItem == ListData->StartOfList)
  {
    *Error = DLIST_ALREADY_AT_START;
    return NULL;
  }

  /* Save the current item pointer so that we can restore it should something
     go wrong later.                                                          */
  OriginalCurrentLinkNode = ListData->CurrentItem;

  /* Position the current item pointer. */
  CurrentLinkNode = ListData->CurrentItem;
  ListData->CurrentItem = CurrentLinkNode->PreviousLinkNode;

  /* Lets get the current node from the DLIST. */
  CurrentLinkNode = ListData->CurrentItem;

  /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
  if (CurrentLinkNode->DataTag != ItemTag)
  {
    *Error =DLIST_ITEM_TAG_WRONG;
    ListData->CurrentItem = OriginalCurrentLinkNode;
    return NULL;
  }

  /* Check for an item size mismatch. */
  if (CurrentLinkNode->DataSize != ItemSize)
  {
    *Error = DLIST_ITEM_SIZE_WRONG;
    ListData->CurrentItem = OriginalCurrentLinkNode;
    return NULL;
  }


#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetItemFrom ) );

#endif


  /* Since everything checks out, lets signal success and return the address of the data. */
  *Error = DLIST_SUCCESS;
  return CurrentLinkNode->DataLocation;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ExtractItem                                     */
/*                                                                   */
/*   Descriptive Name:  This function copies the specified item in   */
/*                      the list to a buffer provided by the caller  */
/*                      and removes the item from the list.          */
/*                                                                   */
/*   Input:  DLIST   ListToGetItemFrom : The list whose current item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL32 ItemSize : What the caller thinks the size of*/
/*                                 the current item is.              */
/*           ADDRESS     ItemLocation : This is the location of the  */
/*                                      buffer into which the current*/
/*                                      item is to be copied.        */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToGetItemFrom, or NULL.  If     */
/*                            NULL, then the current item in the list*/
/*                            will be used.                          */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The buffer at ItemLocation will contain a copy of */
/*                    the current item from ListToGetItemFrom.       */
/*                 The item will have been removed from the list and */
/*                    its memory deallocated.                        */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemLocation is NULL                      */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle which refers to the current item in the    */
/*                 list.                                             */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list, unless the handle specified belongs   */
/*           to the current item in the list, in which case the      */
/*           item following the current item becomes the current     */
/*           item in the list.  If there is no item following the    */
/*           current item in the list, then the item preceeding the  */
/*           current item will become the current item in the list.  */
/*                                                                   */
/*********************************************************************/
void _System ExtractItem( DLIST          ListToGetItemFrom,
                          CARDINAL32     ItemSize,
                          ADDRESS        ItemLocation,
                          TAG            ItemTag,
                          ADDRESS        Handle,
                          CARDINAL32 *   Error)
{
  /* Since ListToGetItemFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToGetItemFrom.  This way we just do the
     cast once.                                                            */

  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode;  /* Used to point to the LinkNode of the item being extracted. */
  void *             NotNeeded;        /* Needed to avoid certain compiler warnings. */
  LinkNode *         NextLinkNode;     /* Used to point to the item immediately following
                                          the one being extracted so that its fields can
                                          be updated.                                      */
  LinkNode *         PreviousLinkNode; /* Used to point to the item immediately before
                                            the one being deleted so that its fields can
                                            be updated.                                  */


  /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */

  ListData = (ControlNode *) ListToGetItemFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetItemFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

  /* Lets check the pointer to the location of where we are to put the data. */
  if (ItemLocation == NULL)
  {
    *Error = DLIST_BAD_ITEM_POINTER;
    return;
  }

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return;
  }

  /* Were we given a handle? */
  if ( Handle != NULL )
  {

    /* Lets check the handle we were given.*/
    CurrentLinkNode = (LinkNode*) Handle;

    /* Is the handle valid? */
    if ( CurrentLinkNode->ControlNodeLocation != ListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return;

    }

  }
  else
  {

    /* Since we were not given a handle, we will use the current item in the list for this operation. */
    CurrentLinkNode = ListData->CurrentItem;

  }

  /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
  if (CurrentLinkNode->DataTag != ItemTag)
  {
    *Error =DLIST_ITEM_TAG_WRONG;
    return;
  }

  /* Check for an item size mismatch. */
  if (CurrentLinkNode->DataSize != ItemSize)
  {
    *Error = DLIST_ITEM_SIZE_WRONG;
    return;
  }

  /* Since everything checks out, lets transfer the data. */
  NotNeeded = (void *) memcpy(ItemLocation,CurrentLinkNode->DataLocation,ItemSize);

  /* Now we must remove the current item from the DLIST. */

  /* Find the previous and next list nodes in the source list. */
  PreviousLinkNode = CurrentLinkNode->PreviousLinkNode;
  NextLinkNode = CurrentLinkNode->NextLinkNode;

  /* Take the current node out of the source list. */
  if (PreviousLinkNode != NULL)
  {
    /* The current item was not the first item in the list. */

    /* Remove the current item from the list. */
    PreviousLinkNode->NextLinkNode = NextLinkNode;

  }

  if ( NextLinkNode != NULL )
  {

    /* The current item was not the last item in the list. */
    NextLinkNode->PreviousLinkNode = PreviousLinkNode;

  }

  /* Was the current link node the first item in the list? */
  if ( ListData->StartOfList == CurrentLinkNode )
    ListData->StartOfList = NextLinkNode;

  /* Was the current link node the last item in the list?*/
  if ( ListData->EndOfList == CurrentLinkNode )
    ListData->EndOfList = PreviousLinkNode;

  /* Was the node being extracted the current item in the list? */
  if ( ListData->CurrentItem == CurrentLinkNode )
  {

    /* The current item in the list will be the item which follows the
       item we are extracting.  If the item being extracted is the last
       item in the list, then the current item becomes the item immediately
       before the item being extracted.  If there are no items before or
       after the item being extracted, then the list is empty!                */
    if ( NextLinkNode != NULL )
      ListData->CurrentItem = NextLinkNode;
    else
      if ( PreviousLinkNode != NULL )
      ListData->CurrentItem = PreviousLinkNode;
    else
      ListData->CurrentItem = NULL;

  }

  /* Adjust the count of items in the list. */
  ListData->ItemCount = ListData->ItemCount - 1;

  /* Now we must free the memory associated with the current node. */
#ifdef USE_POOLMAN
  SmartFree(CurrentLinkNode->DataLocation);
#else
  free(CurrentLinkNode->DataLocation);
#endif

  CurrentLinkNode->ControlNodeLocation = NULL;
#ifdef USE_POOLMAN
  DeallocateToPool(ListData->NodePool,CurrentLinkNode);    /* Return LinkNode to the Node Pool. */
#else
  free(CurrentLinkNode);
#endif

#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetItemFrom ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ExtractObject                                   */
/*                                                                   */
/*   Descriptive Name:  This function returns the address of the data*/
/*                      associated with the specified item in the    */
/*                      list and then removes that item from the list*/
/*                                                                   */
/*   Input:  DLIST   ListToGetItemFrom : The list whose current item */
/*                                      is to be copied and returned */
/*                                      to the caller.               */
/*           CARDINAL32 ItemSize : What the caller thinks the size of*/
/*                                 the current item is.              */
/*           TAG     ItemTag : What the caller thinks the item tag   */
/*                             of the current item is.               */
/*           ADDRESS Handle : The handle of the item to get.  This   */
/*                            handle must be of an item which resides*/
/*                            in ListToGetItemFrom, or NULL.  If     */
/*                            NULL, then the current item in the     */
/*                            list will be used.                     */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code.             */
/*                                                                   */
/*   Output:  If Successful :                                        */
/*                 *Error will be set to 0.                          */
/*                 The function return value will be the address of  */
/*                 the data associated with the current item in the  */
/*                 list.                                             */
/*                 The current item is removed from the list.        */
/*            If Failure :                                           */
/*                 *Error will contain an error code.                */
/*                 The function return value will be NULL.           */
/*                                                                   */
/*   Error Handling: This function will fail under any of the        */
/*                   following conditions:                           */
/*                         ListToGetItemFrom is not a valid list     */
/*                         ItemSize does not match the size of the   */
/*                             current item in the list              */
/*                         ItemTag does not match the item tag       */
/*                             of the current item in the list       */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                   If any of these conditions occur, *Error will   */
/*                   contain a non-zero error code.                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user is responsible for the memory associated with  */
/*           the address returned by this function since this        */
/*           function removes that object from the list.  This means */
/*           that, when the user is through with the object, they    */
/*           should free it.                                         */
/*                                                                   */
/*           It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap may   */
/*           occur.                                                  */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle which refers to the current item in the    */
/*                 list.                                             */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list, unless the handle specified belongs   */
/*           to the current item in the list, in which case the      */
/*           item following the current item becomes the current     */
/*           item in the list.  If there is no item following the    */
/*           current item in the list, then the item preceeding the  */
/*           current item will become the current item in the list.  */
/*                                                                   */
/*********************************************************************/
ADDRESS _System ExtractObject( DLIST          ListToGetItemFrom,
                               CARDINAL32     ItemSize,
                               TAG            ItemTag,
                               ADDRESS        Handle,
                               CARDINAL32 *   Error)
{
  /* Since ListToGetItemFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToGetItemFrom.  This way we just do the
     cast once.                                                            */

  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode;  /* Used to point to the LinkNode of the current item. */
  void *             DataLocation;     /* Used to store the address that will be returned
                                          as the function value.                             */
  LinkNode *         NextLinkNode;     /* Used to point to the item immediately following
                                          the one being extracted so that its fields can
                                          be updated.                                      */
  LinkNode *         PreviousLinkNode;   /* Used to point to the item immediately before
                                            the one being deleted so that its fields can
                                            be updated.                                  */


  /* We will assume that ListToGetItemFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */

  ListData = (ControlNode *) ListToGetItemFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetItemFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return NULL;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return NULL;
  }

  #endif

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return NULL;
  }

  /* Were we given a handle? */
  if ( Handle != NULL )
  {

    /* Lets check the handle we were given.*/
    CurrentLinkNode = (LinkNode*) Handle;

    /* Is the handle valid? */
    if ( CurrentLinkNode->ControlNodeLocation != ListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return NULL;

    }

  }
  else
  {

    /* Since we were not given a handle, we will use the current item in the list for this operation. */
    CurrentLinkNode = ListData->CurrentItem;

  }

  /* We must check the actual item tag against the item tag expected by the user.  A mismatch could lead to errors! */
  if (CurrentLinkNode->DataTag != ItemTag)
  {
    *Error =DLIST_ITEM_TAG_WRONG;
    return NULL;
  }

  /* Check for an item size mismatch. */
  if (CurrentLinkNode->DataSize != ItemSize)
  {
    *Error = DLIST_ITEM_SIZE_WRONG;
    return NULL;
  }

  /* Since everything checks out, lets store the address of the data so that we can return it later. */
  DataLocation = CurrentLinkNode->DataLocation;

  /* Now we must remove the current item from the DLIST. */

  /* Find the next and previous link nodes in the source list. */
  NextLinkNode = CurrentLinkNode->NextLinkNode;
  PreviousLinkNode = CurrentLinkNode->PreviousLinkNode;

  /* Take the current node out of the source list. */
  if (PreviousLinkNode != NULL)
  {
    /* The current item was not the first item in the list. */

    /* Remove the current item from the list. */
    PreviousLinkNode->NextLinkNode = NextLinkNode;

  }

  if ( NextLinkNode != NULL )
  {

    /* The current item was not the last item in the list. */
    NextLinkNode->PreviousLinkNode = PreviousLinkNode;

  }

  /* Was the current link node the first item in the list? */
  if ( ListData->StartOfList == CurrentLinkNode )
    ListData->StartOfList = NextLinkNode;

  /* Was the current link node the last item in the list?*/
  if ( ListData->EndOfList == CurrentLinkNode )
    ListData->EndOfList = PreviousLinkNode;

  /* Was the node being extracted the current item in the list? */
  if ( ListData->CurrentItem == CurrentLinkNode )
  {

    /* The current item in the list will be the item which follows the
       item we are extracting.  If the item being extracted is the last
       item in the list, then the current item becomes the item immediately
       before the item being extracted.  If there are no items before or
       after the item being extracted, then the list is empty!                */
    if ( NextLinkNode != NULL )
      ListData->CurrentItem = NextLinkNode;
    else
      if ( PreviousLinkNode != NULL )
      ListData->CurrentItem = PreviousLinkNode;
    else
      ListData->CurrentItem = NULL;

  }

  /* Adjust the count of items in the list. */
  ListData->ItemCount = ListData->ItemCount - 1;

  /* Now we must free the memory associated with the current node. */
  CurrentLinkNode->ControlNodeLocation = NULL;
#ifdef USE_POOLMAN
  DeallocateToPool(ListData->NodePool,CurrentLinkNode);    /* Return LinkNode to the Node Pool. */
#else
  free(CurrentLinkNode);
#endif


#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetItemFrom ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

  /* Time to return the address of the data. */
  return DataLocation;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ReplaceItem                                     */
/*                                                                   */
/*   Descriptive Name:  This function replaces the specified item in */
/*                      the list with the one provided as its        */
/*                      argument.                                    */
/*                                                                   */
/*   Input: DLIST   ListToReplaceItemIn : The list whose current item*/
/*                                       is to be replaced           */
/*          CARDINAL32 ItemSize : The size, in bytes, of the         */
/*                              replacement item                     */
/*          ADDRESS     ItemLocation : The address of the replacement*/
/*                                     item                          */
/*          TAG     ItemTag : The item tag that the user wishes to   */
/*                            associate with the replacement item    */
/*          ADDRESS Handle : The handle of the item to get.  This    */
/*                           handle must be of an item which resides */
/*                           in ListToGetItemFrom, or NULL.  If NULL */
/*                           then the current item in the list will  */
/*                           used.                                   */
/*          BOOLEAN MakeCurrent : If TRUE, the item to get will      */
/*                                become the current item in the     */
/*                                list.                              */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code               */
/*                                                                   */
/*   Output:  If Successful then *Error will be set to 0.            */
/*            If Unsuccessful, then *Error will be set to a non-zero */
/*              error code.                                          */
/*                                                                   */
/*   Error Handling:  This function will fail under the following    */
/*                    conditions:                                    */
/*                         ListToReplaceItemIn is empty              */
/*                         ItemSize is 0                             */
/*                         ItemLocation is NULL                      */
/*                         The memory required can not be allocated. */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                    If any of these conditions occurs, *Error      */
/*                    will contain a non-zero error code.            */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is a valid handle which   */
/*                 refers to the current item in the list.           */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list.                                       */
/*                                                                   */
/*********************************************************************/
void _System ReplaceItem( DLIST         ListToReplaceItemIn,
                          CARDINAL32    ItemSize,
                          ADDRESS       ItemLocation,
                          TAG           ItemTag,
                          ADDRESS       Handle,
                          BOOLEAN       MakeCurrent,
                          CARDINAL32 *  Error)
{
  /* Since ListToReplaceItemIn is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToReplaceItemIn.  This way we just do the
     cast once.                                                            */

  ControlNode *      ListData;

  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         item while we replace its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */
  ADDRESS            NewData;   /* Used to point to the location on the heap
                                   where the replacement item will be stored. */
  void *             NotNeeded; /* Needed to avoid certain compiler warnings. */



  /* We will assume that ListToReplaceItemIn points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToReplaceItemIn;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToReplaceItemIn) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

  /* Lets check the replacement data. */
  if (ItemLocation == NULL)
  {
    *Error = DLIST_BAD_ITEM_POINTER;
    return;
  }

  if ( ItemSize == 0)
  {
    *Error = DLIST_ITEM_SIZE_ZERO;
    return;
  }

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return;
  }

  /* Were we given a handle? */
  if ( Handle != NULL )
  {

    /* Lets check the handle we were given.*/
    CurrentLinkNode = (LinkNode*) Handle;

    /* Is the handle valid? */
    if ( CurrentLinkNode->ControlNodeLocation != ListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return;

    }

  }
  else
    CurrentLinkNode = ListData->CurrentItem;    /* Handle was NULL, so use the current item in the list. */

  /* Since our replacement checks out, we can allocate memory to hold it. */
  if (ItemSize != CurrentLinkNode->DataSize)
  {
#ifdef USE_POOLMAN
    NewData = SmartMalloc(ItemSize);
#else
    NewData = malloc(ItemSize);
#endif
    if (NewData == NULL)
    {
      *Error = DLIST_OUT_OF_MEMORY;
      return;
    }
  }
  else
    NewData = CurrentLinkNode->DataLocation;

  /* Now we must copy the replacement data to its new home on the heap. */
  NotNeeded = (void *) memcpy(NewData,ItemLocation,ItemSize);

  /* Now, if we are not reusing the memory occupied by the old item, we can
     dispose of the old item. */
  if (CurrentLinkNode->DataLocation != NewData)
  {
#ifdef USE_POOLMAN
    SmartFree(CurrentLinkNode->DataLocation);
#else
    free(CurrentLinkNode->DataLocation);
#endif
  }

  /* Now lets put our replacement into the list. */
  CurrentLinkNode->DataSize = ItemSize;
  CurrentLinkNode->DataTag = ItemTag;
  CurrentLinkNode->DataLocation = NewData;

  /* Did the user want this item to become the current item in the list? */
  if ( MakeCurrent )
  {

    /* Make this item the current item in the list. */
    ListData->CurrentItem = CurrentLinkNode;

  }

#ifdef PARANOID

  assert (CheckListIntegrity( ListToReplaceItemIn ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: ReplaceObject                                    */
/*                                                                   */
/*   Descriptive Name:  This function replaces the specified object  */
/*                      in the list with the one provided as its     */
/*                      argument.                                    */
/*                                                                   */
/*   Input: DLIST   ListToReplaceItemIn : The list whose current     */
/*                                       object is to be replaced    */
/*          CARDINAL32 ItemSize : The size, in bytes, of the         */
/*                              replacement object                   */
/*          ADDRESS     ItemLocation : The address of the replacement*/
/*                                     item                          */
/*          TAG     ItemTag : The item tag that the user wishes to   */
/*                            associate with the replacement item    */
/*          ADDRESS Handle : The handle of the item to get.  This    */
/*                           handle must be of an item which resides */
/*                           in ListToGetItemFrom, or NULL.  If NULL */
/*                           then the current item in the list will  */
/*                           be used.                                */
/*          BOOLEAN MakeCurrent : If TRUE, the item to get will      */
/*                                become the current item in the     */
/*                                list.                              */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code               */
/*                                                                   */
/*   Output:  If Successful then *Error will be set to 0 and the     */
/*              return value of the function will be the address     */
/*              of the object that was replaced.                     */
/*            If Unsuccessful, then *Error will be set to a non-zero */
/*              error code and the function return value will be     */
/*              NULL.                                                */
/*                                                                   */
/*   Error Handling:  This function will fail under the following    */
/*                    conditions:                                    */
/*                         ListToReplaceItemIn is empty              */
/*                         ItemSize is 0                             */
/*                         ItemLocation is NULL                      */
/*                         The memory required can not be allocated. */
/*                         Handle is invalid, or is for an item      */
/*                             which is not in ListToGetItemFrom     */
/*                    If any of these conditions occurs, *Error      */
/*                    will contain a non-zero error code.            */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The user is responsible for the memory associated with  */
/*           the object returned by this function as that object is  */
/*           removed from the list.  This means that, when the user  */
/*           is through with the object returned by this function,   */
/*           they should free it.                                    */
/*                                                                   */
/*           It is assumed that Error contains a valid address. It   */
/*           is also assumed that if ItemLocation is not NULL, then  */
/*           it is a valid address that can be dereferenced.  If     */
/*           these assumptions are violated, an exception or trap    */
/*           may occur.                                              */
/*                                                                   */
/*           It is assumed that Handle is valid, or is at least the  */
/*           address of an accessible block of storage.  If Handle   */
/*           is invalid, or is not the address of an accessible block*/
/*           of storage, then a trap or exception may occur.         */
/*           NOTE: For this function, NULL is a valid handle for the */
/*                 current item in the list.                         */
/*                                                                   */
/*           This function does not alter which item is the current  */
/*           item in the list.                                       */
/*                                                                   */
/*********************************************************************/
ADDRESS _System ReplaceObject( DLIST         ListToReplaceItemIn,
                               CARDINAL32 *  ItemSize,             /* On input - size of new object.  On return = size of old object. */
                               ADDRESS       ItemLocation,
                               TAG        *  ItemTag,              /* On input - TAG of new object.  On return = TAG of old object. */
                               ADDRESS       Handle,
                               BOOLEAN       MakeCurrent,
                               CARDINAL32 *  Error)
{

  /* Since ListToReplaceItemIn is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToReplaceItemIn.  This way we just do the
     cast once.                                                            */

  ControlNode *      ListData;

  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         item while we replace its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */
  CARDINAL32         OldItemSize;
  TAG                OldItemTag;
  ADDRESS            OldItemLocation;


  /* We will assume that ListToReplaceItemIn points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToReplaceItemIn;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToReplaceItemIn) )
  {
    *Error = DLIST_CORRUPTED;
    return NULL;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return NULL;
  }

  #endif

  /* Lets check the replacement data. */
  if (ItemLocation == NULL)
  {
    *Error = DLIST_BAD_ITEM_POINTER;
    return NULL;
  }

  if ( ItemSize == 0)
  {
    *Error = DLIST_ITEM_SIZE_ZERO;
    return NULL;
  }

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return NULL;
  }

  /* Were we given a handle? */
  if ( Handle != NULL )
  {

    /* Lets check the handle we were given.*/
    CurrentLinkNode = (LinkNode*) Handle;

    /* Is the handle valid? */
    if ( CurrentLinkNode->ControlNodeLocation != ListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return NULL;

    }

  }
  else
  {

    /* Since we were not given a handle, we will use the current item in the list for this operation. */
    CurrentLinkNode = ListData->CurrentItem;

  }

  /* Save the old values of DataSize, DataTag, and DataLocation for return to
     the caller.                                                              */
  OldItemSize = CurrentLinkNode->DataSize;
  OldItemTag = CurrentLinkNode->DataTag;
  OldItemLocation = CurrentLinkNode->DataLocation;

  /* Now lets put our replacement into the list. */
  CurrentLinkNode->DataSize = *ItemSize;
  CurrentLinkNode->DataTag = *ItemTag;
  CurrentLinkNode->DataLocation = ItemLocation;

  /* Setup return values for user. */
  *ItemSize = OldItemSize;
  *ItemTag = OldItemTag;

  /* Did the user want this item to become the current item in the list? */
  if ( MakeCurrent )
  {

    /* Make this item the current item in the list. */
    ListData->CurrentItem = CurrentLinkNode;

  }

#ifdef PARANOID

  assert (CheckListIntegrity( ListToReplaceItemIn ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

  return OldItemLocation;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetTag                                          */
/*                                                                   */
/*   Descriptive Name:  This function returns the item tag associated*/
/*                      with the current item in the list.           */
/*                                                                   */
/*   Input:  DLIST   ListToGetTagFrom : The list from which the item */
/*                                     tag of the current item is to */
/*                                     be returned                   */
/*           ADDRESS Handle : The handle of the item whose TAG and   */
/*                            size we are to get.  This handle must  */
/*                            be of an item which resides in         */
/*                            in ListToGetTagFrom, or NULL.  If NULL */
/*                            then the current item in the list will */
/*                            be used.                               */
/*           CARDINAL32 * ItemSize : The size, in bytes, of the      */
/*                                   current item in the list.       */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, the function returns the item tag & size*/
/*               associated with the current item in ListToGetTagFrom*/
/*               and *Error is set to 0.                             */
/*            If unsuccessful, the function returns 0 and *Error is  */
/*               set to a non-zero error code.                       */
/*                                                                   */
/*   Error Handling: This function will fail if ListToGetTagFrom is  */
/*                   not a valid list or is an empty list.  In either*/
/*                   of these cases, *Error is set to a non-zero     */
/*                   error code.                                     */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
TAG _System GetTag( DLIST  ListToGetTagFrom,
                    ADDRESS Handle,
                    CARDINAL32 * ItemSize,
                    CARDINAL32 * Error)
{

  /* Since ListToGetTagFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToGetTagFrom.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;

  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         item while we access its data.        */

  /* We will assume that ListToGetTagFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToGetTagFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetTagFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return 0;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return 0;
  }

  #endif

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return (0);
  }

  /* Were we given a handle? */
  if ( Handle != NULL )
  {

    /* Lets check the handle we were given.*/
    CurrentLinkNode = (LinkNode*) Handle;

    /* Is the handle valid? */
    if ( CurrentLinkNode->ControlNodeLocation != ListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return NULL;

    }

  }
  else
  {

    /* Since we were not given a handle, we will use the current item in the list for this operation. */
    CurrentLinkNode = ListData->CurrentItem;

  }

  /* Indicate success */
  *Error = DLIST_SUCCESS;


#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetTagFrom ) );

#endif


  /* Return the tag and item size from the current node. */
  *ItemSize = CurrentLinkNode->DataSize;
  return (CurrentLinkNode->DataTag);

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetHandle                                       */
/*                                                                   */
/*   Descriptive Name:  This function returns a handle for the       */
/*                      current item in the list.  This handle is    */
/*                      then associated with that item regardless of */
/*                      its position in the list.  This handle can be*/
/*                      used to make its associated item the current */
/*                      item in the list.                            */
/*                                                                   */
/*   Input:  DLIST   ListToGetHandleFrom : The list from which a     */
/*                                        handle is needed.          */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, the function returns a handle for the   */
/*               the current item in ListToGetHandleFrom, and *Error */
/*               is set to 0.                                        */
/*            If unsuccessful, the function returns 0 and *Error is  */
/*               set to a non-zero error code.                       */
/*                                                                   */
/*   Error Handling: This function will fail if ListToGetHandleFrom  */
/*                   is not a valid list or is an empty list.  In    */
/*                   either of these cases, *Error is set to a       */
/*                   non-zero error code.                            */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*           The handle returned is a pointer to the LinkNode of the */
/*           current item in the list.  This allows the item to move */
/*           around in the list without losing its associated handle.*/
/*           However, if the item is deleted from the list, then the */
/*           handle is invalid and its use could result in a trap.   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System GetHandle ( DLIST ListToGetHandleFrom, CARDINAL32 * Error)
{

  /* Since ListToGetHandleFrom is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToGetHandleFrom.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  /* We will assume that ListToGetHandleFrom points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToGetHandleFrom;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetHandleFrom) )
  {
    *Error = DLIST_CORRUPTED;
    return NULL;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return NULL;
  }

  #endif

#endif

  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return (0);
  }

#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetHandleFrom ) );

#endif


  /* Indicate success */
  *Error = DLIST_SUCCESS;


  /* Return the address of the CurrentItem. This will serve as the handle. */
  return (ListData->CurrentItem);
}



/*********************************************************************/
/*                                                                   */
/*   Function Name:  GetListSize                                     */
/*                                                                   */
/*   Descriptive Name:  This function returns the number of items in */
/*                      a list.                                      */
/*                                                                   */
/*   Input:  DLIST   ListToGetSizeOf : The list whose size we wish to*/
/*                                    know                           */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, the function returns the a count of the */
/*               number of items in the list, and *Error is set to 0.*/
/*            If unsuccessful, the function returns 0 and *Error is  */
/*               set to a non-zero error code.                       */
/*                                                                   */
/*   Error Handling: This function will fail if ListToGetSizeOf is   */
/*                   not a valid list.  If this happens, then *Error */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _System GetListSize( DLIST ListToGetSizeOf, CARDINAL32 * Error)
{

  /* Since ListToGetSizeOf is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToGetSizeOf.  This way we just do the
     cast once.                                                            */
  ControlNode *   ListData;


  /* We will assume that ListToGetSizeOf points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToGetSizeOf;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToGetSizeOf) )
  {
    *Error = DLIST_CORRUPTED;
    return 0;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return 0;
  }

  #endif

#endif

#ifdef PARANOID

  assert (CheckListIntegrity( ListToGetSizeOf ) );

#endif


  /* Indicate success. */
  *Error = DLIST_SUCCESS;


  /* Return the size of the list. */
  return (ListData->ItemCount);
}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ListEmpty                                       */
/*                                                                   */
/*   Descriptive Name:  This function returns TRUE if the            */
/*                      specified list is empty, otherwise it returns*/
/*                      FALSE.                                       */
/*                                                                   */
/*   Input:  DLIST       ListToCheck : The list to check to see if it*/
/*                                    is empty                       */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, the function returns TRUE if the        */
/*               number of items in the list is 0, otherwise it      */
/*               returns FALSE.  Also, *Error is set to 0.           */
/*            If unsuccessful, the function returns TRUE and         */
/*               *Error is set to a non-zero error code.             */
/*                                                                   */
/*   Error Handling: This function will fail if ListToCheck is not   */
/*                   a valid list.  If this happens, then *Error     */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System ListEmpty( DLIST ListToCheck, CARDINAL32 * Error)
{
  /* Since ListToCheck is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToCheck.  This way we just do the
     cast once.                                                            */
  ControlNode *   ListData;


  /* We will assume that ListToCheck points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToCheck;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToCheck) )
  {
    *Error = DLIST_CORRUPTED;
    return TRUE;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return TRUE;
  }

  #endif

#endif

  /* Indicate Success */
  *Error = DLIST_SUCCESS;


  /* Check to see if the DLIST is empty. */
  if (ListData->ItemCount == 0)
    return (TRUE);
  else
    return (FALSE);
}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  AtEndOfList                                     */
/*                                                                   */
/*   Descriptive Name:  This function returns TRUE if the            */
/*                      current item in the list is the last item    */
/*                      in the list.  Returns FALSE otherwise.       */
/*                                                                   */
/*   Input:  DLIST       ListToCheck : The list to check.            */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, the function returns TRUE if the        */
/*               current item in the list is the last item in the    */
/*               list.  If it is not the last item in the list,      */
/*               FALSE is returned.  *Error_Code is set to           */
/*               DLIST_SUCCESS.                                      */
/*            If unsuccessful, the function returns FALSE and        */
/*               *Error is set to a non-zero error code.             */
/*                                                                   */
/*   Error Handling: This function will fail if ListToCheck is not   */
/*                   a valid list.  If this happens, then *Error     */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System AtEndOfList( DLIST ListToCheck, CARDINAL32 * Error)
{

  /* Since ListToCheck is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToCheck.  This way we just do the
     cast once.                                                            */
  ControlNode *   ListData;


  /* We will assume that ListToCheck points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToCheck;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToCheck) )
  {
    *Error = DLIST_CORRUPTED;
    return FALSE;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return FALSE;
  }

  #endif

#endif

  /* Indicate Success */
  *Error = DLIST_SUCCESS;


  /* Check to see if the current item in the list is also the last item in the list. */
  if (ListData->CurrentItem == ListData->EndOfList)
    return (TRUE);
  else
    return (FALSE);

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  AtStartOfList                                   */
/*                                                                   */
/*   Descriptive Name:  This function returns TRUE if the            */
/*                      current item in the list is the first item   */
/*                      in the list.  Returns FALSE otherwise.       */
/*                                                                   */
/*   Input:  DLIST       ListToCheck : The list to check.            */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, the function returns TRUE if the        */
/*               current item in the list is the first item in the   */
/*               list.  If it is not the first item in the list,     */
/*               FALSE is returned.  *Error_Code is set to           */
/*               DLIST_SUCCESS.                                      */
/*            If unsuccessful, the function returns FALSE and        */
/*               *Error is set to a non-zero error code.             */
/*                                                                   */
/*   Error Handling: This function will fail if ListToCheck is not   */
/*                   a valid list.  If this happens, then *Error     */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System AtStartOfList( DLIST ListToCheck, CARDINAL32 * Error)
{

  /* Since ListToCheck is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToCheck.  This way we just do the
     cast once.                                                            */
  ControlNode *   ListData;


  /* We will assume that ListToCheck points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToCheck;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToCheck) )
  {
    *Error = DLIST_CORRUPTED;
    return FALSE;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return FALSE;
  }

  #endif

#endif

  /* Indicate Success */
  *Error = DLIST_SUCCESS;


  /* Check to see if the current item in the list is also the first item in the list. */
  if (ListData->CurrentItem == ListData->StartOfList)
    return (TRUE);
  else
    return (FALSE);

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  DestroyList                                     */
/*                                                                   */
/*   Descriptive Name:  This function releases the memory associated */
/*                      with the internal data structures of a DLIST.*/
/*                      Once a DLIST has been destroyed by this      */
/*                      function, it must be reinitialized before it */
/*                      can be used again.                           */
/*                                                                   */
/*   Input:  DLIST       ListToDestroy : The list to be eliminated   */
/*                                      from memory.                 */
/*           BOOLEAN FreeItemMemory : If TRUE, all items in the list */
/*                                    will be freed.  If FALSE, all  */
/*                                    items in the list are not      */
/*                                    freed, only the list structures*/
/*                                    associated with them are.      */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail if ListToDestroy is not */
/*                   a valid list.  If this happens, then *Error     */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*           If FreeItemMemory is TRUE, then this function will try  */
/*           to delete any items which may be in the list.  However, */
/*           since this function has no way of knowing the internal  */
/*           structure of an item, items which contain embedded      */
/*           pointers will not be entirely freed.  This can lead to  */
/*           memory leaks.  The programmer should ensure that any    */
/*           list passed to this function when the FreeItemMemory    */
/*           parameter is TRUE is empty or does not contain any      */
/*           items with embedded pointers.                           */
/*                                                                   */
/*********************************************************************/
void _System DestroyList( DLIST *  ListToDestroy, BOOLEAN FreeItemMemory, CARDINAL32 * Error)
{

  /* Since ListToDestroy is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToDestroy.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode;  /* This is used to walk through the
                                          linked list of LinkNodes.        */


  /* We will assume that ListToDestroy points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                 */
  ListData = (ControlNode *) (*ListToDestroy);


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListData) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /*--------------------------------------------------
     To dispose of a DLIST, we must traverse the linked
     list of LinkNodes and dispose of each LinkNode,
     as well as the data item associated with each
     LinkNode.  Once all of the LinkNodes (and their
     data items) have been freed, we can then free the
     ControlNode.
  --------------------------------------------------*/

  /* Loop to dispose of the Listnodes. */
  while (ListData->ItemCount > 0)
  {
    CurrentLinkNode = ListData->StartOfList;                /* Get the first DLIST node. */
    ListData->StartOfList = CurrentLinkNode->NextLinkNode;  /* Remove that DLIST node from the DLIST. */
    ListData->ItemCount--;                                  /* Decrement the number of items in the list or we will never leave the loop! */
    if ( (CurrentLinkNode->DataLocation != NULL) && FreeItemMemory )
    {
#ifdef USE_POOLMAN
      SmartFree(CurrentLinkNode->DataLocation);                /* Free the heap memory used to store the data for the DLIST node. */
#else
      free(CurrentLinkNode->DataLocation);
#endif
    }
    CurrentLinkNode->ControlNodeLocation = NULL;
#ifdef USE_POOLMAN
    DeallocateToPool(ListData->NodePool,CurrentLinkNode);   /* Return LinkNode to the Node Pool. */
#else
    free(CurrentLinkNode);
#endif
  }

#ifdef USE_POOLMAN

  /* Release the memory associated with the NodePool for list being destroyed. */
  DestroyPool(ListData->NodePool);

#endif

#ifdef DEBUG

  /* Set Verify to 0 so that, if the same block of
     memory is reused for another list, the InitializeList
     function does not get fooled into thinking that
     the block of memory already contains a valid list.    */
  ListData->Verify = 0;

#endif



  /* Now free the memory used to store the master DLIST node. */
#ifdef USE_POOLMAN
  SmartFree(*ListToDestroy);
#else
  free(*ListToDestroy);
#endif
  *ListToDestroy = NULL;

  /* Signal success. */
  *Error = DLIST_SUCCESS;


}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  NextItem                                        */
/*                                                                   */
/*   Descriptive Name:  This function makes the next item in the list*/
/*                      the current item in the list (i.e. it        */
/*                      advances the current item pointer).          */
/*                                                                   */
/*   Input:  DLIST       ListToAdvance : The list whose current item */
/*                                      pointer is to be advanced    */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                        ListToAdvance is not a valid list          */
/*                        ListToAdvance is empty                     */
/*                        The current item is the last item in the   */
/*                           list                                    */
/*                   If any of these conditions occurs, then *Error  */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void _System NextItem( DLIST  ListToAdvance, CARDINAL32 * Error)
{

  /* Since ListToAdvance is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToAdvance.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item while we access its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */



  /* We will assume that ListToAdvance points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToAdvance;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToAdvance) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Check for empty list. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return;
  }

  /* Check for end of list. */
  if (ListData->CurrentItem == ListData->EndOfList)
  {
    *Error = DLIST_END_OF_LIST;
    return;
  }

  /* Advance the current item pointer. */
  CurrentLinkNode = ListData->CurrentItem;
  ListData->CurrentItem = CurrentLinkNode->NextLinkNode;

#ifdef PARANOID

  assert (CheckListIntegrity( ListToAdvance ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;


}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  PreviousItem                                    */
/*                                                                   */
/*   Descriptive Name:  This function makes the previous item in the */
/*                      list the current item in the list.           */
/*                                                                   */
/*   Input:  DLIST       ListToChange : The list whose current item  */
/*                                      pointer is to be changed     */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail under the following     */
/*                   conditions:                                     */
/*                        ListToChange is not a valid list           */
/*                        ListToChange is empty                      */
/*                        The current item is the first item in the  */
/*                           list                                    */
/*                   If any of these conditions occurs, then *Error  */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void _System PreviousItem( DLIST  ListToChange, CARDINAL32 * Error)
{

  /* Since ListToAdvance is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToAdvance.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item while we access its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */



  /* We will assume that ListToChange points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToChange;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToChange) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Check for empty list. */
  if (ListData->ItemCount == 0)
  {
    *Error = DLIST_EMPTY;
    return;
  }

  /* Check for beginning of list. */
  if (ListData->CurrentItem == ListData->StartOfList)
  {
    *Error = DLIST_ALREADY_AT_START;
    return;
  }

  /* Position the current item pointer. */
  CurrentLinkNode = ListData->CurrentItem;
  ListData->CurrentItem = CurrentLinkNode->PreviousLinkNode;

#ifdef PARANOID

  assert (CheckListIntegrity( ListToChange ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GoToStartOfList                                  */
/*                                                                   */
/*   Descriptive Name:  This function makes the first item in the    */
/*                      list the current item in the list.           */
/*                                                                   */
/*   Input:  DLIST       ListToReset : The list whose current item   */
/*                                    is to be set to the first item */
/*                                    in the list                    */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail if ListToAdvance is not */
/*                   a valid list.  If this occurs, then *Error      */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void _System GoToStartOfList( DLIST ListToReset, CARDINAL32 * Error)
{

  /* Since ListToReset is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToReset.  This way we just do the
     cast once.                                                            */
  ControlNode *   ListData;



  /* We will assume that ListToReset points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                 */
  ListData = (ControlNode *) ListToReset;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToReset) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Set the current item pointer. */
  ListData->CurrentItem = ListData->StartOfList;


#ifdef PARANOID

  assert (CheckListIntegrity( ListToReset ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GoToEndOfList                                    */
/*                                                                   */
/*   Descriptive Name:  This function makes the last item in the     */
/*                      list the current item in the list.           */
/*                                                                   */
/*   Input:  DLIST       ListToSet : The list whose current item     */
/*                                    is to be set to the last item  */
/*                                    in the list                    */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail if ListToSet is not     */
/*                   a valid list.  If this occurs, then *Error      */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void _System GoToEndOfList( DLIST ListToSet, CARDINAL32 * Error)
{

  /* Since ListToSet is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToSet.  This way we just do the
     cast once.                                                            */
  ControlNode *   ListData;



  /* We will assume that ListToSet points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                 */
  ListData = (ControlNode *) ListToSet;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToSet) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Set the current item pointer. */
  ListData->CurrentItem = ListData->EndOfList;

#ifdef PARANOID

  assert (CheckListIntegrity( ListToSet ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GoToSpecifiedItem                                */
/*                                                                   */
/*   Descriptive Name:  This function makes the item associated with */
/*                      Handle the current item in the list.         */
/*                                                                   */
/*   Input:  DLIST  ListToReposition:  The list whose current item   */
/*                                    is to be set to the item       */
/*                                    associated with Handle.        */
/*           ADDRESS Handle : A handle obtained by using the         */
/*                            GetHandle function.  This handle       */
/*                            identifies a unique item in the list.  */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return code              */
/*                                                                   */
/*   Output:  If successful, *Error will be set to 0.                */
/*            If unsuccessful, *Error will be set to a non-zero error*/
/*               code.                                               */
/*                                                                   */
/*   Error Handling: This function will fail if ListToReposition is  */
/*                   not a valid list.  If this occurs, then *Error  */
/*                   is set to a non-zero error code.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*                                                                   */
/*           It is assumed that Handle is a valid handle and that    */
/*           the item associated with Handle is still in the list.   */
/*           If these conditions are not met, an exception or trap   */
/*           may occur.                                              */
/*                                                                   */
/*********************************************************************/
void _System GoToSpecifiedItem( DLIST ListToReposition, ADDRESS Handle, CARDINAL32 * Error)
{
  /* Since ListToReposition is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToReposition.  This way we just do the
     cast once.                                                            */
  ControlNode *   ListData;

  LinkNode *      CurrentNode = (LinkNode *) Handle;  /* Used to minimize type casting
                                                         when manipulating and testing
                                                         the handle.                   */

  /* We will assume that ListToReposition points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                 */
  ListData = (ControlNode *) ListToReposition;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToReposition) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Since the list is valid, we must now see if the Handle is valid.  We
     will assume that, if the Handle is not NULL, it points to a LinkNode.
     If the ControlNodeLocation field of the LinkNode points to the
     ControlNode for ListToReposition, then the LinkNode is in ListToReposition
     and can therefore become the current item in ListToReposition. */
  if ( (CurrentNode != NULL  ) &&
       (CurrentNode->ControlNodeLocation == ListData) )
  {
    /* The handle pointed to a valid LinkNode which is in ListToReposition.
       Lets make that node the current item in ListToReposition.            */
    ListData->CurrentItem = CurrentNode;
  }
  else
  {
    /* The handle was either NULL or the LinkNode it pointed to was not
       in ListToReposition. */
    *Error = DLIST_BAD_HANDLE;
    return;
  }


#ifdef PARANOID

  assert (CheckListIntegrity( ListToReposition ) );

#endif


  /* Signal success. */
  *Error = DLIST_SUCCESS;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  SortList                                        */
/*                                                                   */
/*   Descriptive Name:  This function sorts the contents of a list.  */
/*                      The sorting algorithm used is a stable sort  */
/*                      whose performance is not dependent upon the  */
/*                      initial order of the items in the list.      */
/*                                                                   */
/*   Input: DLIST ListToSort : The DLIST that is to be sorted.       */
/*                                                                   */
/*          INTEGER32 (*Compare) ( ... )                             */
/*                                                                   */
/*              This is a pointer to a function that can compare any */
/*              two items in the list.  It should return -1 if       */
/*              Object1 is less than Object2, 0 if Object1 is equal  */
/*              to Object2, and 1 if Object1 is greater than Object2.*/
/*              This function will be called during the sort whenever*/
/*              the sorting algorithm needs to compare two objects.  */
/*                                                                   */
/*              The Compare function takes the following parameters: */
/*                                                                   */
/*              ADDRESS Object1 : The address of the data for the    */
/*                                first object to be compared.       */
/*              TAG Object1Tag : The user assigned TAG value for the */
/*                               first object to be compared.        */
/*              ADDRESS Object2 : The address of the data for the    */
/*                                second object to be compared.      */
/*              TAG Object2Tag : The user assigned TAG value for the */
/*                               second object to be compared.       */
/*              CARDINAL32 * Error : The address of a variable to    */
/*                                   hold the error return value.    */
/*                                                                   */
/*              If this function ever sets *Error to a non-zero value*/
/*              the sort will terminate and the error code will be   */
/*              returned to the caller of the SortList function.     */
/*                                                                   */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return value.             */
/*                                                                   */
/*   Output:  If successful, this function will set *Error to        */
/*               DLIST_SUCCESS and ListToSort will have been sorted. */
/*            If unsuccessful, *Error will contain an error code.    */
/*               The order of the items in ListToSort is undefined   */
/*               and may have changed.                               */
/*                                                                   */
/*   Error Handling: This function will terminate if *Compare sets   */
/*                   *Error to a non-zero value, or if ListToSort    */
/*                   is invalid.  If this function does terminate in */
/*                   the middle of a sort, the order of the items in */
/*                   ListToSort may be different than it was before  */
/*                   the function was called.                        */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  It is assumed that Error contains a valid address. If   */
/*           this assumption is violated, an exception or trap       */
/*           may occur.                                              */
/*                                                                   */
/*           This function works by breaking the list into sublists  */
/*           and merging the sublists back into one list.  The size  */
/*           of the sublists starts at 1, and with each pass, the    */
/*           of the sublists is doubled.  The sort ends when the size*/
/*           of a sublist is greater than the size of the original   */
/*           list.                                                   */
/*                                                                   */
/*********************************************************************/
void _System SortList(DLIST ListToSort,
                      INTEGER32 (* _System Compare) (ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag,CARDINAL32 * Error),
                      CARDINAL32 * Error)
{
  ControlNode *   ListData;

  LinkNode *      NodeToMove;

  LinkNode *      MergeList1;
  CARDINAL32      MergeList1Size;

  LinkNode *      MergeList2;
  CARDINAL32      MergeList2Size;

  CARDINAL32      MergeListMaxSize;
  CARDINAL32      ListSize;

  INTEGER32       CompareResult;

  /* We will assume that ListToSort points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                 */
  ListData = (ControlNode *) ListToSort;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToSort) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* We will assume success until proven otherwise. */
  *Error = DLIST_SUCCESS;

  /* Is the list big enough to sort? */
  if ( ListData->ItemCount > 1)
  {

    /* The original list will be repeatedly broken into sublists, which are then
       merged back into one list.  This process is done two sublists at a time.
       The two sublists are MergeList1 and MergeList2.  Both sublists are the
       same size.  The only exception occurs when there are not enough items
       remaining to create a MergeList2 of the same size as the MergeList1.
       The size of MergeList1 and MergeList2 starts out at 1, and will be doubled
       with each iteration of the outer "do" loop below.                            */
    MergeListMaxSize = 1;

    /* This is the outer "do" loop which controls the size of the sublists being
       merged.  The sublists are merged two at a time, with MergeList1 and
       MergeList2 representing the two sublists being merged.                     */
    do
    {

      /* The first sublist will always start with the first element of the
         list being sorted.                                                  */
      MergeList1 = ListData->StartOfList;

      /* This loop controls the merging of sublists back into one list. */
      do
      {

        /* The maximum number of items in each of the sublists to be merged
           is MergeListMaxSize.  As items are merged, they are removed from
           the sublist they were in and placed in the single list which results
           from the merging process.                                             */
        MergeList1Size = MergeListMaxSize;
        MergeList2Size = MergeListMaxSize;

        /* Find the start of the second list for merging. */
        ListSize = MergeList1Size;
        MergeList2 = MergeList1;
        while ( ( MergeList2 != NULL  ) && (ListSize > 0) )
        {

          MergeList2 = MergeList2->NextLinkNode;
          ListSize--;

        }

        /* Now merge the two lists */
        while ( (MergeList1 != NULL) && (MergeList2 != NULL) &&
                (MergeList1Size > 0) && (MergeList2Size > 0) )
        {

          /* Compare the first item in MergeList1 with the first item in MergeList2. */
          CompareResult = (*Compare)(MergeList1->DataLocation,MergeList1->DataTag,MergeList2->DataLocation,MergeList2->DataTag,Error);

          /* If there was an error during the comparision, bail out! */
          if ( *Error != DLIST_SUCCESS )
          {

            return;

          }

          /* See who gets moved. */
          if ( CompareResult > 0 )
          {
            /* Object1 is greater than Object2. */

            /* Object2 must be placed before Object 1. */
            NodeToMove = MergeList2;

            /* Make MergeList2 point to the new start of the second list. */
            MergeList2 = MergeList2->NextLinkNode;

            /* If NodeToMove was the last item in the list, we must update EndOfList since
               NodeToMove will no longer be the last item in the list!                           */
            if ( NodeToMove == ListData->EndOfList )
            {
              ListData->EndOfList = NodeToMove->PreviousLinkNode;
            }

            /* Remove NodeToMove from the list. */
            if ( NodeToMove->PreviousLinkNode != NULL)
            {
              NodeToMove->PreviousLinkNode->NextLinkNode = MergeList2;

              if (MergeList2 != NULL)
              {
                MergeList2->PreviousLinkNode = NodeToMove->PreviousLinkNode;
              }

            }

            /* NodeToMove must go in front of the current item in the first list.  The
              current item in the first list is given by MergeList1.                          */
            if (MergeList1->PreviousLinkNode != NULL)
            {
              /* Make the item before MergeList1 point to NodeToMove. */
              MergeList1->PreviousLinkNode->NextLinkNode = NodeToMove;
            }

            /* Make NodeToMove->PreviousLinkNode point to the item before MergeList1. */
            NodeToMove->PreviousLinkNode = MergeList1->PreviousLinkNode;

            /* Make NodeToMove->NextLinkNode point to MergeList1. */
            NodeToMove->NextLinkNode = MergeList1;

            /* Complete the process by making MergeList1->PreviousLinkNode point to NodeToMove. */
            MergeList1->PreviousLinkNode = NodeToMove;

            /* If MergeList1 was the first item in the list, we must update StartOfList since
              MergeList1 is nolonger the first item in the list!                             */
            if ( MergeList1 == ListData->StartOfList )
            {
              ListData->StartOfList = NodeToMove;
            }

            MergeList2Size--;
          }
          else
          {
            /* Object1 is less than or equal to Object2. */

            /* Remove Object1 from the first list.  To do this, we just need to
               advance the MergeList1 pointer, since it always points to the
               first item in the first of the lists which are being merged.      */
            MergeList1 = MergeList1->NextLinkNode;
            MergeList1Size--;
          }

        }

        /* We have left the while loop.  All of the items in one of the merge lists
           must have been used.  We must now setup MergeList1 to point to the first
           of the next two lists to be merged.                                      */
        if ( (MergeList2Size == 0) || (MergeList2 == NULL) )
        {

          /* MergeList2 is empty.  Either MergeList2 now points to the first
             item in the next list to be merged, or MergeList2 is NULL.  Thus,
             MergeList2 points to what MergeList1 should point to.  So make
             MergeList1 equal to MergeList2.  When we reach the top of the
             "do" loop, MergeList2 will be set to point to the proper location. */
          MergeList1 = MergeList2;

        }
        else
        {

          /* The first of the next two lists to be merged starts after the end of the
             list pointed to by MergeList2.  Thus, we must start MergeList1 at
             MergeList2 and advance it past the remaining items in MergeList2.        */
          ListSize = MergeList2Size;
          MergeList1 = MergeList2;
          while ( ( MergeList1 != NULL  ) && (ListSize > 0) )
          {

            MergeList1 = MergeList1->NextLinkNode;
            ListSize--;

          }

        }

      } while (MergeList1 != NULL);

      MergeListMaxSize = MergeListMaxSize * 2;

    } while ( ListData->ItemCount > MergeListMaxSize);

  }

#ifdef PARANOID

  assert (CheckListIntegrity( ListToSort ) );

#endif

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  ForEachItem                                     */
/*                                                                   */
/*   Descriptive Name:  This function passes a pointer to each item  */
/*                      in a list to a user provided function for    */
/*                      processing by the user provided function.    */
/*                                                                   */
/*   Input:  DLIST ListToProcess : The DLIST whose items are to be   */
/*                                processed by the user provided     */
/*                                function.                          */
/*                                                                   */
/*           void (*ProcessItem) (...)                               */
/*                                                                   */
/*               This is a pointer to the user provided function.    */
/*               This user provided function takes the following     */
/*                  parameters:                                      */
/*                                                                   */
/*                  ADDRESS Object : A pointer to an item in         */
/*                                   ListToProcess.                  */
/*                  TAG Object1Tag : The user assigned TAG value for */
/*                                   the item pointed to by Object.  */
/*                  ADDRESS Parameter : The address of a block of    */
/*                                      memory containing any        */
/*                                      parameters that the user     */
/*                                      wishes to have passed to this*/
/*                                      function.                    */
/*                  CARDINAL32 * Error : The address of a variable to*/
/*                                       hold the error return value.*/
/*                                                                   */
/*           ADDRESS Parameters : This field is passed through to    */
/*                                *ProcessItem.  This function does  */
/*                                not even look at the contents of   */
/*                                this field.  This field is here to */
/*                                provide the user a way to pass     */
/*                                additional data to *ProcessItem    */
/*                                that *ProcessItem may need to      */
/*                                function correctly.                */
/*                                                                   */
/*           CARDINAL32 * Error : The address of a variable to hold  */
/*                                the error return value.            */
/*                                                                   */
/*   Output:  If successful, this function will set *Error to        */
/*               DLIST_SUCCESS.                                      */
/*            If unsuccessful, then this function will set *Error to */
/*               a non-zero error code.                              */
/*                                                                   */
/*   Error Handling: This function aborts immediately when an error  */
/*                   is detected, and any remaining items in the list*/
/*                   will not be processed.                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function allows the user to access all of the items */
/*          in a list and perform an operation on them.  The         */
/*          operation performed must not free any items in the list, */
/*          or perform any list operations on the list being         */
/*          processed.                                               */
/*                                                                   */
/*          As an example of when this would be useful, consider a   */
/*          a list of graphic objects (rectangles, triangles, circles*/
/*          etc.)  which comprise a drawing.  To draw the picture    */
/*          that these graphic objects represent, one could build a  */
/*          loop which gets and draws each item.  Another way to     */
/*          do this would be to build a drawing function which can   */
/*          draw any of the graphic objects, and then use that       */
/*          function as the ProcessItem function in a call to        */
/*          ForEachItem.                                             */
/*                                                                   */
/*          If the ProcessItem function sets *Error to something     */
/*          other than DLIST_SUCCESS, then ForEachItem will terminate*/
/*          and return an error to whoever called it.  The single    */
/*          exception to this is if ProcessItem sets *Error to       */
/*          DLIST_SEARCH_COMPLETE, in which case ForEachItem         */
/*          terminates and sets *Error to DLIST_SUCCESS.  This is    */
/*          usefull for using ForEachItem to search a list and then  */
/*          terminating the search once the desired item is found.   */
/*                                                                   */
/*          A word about the Parameters parameter.  This parameter   */
/*          is passed through to *ProcessItem and is never looked at */
/*          by this function.  This means that the user can put any  */
/*          value they desire into Parameters as long as it is the   */
/*          same size (in bytes) as Parameters.  The intended use of */
/*          Parameters is to allow the user to pass information to   */
/*          *ProcessItem that *ProcessItem may need.  Either way,    */
/*          how Parameters is used is literally up to the user.      */
/*                                                                   */
/*********************************************************************/
void _System ForEachItem(DLIST        ListToProcess,
                         void         (* _System ProcessItem) (ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error),
                         ADDRESS      Parameters,
                         BOOLEAN      Forward,
                         CARDINAL32 * Error)
{

  /* Since ListToProcess is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToProcess.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item while we access its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */


  /* We will assume that ListToProcess points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToProcess;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToProcess) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Check for empty list. */
  if (ListData->ItemCount == 0)
  {
    return;
  }

  /* Set CurrentLinkNode based upon the direction we are going to traverse the list. */
  if ( Forward )
  {

    /* Get the first link node in the list. */
    CurrentLinkNode = ListData->StartOfList;

  }
  else
  {

    /* Get the last link node in the list. */
    CurrentLinkNode = ListData->EndOfList;

  }

  /* Now loop through the items in the list. */
  while ( CurrentLinkNode != NULL )
  {

    /* Call the user provided function to process the current item in the list. */
    (*ProcessItem)(CurrentLinkNode->DataLocation,CurrentLinkNode->DataTag,CurrentLinkNode->DataSize, CurrentLinkNode, Parameters,Error);
    if ( *Error != DLIST_SUCCESS )
    {

      if ( *Error == DLIST_SEARCH_COMPLETE )
        *Error = DLIST_SUCCESS;

      return;

    }

    /* Advance to the next item in the list based upon the direction that we are traversing the list in. */
    if ( Forward )
    {

      CurrentLinkNode = CurrentLinkNode->NextLinkNode;

    }
    else
    {

      CurrentLinkNode = CurrentLinkNode->PreviousLinkNode;

    }

  }

  /* All items in the list have been processed. */
#ifdef PARANOID

  assert (CheckListIntegrity( ListToProcess ) );

#endif


}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  PruneList                                       */
/*                                                                   */
/*   Descriptive Name:  This function allows the caller to examine   */
/*                      each item in a list and optionally delete    */
/*                      it from the list.                            */
/*                                                                   */
/*   Input:  DLIST ListToProcess : The DLIST to be pruned.           */
/*                                                                   */
/*           BOOLEAN (*KillItem) (...)                               */
/*                                                                   */
/*               This is a pointer to a user provided function.      */
/*               This user provided function takes the following     */
/*                  parameters:                                      */
/*                                                                   */
/*                  ADDRESS Object : A pointer to an item in         */
/*                                   ListToProcess.                  */
/*                  TAG Object1Tag : The user assigned TAG value for */
/*                                   the item pointed to by Object.  */
/*                  ADDRESS Parameter : The address of a block of    */
/*                                      memory containing any        */
/*                                      parameters that the user     */
/*                                      wishes to have passed to this*/
/*                                      function.                    */
/*                  BOOLEAN * FreeMemory : The address of a BOOLEAN  */
/*                                         variable which this       */
/*                                         function will set to      */
/*                                         either TRUE or FALSE.     */
/*                                         If the function return    */
/*                                         value is TRUE, then the   */
/*                                         value in *FreeMemory will */
/*                                         be examined.  If it is    */
/*                                         TRUE, then PruneList will */
/*                                         free the memory associated*/
/*                                         with the item being       */
/*                                         deleted.  If *FreeMemory  */
/*                                         is FALSE, then the item   */
/*                                         being removed from the    */
/*                                         DLIST will not be freed,  */
/*                                         and it is up to the user  */
/*                                         to ensure that this memory*/
/*                                         is handled properly.      */
/*                  CARDINAL32 * Error : The address of a variable to*/
/*                                       hold the error return value.*/
/*                                                                   */
/*           ADDRESS Parameters : This field is passed through to    */
/*                                *ProcessItem.  This function does  */
/*                                not even look at the contents of   */
/*                                this field.  This field is here to */
/*                                provide the user a way to pass     */
/*                                additional data to *ProcessItem    */
/*                                that *ProcessItem may need to      */
/*                                function correctly.                */
/*                                                                   */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return value.             */
/*                                                                   */
/*   Output:  If successful, this function will set *Error to        */
/*               DLIST_SUCCESS.                                      */
/*            If unsuccessful, then this function will set *Error to */
/*               a non-zero error code.                              */
/*                                                                   */
/*   Error Handling: This function aborts immediately when an error  */
/*                   is detected, and any remaining items in the list*/
/*                   will not be processed.                          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function allows the user to access all of the items */
/*          in a list, perform an operation on them, and then        */
/*          optionally delete ("remove") them from the DLIST.  The   */
/*          operation performed must not free any items in the list, */
/*          or perform any list operations on the list being         */
/*          processed.                                               */
/*                                                                   */
/*          If the KillItem function sets *Error to something other  */
/*          than DLIST_SUCCESS, then PruneList will terminate and    */
/*          return an error to whoever called it.  The single        */
/*          exception to this is if KillItem sets *Error to          */
/*          DLIST_SEARCH_COMPLETE, in which case KillItem            */
/*          terminates and sets *Error to DLIST_SUCCESS.  This is    */
/*          usefull for using KillItem to search a list and then     */
/*          terminating the search once the desired item is found.   */
/*                                                                   */
/*          A word about the Parameters parameter.  This parameter   */
/*          is passed through to *ProcessItem and is never looked at */
/*          by this function.  This means that the user can put any  */
/*          value they desire into Parameters as long as it is the   */
/*          same size (in bytes) as Parameters.  The intended use of */
/*          Parameters is to allow the user to pass information to   */
/*          *ProcessItem that *ProcessItem may need.  Either way,    */
/*          how Parameters is used is literally up to the user.      */
/*                                                                   */
/*********************************************************************/
void _System PruneList(DLIST        ListToProcess,
                       BOOLEAN      (* _System KillItem) (ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error),
                       ADDRESS      Parameters,
                       CARDINAL32 * Error)
{

  /* Since ListToProcess is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToProcess.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item while we access its data.
                                         This limits the levels of indirection
                                         to one, which should result in faster
                                         execution. */
  LinkNode *         PreviousLinkNode;/* Used to point to the LinkNode immediately
                                         prior to the CurrentLinkNode.  This is
                                         needed if an item is deleted.             */
  BOOLEAN            FreeMemory;      /* Used as a parameter to KillItem to let the
                                         user indicate whether or not to free the
                                         memory associated with an item that is being
                                         removed from the list.                    */


  /* We will assume that ListToProcess points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToProcess;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(ListToProcess) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Check for empty list. */
  if (ListData->ItemCount == 0)
  {
    return;
  }

  /* Get the first link node in the list. */
  CurrentLinkNode = ListData->StartOfList;

  /* Now loop through the items in the list. */
  while ( CurrentLinkNode != NULL )
  {

    /* Call the user provided function to decide whether or not to keep the
       current item in the list.                                             */
    if ( (*KillItem)(CurrentLinkNode->DataLocation,CurrentLinkNode->DataTag,CurrentLinkNode->DataSize,CurrentLinkNode,Parameters,&FreeMemory,Error) )
    {

      if ( ( *Error != DLIST_SUCCESS ) && ( *Error != DLIST_SEARCH_COMPLETE ) )
      {

        return;

      }

      /* We are to remove the current item from the list. */

      /* Initialize PreviousLinkNode. */
      PreviousLinkNode = CurrentLinkNode->PreviousLinkNode;

      /* Is the item being removed from the list the first item in the list? */
      if ( PreviousLinkNode == NULL )
      {

        /* We are at the start of the list.  Update the StartOfList field. */
        ListData->StartOfList = CurrentLinkNode->NextLinkNode;

      }
      else
      {

        /* We are somewhere in the middle of the list, or possibly even the
           last item in the list.                                            */
        PreviousLinkNode->NextLinkNode = CurrentLinkNode->NextLinkNode;

      }

      /* Is the item we are removing the last item in the list?  If so, update the
         pointer to the last item in the list.                                     */
      if ( CurrentLinkNode == ListData->EndOfList )
      {

        ListData->EndOfList = PreviousLinkNode;

      }
      else
      {

        /* Since CurrentLinkNode is not the last item in the list, we must adjust the
           PreviousLinkNode field of the item following CurrentLinkNode.                 */
        CurrentLinkNode->NextLinkNode->PreviousLinkNode = PreviousLinkNode;

      }

      /* CurrentLinkNode has now been removed from the list. */

      /* Is the item we are deleting the current item in the list? */
      if ( CurrentLinkNode == ListData->CurrentItem )
      {

        /* Since the item we are deleting is the current item, we must choose
           a new current item.  If the item we are deleting is NOT the last
           item in the list, then we will choose the item following it as the
           new current item.  If the item we are deleting is the last item
           in the list, then we will choose the item immediately before it to
           be the new current item.                                            */
        if ( CurrentLinkNode->NextLinkNode != NULL )
        {

          /* We are removing an item from the beginning or middle of the list. */
          ListData->CurrentItem = CurrentLinkNode->NextLinkNode;

        }
        else
        {

          /* We are removing the last item in the list. */
          ListData->CurrentItem = PreviousLinkNode;

        }

      }

      /* Adjust the count of items in the list. */
      ListData->ItemCount = ListData->ItemCount - 1;

      /* Now we must free the memory associated with the current node. */
      if ( FreeMemory )
      {
        /* Free the memory associated with the actual item stored in the list. */
#ifdef USE_POOLMAN
        SmartFree(CurrentLinkNode->DataLocation);
#else
        free(CurrentLinkNode->DataLocation);
#endif
      }

      /* Free the memory associated with the control structures used to manage items in the list. */
      CurrentLinkNode->ControlNodeLocation = NULL;
#ifdef USE_POOLMAN
      DeallocateToPool(ListData->NodePool,CurrentLinkNode);    /* Return LinkNode to the Node Pool. */
#else
      free(CurrentLinkNode);
#endif

      /* Resume our traversal of the tree. */

      /* Are we at the start of the list?  If so, then PreviousLinkNode will be
         NULL since there is no previous link node.                             */
      if ( PreviousLinkNode != NULL )
      {

        /* Since we did not delete the first item in the list, we can resume our
           tree traversal with the item following PreviousLinkNode.               */
        CurrentLinkNode = PreviousLinkNode->NextLinkNode;

      }
      else
      {

        /* The item we deleted was the first item in the list.  We can resume our
           list traversal with the item which is now the first item in the list.  */
        CurrentLinkNode = ListData->StartOfList;

      }

      /* Did the user indicate that we are to stop the list traversal? */
      if ( *Error == DLIST_SEARCH_COMPLETE )
      {

        /* Convert the error code to success and stop the list traversal. */
        *Error = DLIST_SUCCESS;
        return;

      }

    }
    else
    {

      if ( *Error != DLIST_SUCCESS )
      {

        if ( *Error == DLIST_SEARCH_COMPLETE )
          *Error = DLIST_SUCCESS;

        return;

      }

      /* We are keeping the current item in the list. */

      /* Advance to the next item in the list. */
      CurrentLinkNode = CurrentLinkNode->NextLinkNode;

    }

  }

  /* All items in the list have been processed. */
#ifdef PARANOID

  assert (CheckListIntegrity( ListToProcess ) );

#endif



}

/*********************************************************************/
/*                                                                   */
/*   Function Name:  AppendList                                      */
/*                                                                   */
/*   Descriptive Name: Removes the items in SourceList and appends   */
/*                     them to TargetList.                           */
/*                                                                   */
/*   Input:  DLIST TargetList : The DLIST which is to have the items */
/*                             from SourceList appended to it.       */
/*           DLIST SourceList : The DLIST whose items are to be      */
/*                              removed and appended to TargetList.  */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return value.             */
/*                                                                   */
/*   Output: If successful, *Error will be set to DLIST_SUCCESS,     */
/*              SourceList will be empty, and TargetList will contain*/
/*              all of its original items and all of the items that  */
/*              were in SourceList.                                  */
/*           If unsuccessful, *Error will be set to a non-zero value */
/*              and SourceList and TargetList will be unmodified.    */
/*                                                                   */
/*   Error Handling:  This function will abort immediately upon      */
/*                    detection of an error.  All errors that can be */
/*                    detected are detected before the contents of   */
/*                    SourceList are appended to TargetList, so if an*/
/*                    error is detected and the function aborts,     */
/*                    SourceList and TargetList are unaltered.       */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: None.                                                    */
/*                                                                   */
/*********************************************************************/
void _System AppendList(DLIST         TargetList,
                        DLIST         SourceList,
                        CARDINAL32 * Error)
{

  /* Since TargetList is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using TargetList.  This way we just do the
     cast once.                                                            */
  ControlNode *      TargetListData;

  /* Since SourceList is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using SourceList.  This way we just do the
     cast once.                                                            */
  ControlNode *      SourceListData;

  ControlNode        TempListData;    /* Used to hold the control node for
                                         list when the control nodes for
                                         TargetList and SourceList are being
                                         swapped.                             */

  LinkNode *         CurrentLinkNode; /* Used to point to the LinkNode of the
                                         current item  in TargetList while we
                                         access its data.  This limits the
                                         levels of indirection to one, which
                                         should result in faster execution. */
  LinkNode *         SourceLinkNode;  /* Used to point to the LinkNode of
                                         the first item in the SourceList.  */

  /* We will assume that TargetList and SourceList both point to a valid lists.
     Given this, we will initialize TargetListData and SourceListData to point
     to the ControlNode of TargetList and SourceList, respectively.            */
  TargetListData = (ControlNode *) TargetList;
  SourceListData = (ControlNode *) SourceList;


#ifdef DEBUG

  #ifdef PARANOID

  if ( !CheckListIntegrity(TargetList) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  if ( !CheckListIntegrity(SourceList) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  #else

  /* We must now validate the lists before we attempt to use them.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((TargetListData == NULL) || (TargetListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  if ((SourceListData == NULL) || (SourceListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  #endif

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Is the source list empty?  If so, we have nothing to do! */
  if (SourceListData->ItemCount == 0)
  {

    /* The source list is empty!  We are done! */
    return;

  }

  /* Is the target list currently empty? */
  if (TargetListData->ItemCount == 0)
  {

    /* Since the target list is empty but the source list is not, we will just swap the
       control record from the source list with the control record of the target list.    */
    TempListData = *TargetListData;
    *TargetListData = *SourceListData;
    *SourceListData = TempListData;

    /* Get the first item in the target list. */
    CurrentLinkNode = TargetListData->StartOfList;

    /* Adjust the the ControlNodeLocation field of this Link Node. */
    CurrentLinkNode->ControlNodeLocation = TargetListData;

  }
  else
  {

    /* Here's where we do the most work.  Both the TargetList and the SourceList contain
       items.  We must append the items from the SourceList to the TargetList.             */

    /* Get the last item in the target list. */
    CurrentLinkNode = TargetListData->EndOfList;

    /* Get the first item in the source list. */
    SourceLinkNode = SourceListData->StartOfList;

    /* Attach the items from the Source List to the end of those from the Target List. */
    CurrentLinkNode->NextLinkNode = SourceLinkNode;
    SourceLinkNode->PreviousLinkNode = CurrentLinkNode;

    /* Update the Target List Control Information. */
    TargetListData->EndOfList = SourceListData->EndOfList;
    TargetListData->ItemCount = TargetListData->ItemCount + SourceListData->ItemCount;

    /* Update the Source List Control Information. */
    SourceListData->StartOfList = NULL;
    SourceListData->EndOfList = NULL;
    SourceListData->CurrentItem = NULL;
    SourceListData->ItemCount = 0;

  }

  /* Adjust the ControlNodeLocation field of all of the items being moved from the Source List to the Target List. */
  while (CurrentLinkNode->NextLinkNode != NULL)
  {
    CurrentLinkNode = CurrentLinkNode->NextLinkNode;
    CurrentLinkNode->ControlNodeLocation = TargetListData;
  }


#ifdef PARANOID

  assert (CheckListIntegrity( SourceList ) );
  assert (CheckListIntegrity( TargetList ) );

#endif


  /* All done! */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  CheckListIntegrity                              */
/*                                                                   */
/*   Descriptive Name: Checks the integrity of a DLIST.  All link    */
/*                     nodes in the list are checked, as are all     */
/*                     fields in the list control block.             */
/*                                                                   */
/*   Input:  DLIST ListToCheck - The list whose integrity is to be   */
/*                               checked.                            */
/*                                                                   */
/*   Output: The function return value will be TRUE if all of the    */
/*           elements in the DLIST are correct.  If this function    */
/*           returns FALSE, then the DLIST being checked has been    */
/*           corrupted!                                              */
/*                                                                   */
/*   Error Handling: If this function encounters an error in a DLIST,*/
/*                   it will return FALSE.                           */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: None.                                                    */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System CheckListIntegrity(DLIST ListToCheck)
{

  /* Since ListToProcess is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using ListToProcess.  This way we just do the
     cast once.                                                            */
  ControlNode *      ListData;


  LinkNode *         CurrentNode;         /* Used to point to the LinkNode of the
                                             current item while we access its data.
                                             This limits the levels of indirection
                                             to one, which should result in faster
                                             execution. */
  LinkNode *         PreviousNode;        /* Used to point to the LinkNode immediately
                                             prior to the CurrentNode.                 */
  CARDINAL32         ItemCounter;         /* Used to count the number of items in a list. */
  BOOLEAN            EndFound;            /* Used to track whether or not the item listed
                                             in the EndOfList field of the control record
                                             was found when the list was traversed.         */
  BOOLEAN            CurrentFound;        /* Used to track whether or not the item listed
                                             in the CurrentItem field of the control record
                                             was found when the list was traversed.         */


  /* We will assume that ListToCheck points to a valid list.  Given this,
     we will initialize ListData to point to the ControlNode of this
     list.                                                                     */
  ListData = (ControlNode *) ListToCheck;


  /* We must now validate the list before we attempt to use it.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((ListData == NULL) || (ListData->Verify != VerifyValue))
  {

    ErrorsFound = TRUE;
    return FALSE;

  }

  /* Begin Checking. */

  if ( ListData->ItemCount == 0 )
  {

    /* If the list is empty, then all of the pointers to link nodes must be NULL. */
    if ( ( ListData->StartOfList != NULL ) ||
         ( ListData->EndOfList != NULL )   ||
         ( ListData->CurrentItem != NULL ) )
    {

      ErrorsFound = TRUE;
      return FALSE;

    }

  }
  else
  {
    /* The list is not empty!  Does it have more than 1 element in it? */
    if ( ListData->ItemCount == 1 )
    {

      /* This is a special case.  In this case, all of the link node pointers in the control
         record should point to the same link node.                                           */
      if ( ( ListData->StartOfList != ListData->EndOfList ) ||
           ( ListData->EndOfList != ListData->CurrentItem ) )
      {
        ErrorsFound = TRUE;
        return FALSE;
      }

    }
    else
    {
      /* Lets traverse the list and count how many items are in it! */
      ItemCounter = 0;

      /* Get the first item in the list. */
      CurrentNode = ListData->StartOfList;

      /* Initialize variables prior to the list traversal. */
      PreviousNode = NULL;
      CurrentFound = FALSE;
      EndFound = FALSE;

      while ( CurrentNode != NULL )
      {

        /* Does this link node claim to be a part of this list? */
        if ( CurrentNode->ControlNodeLocation != ListData )
        {

          /* This item does not claim to be a part of this list! */
          ErrorsFound = TRUE;
          return FALSE;

        }

        /* Is the PreviousLinkNode field correct? */
        if ( CurrentNode->PreviousLinkNode != PreviousNode )
        {

          ErrorsFound = TRUE;
          return FALSE;

        }

        /* Does the previous item point to the current item? */
        if ( ( PreviousNode != NULL ) &&
             ( PreviousNode->NextLinkNode != CurrentNode ) )
        {

          ErrorsFound = TRUE;
          return FALSE;

        }

        /* Is CurrentNode equal to CurrentItem? */
        if ( CurrentNode == ListData->CurrentItem )
        {

          CurrentFound = TRUE;

        }

        /* Is CurrentNode equal to EndOfList? */
        if ( CurrentNode == ListData->EndOfList )
        {

          EndFound = TRUE;

          /* Since this is supposed to be the last item in the list, is the NextLinkNode field NULL? */
          if ( CurrentNode->NextLinkNode != NULL )
          {

            ErrorsFound = TRUE;
            return FALSE;

          }

        }

        /* Count the current item being checked. */
        ItemCounter++;

        /* Advance to the next item. */
        PreviousNode = CurrentNode;
        CurrentNode = CurrentNode->NextLinkNode;

      }

      /* Were both the CurrentItem and EndOfList items found in the list? */
      if ( ( !EndFound ) || ( !CurrentFound ) )
      {

        ErrorsFound = TRUE;
        return FALSE;

      }

      /* Was the proper number of items found in the list? */
      if ( ItemCounter != ListData->ItemCount )
      {

        ErrorsFound = TRUE;
        return FALSE;

      }

    }

  }

  return TRUE;
}


/*********************************************************************/
/*                                                                   */
/*   Function Name:  TransferItem                                    */
/*                                                                   */
/*   Descriptive Name: Removes an item in SourceList and places in   */
/*                     TargetList.                                   */
/*                                                                   */
/*   Input:  DLIST SourceList : The DLIST containing the item which  */
/*                              is to be transferred.                */
/*           ADDRESS SourceHandle : The handle of the item in        */
/*                                   SourceList which is to be       */
/*                                   transferred to another DLIST.   */
/*                                   If this is NULL, then the       */
/*                                   current item in SourceList will */
/*                                   be used.                        */
/*           DLIST TargetList : The DLIST which is to receive the    */
/*                              item being transferred.              */
/*           ADDRESS TargetHandle : The item in TargetList which     */
/*                                   is used to determine where      */
/*                                   the item being transferred will */
/*                                   be placed.  If this is NULL,    */
/*                                   then the current item in        */
/*                                   TargetList will be used.        */
/*           Insertion_Modes TransferMode : This indicates where,    */
/*                                   relative to the item in         */
/*                                   TargetList specified by         */
/*                                   Target_Handle, the item being   */
/*                                   transferred can be placed.      */
/*          BOOLEAN MakeCurrent : If TRUE, the item transferred to   */
/*                                 TargetList becomes the current    */
/*                                 item in TargetList.               */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return value.             */
/*                                                                   */
/*   Output: If successful, *Error will be set to DLIST_SUCCESS,     */
/*              SourceList will be empty, and TargetList will contain*/
/*              all of its original items and all of the items that  */
/*              were in SourceList.                                  */
/*           If unsuccessful, *Error will be set to a non-zero value */
/*              and SourceList and TargetList will be unmodified.    */
/*                                                                   */
/*   Error Handling:  This function will abort immediately upon      */
/*                    detection of an error.  All errors that can be */
/*                    detected are detected before the contents of   */
/*                    SourceList are appended to TargetList, so if an*/
/*                    error is detected and the function aborts,     */
/*                    SourceList and TargetList are unaltered.       */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: None.                                                    */
/*                                                                   */
/*********************************************************************/
void _System TransferItem(DLIST             SourceList,
                          ADDRESS           SourceHandle,
                          DLIST             TargetList,
                          ADDRESS           TargetHandle,
                          Insertion_Modes   TransferMode,
                          BOOLEAN           MakeCurrent,
                          CARDINAL32 *      Error)
{

  /* Since TargetList is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using TargetList.  This way we just do the
     cast once.                                                            */
  ControlNode *      TargetListData;

  /* Since SourceList is of type DLIST, we can not use it without
     having to type cast it each time.  To avoid all of the type casting,
     we will declare a local variable of type ControlNode * and then
     initialize it once using SourceList.  This way we just do the
     cast once.                                                            */
  ControlNode *      SourceListData;

  LinkNode *         TargetLinkNode; /* Used to point to the LinkNode of the
                                        item in TargetList which is to be used
                                        as the reference for inserting the item
                                        being removed from SourceList.             */
  LinkNode *         SourceLinkNode;  /* Used to point to the LinkNode of
                                         the item in SourceList which
                                         is being removed from SourceList and
                                         inserted into TargetList.                 */
  LinkNode *         PreviousNode;    /* Used when removing or inserting an item in a list. */
  LinkNode *         NextNode;        /* Used when removing or inserting an item in a list. */


  /* We will assume that TargetList and SourceList both point to a valid lists.
     Given this, we will initialize TargetListData and SourceListData to point
     to the ControlNode of TargetList and SourceList, respectively.            */
  TargetListData = (ControlNode *) TargetList;
  SourceListData = (ControlNode *) SourceList;


#ifdef DEBUG

#ifdef PARANOID

  if ( !CheckListIntegrity(TargetList) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

  if ( !CheckListIntegrity(SourceList) )
  {
    *Error = DLIST_CORRUPTED;
    return;
  }

#else

  /* We must now validate the lists before we attempt to use them.  We will
     do this by checking the Verify field in the ControlNode.               */
  if ((TargetListData == NULL) || (TargetListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

  if ((SourceListData == NULL) || (SourceListData->Verify != VerifyValue))
  {
    *Error = DLIST_NOT_INITIALIZED;
    return;
  }

#endif

#endif

  /* We must check the transfer mode. */
  if ( TransferMode > AppendToList )
  {

    *Error = DLIST_INVALID_INSERTION_MODE;
    return;

  }

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Is the source list empty?  If so, we have nothing to do! */
  if (SourceListData->ItemCount == 0)
  {

    /* The source list is empty!  We are done! */
    return;

  }

  /* Were we given a source handle? */
  if ( SourceHandle != NULL )
  {

    /* Lets check the handle we were given.*/
    SourceLinkNode = (LinkNode*) SourceHandle;

    /* Is the handle valid? */
    if ( SourceLinkNode->ControlNodeLocation != SourceListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return;

    }

  }
  else
  {

    /* Since we were not given a source handle, we will use the current item in SourceList. */
    SourceLinkNode = SourceListData->CurrentItem;

  }

  /* Were we given a target handle? */
  if ( TargetHandle != NULL )
  {

    /* Lets check the handle we were given.*/
    TargetLinkNode = (LinkNode*) TargetHandle;

    /* Is the handle valid? */
    if ( TargetLinkNode->ControlNodeLocation != TargetListData )
    {

      /* The handle is not valid!  Abort! */
      *Error = DLIST_BAD_HANDLE;
      return;

    }

  }
  else
  {

    /* Since we were not given a target handle, we will use the current item in TargetList. */
    TargetLinkNode = TargetListData->CurrentItem;

  }

  /* Remove SourceLinkNode from the SourceList. */
  PreviousNode = SourceLinkNode->PreviousLinkNode;
  NextNode = SourceLinkNode->NextLinkNode;
  if ( PreviousNode != NULL )
    PreviousNode->NextLinkNode = NextNode;

  if ( NextNode != NULL )
    NextNode->PreviousLinkNode = PreviousNode;

  /* Update SourceList's control data. */
  SourceListData->ItemCount -= 1;

  if ( SourceListData->StartOfList == SourceLinkNode )
    SourceListData->StartOfList = NextNode;

  if ( SourceListData->EndOfList == SourceLinkNode )
    SourceListData->EndOfList == PreviousNode;

  if ( SourceListData->CurrentItem == SourceLinkNode )
  {

    if ( NextNode == NULL )
      SourceListData->CurrentItem = PreviousNode;
    else
      SourceListData->CurrentItem = NextNode;

  }

  /* Is the target list empty?  If so, then the TransferMode does not matter! */
  if (TargetListData->CurrentItem == NULL)
  {

    /* The List is empty.  This will be the first (and only) item in the list.
       Also, since this will be the only item in the list, it automatically
       becomes the current item.                                               */
    TargetListData->EndOfList = SourceLinkNode;
    TargetListData->StartOfList = SourceLinkNode;
    TargetListData->CurrentItem = SourceLinkNode;
    SourceLinkNode->PreviousLinkNode = NULL;
    SourceLinkNode->NextLinkNode = NULL;

  }
  else
  {
    /* The target list was not empty.  */

    /* Now lets insert the item according to the specified TransferMode. */
    switch ( TransferMode )
    {

      case InsertAtStart: /* Get the first item in the list. */
                          TargetLinkNode = TargetListData->StartOfList;

                          /* Now insert SourceLinkNode before TargetLinkNode. */
                          SourceLinkNode->NextLinkNode = TargetLinkNode;
                          SourceLinkNode->PreviousLinkNode = NULL;
                          TargetLinkNode->PreviousLinkNode = SourceLinkNode;

                          /* Now update the ControlNode. */
                          TargetListData->StartOfList = SourceLinkNode;

                          break;
      case InsertBefore:  /* TargetLinkNode already points to the Item we are to insert SourceLinkNode before. */

                          /* Is TargetLinkNode the first item in the list? */
                          if ( TargetListData->StartOfList != TargetLinkNode )
                          {

                            /* Since TargetLinkNode is not the first item in the list, we need the node prior to TargetLinkNode
                               so we can adjust its link fields.                                                                 */
                            PreviousNode = TargetLinkNode->PreviousLinkNode;

                            /* Now make PreviousLinkNode point to SourceLinkNode and vice versa. */
                            PreviousNode->NextLinkNode = SourceLinkNode;
                            SourceLinkNode->PreviousLinkNode = PreviousNode;

                          }
                          else
                          {

                            /* Since TargetLinkNode is the first item in the list, that means that SourceLinkNode will be
                               the first item in the list after it is inserted.  Update the ControlNode for this
                               list to reflect that SourceLinkNode will be the first item in the list.                     */
                            TargetListData->StartOfList = SourceLinkNode;

                            /* Since SourceLinkNode will be the first item in the list, make sure that its PreviousLinkNode
                               field is NULL since we did not do this earlier when we removed it from SourceList.             */
                            SourceLinkNode->PreviousLinkNode = NULL;

                          }

                          /* Now make SourceLinkNode point to TargetLinkNode and vice versa. */
                          SourceLinkNode->NextLinkNode = TargetLinkNode;
                          TargetLinkNode->PreviousLinkNode = SourceLinkNode;

                          break;
      case InsertAfter:   /* TargetLinkNode already points to the Item we are to insert SourceLinkNode after. */

                          /* Is TargetLinkNode the last item in the list? */
                          if ( TargetListData->EndOfList != TargetLinkNode )
                          {

                            /* Since TargetLinkNode is not the last item in the list, we need the node after to TargetLinkNode
                               so we can adjust its link fields.                                                                */
                            NextNode = TargetLinkNode->NextLinkNode;

                            /* Now make NextLinkNode point to SourceLinkNode and vice versa. */
                            NextNode->PreviousLinkNode = SourceLinkNode;
                            SourceLinkNode->NextLinkNode = NextNode;

                          }
                          else
                          {

                            /* Since TargetLinkNode is the last item in the list, that means that SourceLinkNode will
                               be the last item in the list after it is inserted.  Update the ControlNode for this
                               list to reflect that SourceLinkNode will be the last item in the list.                     */
                            TargetListData->EndOfList = SourceLinkNode;

                            /* Since SourceLinkNode will be the last item in the list, make sure that its NextLinkNode
                               field is NULL since we did not do this earlier when we removed it from SourceList.             */
                            SourceLinkNode->NextLinkNode = NULL;


                          }

                          /* Now make SourceLinkNode point to TargetLinkNode and vice versa. */
                          TargetLinkNode->NextLinkNode = SourceLinkNode;
                          SourceLinkNode->PreviousLinkNode = TargetLinkNode;

                          break;
      case AppendToList:  /* Get the last item in the list. */
                          TargetLinkNode = TargetListData->EndOfList;

                          /* Now insert SourceLinkNode after TargetNode. */
                          TargetLinkNode->NextLinkNode = SourceLinkNode;
                          SourceLinkNode->PreviousLinkNode = TargetLinkNode;
                          SourceLinkNode->NextLinkNode = NULL;

                          /* Now update the ControlNode. */
                          TargetListData->EndOfList = SourceLinkNode;

                          break;
      default :
                *Error = DLIST_INVALID_INSERTION_MODE;
                return;

    }

  }

  /* Adjust the count of the number of items in the list. */
  TargetListData->ItemCount++;

  /* Adjust the ControlNodeLocation of SourceLinkNode so that it thinks it is now a member of TargetList. */
  SourceLinkNode->ControlNodeLocation = TargetListData;

  /* Should the transferred item become the current item in TargetList? */
  if ( MakeCurrent )
  {

    /* Adjust the control node so that SourceLinkNode becomes the current item in the list. */
    TargetListData->CurrentItem = SourceLinkNode;

  }


#ifdef PARANOID

  assert (CheckListIntegrity( SourceList ) );
  assert (CheckListIntegrity( TargetList ) );

#endif


  /* All done! */
  return;

}



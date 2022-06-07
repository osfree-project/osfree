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
* Module: lvm_list.h
*/

/*
* Change History:
*
*/

/*
* Functions: DLIST       CreateList
*            void        InsertItem
*            void        InsertObject
*            void        GetItem
*            void        GetNextItem
*            void        GetPreviousItem
*            ADDRESS     GetObject
*            ADDRESS     GetNextObject
*            ADDRESS     GetPreviousObject
*            void        ExtractItem
*            ADDRESS     ExtractObject
*            void        ReplaceItem
*            ADDRESS     ReplaceObject
*            void        DeleteItem
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
*            void        TransferItem
*
* Description:  This module implements a simple, generic, doubly linked list.
*               Data objects of any type can be placed into a linked list
*               created by this module.  Futhermore, data objects of different
*               types may be placed into the same linked list.
*
* Notes:  This linked list implementation makes use of the concept of the
*         current item.  In any non-empty list, one item in the list will
*         be designated as the current item.  When any of the following
*         functions are called, they will operate upon the current item
*         only: GetItem, ReplaceItem, DeleteItem, GetTag, NextItem,
*         PreviousItem, GetObject, ExtractItem, and ExtractObject.  The
*         user of this module may set the current item through the use of
*         the GoToStartOfList, GoToEndOfList, NextItem, PreviousItem,
*         and GoToSpecifiedItem functions.
*
*         Since a linked list created by this module may contain items
*         of different types, the user will need a way to identify items
*         of different types which may be in the same list.  To allow users
*         to do this, the concept of an item tag is used.  When an item is
*         added to the list, the user must enter an item tag.  The item
*         tag is merely some identifier that the user wishes to associate
*         with the item being placed into the list.  When used as intended,
*         each type of data item will have a unique tag associated with it.
*         This way, all data items of the same type will have the same tag
*         while data items of different types will have different tags.
*         Thus, by using the GetTag function, the user can get the item
*         tag for the current item without having to get the item from the
*         list.  This allows the user to differentiate between items of
*         different types which reside in the same list.
*
*         This module is single threaded.  If used in a multi-threaded
*         environment, the user must implement appropriate access controls.
*
*         When an item is inserted or appended to a list, this module
*         allocates memory on the heap to hold the item and then copies
*         the item to the memory that it allocated.  This allows local
*         variables to be safely inserted or appended to a list.  However,
*         it should be noted that under certain circumstances a copy of the
*         entire data item will NOT be made.  Specifically, if the data item
*         is a structure or array containing pointers, then the data pointed
*         to by the pointers will NOT be copied even though the structure or
*         array is!  This results from the fact that, when an item is being
*         inserted or appended to a list, the user provides just an address
*         and size.  This module assumes that the item to inserted or append
*         lies in a contiguous block of memory at the address provided by the
*         user.  This module has no way of knowing the structure of the data
*         at the specified address, and therefore can not know about any
*         embedded pointers which may lie within that block of memory.
*
*         This module now employs the concept of a handle.  A handle is a
*         reference to a specific item in a list which allows that item to
*         be made the current item in the list quickly.  Example:  If you
*         use the GetHandle function to get a handle for the current item
*         (lets call the item B1), then, regardless of where you are in the
*         list (or any reodering of the items in the list), you can make item
*         B1 the current item by passing its handle to the GoToSpecifiedItem
*         function.  Alternatively, you could operate directly on B1 using
*         the other handle based functions, such as GetItem_By_Handle, for
*         example.  GetItem_By_Handle gets the item associated with the
*         specified handle without changing which item in the list is the
*         current item in the list.
*
*         The functions of this module refer to user data as either items or
*         objects.  The difference between the two is simple, yet subtle.  It
*         deals with who is responsible for the memory used to hold the data.
*         In the case of an item, this module is responsible for the memory
*         used to hold the user data.  In the case of an object, the user
*         is responsible for the memory used to hold the data.
*
*         What this means is that, for functions adding ITEMS to a list,
*         this module will be responsible for allocating memory to hold
*         the user data and then copying the user data into the memory
*         that was allocated.  For functions which return items, this
*         module will COPY the user data from the LIST into a buffer
*         specified by the user.  For functions which add objects to a
*         list, the user provides a pointer to a block of memory holding
*         user data.  This block of memory was allocated by the user, and
*         becomes the "property" of this module once it has been added to
*         a LIST.  For functions which return objects, a pointer to the
*         memory where the data is stored is returned.  As long as an item/object
*         is in a LIST, this module will be responsible for the memory that
*         is used to store the data associated with that item.  This means that
*         users of this module should not call free on an object returned by this
*         module as long as that object is still within a list.
*
*
*/

#ifndef LVM_DLISTHANDLER

#define LVM_DLISTHANDLER  1

/*--------------------------------------------------
* Type definitions
--------------------------------------------------*/

#include "LVM_GBLS.h"

typedef unsigned long TAG;

typedef ADDRESS DLIST;

/*--------------------------------------------------
* Type definitions
--------------------------------------------------*/

typedef enum _Insertion_Modes {
InsertAtStart,
InsertBefore,
InsertAfter,
AppendToList
} Insertion_Modes;

/************************************************
*           Functions Available                *
************************************************/

/*
* The parameter *Error is set by every function in this module.  It
* will be set to 0 to indicate success, and will be > 0 if an
* error occurs.  The following table lists the possible error codes:
*     0 : No error.
*     1 : Out of memory
*     2 : Memory has been corrupted!
*     3 : Bad List Record!
*     4 : List Record not initialized yet!
*     5 : List is empty!
*     6 : Item size mismatch!
*     7 : Bad item pointer!
*     8 : Item has zero size!
*     9 : Item tag mismatch!
*    10 : Already at end of list!
*    11 : Already at start of list!
*    12 : Bad Handle!
*    13 : Invalid Insertion Mode!
*/

#define DLIST_SUCCESS                    0
#define DLIST_OUT_OF_MEMORY              1
#define DLIST_CORRUPTED                  2
#define DLIST_BAD                        3
#define DLIST_NOT_INITIALIZED            4
#define DLIST_EMPTY                      5
#define DLIST_ITEM_SIZE_WRONG            6
#define DLIST_BAD_ITEM_POINTER           7
#define DLIST_ITEM_SIZE_ZERO             8
#define DLIST_ITEM_TAG_WRONG             9
#define DLIST_END_OF_LIST               10
#define DLIST_ALREADY_AT_START          11
#define DLIST_BAD_HANDLE                12
#define DLIST_INVALID_INSERTION_MODE    13

/* The following code is special.  It is for use with the PruneList and ForEachItem functions.  Basically, these functions
can be thought of as "searching" a list.  They present each item in the list to a user supplied function which can then
operate on the items.  If the user supplied function returns a non-zero error code, ForEachItem and PruneList abort and
return an error to the caller.  This may be undesirable.  If the user supplied function used with PruneList and ForEachItem
returns the code below, PruneList/ForEachItem will abort and return DLIST_SUCCESS.  This allows PruneList and ForEachItem
to be used to search a list and terminate the search when the desired item is found without having to traverse the
remaining items in the list.                                                                                                  */
#define DLIST_SEARCH_COMPLETE  0xFF

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
CARDINAL32 PoolIncrement);

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
DLIST _System CreateList( void );

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
CARDINAL32 *    Error);

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
CARDINAL32 *    Error);

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
/*           NOTE: For this function, NULL is considered a valid     */
/*                 handle which refers to the current item in        */
/*                 ListToDeleteFrom.                                 */
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
void _System DeleteItem (DLIST        ListToDeleteFrom,
BOOLEAN      FreeMemory,
ADDRESS      Handle,
CARDINAL32 * Error);

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
CARDINAL32 * Error);

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
CARDINAL32 *   Error);

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
/*                                 the current item is.              */
/*           ADDRESS     ItemLocation : This is the location of the  */
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
void _System GetNextItem( DLIST          ListToGetItemFrom,
CARDINAL32     ItemSize,
ADDRESS        ItemLocation,
TAG            ItemTag,
CARDINAL32 *   Error);

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
CARDINAL32 *   Error);

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
CARDINAL32 *   Error);

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
ADDRESS _System GetNextObject( DLIST          ListToGetItemFrom,
CARDINAL32     ItemSize,
TAG            ItemTag,
CARDINAL32 *   Error);

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
CARDINAL32 *   Error);

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
CARDINAL32 *   Error);

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
CARDINAL32 *   Error);

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
CARDINAL32 *  Error);

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
CARDINAL32 *  Error);

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
CARDINAL32 * Error);

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
/*           The handle returned is a pointer to an internal         */
/*           structure within the list.  If the item associated      */
/*           with this handle is removed from the list, the handle   */
/*           will be invalid and should not be used as the internal  */
/*           structure it points to will nolonger exist!             */
/*                                                                   */
/*********************************************************************/
ADDRESS _System GetHandle ( DLIST ListToGetHandleFrom, CARDINAL32 * Error);

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
CARDINAL32 _System GetListSize( DLIST ListToGetSizeOf, CARDINAL32 * Error);

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
BOOLEAN _System ListEmpty( DLIST ListToCheck, CARDINAL32 * Error);

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
BOOLEAN _System AtEndOfList( DLIST ListToCheck, CARDINAL32 * Error);

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
BOOLEAN _System AtStartOfList( DLIST ListToCheck, CARDINAL32 * Error);

/*********************************************************************/
/*                                                                   */
/*   Function Name:  DestroyList                                     */
/*                                                                   */
/*   Descriptive Name:  This function releases the memory associated */
/*                      with the internal data structures of a DLIST.*/
/*                      Once a DLIST has been eliminated by this     */
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
void _System DestroyList( DLIST *  ListToDestroy, BOOLEAN FreeItemMemory, CARDINAL32 * Error);

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
void _System NextItem( DLIST  ListToAdvance, CARDINAL32 * Error);

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
void _System PreviousItem( DLIST  ListToChange, CARDINAL32 * Error);

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
/*   Error Handling: This function will fail if ListToReset is not   */
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
void _System GoToStartOfList( DLIST ListToReset, CARDINAL32 * Error);

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
void _System GoToEndOfList( DLIST ListToSet, CARDINAL32 * Error);

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
void _System GoToSpecifiedItem( DLIST ListToReposition, ADDRESS Handle, CARDINAL32 * Error);

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
/*********************************************************************/
void _System SortList(DLIST        ListToSort,
INTEGER32    (* _System Compare) (ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag,CARDINAL32 * Error),
CARDINAL32 * Error);

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
/*           BOOLEAN Forward : If TRUE, then the list is traversed   */
/*                             from the start of the list to the end */
/*                             of the list.  If FALSE, then the list */
/*                             is traversed from the end of the list */
/*                             to the beginning.                     */
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
CARDINAL32 * Error);

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
CARDINAL32 * Error);

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
void _System AppendList(DLIST        TargetList,
DLIST        SourceList,
CARDINAL32 * Error);

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
CARDINAL32 *      Error);

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
BOOLEAN _System CheckListIntegrity(DLIST ListToCheck);

#endif


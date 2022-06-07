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
 * Module: Dlist.h
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
 *****************************************************************************/

#ifndef DLISTHANDLER

#define DLISTHANDLER  1

#include "LVM_LIST.H"

#endif



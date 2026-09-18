/*!
   $Id: all_dlist_private.h,v 1.3 2026/01/01 00:00:00 osfree Exp $

   @file all_dlist_private.h

   @brief Private declaration of the all_dlist interface.

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Cristiano Guadagnino <criguada@tin.it>

   This header mirrors the public all_dlist.h that is part of the
   fixed osFree interface. The public header includes all_shared.h,
   which in turn pulls in <os2.h> and cannot be used on host builds.
   This private copy repeats the declarations with the same layout
   and signatures, so that all_dlist.c can be compiled in the host
   tools without the OS/2 SDK headers.

   The node layout is shared with the IBM Dlist module. The struct
   tag and the first three fields are defined by linked_list_private.h
   and are aliased here as stack_cmp.

   Any change to the layout or to the signatures here must be
   matched by the same change in all_dlist.h. The two files describe
   the same binary interface.
*/

#ifndef _ALL_DLIST_PRIVATE_H_
#define _ALL_DLIST_PRIVATE_H_

#include "os2types.h"
#include "os2err.h"
#include "linked_list_private.h"

/* --------------------------------------------------------------------------
   Return codes
*/

/** @brief Handle points to NULL (empty stack or list). */
#define all_RC_DLIST_NULL   1L
/** @brief At either end of the list. */
#define all_RC_DLIST_END    2L

/* --------------------------------------------------------------------------
   Data types
*/

/*!
    @brief Stack component of the dynamic stack.

    This is the stack component definition for the dynamic stack. It
    is a generic stack component that can hold any type of info. Note
    that the allocation/deallocation of the stack component is
    managed by the functions implemented here, but the allocation and
    deallocation of the elements placed on the stack is the
    responsibility of the user code.

    The public all_dlist.h declares this as a three-field struct.
    The private header shares one struct definition with the Dlist
    module; the extra size and tag fields are ignored by all_dlist.
*/
typedef LINKED_NODE stack_cmp;

typedef stack_cmp *hStack;  /*!< A stack handle                     */
typedef hStack    *phStack; /*!< A pointer to a stack handle        */
typedef hStack     hList;   /*!< A list handle                      */
typedef phStack    phList;  /*!< A pointer to a list handle         */

/* --------------------------------------------------------------------------
   Exported functions
*/

/* -- Stack -- */
phStack       stack_init(void);                   /*!< Initializes pointers for the stack                        */
unsigned long stack_push(phStack, void *);        /*!< Pushes new content on the stack                           */
unsigned long stack_pop(phStack, void **);        /*!< Pops content from the stack, deallocates top element      */
void         *stack_top(phStack);                 /*!< Returns contents of the top of the stack (no dealloc)     */
unsigned long stack_remove(phStack, void *);      /*!< Removes an arbitrary component from the stack             */

/* -- List -- */
phList        list_init(void);                    /*!< Initializes pointers for the list                         */
void         *list_get(phList);                   /*!< Returns contents of the current list element              */
unsigned long list_insert(phList, void *);        /*!< Inserts new list element after the current position       */
unsigned long list_remove(phList);                /*!< Removes current element from the list                     */
unsigned long list_add(phList, void *);           /*!< Adds new element to the end of the list                   */
unsigned long list_set_start(phList);             /*!< Moves current location to the start of the list           */
unsigned long list_set_end(phList);               /*!< Moves current location to the end of the list             */
unsigned long list_prev(phList);                  /*!< Moves current location to the previous list element       */
unsigned long list_next(phList);                  /*!< Moves current location to the next list element           */
unsigned long list_free(phList);                  /*!< Completely deallocates list (and contents)                */
BOOL          list_isempty(phList);               /*!< Tests if the list is empty (no elements)                  */

#endif /* _ALL_DLIST_PRIVATE_H_ */

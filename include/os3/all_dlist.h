/*! 
   $Id: all_dlist.h,v 1.1.1.1 2003/10/04 08:27:16 prokushev Exp $ 
   
   @file all_dlist.h

    @brief Utility functions related to dynamic lists and stacks

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Cristiano Guadagnino <criguada@tin.it>
*/

#ifndef _ALL_DLIST_H_
#define _ALL_DLIST_H_

#define INCL_DOSERRORS
#define INCL_DOSMEMMGR
#include <osfree.h>

/* C standard library headers */
#include <stdlib.h>

// --------------------------------------------------------------------------
// Return codes

#define all_RC_DLIST_NULL   1L  //!< Handle points to NULL (empty stack/list)
#define all_RC_DLIST_END    2L  //!< At either end of the list


// --------------------------------------------------------------------------
// Data types

/*!
    @brief Stack component of the dynamic stack

    This is the stack component definition for the dynamic stack. It is a
    generic stack component, that can hold any type of info. Note that
    the allocation/deallocation of stack component is managed by the
    functions implemented here, but the allocation/deallocation of the
    elements you put on the stack is to be managed by the user code.
*/
typedef struct _stack_component {
    void *elemt;                    //!< A pointer to a generic element
    struct _stack_component *next;  //!< Pointer to the next stack component
    struct _stack_component *prev;  //!< Pointer to the prev stack component
} stack_cmp;

typedef stack_cmp *hStack;  //!< A stack handle
typedef hStack *phStack;    //!< A pointer to a stack handle
typedef hStack hList;       //!< A list handle
typedef phStack phList;     //!< A pointer to a list handle


// --------------------------------------------------------------------------
// Exported functions

// -- Stack --
phStack stack_init(void);                   //!< Initializes pointers for the stack
unsigned long stack_push(phStack, void *);  //!< Pushes new content on the stack
unsigned long stack_pop(phStack, void **);  //!< Pops content from the stack, dealloc top stack element
void *stack_top(phStack);                   //!< Returns contents of the top of the stack (no dealloc)
unsigned long stack_remove(phStack, void *);//!< Removes an arbitrary component from the stack

// -- List --
phList list_init(void);                     //!< Initializes pointers for the list
void *list_get(phList);                     //!< Returns contents of current list element
unsigned long list_insert(phList, void *);  //!< Insert new list element after current position
unsigned long list_remove(phList);          //!< Removes current element from the list
unsigned long list_add(phList, void*);      //!< Adds new element to the end of the list
unsigned long list_set_start(phList);       //!< Moves current location to the start of the list
unsigned long list_set_end(phList);         //!< Moves current location to the end of the list
unsigned long list_prev(phList);            //!< Moves current location to the previous list element
unsigned long list_next(phList);            //!< Moves current location to the next list element
unsigned long list_free(phList);            //!< Completely deallocates list (and contents)
BOOL list_isempty(phList);                  //!< Tests if the list is empty (no elements)

#endif /* _ALL_DLIST_H_ */

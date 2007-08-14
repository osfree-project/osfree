/*!
   $Id: all_dlist.c,v 1.1.1.1 2003/10/04 08:36:16 prokushev Exp $ 

   @file all_dlist.c

   @brief Utility functions related to dynamic lists and stacks

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Cristiano Guadagnino <criguada@tin.it>
*/

#define INCL_DOSERRORS
#include <osfree.h>
#include <all_shared.h>

/* --------------------------------------------------------------------------
    Implementation of a dynamic stack structure
*/

/*!
    Initialization of the dynamic stack. It allocates a new stack handle
    (hStack) and returns it to the caller.

    @return A pointer to a stack handle
*/
phStack stack_init(void)
{
    hStack  *pphsStack;

    // Allocate memory for a new stack handle
    pphsStack = (hStack *)malloc(sizeof(hStack));
    if(pphsStack == NULL) return(NULL);

    // Since there is no stack element yet, initialize to NULL
    *pphsStack = NULL;

    return(pphsStack);
}

/*!
    Push a new element on the stack. The element can be of any kind
    (variable, structure, etc). Allocation of the memory for the
    element is done by the user code.

    @param pphsStack    A pointer to a stack handle
    @param content      A pointer to the new element

    @return
        - ERROR_NOT_ENOUGH_MEMORY if memory allocation failed
        - NO_ERROR if there are no errors
*/
unsigned long stack_push(hStack *pphsStack, void *content)
{
    stack_cmp  *pPrev;

    /* save pointer to previous component */
    pPrev = *pphsStack;

    /* alloc memory for new component */
    *pphsStack = malloc(sizeof(stack_cmp));
    if(*pphsStack == NULL) return(ERROR_NOT_ENOUGH_MEMORY);

    /* update the pointers */
    (*pphsStack)->elemt = content;
    (*pphsStack)->next = NULL;
    (*pphsStack)->prev = pPrev;

    /* update the 'next' pointer of the previous component */
    /* if it exists                                        */
    if(pPrev != NULL) pPrev->next = *pphsStack;

    return(NO_ERROR);
}

/*!
    Pop the top element of the stack. When the user is done with
    the returned element, he is responsible for deallocating its
    memory.

    @param pphsStack    A pointer to a stack handle
    @param content      A pointer to a pointer to the element

    @return
        - all_RC_DLIST_NULL if the stack is empty
        - NO_ERROR if there are no errors
*/
unsigned long stack_pop(hStack *pphsStack, void **content)
{
    stack_cmp *pPrev;

    /* don't "pop" if the stack is empty */
    if(*pphsStack == NULL) {
        *content = NULL;
        return(all_RC_DLIST_NULL);
    }

    /* save the address of the previous component */
    pPrev = (*pphsStack)->prev;

    /* update the 'next' pointer of the previous component */
    /* if it exists                                        */
    if(pPrev != NULL) pPrev->next = NULL;

    /* save the pointer to the contents */
    *content = (*pphsStack)->elemt;

    /* release memory for the current component */
    free(*pphsStack);

    /* updates the stack pointer */
    *pphsStack = pPrev;

    return(NO_ERROR);
}

/*!
    Returns the top element of the stack, without removing it.

    @param pphsStack    A pointer to a stack handle

    @return
        - NULL if the stack is empty
        - A pointer to the top element if there are no errors
*/
void *stack_top(hStack *pphsStack)
{
    if(*pphsStack == NULL) return(NULL);

    return((*pphsStack)->elemt);
}

/*!
    Removes an element in the middle of the stack; does not affect
    the stack pointer.

    @param pphsStack    A pointer to a stack handle
    @param component    A pointer to the stack element to remove

    @return
        - all_RC_DLIST_NULL if the pointer is NULL
        - NO_ERROR if there are no errors
*/
unsigned long stack_remove(hStack *pphsStack, void *component)
{
    stack_cmp *pPrev, *pNext;

    if(component == NULL) return(all_RC_DLIST_NULL);

    /* saves the pointers to the previous and next components */
    pPrev = ((stack_cmp *)component)->prev;
    pNext = ((stack_cmp *)component)->next;

    /* update the previous and next components to point to the */
    /* correct neighbours                                      */
    pPrev->next = pNext;
    pNext->prev = pPrev;

    /* removes the current component */
    free((stack_cmp *)component);

    return(NO_ERROR);
}


/* --------------------------------------------------------------------------
    Implementation of a dynamic list structure
*/

/*!
    Initialization of the dynamic list. It allocates a new list handle
    (hList) and returns it to the caller.

    @return A pointer to a list handle
*/

phList list_init(void)
{
    phList pphlList;

    // Allocate memory for a new list handle
    pphlList = (hList *)malloc(sizeof(hList));
    if(pphlList == NULL) return(NULL);

    // Since there is no stack element yet, initialize to NULL
    *pphlList = NULL;

    return(pphlList);
}

/*!
    Returns the current element of the list.

    @param pphlList     A pointer to a list handle

    @return
        - NULL if the list is empty
        - A pointer to the current list element if there are no errors
*/
void *list_get(phList pphlList)
{
    /* if the list is empty return NULL */
    if(*pphlList == NULL) return(NULL);

    /* return a pointer to the contents */
    return((*pphlList)->elemt);
}

/*!
    Inserts a new list element after the current element.
    Allocation of the memory for the element is done by the user code.

    @param pphlList     A pointer to a list handle
    @param content      A pointer to the new element

    @return
        - ERROR_NOT_ENOUGH_MEMORY if memory allocation failed
        - NO_ERROR if there are no errors
*/
unsigned long list_insert(phList pphlList, void *content)
{
    hList   pPrev;

    /* save pointer to previous component */
    pPrev = *pphlList;

    /* alloc memory for new component */
    *pphlList = malloc(sizeof(stack_cmp));
    if(*pphlList == NULL) return(ERROR_NOT_ENOUGH_MEMORY);

    /* update the pointers */
    (*pphlList)->elemt = content;
    (*pphlList)->next = NULL;
    (*pphlList)->prev = pPrev;

    /* update the 'next' pointer of the previous component */
    /* if it exists                                        */
    if(pPrev != NULL) pPrev->next = *pphlList;

    return(NO_ERROR);
}

/*!
    Removes the current element from the list. Does \a not
    deallocate memory for the element. User is responsible
    for doing it.

    @param pphlList     A pointer to a list handle

    @return
        - all_RC_DLIST_NULL if the list is empty
        - NO_ERROR if there are no errors
*/
unsigned long list_remove(phList pphlList)
{
    hList pPrev, pNext;

    if(*pphlList == NULL) return(all_RC_DLIST_NULL);

    /* saves the pointers to the previous and next elements */
    pPrev = (*pphlList)->prev;
    pNext = (*pphlList)->next;

    /* update the previous and next elements to point to the */
    /* correct neighbours                                    */
    pPrev->next = pNext;
    pNext->prev = pPrev;

    /* removes the current element */
    free(*pphlList);

    return(NO_ERROR);
}

/*!
    Adds an element to the end of the list.

    @param pphlList     A pointer to a list handle
    @param content      A pointer to the new element

    @return
        - ERROR_NOT_ENOUGH_MEMORY if memory allocation failed
        - NO_ERROR if there are no errors
*/
unsigned long list_add(phList pphlList, void *content)
{
    hList   pPrev;

    // Let's move to the end of the list
    if(*pphlList != NULL)
        while((*pphlList)->next != NULL) {
            *pphlList = (*pphlList)->next;
        }

    /* save pointer to previous component */
    pPrev = *pphlList;

    /* alloc memory for new component */
    *pphlList = malloc(sizeof(stack_cmp));
    if(*pphlList == NULL) return(ERROR_NOT_ENOUGH_MEMORY);

    /* update the pointers */
    (*pphlList)->elemt = content;
    (*pphlList)->next = NULL;
    (*pphlList)->prev = pPrev;

    /* update the 'next' pointer of the previous component */
    /* if it exists                                        */
    if(pPrev != NULL) pPrev->next = *pphlList;

    return(NO_ERROR);
}

/*!
    Moves current position to the start of the list.

    @param pphlList     A pointer to a list handle

    @return
        - all_RC_DLIST_NULL if the list is empty
        - NO_ERROR if there are no errors
*/
unsigned long list_set_start(phList pphlList)
{
    if(*pphlList == NULL) return(all_RC_DLIST_NULL);

    // Let's move to the start of the list
    while((*pphlList)->prev != NULL) {
        *pphlList = (*pphlList)->prev;
    }

    return(NO_ERROR);
}

/*!
    Moves current position to the end of the list.

    @param pphlList     A pointer to a list handle

    @return
        - all_RC_DLIST_NULL if the list is empty
        - NO_ERROR if there are no errors
*/
unsigned long list_set_end(phList pphlList)
{
    if(*pphlList == NULL) return(all_RC_DLIST_NULL);

    // Let's move to the end of the list
    while((*pphlList)->next != NULL) {
        *pphlList = (*pphlList)->next;
    }

    return(NO_ERROR);
}

/*!
    Moves current position to the previous element.

    @param pphlList     A pointer to a list handle

    @return
        - all_RC_DLIST_NULL if the list is empty
        - all_RC_DLIST_END if trying to go beyond the start of the list
        - NO_ERROR if there are no errors
*/
unsigned long list_prev(phList pphlList)
{
    if(*pphlList == NULL) return(all_RC_DLIST_NULL);

    // Let's move to the previous list element
    if((*pphlList)->prev != NULL)
        *pphlList = (*pphlList)->prev;
    else
        return(all_RC_DLIST_END);

    return(NO_ERROR);
}

/*!
    Moves current position to the next element.

    @param pphlList     A pointer to a list handle

    @return
        - all_RC_DLIST_NULL if the list is empty
        - all_RC_DLIST_END if trying to go beyond the end of the list
        - NO_ERROR if there are no errors
*/
unsigned long list_next(phList pphlList)
{
    if(*pphlList == NULL) return(NO_ERROR);

    // Let's move to the next list element
    if((*pphlList)->next != NULL)
        *pphlList = (*pphlList)->next;
    else
        return(all_RC_DLIST_END);

    return(NO_ERROR);
}

/*!
    Deallocates all the memory used by the list.

    @warning This deallocates all the memory for the contents also!

    @param pphlList     A pointer to a list handle

    @return
        - all_RC_DLIST_NULL if the list is empty
        - NO_ERROR if there are no errors
*/
unsigned long list_free(phList pphlList)
{
    hList  hlNext;


    if (*pphlList == NULL) {  return(all_RC_DLIST_NULL); };

    // Let's move to the start of the list
    while((*pphlList)->prev != NULL) { 
        *pphlList = (*pphlList)->prev;
    }


    // Now free all the memory blocks
    while(*pphlList != NULL) {
        hlNext = (*pphlList)->next;
        free((*pphlList)->elemt);
        free(*pphlList);
        *pphlList = hlNext;
    }


    return(NO_ERROR);
}

/*!
    Determines if the list is empty or not.

    @param pphlList     A pointer to a list handle

    @return
        - FALSE if list contains at least one element
        - TRUE if the list contains nothing
*/
BOOL list_isempty(phList pphlList)
{
    if(*pphlList == NULL)
        return(TRUE);
    else
        return(FALSE);
}


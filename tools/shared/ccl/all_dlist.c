/*!
 *
 * @file all_dlist.c
 *
 * @brief Utility functions related to dynamic lists and stacks.
 *
 * (c) osFree Project 2002, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * @author Cristiano Guadagnino <criguada@tin.it>
 *
 * Part of the ccl container library. The node layout is shared with
 * the IBM-style Dlist module (see linked_list_private.h). The first
 * three fields of LINKED_NODE (elemt, next, prev) match stack_cmp
 * byte for byte; a typecast between them is safe.
 */

#include <stdlib.h>
#include "all_dlist_private.h"
#include "linked_list_private.h"

/* --------------------------------------------------------------------------
   Helpers

   The public all_dlist.h exposes stack_cmp. Internally the same
   physical nodes are allocated as LINKED_NODE, whose first three
   fields match stack_cmp byte for byte. The size and tag fields of
   LINKED_NODE are used only by the IBM-style Dlist module and are
   ignored here.
 */

/*!
 * @brief Allocate a stack node.
 *
 * Wraps LinkedNodeAlloc, supplying zero for the size and tag fields
 * that all_dlist does not use.
 *
 * @param[in] pElem  User data pointer to store in the node.
 *
 * @return A pointer to the new node, or NULL on allocation failure.
 *
 * @retval NULL  Allocation failed.
 */
static stack_cmp *node_alloc(void *pElem) {
    return (stack_cmp *)LinkedNodeAlloc(pElem, 0, 0);
}

/*!
 * @brief Release a stack node.
 *
 * Does not touch the user data pointer stored in the node.
 *
 * @param[in] pNode  Node to release.
 */
static void node_free(stack_cmp *pNode) {
    LinkedNodeFree((PLINKED_NODE)pNode);
}

/* --------------------------------------------------------------------------
   Implementation of a dynamic stack structure
 */

/*!
 * @brief Initialize a dynamic stack.
 *
 * Allocates a new stack handle (hStack) and returns it to the
 * caller.
 *
 * @return A pointer to a stack handle, or NULL on error.
 *
 * @retval NULL  Allocation failed.
 */
phStack stack_init(void)
{
    phStack pph;

    /* Allocate memory for a new stack handle */
    pph = (phStack)malloc(sizeof(hStack));
    if (pph == NULL) return NULL;

    /* Since there is no stack element yet, initialize to NULL */
    *pph = NULL;

    return pph;
}

/*!
 * @brief Push a new element on the stack.
 *
 * The element can be of any kind (variable, structure, etc).
 * Allocation of the memory for the element is done by the user
 * code.
 *
 * @param[in,out] pph      A pointer to a stack handle.
 * @param[in]     content  A pointer to the new element.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pph is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failed.
 */
unsigned long stack_push(phStack pph, void *content)
{
    stack_cmp *pPrev, *pNew;

    if (pph == NULL) return ERROR_INVALID_PARAMETER;

    /* Save pointer to previous component */
    pPrev = *pph;

    /* Allocate memory for the new component */
    pNew = node_alloc(content);
    if (pNew == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    /* Update the pointers */
    pNew->prev = pPrev;
    if (pPrev != NULL) pPrev->next = pNew;
    *pph = pNew;

    return NO_ERROR;
}

/*!
 * @brief Pop the top element of the stack.
 *
 * When the user is done with the returned element, the user is
 * responsible for deallocating its memory.
 *
 * @param[in,out] pph      A pointer to a stack handle.
 * @param[out]    content  A pointer to a pointer to the element.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pph or content is NULL.
 * @retval all_RC_DLIST_NULL        The stack is empty.
 */
unsigned long stack_pop(phStack pph, void **content)
{
    stack_cmp *pPrev, *pCur;

    if (pph == NULL || content == NULL) return ERROR_INVALID_PARAMETER;

    *content = NULL;
    pCur = *pph;

    /* Don't "pop" if the stack is empty */
    if (pCur == NULL) return all_RC_DLIST_NULL;

    /* Save the address of the previous component */
    pPrev = pCur->prev;

    /* Update the 'next' pointer of the previous component, if it exists */
    if (pPrev != NULL) pPrev->next = NULL;

    /* Save the pointer to the contents */
    *content = pCur->elemt;

    /* Release memory for the current component */
    node_free(pCur);

    /* Update the stack pointer */
    *pph = pPrev;

    return NO_ERROR;
}

/*!
 * @brief Return the top element of the stack, without removing it.
 *
 * @param[in] pph  A pointer to a stack handle.
 *
 * @return A pointer to the top element, or NULL if the stack is
 *         empty or pph is NULL.
 *
 * @retval NULL  The stack is empty, or pph is NULL.
 */
void *stack_top(phStack pph)
{
    if (pph == NULL) return NULL;
    if (*pph == NULL) return NULL;

    return (*pph)->elemt;
}

/*!
 * @brief Remove an arbitrary component from the stack.
 *
 * Does not affect the stack pointer, unless the removed element is
 * the current top, in which case the pointer moves to the previous
 * element.
 *
 * @param[in,out] pph        A pointer to a stack handle.
 * @param[in]     component  A pointer to the stack element to remove.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pph is NULL.
 * @retval all_RC_DLIST_NULL        component is NULL.
 */
unsigned long stack_remove(phStack pph, void *component)
{
    stack_cmp *pCur, *pPrev, *pNext;

    if (pph == NULL) return ERROR_INVALID_PARAMETER;
    if (component == NULL) return all_RC_DLIST_NULL;

    /* Save the pointers to the previous and next components */
    pCur  = (stack_cmp *)component;
    pPrev = pCur->prev;
    pNext = pCur->next;

    /* Update the previous and next components to point to the correct
       neighbours. The original implementation assumed both neighbours
       non-NULL; that assumption is not guaranteed when the removed
       element is at either end of the stack. */
    if (pPrev != NULL) pPrev->next = pNext;
    if (pNext != NULL) pNext->prev = pPrev;
    if (*pph == pCur) *pph = pPrev;

    /* Remove the current component */
    node_free(pCur);

    return NO_ERROR;
}

/* --------------------------------------------------------------------------
   Implementation of a dynamic list structure
 */

/*!
 * @brief Initialize a dynamic list.
 *
 * Allocates a new list handle (hList) and returns it to the caller.
 *
 * @return A pointer to a list handle, or NULL on error.
 *
 * @retval NULL  Allocation failed.
 */
phList list_init(void)
{
    phList ppl;

    /* Allocate memory for a new list handle */
    ppl = (phList)malloc(sizeof(hList));
    if (ppl == NULL) return NULL;

    /* Since there is no list element yet, initialize to NULL */
    *ppl = NULL;

    return ppl;
}

/*!
 * @brief Return the current element of the list.
 *
 * @param[in] ppl  A pointer to a list handle.
 *
 * @return A pointer to the current list element, or NULL if the
 *         list is empty or ppl is NULL.
 *
 * @retval NULL  The list is empty, or ppl is NULL.
 */
void *list_get(phList ppl)
{
    if (ppl == NULL) return NULL;
    if (*ppl == NULL) return NULL;

    return (*ppl)->elemt;
}

/*!
 * @brief Insert a new list element after the current element.
 *
 * Allocation of the memory for the element is done by the user
 * code. The cursor moves to the newly inserted element.
 *
 * @param[in,out] ppl      A pointer to a list handle.
 * @param[in]     content  A pointer to the new element.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failed.
 */
unsigned long list_insert(phList ppl, void *content)
{
    stack_cmp *pPrev, *pNext, *pNew;

    if (ppl == NULL) return ERROR_INVALID_PARAMETER;

    /* Save pointer to previous component */
    pPrev = *ppl;

    /* Allocate memory for the new component */
    pNew = node_alloc(content);
    if (pNew == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    if (pPrev == NULL) {
        /* Empty list: the new element becomes the only element. */
        *ppl = pNew;
        return NO_ERROR;
    }

    pNext = pPrev->next;
    pNew->next = pNext;
    pNew->prev = pPrev;
    pPrev->next = pNew;
    if (pNext != NULL) pNext->prev = pNew;
    *ppl = pNew;

    return NO_ERROR;
}

/*!
 * @brief Remove the current element from the list.
 *
 * Does @a not deallocate memory for the element. The user is
 * responsible for doing it. The cursor moves to the next element,
 * or to the previous one if there is no next element.
 *
 * @param[in,out] ppl  A pointer to a list handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval all_RC_DLIST_NULL        The list is empty.
 */
unsigned long list_remove(phList ppl)
{
    stack_cmp *pCur, *pPrev, *pNext;

    if (ppl == NULL) return ERROR_INVALID_PARAMETER;

    pCur = *ppl;
    if (pCur == NULL) return all_RC_DLIST_NULL;

    /* Save the pointers to the previous and next elements */
    pPrev = pCur->prev;
    pNext = pCur->next;

    /* Update the previous and next elements to point to the correct
       neighbours. The original implementation assumed both neighbours
       non-NULL; that assumption is not guaranteed when the removed
       element is at either end of the list. */
    if (pPrev != NULL) pPrev->next = pNext;
    if (pNext != NULL) pNext->prev = pPrev;

    /* Remove the current element */
    node_free(pCur);

    /* The cursor moves to the next element if present, otherwise to
       the previous one. If the list becomes empty the cursor is NULL. */
    if (pNext != NULL) *ppl = pNext;
    else *ppl = pPrev;

    return NO_ERROR;
}

/*!
 * @brief Add an element to the end of the list.
 *
 * The cursor moves to the newly added element, matching the
 * original implementation.
 *
 * @param[in,out] ppl      A pointer to a list handle.
 * @param[in]     content  A pointer to the new element.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Memory allocation failed.
 */
unsigned long list_add(phList ppl, void *content)
{
    stack_cmp *pPrev, *pNew;

    if (ppl == NULL) return ERROR_INVALID_PARAMETER;

    /* Move to the end of the list */
    if (*ppl != NULL) {
        while ((*ppl)->next != NULL) {
            *ppl = (*ppl)->next;
        }
    }

    /* Save pointer to previous component */
    pPrev = *ppl;

    /* Allocate memory for the new component */
    pNew = node_alloc(content);
    if (pNew == NULL) return ERROR_NOT_ENOUGH_MEMORY;

    /* Update the pointers */
    pNew->prev = pPrev;
    if (pPrev != NULL) pPrev->next = pNew;
    *ppl = pNew;

    return NO_ERROR;
}

/*!
 * @brief Move current position to the start of the list.
 *
 * @param[in,out] ppl  A pointer to a list handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval all_RC_DLIST_NULL        The list is empty.
 */
unsigned long list_set_start(phList ppl)
{
    if (ppl == NULL) return ERROR_INVALID_PARAMETER;
    if (*ppl == NULL) return all_RC_DLIST_NULL;

    /* Move to the start of the list */
    while ((*ppl)->prev != NULL) {
        *ppl = (*ppl)->prev;
    }

    return NO_ERROR;
}

/*!
 * @brief Move current position to the end of the list.
 *
 * @param[in,out] ppl  A pointer to a list handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval all_RC_DLIST_NULL        The list is empty.
 */
unsigned long list_set_end(phList ppl)
{
    if (ppl == NULL) return ERROR_INVALID_PARAMETER;
    if (*ppl == NULL) return all_RC_DLIST_NULL;

    /* Move to the end of the list */
    while ((*ppl)->next != NULL) {
        *ppl = (*ppl)->next;
    }

    return NO_ERROR;
}

/*!
 * @brief Move current position to the previous element.
 *
 * @param[in,out] ppl  A pointer to a list handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval all_RC_DLIST_NULL        The list is empty.
 * @retval all_RC_DLIST_END         Trying to go beyond the start of
 *                                  the list.
 */
unsigned long list_prev(phList ppl)
{
    if (ppl == NULL) return ERROR_INVALID_PARAMETER;
    if (*ppl == NULL) return all_RC_DLIST_NULL;

    /* Move to the previous list element */
    if ((*ppl)->prev != NULL)
        *ppl = (*ppl)->prev;
    else
        return all_RC_DLIST_END;

    return NO_ERROR;
}

/*!
 * @brief Move current position to the next element.
 *
 * @param[in,out] ppl  A pointer to a list handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval all_RC_DLIST_NULL        The list is empty.
 * @retval all_RC_DLIST_END         Trying to go beyond the end of
 *                                  the list.
 */
unsigned long list_next(phList ppl)
{
    if (ppl == NULL) return ERROR_INVALID_PARAMETER;
    if (*ppl == NULL) return all_RC_DLIST_NULL;

    /* Move to the next list element */
    if ((*ppl)->next != NULL)
        *ppl = (*ppl)->next;
    else
        return all_RC_DLIST_END;

    return NO_ERROR;
}

/*!
 * @brief Deallocate all the memory used by the list.
 *
 * @warning This deallocates all the memory for the contents also!
 *
 * @param[in,out] ppl  A pointer to a list handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval all_RC_DLIST_NULL        The list is empty.
 */
unsigned long list_free(phList ppl)
{
    stack_cmp *pCur, *pNext;

    if (ppl == NULL) return ERROR_INVALID_PARAMETER;
    if (*ppl == NULL) return all_RC_DLIST_NULL;

    /* Move to the start of the list */
    while ((*ppl)->prev != NULL) {
        *ppl = (*ppl)->prev;
    }

    /* Now free all the memory blocks */
    pCur = *ppl;
    while (pCur != NULL) {
        pNext = pCur->next;
        if (pCur->elemt != NULL) free(pCur->elemt);
        node_free(pCur);
        pCur = pNext;
    }
    *ppl = NULL;

    return NO_ERROR;
}

/*!
 * @brief Determine whether the list is empty.
 *
 * @param[in] ppl  A pointer to a list handle.
 *
 * @return TRUE if the list is empty or ppl is NULL, FALSE otherwise.
 *
 * @retval TRUE   The list is empty, or ppl is NULL.
 * @retval FALSE  The list contains at least one element.
 */
BOOL list_isempty(phList ppl)
{
    if (ppl == NULL) return TRUE;
    if (*ppl == NULL) return TRUE;
    return FALSE;
}

/*!
 *
 * @file all_dlist_private.h
 *
 * @brief Private declaration of the all_dlist interface.
 *
 * (c) osFree Project 2002, <http://www.osFree.org>
 * for licence see licence.txt in root directory, or project website
 *
 * @author Cristiano Guadagnino <criguada@tin.it>
 *
 * This header mirrors the public all_dlist.h that is part of the
 * fixed osFree interface. The public header includes all_shared.h,
 * which in turn pulls in <os2.h> and cannot be used on host builds.
 * This private copy repeats the declarations with the same layout
 * and signatures, so that all_dlist.c can be compiled in the host
 * tools without the OS/2 SDK headers.
 *
 * The node layout is shared with the IBM Dlist module. The struct
 * tag and the first three fields are defined by linked_list_private.h
 * and are aliased here as stack_cmp.
 *
 * Any change to the layout or to the signatures here must be
 * matched by the same change in all_dlist.h. The two files describe
 * the same binary interface.
 */

#ifndef _ALL_DLIST_PRIVATE_H_
#define _ALL_DLIST_PRIVATE_H_

#include "os2types.h"
#include "os2err.h"
#include "linked_list_private.h"

/* --------------------------------------------------------------------------
   Return codes
 */

/*!
 * @brief Handle points to NULL (empty stack or list).
 */
#define all_RC_DLIST_NULL   1L

/*!
 * @brief At either end of the list.
 */
#define all_RC_DLIST_END    2L

/* --------------------------------------------------------------------------
   Data types
 */

/*!
 * @brief Stack component of the dynamic stack.
 *
 * The public all_dlist.h declares this as a three-field struct.
 * The private header shares one struct definition with the Dlist
 * module; the extra size and tag fields are ignored by all_dlist.
 */
typedef LINKED_NODE stack_cmp;

/*!
 * @brief A stack handle.
 */
typedef stack_cmp *hStack;

/*!
 * @brief A pointer to a stack handle.
 */
typedef hStack    *phStack;

/*!
 * @brief A list handle.
 */
typedef hStack     hList;

/*!
 * @brief A pointer to a list handle.
 */
typedef phStack    phList;

/* --------------------------------------------------------------------------
   Exported functions
 */

/* -- Stack -- */

/*!
 * @brief Initialize a dynamic stack.
 *
 * @return A pointer to a stack handle, or NULL on error.
 *
 * @retval NULL  Allocation failed.
 */
phStack       stack_init(void);

/*!
 * @brief Push a new element on the stack.
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
unsigned long stack_push(phStack pph, void *content);

/*!
 * @brief Pop the top element of the stack.
 *
 * @param[in,out] pph      A pointer to a stack handle.
 * @param[out]    content  A pointer to a pointer that receives the
 *                         element.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pph or content is NULL.
 * @retval all_RC_DLIST_NULL        The stack is empty.
 */
unsigned long stack_pop(phStack pph, void **content);

/*!
 * @brief Return the top element of the stack without removing it.
 *
 * @param[in] pph  A pointer to a stack handle.
 *
 * @return A pointer to the top element, or NULL on failure.
 *
 * @retval NULL  The stack is empty, or pph is NULL.
 */
void         *stack_top(phStack pph);

/*!
 * @brief Remove an arbitrary component from the stack.
 *
 * @param[in,out] pph        A pointer to a stack handle.
 * @param[in]     component  A pointer to the component to remove.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pph is NULL.
 * @retval all_RC_DLIST_NULL        component is NULL.
 */
unsigned long stack_remove(phStack pph, void *component);

/* -- List -- */

/*!
 * @brief Initialize a dynamic list.
 *
 * @return A pointer to a list handle, or NULL on error.
 *
 * @retval NULL  Allocation failed.
 */
phList        list_init(void);

/*!
 * @brief Return the current element of the list.
 *
 * @param[in] ppl  A pointer to a list handle.
 *
 * @return A pointer to the current element, or NULL on failure.
 *
 * @retval NULL  The list is empty, or ppl is NULL.
 */
void         *list_get(phList ppl);

/*!
 * @brief Insert a new element after the current position.
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
unsigned long list_insert(phList ppl, void *content);

/*!
 * @brief Remove the current element from the list.
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
unsigned long list_remove(phList ppl);

/*!
 * @brief Add a new element to the end of the list.
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
unsigned long list_add(phList ppl, void *content);

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
unsigned long list_set_start(phList ppl);

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
unsigned long list_set_end(phList ppl);

/*!
 * @brief Move current position to the previous list element.
 *
 * @param[in,out] ppl  A pointer to a list handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval all_RC_DLIST_NULL        The list is empty.
 * @retval all_RC_DLIST_END         Already at the start of the list.
 */
unsigned long list_prev(phList ppl);

/*!
 * @brief Move current position to the next list element.
 *
 * @param[in,out] ppl  A pointer to a list handle.
 *
 * @return NO_ERROR on success, or one of the error codes listed
 *         below.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  ppl is NULL.
 * @retval all_RC_DLIST_NULL        The list is empty.
 * @retval all_RC_DLIST_END         Already at the end of the list.
 */
unsigned long list_next(phList ppl);

/*!
 * @brief Completely deallocate the list (and its contents).
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
unsigned long list_free(phList ppl);

/*!
 * @brief Test whether the list is empty.
 *
 * @param[in] ppl  A pointer to a list handle.
 *
 * @return TRUE if the list is empty or ppl is NULL, FALSE otherwise.
 *
 * @retval TRUE   The list is empty, or ppl is NULL.
 * @retval FALSE  The list contains at least one element.
 */
BOOL          list_isempty(phList ppl);

#endif /* _ALL_DLIST_PRIVATE_H_ */

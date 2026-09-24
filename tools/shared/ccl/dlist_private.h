/*!
 * @file dlist_private.h
 *
 * @brief Private declaration of the IBM Dlist container.
 *
 * This header mirrors the public IBM Dlist interface. The public
 * headers are kept as they are; this private copy repeats the same
 * types and signatures so that dlist.c can be compiled on host
 * builds without pulling in <os2.h> or any other OS/2 SDK header.
 *
 * The type names, function names, error codes and signatures are
 * part of the fixed IBM Dlist interface and are reproduced here
 * unchanged so that ccl remains binary compatible with the original.
 * Their copyright notice is preserved below.
 *
 * The implementation in dlist.c is the work of the osFree Project.
 *
 * This module is single threaded. If it is used in a multithreaded
 * environment, the caller must provide the necessary locking.
 *
 * @par Containers of mixed items
 * A single list may hold items of different kinds. Each item is
 * given a tag by the caller. Callers use the tag to tell items of
 * one kind from items of another without fetching the item data.
 * Callers are expected to use one tag per kind of item; the module
 * itself does not interpret the tag.
 *
 * @par Current item and cursor
 * Each non-empty list has a current item. The functions that work
 * on "the current item" (GetItem, GetObject, GetNextItem,
 * GetPreviousItem, GetNextObject, GetPreviousObject, ExtractItem,
 * ExtractObject, ReplaceItem, ReplaceObject, DeleteItem) operate on
 * that item unless a different one is selected through a handle.
 * The cursor is moved by GoToStartOfList, GoToEndOfList, NextItem,
 * PreviousItem and GoToSpecifiedItem.
 *
 * @par Items and objects
 * The two words refer to who owns the memory that holds the user's
 * data. For an item, the module owns the memory: InsertItem copies
 * the caller's data into a block that the module allocates, and
 * GetItem copies that data back into a caller buffer. For an
 * object, the caller owns the memory: InsertObject stores the
 * caller's pointer as-is, and GetObject returns it. While an item
 * or object is in the list, the module is responsible for the link
 * nodes only. Data memory of objects stays the caller's until the
 * object is removed; data memory of items is the module's from
 * insertion to removal.
 *
 * @par Handles
 * GetHandle returns an opaque handle for an item. A handle stays
 * valid while the item is in the list, survives any reordering,
 * and can be used to make the item current (GoToSpecifiedItem) or
 * to operate on it without moving the cursor (functions that take
 * a Handle argument).
 *
 * @par Copying of embedded pointers
 * When InsertItem copies user data, it copies exactly ItemSize
 * bytes. If the caller's data is a structure or an array that
 * contains pointers, the pointers themselves are copied, but the
 * memory they point to is not. This is inherent to the interface:
 * the module only knows the address and the size that the caller
 * provides, and cannot know the internal structure of the data.
 *
 * @par Insertion modes
 * InsertItem, InsertObject and TransferItem share an Insertion_Modes
 * parameter that selects where the new item is placed: at the start
 * of the list, before a target item, after a target item, or at the
 * end.
 *
 * @par Error reporting
 * Every function of the module takes a CARDINAL32* argument into
 * which it writes a return code. DLIST_SUCCESS means the operation
 * completed. Any other value is one of the DLIST_* constants below.
 * User callbacks passed to ForEachItem and PruneList may also write
 * DLIST_SEARCH_COMPLETE; the module treats that as a successful
 * early termination of the traversal.
 *
 * Copyright (c) International Business Machines Corp., 2000
 *   The Dlist interface described here originates from IBM. The
 *   type names, function names, error codes and signatures are used
 *   with the same names and signatures as in the original.
 *
 * Copyright (c) osFree Project 2026, <http://www.osFree.org>
 *   This private header, and the implementation in dlist.c, are
 *   part of the ccl container library.
 *   for licence see licence.txt in root directory, or project website
 *
 * Any change to a signature or to a type in this file must be
 * matched by the same change in the public headers.
 */

#ifndef CCL_DLIST_PRIVATE_H
#define CCL_DLIST_PRIVATE_H

#include "os2types.h"
#include "os2err.h"

/* ==================================================================
 * Base types
 *
 * Equivalents of the base types used by the IBM Dlist interface. The
 * definitions are repeated here so that ccl does not depend on any
 * external header on host builds.
 * ================================================================== */

/*!
 * @typedef INTEGER32
 * @brief Signed 32-bit integer.
 */
typedef long int          INTEGER32;

/*!
 * @typedef CARDINAL32
 * @brief Unsigned 32-bit integer.
 */
typedef unsigned long     CARDINAL32;

/*!
 * @typedef BOOLEAN
 * @brief Boolean value: TRUE (1) or FALSE (0).
 */
typedef unsigned char     BOOLEAN;

/*!
 * @typedef ADDRESS
 * @brief Generic pointer to user data.
 */
typedef void             *ADDRESS;

#ifndef TRUE
/*!
 * @def TRUE
 * @brief Boolean true value.
 */
#define TRUE  1
#endif

#ifndef FALSE
/*!
 * @def FALSE
 * @brief Boolean false value.
 */
#define FALSE 0
#endif

/* ==================================================================
 * Dlist types
 * ================================================================== */

/*!
 * @typedef TAG
 * @brief User-assigned identifier of an item's kind.
 *
 * The module does not interpret the tag. Callers are expected to
 * assign one tag per kind of item placed in a list. The tag is
 * returned by GetTag and is passed to user callbacks of ForEachItem
 * and PruneList.
 */
typedef unsigned long TAG;

/*!
 * @typedef DLIST
 * @brief Handle to a list.
 *
 * Created by CreateList and released by DestroyList. The value is
 * opaque; its internal representation is not part of the interface.
 */
typedef ADDRESS DLIST;

/*!
 * @enum _Insertion_Modes
 * @brief Position of an item relative to a target item.
 */
typedef enum _Insertion_Modes {
    InsertAtStart,   /*!< At the start of the list. */
    InsertBefore,    /*!< Before the target item.  */
    InsertAfter,     /*!< After the target item.   */
    AppendToList     /*!< At the end of the list.  */
} Insertion_Modes;

/* ==================================================================
 * Error codes
 *
 * Every function of this module writes a return code into the
 * *Error argument: DLIST_SUCCESS on success, one of the other
 * constants on failure. DLIST_SEARCH_COMPLETE is written by user
 * callbacks to ForEachItem and PruneList; the module never returns
 * it on its own.
 * ================================================================== */

/*!
 * @def DLIST_SUCCESS
 * @brief No error. Value: 0.
 */
#define DLIST_SUCCESS                    0

/*!
 * @def DLIST_OUT_OF_MEMORY
 * @brief Memory allocation failed. Value: 1.
 */
#define DLIST_OUT_OF_MEMORY              1

/*!
 * @def DLIST_CORRUPTED
 * @brief The list's internal structures are inconsistent. Value: 2.
 */
#define DLIST_CORRUPTED                  2

/*!
 * @def DLIST_BAD
 * @brief The list handle is not recognized. Value: 3.
 */
#define DLIST_BAD                        3

/*!
 * @def DLIST_NOT_INITIALIZED
 * @brief The list handle has not been initialized. Value: 4.
 */
#define DLIST_NOT_INITIALIZED            4

/*!
 * @def DLIST_EMPTY
 * @brief The list is empty. Value: 5.
 */
#define DLIST_EMPTY                      5

/*!
 * @def DLIST_ITEM_SIZE_WRONG
 * @brief The provided item size does not match the stored size.
 * Value: 6.
 */
#define DLIST_ITEM_SIZE_WRONG            6

/*!
 * @def DLIST_BAD_ITEM_POINTER
 * @brief The item pointer is invalid where a valid one is required.
 * Value: 7.
 */
#define DLIST_BAD_ITEM_POINTER           7

/*!
 * @def DLIST_ITEM_SIZE_ZERO
 * @brief The provided item size is zero. Value: 8.
 */
#define DLIST_ITEM_SIZE_ZERO             8

/*!
 * @def DLIST_ITEM_TAG_WRONG
 * @brief The provided tag does not match the stored tag. Value: 9.
 */
#define DLIST_ITEM_TAG_WRONG             9

/*!
 * @def DLIST_END_OF_LIST
 * @brief The cursor is already at the end of the list. Value: 10.
 */
#define DLIST_END_OF_LIST               10

/*!
 * @def DLIST_ALREADY_AT_START
 * @brief The cursor is already at the start of the list. Value: 11.
 */
#define DLIST_ALREADY_AT_START          11

/*!
 * @def DLIST_BAD_HANDLE
 * @brief The provided item handle is not valid for this list.
 * Value: 12.
 */
#define DLIST_BAD_HANDLE                12

/*!
 * @def DLIST_INVALID_INSERTION_MODE
 * @brief The insertion mode is not one of the Insertion_Modes.
 * Value: 13.
 */
#define DLIST_INVALID_INSERTION_MODE    13

/*!
 * @def DLIST_SEARCH_COMPLETE
 * @brief Traversal aborted by the user callback; not an error.
 * Value: 0xFF.
 */
#define DLIST_SEARCH_COMPLETE          0xFF

/* ==================================================================
 * Lifecycle
 * ================================================================== */

#ifdef USE_POOLMAN
/*!
 * @brief Create a new list (pool variant).
 *
 * If the module is built with USE_POOLMAN defined, the list owns a
 * pool of link nodes. InitialPoolSize is the number of link nodes
 * created with the pool. When the pool runs out, PoolIncrement more
 * nodes are allocated, up to MaximumPoolSize. Once the pool has
 * reached MaximumPoolSize, further nodes returned to the pool are
 * deallocated instead of being kept.
 *
 * @param[in] InitialPoolSize  Initial number of link nodes.
 * @param[in] MaximumPoolSize  Maximum number of link nodes.
 * @param[in] PoolIncrement    Number of nodes to allocate when the
 *                             pool runs out.
 *
 * @return A new list handle, or NULL on allocation failure.
 *
 * @retval NULL  Allocation failed.
 */
DLIST _System CreateList(CARDINAL32 InitialPoolSize,
                          CARDINAL32 MaximumPoolSize,
                          CARDINAL32 PoolIncrement);
#else
/*!
 * @brief Create a new list.
 *
 * Each link node is allocated on demand.
 *
 * @return A new list handle, or NULL on allocation failure. On
 *         failure the caller receives no handle and no error code;
 *         allocation of the very first structures is the only case
 *         in which errors are not reported through *Error.
 *
 * @retval NULL  Allocation failed.
 */
DLIST _System CreateList(void);
#endif

/*!
 * @brief Destroy a list.
 *
 * Releases the internal structures of the list. If @p FreeItemMemory
 * is TRUE, the data memory of every stored element is released as
 * well; if FALSE, only the list's own structures are released and
 * the caller remains responsible for the stored data.
 *
 * @warning The module has no way to know the internal structure of
 *          the stored items. Items that contain embedded pointers
 *          cannot be entirely freed by the module. When this
 *          matters, empty the list manually before calling
 *          DestroyList.
 *
 * On success @p *ListToDestroy is set to NULL.
 *
 * @param[in,out] ListToDestroy   Pointer to the list handle. Not
 *                                NULL.
 * @param[in]     FreeItemMemory  Free the stored elements as well.
 * @param[out]    Error           Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED.
 */
void _System DestroyList(DLIST * ListToDestroy,
                          BOOLEAN FreeItemMemory,
                          CARDINAL32 * Error);

/* ==================================================================
 * Insert
 *
 * InsertItem copies the caller's data into a block that the module
 * allocates. InsertObject stores the caller's pointer and takes
 * ownership of the pointed-to block.
 *
 * @par Selecting the reference item
 * @p TargetHandle chooses the reference item:
 *   - NULL: the current item is used;
 *   - otherwise: the item identified by the handle.
 * InsertAtStart and AppendToList do not consult TargetHandle.
 *
 * @par Cursor
 * If @p MakeCurrent is TRUE, the newly inserted item becomes the
 * current item.
 *
 * @par Returned handle
 * On success the returned handle identifies the new item. The
 * handle remains valid while the item stays in the list.
 * ================================================================== */

/*!
 * @brief Insert a copy of caller data.
 *
 * @param[in]  ListToAddTo   List handle.
 * @param[in]  ItemSize      Size of the caller's data in bytes.
 * @param[in]  ItemLocation  Pointer to the caller's data.
 * @param[in]  ItemTag       Tag for the new item.
 * @param[in]  TargetHandle  Reference item, or NULL.
 * @param[in]  Insert_Mode   Position relative to the reference item.
 * @param[in]  MakeCurrent   Make the new item the current item.
 * @param[out] Error         Return code. Not NULL. On success
 *                           DLIST_SUCCESS; on failure one of
 *                           DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                           DLIST_ITEM_SIZE_ZERO,
 *                           DLIST_BAD_ITEM_POINTER,
 *                           DLIST_INVALID_INSERTION_MODE,
 *                           DLIST_BAD_HANDLE, DLIST_OUT_OF_MEMORY.
 *
 * @return Handle to the new item, or NULL on failure.
 *
 * @retval NULL  On failure; see @p Error.
 */
ADDRESS _System InsertItem(DLIST           ListToAddTo,
                            CARDINAL32      ItemSize,
                            ADDRESS         ItemLocation,
                            TAG             ItemTag,
                            ADDRESS         TargetHandle,
                            Insertion_Modes Insert_Mode,
                            BOOLEAN         MakeCurrent,
                            CARDINAL32 *    Error);

/*!
 * @brief Insert an object supplied by the caller.
 *
 * The pointer @p ItemLocation is stored as-is; the module does not
 * copy the data. Ownership of the pointed-to block transfers to the
 * list.
 *
 * @param[in]  ListToAddTo   List handle.
 * @param[in]  ItemSize      Size of the caller's data in bytes.
 * @param[in]  ItemLocation  Pointer to the caller's data.
 * @param[in]  ItemTag       Tag for the new item.
 * @param[in]  TargetHandle  Reference item, or NULL.
 * @param[in]  Insert_Mode   Position relative to the reference item.
 * @param[in]  MakeCurrent   Make the new item the current item.
 * @param[out] Error         Return code. Not NULL. On success
 *                           DLIST_SUCCESS; on failure one of
 *                           DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                           DLIST_ITEM_SIZE_ZERO,
 *                           DLIST_BAD_ITEM_POINTER,
 *                           DLIST_INVALID_INSERTION_MODE,
 *                           DLIST_BAD_HANDLE, DLIST_OUT_OF_MEMORY.
 *
 * @return Handle to the new item, or NULL on failure.
 *
 * @retval NULL  On failure; see @p Error.
 */
ADDRESS _System InsertObject(DLIST           ListToAddTo,
                              CARDINAL32      ItemSize,
                              ADDRESS         ItemLocation,
                              TAG             ItemTag,
                              ADDRESS         TargetHandle,
                              Insertion_Modes Insert_Mode,
                              BOOLEAN         MakeCurrent,
                              CARDINAL32 *    Error);

/* ==================================================================
 * Delete
 * ================================================================== */

/*!
 * @brief Delete the item identified by @p Handle from the list.
 *
 * If @p Handle is NULL, the current item is deleted. @p FreeMemory
 * selects whether the item's stored data memory is released. When
 * the deleted item is the current item, the cursor moves to the
 * next item if one exists, otherwise to the previous one.
 *
 * @param[in,out] ListToDeleteFrom  List handle.
 * @param[in]     FreeMemory        Free the stored data memory.
 * @param[in]     Handle            Item handle, or NULL for current.
 * @param[out]    Error             Return code. Not NULL. On success
 *                                  DLIST_SUCCESS; on failure one of
 *                                  DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                  DLIST_EMPTY, DLIST_BAD_HANDLE.
 */
void _System DeleteItem(DLIST        ListToDeleteFrom,
                         BOOLEAN      FreeMemory,
                         ADDRESS      Handle,
                         CARDINAL32 * Error);

/*!
 * @brief Delete every item in the list.
 *
 * @p FreeMemory selects whether the data memory of every stored
 * element is released. On success the list is empty.
 *
 * @param[in,out] ListToDeleteFrom  List handle.
 * @param[in]     FreeMemory        Free the stored data memory.
 * @param[out]    Error             Return code. Not NULL. On success
 *                                  DLIST_SUCCESS; on failure one of
 *                                  DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                  DLIST_EMPTY.
 */
void _System DeleteAllItems(DLIST        ListToDeleteFrom,
                             BOOLEAN      FreeMemory,
                             CARDINAL32 * Error);

/* ==================================================================
 * Get (copy into caller buffer)
 *
 * These functions copy the stored data into a caller-supplied
 * buffer. The buffer must be at least @p ItemSize bytes; @p ItemSize
 * must match the size of the stored item, and @p ItemTag must match
 * the stored tag.
 *
 * GetNextItem and GetPreviousItem move the cursor before copying.
 * On success, the cursor points at the returned item. On failure the
 * cursor is not moved.
 * ================================================================== */

/*!
 * @brief Copy the current or a selected item into a caller buffer.
 *
 * @param[in]  ListToGetItemFrom  List handle.
 * @param[in]  ItemSize           Expected size of the item.
 * @param[out] ItemLocation       Destination buffer. Not NULL.
 * @param[in]  ItemTag            Expected tag of the item.
 * @param[in]  Handle             Item handle, or NULL for current.
 * @param[in]  MakeCurrent        Make the item current on success.
 * @param[out] Error              Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                DLIST_EMPTY, DLIST_ITEM_SIZE_WRONG,
 *                                DLIST_ITEM_TAG_WRONG,
 *                                DLIST_BAD_ITEM_POINTER,
 *                                DLIST_BAD_HANDLE.
 */
void _System GetItem(DLIST          ListToGetItemFrom,
                      CARDINAL32     ItemSize,
                      ADDRESS        ItemLocation,
                      TAG            ItemTag,
                      ADDRESS        Handle,
                      BOOLEAN        MakeCurrent,
                      CARDINAL32 *   Error);

/*!
 * @brief Move the cursor to the next item and copy it.
 *
 * @param[in]  ListToGetItemFrom  List handle.
 * @param[in]  ItemSize           Expected size of the item.
 * @param[out] ItemLocation       Destination buffer. Not NULL.
 * @param[in]  ItemTag            Expected tag of the item.
 * @param[out] Error              Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                DLIST_EMPTY, DLIST_END_OF_LIST,
 *                                DLIST_ITEM_SIZE_WRONG,
 *                                DLIST_ITEM_TAG_WRONG,
 *                                DLIST_BAD_ITEM_POINTER.
 */
void _System GetNextItem(DLIST          ListToGetItemFrom,
                          CARDINAL32     ItemSize,
                          ADDRESS        ItemLocation,
                          TAG            ItemTag,
                          CARDINAL32 *   Error);

/*!
 * @brief Move the cursor to the previous item and copy it.
 *
 * @param[in]  ListToGetItemFrom  List handle.
 * @param[in]  ItemSize           Expected size of the item.
 * @param[out] ItemLocation       Destination buffer. Not NULL.
 * @param[in]  ItemTag            Expected tag of the item.
 * @param[out] Error              Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                DLIST_EMPTY, DLIST_ALREADY_AT_START,
 *                                DLIST_ITEM_SIZE_WRONG,
 *                                DLIST_ITEM_TAG_WRONG,
 *                                DLIST_BAD_ITEM_POINTER.
 */
void _System GetPreviousItem(DLIST          ListToGetItemFrom,
                              CARDINAL32     ItemSize,
                              ADDRESS        ItemLocation,
                              TAG            ItemTag,
                              CARDINAL32 *   Error);

/* ==================================================================
 * Get (return pointer to stored element)
 *
 * These functions return the address of the item as stored inside
 * the list. The caller must not free that memory while the item
 * remains in the list.
 * ================================================================== */

/*!
 * @brief Return a pointer to the stored data of an item.
 *
 * @param[in]  ListToGetItemFrom  List handle.
 * @param[in]  ItemSize           Expected size of the item.
 * @param[in]  ItemTag            Expected tag of the item.
 * @param[in]  Handle             Item handle, or NULL for current.
 * @param[in]  MakeCurrent        Make the item current on success.
 * @param[out] Error              Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                DLIST_EMPTY, DLIST_ITEM_SIZE_WRONG,
 *                                DLIST_ITEM_TAG_WRONG,
 *                                DLIST_BAD_HANDLE.
 *
 * @return The stored data pointer, or NULL on failure.
 *
 * @retval NULL  On failure; see @p Error.
 */
ADDRESS _System GetObject(DLIST          ListToGetItemFrom,
                           CARDINAL32     ItemSize,
                           TAG            ItemTag,
                           ADDRESS        Handle,
                           BOOLEAN        MakeCurrent,
                           CARDINAL32 *   Error);

/*!
 * @brief Move the cursor to the next item and return its data
 *        pointer.
 *
 * @param[in]  ListToGetItemFrom  List handle.
 * @param[in]  ItemSize           Expected size of the item.
 * @param[in]  ItemTag            Expected tag of the item.
 * @param[out] Error              Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                DLIST_EMPTY, DLIST_END_OF_LIST,
 *                                DLIST_ITEM_SIZE_WRONG,
 *                                DLIST_ITEM_TAG_WRONG.
 *
 * @return The stored data pointer, or NULL on failure.
 *
 * @retval NULL  On failure; see @p Error.
 */
ADDRESS _System GetNextObject(DLIST          ListToGetItemFrom,
                               CARDINAL32     ItemSize,
                               TAG            ItemTag,
                               CARDINAL32 *   Error);

/*!
 * @brief Move the cursor to the previous item and return its data
 *        pointer.
 *
 * @param[in]  ListToGetItemFrom  List handle.
 * @param[in]  ItemSize           Expected size of the item.
 * @param[in]  ItemTag            Expected tag of the item.
 * @param[out] Error              Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                DLIST_EMPTY,
 *                                DLIST_ALREADY_AT_START,
 *                                DLIST_ITEM_SIZE_WRONG,
 *                                DLIST_ITEM_TAG_WRONG.
 *
 * @return The stored data pointer, or NULL on failure.
 *
 * @retval NULL  On failure; see @p Error.
 */
ADDRESS _System GetPreviousObject(DLIST          ListToGetItemFrom,
                                   CARDINAL32     ItemSize,
                                   TAG            ItemTag,
                                   CARDINAL32 *   Error);

/* ==================================================================
 * Extract (remove from list, return value)
 *
 * The item is removed from the list as part of the operation.
 * ExtractItem copies the data into the caller's buffer; the stored
 * data memory is released. ExtractObject returns a pointer to the
 * stored data and transfers ownership to the caller.
 * ================================================================== */

/*!
 * @brief Remove an item and copy its data into a caller buffer.
 *
 * @param[in]  ListToGetItemFrom  List handle.
 * @param[in]  ItemSize           Expected size of the item.
 * @param[out] ItemLocation       Destination buffer. Not NULL.
 * @param[in]  ItemTag            Expected tag of the item.
 * @param[in]  Handle             Item handle, or NULL for current.
 * @param[out] Error              Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                DLIST_EMPTY, DLIST_ITEM_SIZE_WRONG,
 *                                DLIST_ITEM_TAG_WRONG,
 *                                DLIST_BAD_ITEM_POINTER,
 *                                DLIST_BAD_HANDLE.
 */
void _System ExtractItem(DLIST          ListToGetItemFrom,
                          CARDINAL32     ItemSize,
                          ADDRESS        ItemLocation,
                          TAG            ItemTag,
                          ADDRESS        Handle,
                          CARDINAL32 *   Error);

/*!
 * @brief Remove an item and return ownership of its data.
 *
 * @param[in]  ListToGetItemFrom  List handle.
 * @param[in]  ItemSize           Expected size of the item.
 * @param[in]  ItemTag            Expected tag of the item.
 * @param[in]  Handle             Item handle, or NULL for current.
 * @param[out] Error              Return code. Not NULL. On success
 *                                DLIST_SUCCESS; on failure one of
 *                                DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                DLIST_EMPTY, DLIST_ITEM_SIZE_WRONG,
 *                                DLIST_ITEM_TAG_WRONG,
 *                                DLIST_BAD_HANDLE.
 *
 * @return The stored data pointer, or NULL on failure. Ownership
 *         passes to the caller.
 *
 * @retval NULL  On failure; see @p Error.
 */
ADDRESS _System ExtractObject(DLIST          ListToGetItemFrom,
                               CARDINAL32     ItemSize,
                               TAG            ItemTag,
                               ADDRESS        Handle,
                               CARDINAL32 *   Error);

/* ==================================================================
 * Replace
 *
 * ReplaceItem replaces the stored data with a copy of the caller's
 * buffer. ReplaceObject replaces the stored pointer with the
 * caller's pointer and returns the previous pointer, whose
 * ownership passes back to the caller.
 * ================================================================== */

/*!
 * @brief Replace the stored data of an item with a copy.
 *
 * @param[in]  ListToReplaceItemIn  List handle.
 * @param[in]  ItemSize             Size of the new data.
 * @param[in]  ItemLocation         Pointer to the new data.
 * @param[in]  ItemTag              New tag.
 * @param[in]  Handle               Item handle, or NULL for current.
 * @param[in]  MakeCurrent          Make the item current on success.
 * @param[out] Error                Return code. Not NULL. On success
 *                                  DLIST_SUCCESS; on failure one of
 *                                  DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                  DLIST_EMPTY,
 *                                  DLIST_ITEM_SIZE_ZERO,
 *                                  DLIST_BAD_ITEM_POINTER,
 *                                  DLIST_BAD_HANDLE,
 *                                  DLIST_OUT_OF_MEMORY.
 */
void _System ReplaceItem(DLIST         ListToReplaceItemIn,
                          CARDINAL32    ItemSize,
                          ADDRESS       ItemLocation,
                          TAG           ItemTag,
                          ADDRESS       Handle,
                          BOOLEAN       MakeCurrent,
                          CARDINAL32 *  Error);

/*!
 * @brief Replace the stored pointer of an item with a caller pointer.
 *
 * @param[in]     ListToReplaceItemIn  List handle.
 * @param[in,out] ItemSize             On input: size of the new
 *                                     data. On output: size of the
 *                                     data that was replaced.
 * @param[in]     ItemLocation         Pointer to the new data.
 * @param[in,out] ItemTag              On input: new tag. On output:
 *                                     tag of the data that was
 *                                     replaced.
 * @param[in]     Handle               Item handle, or NULL for
 *                                     current.
 * @param[in]     MakeCurrent          Make the item current on
 *                                     success.
 * @param[out]    Error                Return code. Not NULL. On
 *                                     success DLIST_SUCCESS; on
 *                                     failure one of DLIST_BAD,
 *                                     DLIST_NOT_INITIALIZED,
 *                                     DLIST_EMPTY,
 *                                     DLIST_ITEM_SIZE_ZERO,
 *                                     DLIST_BAD_ITEM_POINTER,
 *                                     DLIST_BAD_HANDLE.
 *
 * @return Pointer to the data that was replaced, or NULL on
 *         failure. Ownership of the returned pointer passes to the
 *         caller.
 *
 * @retval NULL  On failure; see @p Error.
 */
ADDRESS _System ReplaceObject(DLIST         ListToReplaceItemIn,
                               CARDINAL32 *  ItemSize,
                               ADDRESS       ItemLocation,
                               TAG        *  ItemTag,
                               ADDRESS       Handle,
                               BOOLEAN       MakeCurrent,
                               CARDINAL32 *  Error);

/* ==================================================================
 * Metadata
 * ================================================================== */

/*!
 * @brief Return the tag and size of an item.
 *
 * @param[in]  ListToGetTagFrom  List handle.
 * @param[in]  Handle            Item handle, or NULL for current.
 * @param[out] ItemSize          Size of the item. Not NULL.
 * @param[out] Error             Return code. Not NULL. On success
 *                               DLIST_SUCCESS; on failure one of
 *                               DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                               DLIST_EMPTY, DLIST_BAD_HANDLE.
 *
 * @return The tag of the item, or zero on failure.
 *
 * @retval 0  On failure; see @p Error.
 */
TAG _System GetTag(DLIST        ListToGetTagFrom,
                    ADDRESS      Handle,
                    CARDINAL32 * ItemSize,
                    CARDINAL32 * Error);

/*!
 * @brief Return a handle to the current item.
 *
 * The handle stays valid while the item is in the list and survives
 * any reordering. It becomes invalid once the item is removed.
 *
 * @param[in]  ListToGetHandleFrom  List handle.
 * @param[out] Error                Return code. Not NULL. On success
 *                                  DLIST_SUCCESS; on failure one of
 *                                  DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                  DLIST_EMPTY.
 *
 * @return Handle to the current item, or NULL on failure.
 *
 * @retval NULL  On failure; see @p Error.
 */
ADDRESS _System GetHandle(DLIST        ListToGetHandleFrom,
                           CARDINAL32 * Error);

/*!
 * @brief Return the number of items in the list.
 *
 * @param[in]  ListToGetSizeOf  List handle.
 * @param[out] Error            Return code. Not NULL. On success
 *                              DLIST_SUCCESS; on failure one of
 *                              DLIST_BAD, DLIST_NOT_INITIALIZED.
 *
 * @return The number of items, or zero on failure.
 *
 * @retval 0  On failure; see @p Error.
 */
CARDINAL32 _System GetListSize(DLIST        ListToGetSizeOf,
                                CARDINAL32 * Error);

/*!
 * @brief Test whether the list is empty.
 *
 * @param[in]  ListToCheck  List handle.
 * @param[out] Error        Return code. Not NULL. On success
 *                          DLIST_SUCCESS; on failure one of
 *                          DLIST_BAD, DLIST_NOT_INITIALIZED.
 *
 * @return TRUE if the list is empty, FALSE otherwise.
 *
 * @retval TRUE   The list is empty, or an argument is invalid.
 * @retval FALSE  The list contains at least one item.
 */
BOOLEAN _System ListEmpty(DLIST        ListToCheck,
                           CARDINAL32 * Error);

/*!
 * @brief Test whether the cursor is on the last item.
 *
 * @param[in]  ListToCheck  List handle.
 * @param[out] Error        Return code. Not NULL. On success
 *                          DLIST_SUCCESS; on failure one of
 *                          DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                          DLIST_EMPTY.
 *
 * @return TRUE if the cursor is on the last item, FALSE otherwise.
 *
 * @retval TRUE   The cursor is on the last item.
 * @retval FALSE  Otherwise, or on failure.
 */
BOOLEAN _System AtEndOfList(DLIST        ListToCheck,
                             CARDINAL32 * Error);

/*!
 * @brief Test whether the cursor is on the first item.
 *
 * @param[in]  ListToCheck  List handle.
 * @param[out] Error        Return code. Not NULL. On success
 *                          DLIST_SUCCESS; on failure one of
 *                          DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                          DLIST_EMPTY.
 *
 * @return TRUE if the cursor is on the first item, FALSE otherwise.
 *
 * @retval TRUE   The cursor is on the first item.
 * @retval FALSE  Otherwise, or on failure.
 */
BOOLEAN _System AtStartOfList(DLIST        ListToCheck,
                               CARDINAL32 * Error);

/* ==================================================================
 * Cursor movement
 * ================================================================== */

/*!
 * @brief Move the cursor to the next item.
 *
 * @param[in,out] ListToAdvance  List handle.
 * @param[out]    Error          Return code. Not NULL. On success
 *                               DLIST_SUCCESS; on failure one of
 *                               DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                               DLIST_EMPTY, DLIST_END_OF_LIST.
 */
void _System NextItem(DLIST        ListToAdvance,
                       CARDINAL32 * Error);

/*!
 * @brief Move the cursor to the previous item.
 *
 * @param[in,out] ListToChange  List handle.
 * @param[out]    Error         Return code. Not NULL. On success
 *                              DLIST_SUCCESS; on failure one of
 *                              DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                              DLIST_EMPTY,
 *                              DLIST_ALREADY_AT_START.
 */
void _System PreviousItem(DLIST        ListToChange,
                           CARDINAL32 * Error);

/*!
 * @brief Move the cursor to the first item.
 *
 * @param[in,out] ListToReset  List handle.
 * @param[out]    Error        Return code. Not NULL. On success
 *                             DLIST_SUCCESS; on failure one of
 *                             DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                             DLIST_EMPTY.
 */
void _System GoToStartOfList(DLIST        ListToReset,
                              CARDINAL32 * Error);

/*!
 * @brief Move the cursor to the last item.
 *
 * @param[in,out] ListToSet  List handle.
 * @param[out]    Error      Return code. Not NULL. On success
 *                           DLIST_SUCCESS; on failure one of
 *                           DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                           DLIST_EMPTY.
 */
void _System GoToEndOfList(DLIST        ListToSet,
                            CARDINAL32 * Error);

/*!
 * @brief Move the cursor to the item identified by @p Handle.
 *
 * @param[in,out] ListToReposition  List handle.
 * @param[in]     Handle            Item handle, or NULL for the
 *                                  current item.
 * @param[out]    Error             Return code. Not NULL. On success
 *                                  DLIST_SUCCESS; on failure one of
 *                                  DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                                  DLIST_BAD_HANDLE.
 */
void _System GoToSpecifiedItem(DLIST        ListToReposition,
                                ADDRESS      Handle,
                                CARDINAL32 * Error);

/* ==================================================================
 * Sorting and traversal
 * ================================================================== */

/*!
 * @brief Sort the list in place.
 *
 * The comparison function returns negative, zero or positive
 * following the usual ordering contract. The sort is stable.
 *
 * If the comparison function writes a non-zero value into @p Error,
 * the sort stops and the caller receives that value. In that case
 * the order of the items is undefined.
 *
 * @param[in,out] ListToSort  List handle.
 * @param[in]     Compare     Comparison function.
 * @param[out]    Error       Return code. Not NULL. On success
 *                            DLIST_SUCCESS; on failure one of
 *                            DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                            DLIST_OUT_OF_MEMORY.
 */
void _System SortList(DLIST        ListToSort,
                       INTEGER32 (APIENTRY * Compare)(ADDRESS Object1,
                                                     TAG Object1Tag,
                                                     ADDRESS Object2,
                                                     TAG Object2Tag,
                                                     CARDINAL32 * Error),
                       CARDINAL32 * Error);

/*!
 * @brief Call a user function once for each item.
 *
 * The callback receives the item's data pointer, tag, size, item
 * handle, the caller-supplied @p Parameters value, and the address
 * of the *Error variable.
 *
 * The traversal stops early when the callback writes a non-zero
 * value into *Error. The caller then receives that value, except
 * when it is DLIST_SEARCH_COMPLETE, in which case the caller
 * receives DLIST_SUCCESS. This makes ForEachItem usable as a search.
 *
 * The callback must not modify the list.
 *
 * @param[in]  ListToProcess  List handle.
 * @param[in]  ProcessItem    Callback.
 * @param[in]  Parameters     Value passed to the callback untouched.
 * @param[in]  Forward        TRUE from start to end, FALSE from end
 *                            to start.
 * @param[out] Error          Return code. Not NULL. On success
 *                            DLIST_SUCCESS; on failure one of
 *                            DLIST_BAD, DLIST_NOT_INITIALIZED.
 */
void _System ForEachItem(DLIST        ListToProcess,
                          void (APIENTRY * ProcessItem)(ADDRESS Object,
                                                        TAG ObjectTag,
                                                        CARDINAL32 ObjectSize,
                                                        ADDRESS ObjectHandle,
                                                        ADDRESS Parameters,
                                                        CARDINAL32 * Error),
                          ADDRESS      Parameters,
                          BOOLEAN      Forward,
                          CARDINAL32 * Error);

/*!
 * @brief Examine every item and optionally delete it.
 *
 * The callback returns TRUE if the item should be removed. When it
 * does, the BOOLEAN* parameter it receives decides whether the
 * item's data memory is released by PruneList or left to the
 * caller.
 *
 * The traversal can be aborted the same way as in ForEachItem. The
 * callback must not modify the list directly.
 *
 * @param[in]  ListToProcess  List handle.
 * @param[in]  KillItem       Callback.
 * @param[in]  Parameters     Value passed to the callback untouched.
 * @param[out] Error          Return code. Not NULL. On success
 *                            DLIST_SUCCESS; on failure one of
 *                            DLIST_BAD, DLIST_NOT_INITIALIZED.
 */
void _System PruneList(DLIST        ListToProcess,
                        BOOLEAN (APIENTRY * KillItem)(ADDRESS Object,
                                                      TAG ObjectTag,
                                                      CARDINAL32 ObjectSize,
                                                      ADDRESS ObjectHandle,
                                                      ADDRESS Parameters,
                                                      BOOLEAN * FreeMemory,
                                                      CARDINAL32 * Error),
                        ADDRESS      Parameters,
                        CARDINAL32 * Error);

/* ==================================================================
 * Combination
 * ================================================================== */

/*!
 * @brief Move all items from @p SourceList to the end of
 *        @p TargetList.
 *
 * On success @p SourceList is empty and @p TargetList contains its
 * own items followed by the moved ones. All errors are detected
 * before any item is moved; on failure both lists are unchanged.
 *
 * @param[in,out] TargetList  Target list handle.
 * @param[in,out] SourceList  Source list handle.
 * @param[out]    Error       Return code. Not NULL. On success
 *                            DLIST_SUCCESS; on failure one of
 *                            DLIST_BAD, DLIST_NOT_INITIALIZED.
 */
void _System AppendList(DLIST        TargetList,
                         DLIST        SourceList,
                         CARDINAL32 * Error);

/*!
 * @brief Move a single item between lists.
 *
 * @p SourceHandle identifies the item in @p SourceList (NULL means
 * the current item). @p TargetHandle identifies the reference item
 * in @p TargetList (NULL means the current item, and is not
 * consulted for InsertAtStart or AppendToList). @p TransferMode
 * selects the position inside @p TargetList. @p MakeCurrent selects
 * whether the transferred item becomes current in @p TargetList.
 *
 * @param[in,out] SourceList    Source list handle.
 * @param[in]     SourceHandle  Item handle in the source list, or
 *                              NULL.
 * @param[in,out] TargetList    Target list handle.
 * @param[in]     TargetHandle  Reference item in the target list, or
 *                              NULL.
 * @param[in]     TransferMode  Insertion mode in the target list.
 * @param[in]     MakeCurrent   Make the item current in the target
 *                              list.
 * @param[out]    Error         Return code. Not NULL. On success
 *                              DLIST_SUCCESS; on failure one of
 *                              DLIST_BAD, DLIST_NOT_INITIALIZED,
 *                              DLIST_EMPTY,
 *                              DLIST_INVALID_INSERTION_MODE,
 *                              DLIST_BAD_HANDLE.
 */
void _System TransferItem(DLIST             SourceList,
                           ADDRESS           SourceHandle,
                           DLIST             TargetList,
                           ADDRESS           TargetHandle,
                           Insertion_Modes   TransferMode,
                           BOOLEAN           MakeCurrent,
                           CARDINAL32 *      Error);

/* ==================================================================
 * Integrity
 * ================================================================== */

/*!
 * @brief Verify the internal consistency of a list.
 *
 * Every link is checked, as are the control-block fields. Returns
 * TRUE if the list is consistent, FALSE otherwise. Intended for
 * debugging and tests; the module itself does not call it.
 *
 * @param[in] ListToCheck  List handle.
 *
 * @return TRUE if the list is consistent, FALSE otherwise.
 *
 * @retval TRUE   The list is consistent.
 * @retval FALSE  The list is inconsistent or the handle is invalid.
 */
BOOLEAN _System CheckListIntegrity(DLIST ListToCheck);

#endif /* CCL_DLIST_PRIVATE_H */

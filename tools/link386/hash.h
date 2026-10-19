/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/

/*!
 * @file hash.h
 * @brief Generic chained hash table.
 */

#ifndef _HASH_H
#define _HASH_H

#include <stddef.h>


/*
 * Define some types.
 */

/*!
 * @typedef HashTable
 * @brief Opaque hash table handle.
 */
typedef void *          HashTable;

/*!
 * @typedef HashFunction
 * @brief Hash function over an item.
 * @param[in] item  Item to hash. Not NULL.
 * @return The hash value.
 */
typedef unsigned        (*HashFunction)( const void *item );

/*!
 * @typedef HashCompareFunction
 * @brief Equality predicate for two items.
 * @param[in] item1  First item. Not NULL.
 * @param[in] item2  Second item. Not NULL.
 * @retval 0  Items differ.
 * @retval non-zero  Items are equal.
 * @return Non-zero if the items are equal.
 */
typedef int             (*HashCompareFunction)( const void *item1,
                                                const void *item2 );

/*!
 * @typedef HashWalkCallback
 * @brief Callback invoked for each element visited by WalkBucketHash().
 * @param[in] item    Current item. Not NULL.
 * @param[in] cookie  Caller-supplied context. May be NULL.
 * @retval 0  Stop the walk.
 * @retval non-zero  Continue walking.
 * @return Zero to stop, non-zero to continue.
 */
typedef int             (*HashWalkCallback)( const void *item, void *cookie );


/*
 * Function prototypes.
 */

/*!
 * @brief Create an empty hash table.
 * @param[in] numbuckets   Number of buckets (must be > 0).
 * @param[in] hashfunc     Hash function. Not NULL.
 * @param[in] comparefunc  Equality predicate. Not NULL.
 * @retval NULL Allocation failure.
 * @return The new hash table.
 */
extern HashTable    InitHash( unsigned numbuckets, HashFunction hashfunc,
                              HashCompareFunction comparefunc );

/*!
 * @brief Destroy a hash table.
 * @param[in] _table     Table to destroy. Not NULL.
 * @param[in] freeitems  Non-zero to also free the items.
 */
extern void         FiniHash( HashTable _table, int freeitems );

/*!
 * @brief Insert an item into the hash table. Duplicates are ignored.
 * @param[in,out] table  Hash table. Not NULL.
 * @param[in]     item   Item to insert. Not NULL.
 */
extern void         InsertHash( HashTable *table, const void *item );

/*!
 * @brief Walk the bucket that a given item would hash to.
 * @param[in,out] table     Hash table. Not NULL.
 * @param[in]     item      Item that determines the bucket. Not NULL.
 * @param[in]     callback  Walk callback. Not NULL.
 * @param[in]     cookie    Caller-supplied context. May be NULL.
 */
extern void         WalkBucketHash( HashTable *table, const void *item,
                                    HashWalkCallback callback, void *cookie );


#endif

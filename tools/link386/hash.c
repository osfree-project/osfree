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
 * @file hash.c
 * @brief Generic chained hash table.
 */

#include "error.h"
#include "hash.h"
#include "memory.h"


/*
 * Define some types.
 */

/*!
 * @struct ListElem
 * @brief One element in a hash bucket's chain.
 */
typedef struct _ListElem {
    const void *        data;       /*!< The stored item. */
    struct _ListElem *  next;       /*!< Next chain element, or NULL. */
} ListElem;

/*!
 * @struct Hash
 * @brief Internal hash table state.
 */
typedef struct _Hash {
    unsigned            numbuckets; /*!< Number of buckets. */
    HashFunction        hashfunc;   /*!< Hash function. */
    HashCompareFunction comparefunc;/*!< Equality predicate. */
    ListElem **         buckets;    /*!< Bucket array. */
} Hash;


/*!
 * @brief Test whether an item is present in a given bucket.
 * @param[in] table   Hash table. Not NULL.
 * @param[in] item    Item to test. Not NULL.
 * @param[in] bucket  Bucket index.
 * @retval 0  Not present.
 * @retval 1  Present.
 * @return 1 if the item is already in the bucket.
 */
static int is_in_bucket( const Hash *table, const void *item,
                         unsigned bucket )
/***********************************************************/
{
    ListElem *          curelem;

    curelem = table->buckets[bucket];
    while( curelem != NULL ) {
        if( (*table->comparefunc)( item, (const void*)curelem->data ) ) {
            return( 1 );
        }
        curelem = curelem->next;
    }

    return( 0 );
}


/*!
 * @brief Create an empty hash table.
 * @param[in] numbuckets   Number of buckets (must be > 0).
 * @param[in] hashfunc     Hash function. Not NULL.
 * @param[in] comparefunc  Equality predicate. Not NULL.
 * @retval NULL Allocation failure.
 * @return The new hash table.
 */
HashTable InitHash( unsigned numbuckets, HashFunction hashfunc,
                    HashCompareFunction comparefunc )
/*************************************************************/
{
    Hash *              table;
    unsigned            count;

    if( numbuckets == 0 )  Zoinks();
    table = AllocMem( sizeof( Hash ) );
    table->numbuckets = numbuckets;
    table->hashfunc = hashfunc;
    table->comparefunc = comparefunc;
    table->buckets = AllocMem( numbuckets * sizeof( ListElem* ) );
    for( count=0; count<numbuckets; count++ ) {
        table->buckets[count] = NULL;
    }
    return( table );
}


/*!
 * @brief Destroy a hash table.
 * @param[in] _table     Table to destroy. Not NULL.
 * @param[in] freeitems  Non-zero to also free the items via FreeMem().
 */
void FiniHash( HashTable _table, int freeitems )
/**********************************************/
{
    Hash *              table = (Hash*)_table;
    unsigned            count;
    ListElem *          curelem;
    ListElem *          nextelem;

    if( table == NULL )  Zoinks();
    for( count=0; count<table->numbuckets; count++ ) {
        curelem = table->buckets[count];
        while( curelem != NULL ) {
            nextelem = curelem->next;
            if( freeitems )  FreeMem( (void*)curelem->data );
            FreeMem( curelem );
            curelem = nextelem;
        }
    }
    FreeMem( table );
}


/*!
 * @brief Insert an item into a hash table. Duplicates are ignored.
 * @param[in,out] _table  Hash table. Not NULL.
 * @param[in]     item    Item to insert. Not NULL.
 */
void InsertHash( HashTable *_table, const void *item )
/***************************************************/
{
    Hash *              table = (Hash*)_table;
    unsigned            bucket;
    ListElem *          newelem;

    if( table == NULL )  Zoinks();
    bucket = (*table->hashfunc)( item )  %  table->numbuckets;
    if( !is_in_bucket( table, item, bucket ) ) {
        newelem = AllocMem( sizeof( ListElem ) );
        newelem->data = item;
        newelem->next = table->buckets[bucket];
        table->buckets[bucket] = newelem;
    }
}


/*!
 * @brief Walk the bucket that a given item would hash to.
 * @param[in,out] _table    Hash table. Not NULL.
 * @param[in]     item      Item that determines the bucket. Not NULL.
 * @param[in]     callback  Walk callback. Not NULL.
 * @param[in]     cookie    Caller-supplied context. May be NULL.
 */
void WalkBucketHash( HashTable *_table, const void *item,
                     HashWalkCallback callback, void *cookie )
/************************************************************/
{
    Hash *              table = (Hash*)_table;
    unsigned            bucket;
    ListElem *          curelem;
    int                 rc;

    if( table == NULL )  Zoinks();
    bucket = (*table->hashfunc)( item )  %  table->numbuckets;
    curelem = table->buckets[bucket];

    while( curelem != NULL ) {
        rc = (*callback)( curelem->data, cookie );
        if( rc == 0 )  break;
        curelem = curelem->next;
    }
}

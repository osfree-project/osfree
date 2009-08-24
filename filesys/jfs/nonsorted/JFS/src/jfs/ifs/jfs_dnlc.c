/* $Id: jfs_dnlc.c,v 1.2 2003/09/21 08:42:54 pasha Exp $ */

static char *SCCSID = "@(#)1.14  9/13/99 15:00:47 src/jfs/ifs/jfs_dnlc.c, sysjfs, w45.fs32, fixbld";
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
 *
*/
/*
 *  jfs_dnlc.c: directory name lookup cache manager
 *
 * set associative directory name lookup cache;
 *
 * dnlc is advisory cache mapping
 *  (directory inode number, entry name) -> entry inode number, i.e.,
 * the mapping by dnlc must be correct, but
 * there may not be entry corresponding to valid inode in inode cache;
 *
 * dnlc returns inode number which requires inode cache
 * search - but obviate to invalidate dnlc entry when
 * inode is recycled (no name to search dnlc);
 *
 * serialization: per hash anchor simple exclusive lock;
 *
 * TODO: dnlc return inode pointer;
 */

/*
 * Change History :
 * PS 2001-2003 apply IBM fixes
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */
#include "mmph.h"

#include "jfs_types.h"
#include "jfs_lock.h"
#include "jfs_filsys.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_dnlc.h"
#include "jfs_debug.h"

#ifdef  _JFS_UNICODE
#include <uni_inln.h>
#else   /* ! _JFS_UNICODE */
#define UniStrncmp memcmp
#define UniStrncpy(dest, src, len) bcopy(src, dest, len)
#endif  /* _JFS_UNICODE */

/*
 *  dnlc entry
 */
#define DNLCNAMEMAX 15  /* max cached name size */

typedef struct ncache {
    struct ncache   *next;  /* 4: next buffer in hash/lrulist */
    struct ncache   *prev;  /* 4: prev buffer in hash/lrulist */

    inode_t *fileset;   /* 4: fileset handle */

    ino_t   dino;       /* 4: parent directory inode # */
    uint32  dcid;       /* 4: capability of dino (rsrvd) */

    ino_t   ino;        /* 4: named inode # */
    inode_t *ip;        /* 4: */
    uint32  cid;        /* 4: capability of ino (rsrvd) */
    int16   namlen;     /* 2: name length */
    UniChar name[DNLCNAMEMAX]; /* 30: component name */
} ncache_t;         /* (64) */

extern uint32   openMax;
int32       dnlc_cachesize;     /* number of cache entries */
ncache_t    *dnlc_cache;        /* dnlc entry pool */

/*
 *  dnlc hash/lru list anchor table
 *
 * dnlc hash list anchor table contains dnlc_hashsize elements.
 * dnlc_hashsize is scaled by the size of real memory.
 */
typedef struct nhash {
    struct ncache   *head;  /* 4: head buffer in hashlist */
    struct ncache   *tail;  /* 4: tail buffer in hashlist */

    uint32  timestamp;  /* 4: */
    SPINLOCK_T  lock;   /* 4: */
} nhash_t;          /* (16) */

int32   dnlc_hashlistsize = 3;  /* # of cache element per hashlist */
int32   dnlc_hashsize;      /* # of hash/lru list anchors */
int32   modHashSize;        /* dnlc_hashsize - 1 */
nhash_t *dnlc_hash;     /* hash/lru list anchor table */

/* dnlc queue management macro */
#define HASHLIST_INSERT_HEAD(anchor, e)\
{\
    (e)->next = (anchor)->head;\
    (e)->prev = (struct ncache *)(anchor);\
    (anchor)->head->prev = e;\
    (anchor)->head = e;\
}

#define HASHLIST_INSERT_TAIL(anchor, e)\
{\
    (e)->next = (struct ncache *)(anchor);\
    (e)->prev = (anchor)->tail;\
    (anchor)->tail->next = e;\
    (anchor)->tail = e;\
}

#define HASHLIST_REMOVE(e)\
{\
    (e)->prev->next = (e)->next;\
    (e)->next->prev = (e)->prev;\
}

#define NULL_HASH(e)\
    ((ncache_t *)(e))->next = ((ncache_t *)(e))->prev = ((ncache_t *)(e))

/*
 * name cache hashing algorithm
 *
 * based on Pearson,P.K. Fast Hashing of Variable-Length Text Strings.
 * CACM, 33, 6 (June 1990), 677-680
 */
/* Pearson's string hash algorithm random permutation table */
unsigned char table[256] = {
0x8d, 0x23, 0x12, 0x6b, 0x4b, 0x1f, 0x4e, 0x05,
0x54, 0xcb, 0xbd, 0xd4, 0xb3, 0x69, 0xe1, 0x91,
0x7e, 0xaf, 0x31, 0x6e, 0x56, 0x72, 0x83, 0x8a,
0x67, 0x66, 0x36, 0x96, 0xaa, 0x42, 0x73, 0x3d,
0x29, 0xc1, 0xde, 0xe8, 0xa7, 0xfd, 0xa1, 0xd8,
0x1e, 0x41, 0x13, 0x2f, 0x37, 0xae, 0xb4, 0x97,
0xda, 0x6c, 0x75, 0xfc, 0xcc, 0xe3, 0x7d, 0x76,
0x8e, 0x02, 0xa0, 0x61, 0x8f, 0x63, 0x17, 0x0f,
0xee, 0x8c, 0x78, 0x4d, 0xf7, 0xfa, 0xc4, 0x48,
0x6a, 0x53, 0x86, 0x38, 0xdc, 0x08, 0x4f, 0xc8,
0xff, 0x21, 0xf8, 0x5e, 0xea, 0x07, 0x28, 0x7f,
0x58, 0x93, 0x2e, 0xc5, 0x8b, 0x4c, 0x50, 0xcd,
0xb6, 0x99, 0x2b, 0x80, 0x74, 0xd3, 0xf9, 0xbc,
0xa8, 0x9f, 0x20, 0x34, 0xc2, 0xd0, 0x22, 0x33,
0xd6, 0x01, 0xe7, 0x3c, 0xa9, 0x79, 0x1d, 0x81,
0x9c, 0xe5, 0xbe, 0x95, 0x89, 0x3e, 0xc9, 0xb1,
0x46, 0x5f, 0x88, 0x87, 0x45, 0xef, 0xac, 0xad,
0x6f, 0xb2, 0x47, 0xf6, 0xf2, 0xa2, 0x5a, 0x18,
0xb8, 0x9a, 0xe9, 0x2d, 0x39, 0x15, 0x59, 0x5b,
0x98, 0x1b, 0x16, 0xc0, 0xb7, 0xa5, 0x24, 0xce,
0x06, 0x25, 0x0a, 0x51, 0x70, 0xdd, 0xed, 0x40,
0x71, 0xd1, 0x3f, 0x1a, 0x03, 0xc3, 0xfb, 0xbf,
0xcf, 0x0e, 0x94, 0xdb, 0xca, 0xe0, 0x27, 0xf0,
0xf3, 0x11, 0x5c, 0xc6, 0x9d, 0xbb, 0x35, 0x85,
0xab, 0x14, 0x77, 0xf5, 0xd5, 0xec, 0x30, 0xb0,
0x6d, 0xa3, 0x65, 0xba, 0xb5, 0xc7, 0x9e, 0x3a,
0x04, 0x4a, 0x44, 0x90, 0xdf, 0x7a, 0x92, 0xf1,
0x0b, 0xe2, 0xa6, 0x7c, 0x60, 0x0c, 0x55, 0x0d,
0x57, 0x2a, 0x9b, 0x84, 0xf4, 0x19, 0xd2, 0x10,
0x7b, 0xe4, 0x62, 0xd7, 0x00, 0x5d, 0xe6, 0x2c,
0x49, 0x09, 0x43, 0x68, 0x52, 0xd9, 0x3b, 0x1c,
0x32, 0xfe, 0xeb, 0x82, 0x26, 0xb9, 0x64, 0xa4
};

/* Pearson's string hash algorithm */
#define NHASH(fileset, dino, name, hp)\
{\
    int32   i, c, even, odd;\
    UniChar *   cp;\
                        \
    even = odd = 0;\
    cp = name;\
    while ((c = *cp++) != 0)\
    {\
        even = table[even ^ c];\
        if ((c = *cp++) == 0)\
            break;\
        odd = table[odd ^ c];\
    }\
    i = (odd << 8) | even;\
    i += (int32)fileset + (int32)dino;\
    hp = dnlc_hash + (i & modHashSize);\
}

/*
 *  dnlc lock
 */
#define NHASH_LOCK_ALLOC(nhp)\
    SPINLOCK_ALLOC(&nhp->lock,LOCK_ALLOC_PAGED,JFS_NCACHE_LOCK_CLASS,i)
#define NHASH_LOCK_INIT(nhp)    SPINLOCK_INIT(&nhp->lock)
#define NHASH_LOCK(nhp)     SPINLOCK_LOCK(&nhp->lock)
#define NHASH_UNLOCK(nhp)   SPINLOCK_UNLOCK(&nhp->lock)


#ifdef  _JFS_STATISTICS
/*
 *  statistics
 */
static struct statNCache {
    uint32  lookups;    /* 4: dnlc lookup */
    uint32  hits;       /* 4: dnlc hit */
    uint32  misses;     /* 4: dnlc miss */
    uint32  enters;     /* 4: dnlc enter */
    uint32  deletes;    /* 4: dnlc delete */
    uint32  nam2long;   /* 4: name length > DNLCNAMEMAX */
} statNCache = {0};
#endif  /* _JFS_STATISTICS */


#ifdef _JFS_OS2
/*
 *  nameToUpper()
 *
 * function: convert to uppercase
 */
static void
nameToUpper(UniChar * name, uint32 namlen)
{
    UniChar *s;
    int32   i;

    for (i = 0, s = name; i < namlen; i++, s++)
#ifdef _JFS_UNICODE
        *s = UniToupper(*s);
#else   /* ! _JFS_UNICODE */
        if (*s >= 'a' && *s <= 'z')
            *s = *s - 'a' + 'A';
#endif  /* _JFS_UNICODE */
    *s = '\0';
}
#endif  /* _JFS_OS2 */


/*
 * NAME:    ncInit()
 *
 * FUNCTION:    initialize dnlc at jfs initialization.
 *
 * PARMETER:    None
 *
 * RETURN:  None
 */
ncInit()
{
    nhash_t     *nhp;
    ncache_t    *ncp;
    int32       i, k;
    uint32      memsize;

    /*
     * allocate set-associative cache:
     */
    /* scale the number of dnlc elements with max number of inodes */
    dnlc_cachesize = openMax;
    dnlc_hashsize = dnlc_cachesize / dnlc_hashlistsize;
    /* round up to next smallest power of 2 */
    dnlc_hashsize = roundup2power(dnlc_hashsize);
    modHashSize = dnlc_hashsize - 1;
    /* resize for set-associative cache */
    dnlc_cachesize = dnlc_hashsize * dnlc_hashlistsize;

    /* allocate dnlc entry pool */
    dnlc_cache = (ncache_t *)xmalloc(dnlc_cachesize * sizeof(ncache_t),
                    0, kernel_heap|XMALLOC_HIGH);
    if (dnlc_cache == NULL)
        return ENOMEM;
    bzero(dnlc_cache, dnlc_cachesize * sizeof(ncache_t));

    /*
     * allocate hashlist anchor table:
     */
    dnlc_hash = (nhash_t *) xmalloc(dnlc_hashsize * sizeof(nhash_t),
                    0, kernel_heap|XMALLOC_HIGH);
    if (dnlc_hash == NULL)
        return ENOMEM;

    /*
     * initialize set-associative cache
     */
    nhp = dnlc_hash;
    ncp = dnlc_cache;
    /* initialize hash/lru anchors */
    for (i = 0; i < dnlc_hashsize; i++, nhp++)
    {
        NULL_HASH(nhp);

        /* insert entries on the hash/lrulist */
        for (k = 0; k < dnlc_hashlistsize; k++, ncp++)
        {
            ncp->fileset = NULL;

            HASHLIST_INSERT_TAIL(nhp, ncp);
        }

        nhp->timestamp = 0;

        /* allocate/initialize hash anchor lock */
            NHASH_LOCK_ALLOC(nhp);
            NHASH_LOCK_INIT(nhp);
    }

jFYI(1,("nCache: nCacheSize:%d nHashClass:%d\n",
    dnlc_cachesize, dnlc_hashsize));
    return 0;
}


/*
 * NAME:    ncGrow()
 *
 * FUNCTION:    dynamically expand the size of the dnlc.
 *
 * RETURNS:
 *  None
 */
void
ncGrow()
{
    /* maybe someday ... */
    return;
}


/*
 * NAME:    ncSearch()
 *
 * FUNCTION:    internal dnlc search function
 *
 * PARMETER:    fileset - parent dev number
 *      dino    - parent inode number
 *      name    - component name
 *      namlen  - component name length
 *
 * RETURN:  ncache_t if it exists
 *
 * SERIALIZATION: NHASH_LOCK is held on entry/exit
 */
static
ncache_t *
ncSearch(
    nhash_t *nhp,       /* Ptr to hashlist pointer */
    inode_t *fileset,   /* parent directory dev */
    ino_t   dino,       /* parent directory inode # */
    UniChar *   name,       /* Name to hash */
    int32   namlen)     /* Name length */
{
    ncache_t    *ncp;       /* cache element */

   MMPHPrencSearch();      /* MMPH Performance Hook */

    /* search the hashlist */
    for (ncp = nhp->head; ncp != (ncache_t *) nhp; ncp = ncp->next)
    {
        if (ncp->dino == dino &&
            ncp->fileset == fileset &&
            ncp->namlen == namlen &&
            *name == *ncp->name &&
            name[namlen - 1] == ncp->name[ncp->namlen - 1] &&
            UniStrncmp(ncp->name, name, namlen) == 0)
        {
            /*
             * cache hit
             *
             * remove it from hashlist (caller will insert)
             */
            HASHLIST_REMOVE(ncp);
         MMPHPostncSearch();      /* MMPH Performance Hook */
            return ncp;
        }
        else if (ncp->fileset == NULL)
            break;
    }

    /*
     * cache miss
     */
    // return NULL;
   ncp = NULL;
   MMPHPostncSearch();      /* MMPH Performance Hook */

    return(ncp);
}


/*
 * NAME:    ncLookup()
 *
 * FUNCTION:    dnlc lookup;
 *
 * PARMETER:    fileset - parent dev number
 *      dino    - parent inode number
 *      nmp - Pathname argument
 *      cookie  - hashlist anchor (out)
 *
 * RETURN:  0 - name not found
 *      1 - name found *fhp initialized
 */
ino_t
ncLookup(
    inode_t     *fileset,   /* Parent directory dev # */
    ino_t       dino,       /* Parent directory ino # */
    component_t *nmp,       /* component name */
    ncookie_t   *cookie)    /* dnlc cookie */
{
    nhash_t     *nhp;       /* hashlist anchor */
    ncache_t    *ncp;       /* cache element */
    ino_t       ino = 0;    /* return value */
    UniChar     *name;
#ifdef  _JFS_OS2
    UniChar     ci_name[DNLCNAMEMAX+1]; /* case folded name */
#endif  /* _JFS_OS2 */

    /* is name too long ? */
    if (nmp->namlen > DNLCNAMEMAX)
    {
#ifdef FreeJFS
        INCREMENT(statNCache.nam2long);
#endif
        return 0;
    }

#ifdef FreeJFS
    INCREMENT(statNCache.lookups);
#endif

#ifdef  _JFS_OS2
    /* copy the name to local buffer and make it all upper case
     * for case insensitive searching.
     */
    UniStrncpy(ci_name, nmp->name, nmp->namlen);
    nameToUpper(ci_name, nmp->namlen);
    name = ci_name;
#endif  /* _JFS_OS2 */

    /* compute the hash anchor for this name */
    NHASH(fileset, dino, name, nhp);

    NHASH_LOCK(nhp);

    ncp = ncSearch(nhp, fileset, dino, name, nmp->namlen);
    if (ncp)
    {
        /*
         * cache hit:
         *
         * move to head of hashlist (removed by search)
         */
        HASHLIST_INSERT_HEAD(nhp, ncp);

        ino = ncp->ino;
#ifdef FreeJFS
        INCREMENT(statNCache.hits);
#endif
    }
    else
    {
        /*
         * cache miss:
         */
        /* save/return hash anchor stat in dnlc cookie */
        cookie->nhp = nhp;
        cookie->timestamp = nhp->timestamp;

#ifdef FreeJFS
        INCREMENT(statNCache.misses);
#endif
    }

    NHASH_UNLOCK(nhp);

    return ino;
}


/*
 * NAME:    ncEnter()
 *
 * FUNCTION:    insert entry for specified name;
 *
 * PARMETER:    dev - parent dev number
 *      dino    - parent inode number
 *      nmp - Pathname argument
 *      ino - inode number to cache
 *      cookie  - hashlist header
 *
 * NOTE: only called by jfs_lookup() after ncLookup()
 * under multiple readers lock on parent directory inode;
 */
void
ncEnter(
    inode_t *fileset,   /* Parent directory dev # */
    ino_t   dino,       /* Parent directory ino # */
    component_t *nmp,   /* component name */
    ino_t   ino,        /* component fsn */
    ncookie_t *cookie)  /* dnlc cookie */
{
    nhash_t     *nhp;   /* hashlist header */
    ncache_t    *ncp;   /* cache element */
    UniChar     *name;
#ifdef  _JFS_OS2
    UniChar     ci_name[DNLCNAMEMAX+1]; /* case folded name */
#endif  /* _JFS_OS2 */

    /* is name too long ? */
    if (nmp->namlen > DNLCNAMEMAX)
        return;
#ifdef FreeJFS
    INCREMENT(statNCache.enters);
#endif

    /* copy the name to local buffer and make it all upper case
     * for case insensitive searching.
     */
    UniStrncpy(ci_name, nmp->name, nmp->namlen);
    nameToUpper(ci_name, nmp->namlen);
    name = ci_name;

    /* compute the hash anchor for new name */
    if (cookie)
        nhp = cookie->nhp;
    else
        NHASH(fileset, dino, name, nhp);

    NHASH_LOCK(nhp);

    /*
     * check for duplicate entry from race by multiple readers
     * of directory lookup
     */
    if (cookie && nhp->timestamp != cookie->timestamp)
    {
        ncp = ncSearch(nhp, fileset, dino, name, nmp->namlen);
        if (ncp)
        {
            /*
             * cache hit (duplicate found):
             *
             * move to head of hashlist (removed by search)
             */
            HASHLIST_INSERT_HEAD(nhp, ncp);

            NHASH_UNLOCK(nhp);
            return;
        }
    }

    /* recycle least recently used entry at tail of hashlist */
    ncp = nhp->tail;
    HASHLIST_REMOVE(ncp);

    /* initialize entry */
    ncp->fileset = fileset;
    ncp->dino = dino;
    ncp->ino = ino;
    ncp->namlen = nmp->namlen;
    UniStrncpy(ncp->name, name, nmp->namlen);

    /* insert at head of hashlist */
    HASHLIST_INSERT_HEAD(nhp, ncp);

    nhp->timestamp++;

    NHASH_UNLOCK(nhp);
}


/*
 * NAME:    ncDelete()
 *
 * FUNCTION:    invalidate entry for the specified name;
 *      and made available for re-use.
 *
 * PARMETER:    fileset - parent dev number
 *      dino    - parent inode number
 *      nmp - Pathname argument
 *
 * RETURN:  None
 */
void
ncDelete(
    inode_t *fileset,   /* Parent directory dev # */
    ino_t dino,     /* Parent directory ino # */
    component_t *nmp)   /* component name */
{
    nhash_t     *nhp;       /* hashlist header */
    ncache_t    *ncp;       /* cache element */
    UniChar     *name;
#ifdef  _JFS_OS2
    UniChar     ci_name[DNLCNAMEMAX+1]; /* case folded name */
#endif  /* _JFS_OS2 */

    /* is name too long ? */
    if (nmp->namlen > DNLCNAMEMAX)
        return;

#ifdef FreeJFs
    INCREMENT(statNCache.deletes);
#endif

    /* copy the name to local buffer and make it all upper case
     * for case insensitive searching.
     */
    UniStrncpy(ci_name, nmp->name, nmp->namlen);
    nameToUpper(ci_name, nmp->namlen);
    name = ci_name;

    /* compute the hash anchor for this name */
    NHASH(fileset, dino, name, nhp);

    NHASH_LOCK(nhp);

    ncp = ncSearch(nhp, fileset, dino, name, nmp->namlen);
    if (ncp)
    {
        /*
         * cache hit:
         */
        /* invalidate entry */
        ncp->fileset = NULL;

        /* move to tail of hashlist (removed by search) */
        HASHLIST_INSERT_TAIL(nhp, ncp);
    }

    NHASH_UNLOCK(nhp);
}


/*
 * NAME:    ncUnmount()
 *
 * FUNCTION:    invalidate entry associated with the device
 *      being unmounted at umount() time.
 *
 * PARMETER:    fileset - device to purge
 *
 * RETURN:  None
 */
void
ncUnmount(
    inode_t *fileset)
{
    nhash_t     *nhp;       /* hashlist header */
    ncache_t    *ncp;       /* cache entry */
    ncache_t    *nxt;       /* cache entry */
    int32       i;

    nhp = dnlc_hash;

    /* scan each hash list */
    for (i = 0; i < dnlc_hashsize; i++, nhp++)
    {
        NHASH_LOCK(nhp);

        nxt = nhp->head;
        /* scan the hashlist */
        while (nxt != (ncache_t *)nhp)
        {
            ncp = nxt;
            nxt = ncp->next;

            if (ncp->fileset == fileset)
            {
                /* invalidate entry */
                ncp->fileset = NULL;

                /* move to tail of hashlist */
                HASHLIST_REMOVE(ncp);
                HASHLIST_INSERT_TAIL(nhp, ncp);
            }
        }

        NHASH_UNLOCK(nhp);
    }
}


#ifdef drova  //  _JFS_STATISTICS
/*
 *  ncStatistics()
 */
ncStatistics(
    caddr_t arg,
    int32   flag)
{
    int32   rc;

    /* copy out the argument */
    if (rc = copyout((caddr_t)&statNCache, (caddr_t)arg,
             sizeof(struct statNCache)))
        return rc;

    /* reset the counters */
    if (flag)
    {
        statNCache.lookups = 0;
        statNCache.hits = 0;
        statNCache.misses = 0;
        statNCache.enters = 0;
        statNCache.deletes = 0;
        statNCache.nam2long = 0;
    }

    return 0;
}
#endif  /* _JFS_STATISTICS */

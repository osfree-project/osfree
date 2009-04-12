/* $Id: bufpool.c,v 1.1.1.1 2003/05/21 13:41:40 pasha Exp $ */

static char *SCCSID = "@(#)1.2  10/4/96 10:14:38 src/jfs/utils/libfs/bufpool.c, jfslib, w45.fs32, 990417.1";
static char sccsid[] = "@(#)bufpool.c	1.8  @(#)bufpool.c	1.8 3/25/96 12:47:23";
/*
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
 *   COMPONENT_NAME: cmdufs
 *
 *   FUNCTIONS: BHASH
 *		INSQUE
 *		REMQUE
 *		bpassert
 *		bpclose
 *		bpflush
 *		bpinit
 *		bpread
 *		bprelease
 *		bptouch
 *		buf_stat
 *		bufhashstats
 *		bufstats
 *		bwritebuf
 *		fhdr
 *		findhdr
 *		grow_dynamic
 *		hdr_nmbr
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jfs_types.h"
#include <os2.h>
#include <jfs_aixisms.h>
#include "jfs_filsys.h"
#include "jfs_dinode.h"
#include "devices.h"
#include "bufpool.h"

/*
 * Buffer pool for filesystem disk blocks
 * for k > 0 Bufhdr[k] describes contents of Buffer[k-1].
 * Bufhdr[0] is used as anchor for free list. when a
 * Buffer is needed, Bufhdr[0].prev is the Buffer selected.
 */


/*
 * header for Buffer pool (note: hnext and hprev MUST be first in order
 * to match HASHHDR...  this lets all the nifty macros work!)
 */
#define	BUFHDR	struct bufhdr
BUFHDR
{
	BUFHDR	*hnext;		/* next on hash chain		*/
	BUFHDR	*hprev;		/* previous on hash chain	*/
	BUFHDR	*fnext;		/* next on free list		*/
	BUFHDR	*fprev;		/* previous on free list	*/

	char	 modify;	/* Buffer modified?		*/
	char	 hashed;	/* on hash list?		*/
	int	 count;		/* reference count		*/
	uint64	 bn;		/* block number		*/
	void	*buf;		/* actual data			*/
};

/*
 * hash chain hdr
 */
#define	HASHHDR	struct bhash
HASHHDR {
	BUFHDR	*hnext;		/* next on hash chain		*/
	BUFHDR	*hprev;		/* previous on hash chain	*/
	};

/*
 * constants
 */
#define NBUFPOOL	256	/* sizeof Buffer pool. */
#define	NHASH		512	/* sizeof hash table */

/*
 * hash function (assumes NHASH is a power of 2)
 */
#define	BHASH(bn)		((bn) & (NHASH - 1))

/*
 * static (private) data
 */
static BUFHDR Bufhdr[NBUFPOOL];	/* buffer pool headers */
static HASHHDR bhash[NHASH];	/* hash chain headers */
static char *Buffer;		/* actual buffers */
static int Bdfd = -1;	/* Buffer device file descriptor */
static void (*Rwerr)();		/* read/write error function */
static uint32	fs_block_size;	/* Number of bytes per aggregate block */
static uint32	phys_block_size;	/* Number of bytes per physical block */


/*
 * take node 'node' from the queue defined by 'next' and 'prev'
 */
#define	REMQUE(node, next, prev) \
	(node)->next->prev = (node)->prev, (node)->prev->next = (node)->next

/*
 * insert 'second' behind 'first' in the queue defined by 'next' and 'prev'
 */
#define	INSQUE(first, second, next, prev, type) \
	(second)->next = (first)->next,		\
	(second)->prev = (type) (first),	\
	(first)->next->prev = (type) (second),	\
	(first)->next = (type) (second)

/*
 * 'dynamic' buffer pool
 */
#define	DYNAMIC struct dynamic
DYNAMIC {
	BUFHDR	 bufhdr;
	DYNAMIC	*next;
	};

static DYNAMIC *Dynamic = NULL;	/* Dynamic buffer pool itself */

/*
 * NAME: bufhashstats
 *                                                                    
 * FUNCTION: Print out head of hash chains
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS: NONE
 *
 * NOTES: For debug use.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
static void bufhashstats( void )
{
    int i;

    for( i = 0; i < NHASH; i++ ) {
	BPPRINTF(("hash %d: %x, prev = %x, next = %x\n", i, &bhash[i],
		 bhash[i].hprev, bhash[i].hnext));
    }
}


/*
 * NAME: bpinit
 *                                                                    
 * FUNCTION: Initialize buffer pool
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS: After calling this function the buffer pool is ready for use
 *
 * PARAMETERS:
 *	dev_ptr	- Device buffer pool should use for read/write
 *	rwerr	- Function to call in case of read/write error
 *	blksize	- Block size for aggregate
 *	phys_blksize	- Block size for physical block
 *
 * NOTES: This function initializes file static variables used by the buffer
 *	pool.  This function must be called before actually attempting to use
 *	the buffer pool.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
void bpinit( int	dev_ptr,
	     void	(*rwerr)(),
	     uint32	blksize,
	     uint32	phys_blksize )
{
    BUFHDR *hdr;
    HASHHDR *hash;

    BPPRINTF(("bpinit()...\n"));

    /*
     * Allocate space for buffers - each buffer is size of an aggregate block
     * We need enough buffers to correspond to the hash table
     */
    Buffer = malloc( blksize * (NBUFPOOL - 1) * sizeof(char) );

    /*
     * initialize free list
     */
    for( hdr = &Bufhdr[0]; hdr < &Bufhdr[NBUFPOOL]; hdr++ ) {
	if( hdr > Bufhdr ) {
	    hdr->buf = &Buffer[(hdr - Bufhdr - 1) * blksize];
	    hdr->fprev = hdr - 1;
	}
	hdr->fnext = hdr + 1;
	hdr->count = 0;
	hdr->modify = 0;
	hdr->hashed = 0;
    }
    Bufhdr[0].fprev = &Bufhdr[NBUFPOOL - 1];
    Bufhdr[NBUFPOOL - 1].fnext = &Bufhdr[0];

    /*
     * initialize hash list
     */
    for( hash = &bhash[0]; hash < &bhash[NHASH]; hash++ ) {
	hash->hprev = hash->hnext = (BUFHDR *) hash;
    }

    /*
     * save off file descriptor, read/write error function, and block size
     */
    Bdfd = dev_ptr;
    Rwerr = rwerr;
    fs_block_size = blksize;
    phys_block_size = phys_blksize;

#ifdef BPDEBUG
    bufhashstats();
#endif
}


/*
 * NAME: fhdr
 *                                                                    
 * FUNCTION: Return the bufhdr for addr from the dynamic list if found.
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	addr	- Find buffer header corresponding to this address buffer
 *	indx	- Fill in with the index of the bufhdr in the dynamic list.
 *		  Filled in with -1 if not found.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Buffer header corresponding to the specified buffer address from the
 *	dynamic buffer pool. NULL if not found.
 */
static BUFHDR *fhdr( void *addr,
		     int *indx )
{
    int ct;
    BUFHDR *hdr = NULL;
    DYNAMIC *dyn;

    for( ct = 0, dyn = Dynamic; dyn != NULL; dyn = dyn->next, ct++ ) {
	hdr = &dyn->bufhdr;
	if( hdr->buf <= addr &&
	    (char *)addr < (char *)hdr->buf + fs_block_size )
	    break;
    }

    if( dyn == NULL ) {
	*indx = -1;
	return( NULL );
    }

    *indx = ct;

    return( hdr );
}


/*
 * NAME: findhdr
 *                                                                    
 * FUNCTION: Take an address and return the bufhdr
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	addr	- Find the corresponding header for this buffer
 *
 * NOTES: For RAS may want to verify the header we find is actually the right
 *	one for the buffer we are asked about.  (Check hdr->buf == addr )
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 */
static BUFHDR *findhdr( void *addr )
{
    int		ct;
    int		bufno = ((char *)addr - Buffer) / fs_block_size + 1;

    /*
     * Check if the buffer is part of the static buffer pool
     */
    if( bufno > 0 && bufno < NBUFPOOL )
	return( &Bufhdr[bufno] );

    /*
     * Otherwise, check if the buffer is part of the dynamic buffer pool
     */
    return( fhdr( addr, &ct ) );
}


/*
 * NAME: hdr_nmbr
 *                                                                    
 * FUNCTION: Return a header number for this buffer header
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	hdr	- Return number of this buffer header
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Character string of the number for this buffer header
 */
static char *hdr_nmbr( BUFHDR *hdr )
{
    int nmbr;
    static char buf[20];

    nmbr = hdr - Bufhdr;
    if( nmbr >= 0 && nmbr < NBUFPOOL ) {
	sprintf(buf, "%d", nmbr);
    } else if( fhdr(hdr->buf, &nmbr) != NULL ) {
	sprintf(buf, "%d+", nmbr);
    } else {
	strcpy(buf, "?");
    }

    return (buf);
}


/*
 * NAME: bwritebuf
 *                                                                    
 * FUNCTION: Write specified Buffer to disk
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	hdr	- Buffer to be written to disk
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
static void bwritebuf( BUFHDR *hdr )
{
    BPPRINTF(("bwrite(%s)\n", hdr_nmbr(hdr)));

    /*
     * write it out
     */
    if( ujfs_rw_diskblocks( Bdfd, hdr->bn * fs_block_size, fs_block_size,
			    hdr->buf, PUT ) != 0 ) {
	(*Rwerr)(WRITE_ERR, hdr->bn);
    }

    hdr->modify = 0;
}


/*
 * NAME: grow_dynamic
 *                                                                    
 * FUNCTION: Grow buffer pool and hash list dynamically by DYN_BLOCKS
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS: NONE
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
static void grow_dynamic( void )
{
    char *bufs;
    BUFHDR *hdr;
    int i;
    DYNAMIC *dyn, *block;

#define	DYN_BLOCKS	32	/* # of dynamic blocks to grow at a time */

    /*
     * alloc DYN_BLOCKS buffers at a time...
     */
    block = malloc( DYN_BLOCKS * sizeof(*block) );
    bufs = malloc( DYN_BLOCKS * fs_block_size );
    if( block != NULL && bufs != NULL ) {
	/*
	 * walk thru & initialize the new ones
	 */
	for( i = 0; i < DYN_BLOCKS; i++ ) {
	    dyn = block + i;
	    hdr = &dyn->bufhdr;

	    /*
	     * init new bufhdr
	     */
	    hdr->buf = bufs + (fs_block_size * i);
	    hdr->modify = 0;
	    hdr->hashed = 0;
	    hdr->count = 0;

	    /*
	     * stick it onto the free list
	     */
	    INSQUE(&Bufhdr[0], hdr, fnext, fprev, BUFHDR *);

	    /*
	     * link it into the Dynamic list
	     */
	    dyn->next = Dynamic;
	    Dynamic = dyn;
	}
    }

    /*
     * we do no error recovery here since we know the calling
     * function will note that the free list never grew - they
     * will bomb out somehow
     */
}


/*
 * NAME: bpread
 *                                                                    
 * FUNCTION: Return pointer of page in Buffer pool containing disk block
 *	specified
 *                                                                    
 * PRE CONDITIONS: Details of any conditions this function
 *	expects for correct operation.
 *
 * POST CONDITIONS: Details of any conditions after this
 *	function executes.
 *
 * PARAMETERS:
 *	bn	- Physical block number to read
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Pointer to buffer containing disk blocks read
 */
void *bpread( uint64 bn )
{
    BUFHDR	*hdr;
    HASHHDR	*hash;
    char	*cp;

    BPPRINTF(("bpread(addr=%lld): ", bn));

    /*
     * search hash chain for the block
     */
    hash = &bhash[BHASH(bn)];
    for( hdr = hash->hnext; hdr != (BUFHDR *)hash && (hdr->bn != bn);
	 hdr = hdr->hnext) {
	BPPRINTF(("hash: looking at %s\n", hdr_nmbr(hdr)));
    }

    /*
     * If found return that buffer
     */
    if( hdr != (BUFHDR *) hash ) {
	/*
	 * If ref count was 0, remove from the free list;
	 * Increment reference count
	 */
	if( hdr->count == 0 )
	    REMQUE(hdr, fnext, fprev);
	hdr->count++;

	BPPRINTF(("increment...\n"));

	return (hdr->buf);
    }

    /*
     * if not found, use the free list (Bufhdr[0].fprev)
     */
    hdr = Bufhdr[0].fprev;
    if( hdr == &Bufhdr[0] ) {
	/*
	 * try to get some dynamic space...
	 */
	grow_dynamic();

	/*
	 * try it again
	 */
	hdr = Bufhdr[0].fprev;
	if( hdr == &Bufhdr[0] ) {
	    BPPRINTF(("bpread: no free blocks left.  help!\n"));
	    abort();
	    return (NULL);	/* shouldn't get this far, but... */
	}
    }

    BPPRINTF(("bpread: next on free is k = %s\n", hdr_nmbr(hdr)));

    /*
     * remove hdr from free list
     */
    REMQUE(hdr, fnext, fprev);

    /*
     * write it out if necessary
     */
    if( hdr->modify ) {
	bwritebuf(hdr);
    }

    /*
     * remove it from its old position on the hash chain
     */
    if( hdr->hashed ) {
	REMQUE(hdr, hnext, hprev);
    }

    /*
     * insert in new hash position
     */
    INSQUE(hash, hdr, hnext, hprev, BUFHDR *);

    /*
     * fill in bufhdr
     */
    hdr->modify = 0;	/* data has not been touched yet */
    hdr->bn = bn;	/* fill in block number */
    hdr->count = 1;	/* set reference count */
    hdr->hashed = 1;	/* now it's on the hash list! */

    /*
     * Read in data.  If an error occurs attempting to read the 
     * complete block, then read each sector individually.  The 
     * offending sector will be zero'd. 
     */
    if( ujfs_rw_diskblocks( Bdfd, bn * fs_block_size, fs_block_size, hdr->buf,
			    GET ) != 0 ) {
#if 0
	dd_req_desc_t	*data_list;
	uint32	data_list_entries;
	dd_req_status_t	*status_list;
	uint32	status_list_entries;
	uint64		next_sector;
	short		index, rc;

	memset(hdr->buf, 0, fs_block_size);

	data_list_entries = fs_block_size / phys_block_size;
	data_list = malloc( data_list_entries * sizeof( dd_req_desc_t ) );
	next_sector = bn / phys_block_size;
	cp = hdr->buf;
	for( index = 0; index < data_list_entries;
	     index++, next_sector += phys_block_size, cp += phys_block_size ) {
	    data_list[index].record_offset = next_sector;
	    data_list[index].record_count = 1;
	    data_list[index].data_buffer = cp;
	}

	rc = DD_ReadRecordList( Bdfd, 0, DD_NOREORDER, DD_DEFAULT_PRIORITY,
				data_list, data_list_entries, &status_list,
				&status_list_entries );

	for( index = 0; index < status_list_entries; index++ ) {
	    if( status_list[index].status == ESPIPE ) {
		(*Rwerr)(SEEK_ERR, data_list[index].record_offset);
		return NULL;
	    } else if( status_list[index].status != 0 ) {
		/*
		 * Tell the user we cleared this block
		 */
		(*Rwerr)(READ_CLEAR_ERR, data_list[index].record_offset );

		/*
		 * Data has been zero'd and therefore modified.
		 * Bad block relocation will occur.
		 */
		hdr->modify = 1;
	    }
	}

	free( data_list );
	free( status_list );
#endif
    }

    BPPRINTF(("read...\n"));

#ifdef BPDEBUG
    bufhashstats();
#endif

    return (hdr->buf);
}


/*
 * NAME: bptouch
 *                                                                    
 * FUNCTION: Mark addr as modified
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	addr	- Address of the buffer to be marked
 *
 * NOTES: Probably want to make the case where we can't find a particular buffer
 *	in our pool do something to let the caller know the problem, and not
 *	attempt to modify storage we don't have (core dump).
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
void bptouch( void *addr )
{
    BUFHDR *hdr = findhdr(addr);

#ifdef BPDEBUG
    BPPRINTF(("bptouch(0x%x):", (int) addr));

    if( hdr == NULL )
	BPPRINTF(("bptouch: can't find address 0x%x!\n", addr));
    else
#endif /* BPDEBUG */
    {
	BPPRINTF(("k = %s\n", hdr_nmbr(hdr)));

	hdr->modify = 1;
    }
}


/*
 * NAME: bprelease
 *                                                                    
 * FUNCTION: Mark addr as released
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	addr	- Buffer to be released
 *
 * NOTES: Will move the specified buffer to the free list
 *
 *	For RAS we may want to move checks out of BPDEBUG ifdef and tell the
 *	caller we ran into a problem.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
void bprelease( void *addr )
{
    BUFHDR *hdr = findhdr(addr);

#ifdef BPDEBUG
    BPPRINTF(("bprelease(0x%x): ", addr));

    if( hdr == NULL )
	BPPRINTF(("can't find address 0x%x!", addr));
    else if( hdr->count < 1 )
	BPPRINTF(("address 0x%x count gone negative", addr));
    else
#endif /* BPDEBUG */

    /*
     * Decrement counter for this buffer
     */
    hdr->count--;
    if( hdr->count == 0 ) {
	BPPRINTF(("releasing "));

	/*
	 * reference count went to 0; no one else is using it, so add
	 * it to the free list
	 */
	INSQUE(&Bufhdr[0], hdr, fnext, fprev, BUFHDR *);
    }

    BPPRINTF(("k = %s", hdr_nmbr(hdr)));
}


/*
 * NAME: bpflush
 *                                                                    
 * FUNCTION: Flush addr from the Buffer pool
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS: If the specified buffer was marked as modified it will be
 *	written to disk by this function when all references to this buffer have
 *	been released.
 *
 * PARAMETERS:
 *	addr	- Buffer address to clear from the Buffer pool
 *
 * NOTES: For RAS move checks from BPDEBUG ifdef to code to let user know
 *	something went wrong.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
void bpflush( void *addr )
{
    BUFHDR *hdr = findhdr(addr);

#ifdef BPDEBUG
    BPPRINTF(("bpflush(0x%x): ", addr));

    if (hdr == NULL)
	BPPRINTF(("bpflush: can't find address 0x%x!", addr));

    else if (hdr->count < 1)
	BPPRINTF(("bpflush: address 0x%x count gone negative", addr));

    else if (hdr->count > 1)
	BPPRINTF(("bpflush: address 0x%x still being used", addr));

    else
#endif /* BPDEBUG */
    hdr->count--;
    if( hdr->count == 0 ) {
	/*
	 * write it out if modified
	 */
	if( hdr->modify )
	    bwritebuf(hdr);

	/*
	 * remove from the hash list
	 */
	REMQUE(hdr, hnext, hprev);

#ifdef BPDEBUG
	bufhashstats();
#endif

	hdr->hashed = 0;

	/*
	 * add it to the free list
	 */
	INSQUE(&Bufhdr[0], hdr, fnext, fprev, BUFHDR *);
    }
}


/*
 * NAME: bpclose
 *                                                                    
 * FUNCTION: Flush all Buffers and close down the Buffer pool
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS: NONE
 *
 * NOTES: All buffers marked as modified will be written to disk.  The buffer
 *	pool should not be used without another initialization after calling
 *	this function.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
void bpclose( void )
{
    BUFHDR *hdr;
    HASHHDR *hash;

    BPPRINTF(("bpclose()\n"));

    for( hash = &bhash[0]; hash < &bhash[NHASH]; hash++ ) {
	for( hdr = hash->hnext; hdr != (BUFHDR *) hash; hdr = hdr->hnext ) {
#ifdef BPDEBUG
	    if( hdr->count > 0 ) {
		BPPRINTF(("\tbpclose: block (%d) has ref count of %d\n",
			 hdr->bn, hdr->count));
	    }
#endif /* BPDEBUG */
	    if( hdr->modify )
		bwritebuf(hdr);
	}
    }
}


/*
 * NAME: bpassert
 *                                                                    
 * FUNCTION: Assert that addr is still referenced
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	string	- Message string
 *	addr	- Buffer address to check
 *	line	- Line number of caller
 *
 * NOTES: For RAS usage call this function to check if a buffer is still in the
 *	pool.  If not a message will be issued.  Currently this is just in debug
 *	code but we may want to make this actual RAS code.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
void bpassert( char *string,
	       void *addr,
	       int line )
{
    BUFHDR *hdr = findhdr(addr);

    if( hdr == NULL ) {
	BPPRINTF(("bpassert: can't find address 0x%x!", addr));
    } else if( hdr->count <= 0 ) {
	BPPRINTF(("bpassert: %s on line %d (k = %s)", string, line,
		 hdr_nmbr(hdr)));
    }
}


/*
 * NAME: bufstats
 *                                                                    
 * FUNCTION: Print out hash chains and free list
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS: NONE
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
static void bufstats( void )
{
    BUFHDR *hdr;
    HASHHDR *hash;
    FILE *fp;

    if( (fp = popen("pg", "w")) == NULL) {
	printf("can't pipe to pg\n");
    } else {
	fprintf(fp,"Hash chains:\n");
	for( hash = bhash; hash < &bhash[NHASH]; hash++) {
	    hdr = hash->hnext;
	    if( hdr != (BUFHDR *) hash) {
		fprintf(fp,"\tchain %d: ", hash - bhash);
		for(  ; hdr != (BUFHDR *) hash; hdr = hdr->hnext) {
		    fprintf(fp,"%s (%d), ", hdr_nmbr(hdr), hdr->bn);
		}
		putchar('\n');
	    }
	}

	fprintf(fp,"Free list:\n");
	for( hdr = Bufhdr[0].fnext; hdr != Bufhdr; hdr = hdr->fnext) {
	    fprintf(fp,"\t%s (%d)\n", hdr_nmbr(hdr), hdr->bn);
	}

	pclose(fp);
    }
}


/*
 * NAME: buf_stat
 *                                                                    
 * FUNCTION: Print values of buffer addr structure
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	addr	- Buffer to print information for
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
static void buf_stat( void *addr )
{
    BUFHDR *hdr = findhdr(addr);

    printf("0x%x (%s):\n", addr, hdr_nmbr(hdr));

#define	PRADDR(addr) \
    printf("\t addr = 0x%0x (%s)\n", hdr->addr, hdr_nmbr(hdr->addr))

    PRADDR(fnext);
    PRADDR(fprev);
    PRADDR(hnext);
    PRADDR(hprev);

    printf("\t modify = %d\n", hdr->modify);
    printf("\t count = %d\n", hdr->count);
    printf("\t bn = %d\n", hdr->bn);
    printf("\t buf = 0x%x\n", hdr->buf);
}

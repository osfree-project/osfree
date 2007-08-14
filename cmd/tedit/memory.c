/***********************************************************************/
/* MEMORY.C - Memory management functions.                             */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 *
 * The code in this module is borrowed from:
 * The Regina Rexx Interpreter
 * Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
 */

static char RCSid[] = "$Id: memory.c,v 1.4 2002/06/23 10:59:48 mark Exp $";

/*
 * The routines in this file try to minimize the the number of calls
 * to malloc() and free(). Since it would generally not be possible to
 * release memory, unless it actually is last in the virtual memory
 * that the process holds, just don't release it, and let the
 * interpreter grow. Memory are allocated in only certain sizes, and
 * are "freed" to a freelist, which is within the interpreter.
 *
 * The important routines are get_a_chunk() and give_a_chunk(), which 
 * might be called a large number of times. All the other routines are
 * either called once to initiate, or it is used in tracing and 
 * debugging, where speed and space is not important anyway. 
 *
 * The algorithm works something like this: memory can only be allocated
 * in predetermined sizes (8, 12, 16, 24, 32, ...) and allocation of a 
 * size other than that will have to allocate something slightly bigger.
 * For each size, there is a linked list of free pieces of memory of 
 * that size, the first entry of each of these lists can be accessed 
 * through 'flists', which is an array of pointers to these lists. 
 *
 * Every time someone needs a piece of memory, the first piece of the 
 * freelist containing memory of suitable size (as big or slightly 
 * bigger) is returned. If the list is empty, a large piece of 
 * memory is allocated by malloc(), chopped up and put on the freelist
 * that was empty. 
 *
 * When memory is released, the prime problem is to decide which 
 * freelist to put it on. To manage that, each time memory is 
 * allocated by malloc(), the upper and lower address of the memory 
 * is put in hashtable; given a particular address, the hashtable
 * can be sought using the address as hashvalue, and the result will 
 * be the size of the memory chunks at that address.
 *
 * When dealloacting strings, we know the max-size of the string, and 
 * then we can calculate which freelist the string should be put on,
 * without having to search the hashtable structure. Note that there
 * is no need to deallocate strings using the give_a_string() function,
 * the normal give_a_chunk() will work just as well, but is somewhat
 * slower. 
 *
 * If you don't #define FLISTS, then malloc() and free() should be
 * used instead. That might be very slow on some machines, since rexx
 * memory is traced, which also tend to be slow, since there is a lot
 * current implementation you can use malloc()/free() in parallel with
 * the routines defined here.
 *
 * Note that using this metod, the last piece of memory freed will be
 * the first to be used when more memory is needed. 
 *
 * The number of calls to malloc() seems to be negligable when using 
 * this metod (typical less than 100 for medium sized programs). But
 * this is of course dependent on how the program uses memory. 
 *
 * extention for debugging purposes. Whenever memory is allocated,
 * used like this:
 *
 *   0       4       8      12       16 bytes
 *   | count |f|m|seq| prev  | next  | start of allocated memory
 *
 * The 'count' is the number of bytes allocated, 'f' (flag) is used in 
 * garbage collection, and 'prev' and 'next' are pointers used in a 
 * double linked list. seqv is a sequence number which is iterated 
 * for each memoryallocation.
 * 
 * count is int, prev and next are char*, f is char and seqv is a 
 * 16 bit integer. The 'm' is a magic number. Actually it is just 
 * there to fill the space, and can be used for more useful purposed
 * if needed.
 *
 * memory with bitpatterns. If the PATTERN_MEMORY cpp-variable is set,
 * is set BEEN_USED before deallocation.
 * 
 * Garbage-collection is not implemented, but listleaked will list out
 * every chunk of allocated memory that are not currently in use. The
 * array markptrs contains a list of functions for marking memory.
 * There is a potensial problem with garbage collection, since the 
 * interpreter might 'loose' some memory everytime it hits a syntax
 * error, like "say random(.5)". To fix that, memory should either 
 * be traced and then garbage collected, or it should have a sort 
 * of transaction oriented memory management (yuk!).
 *
 *      this:  sizeof(int) = sizeof(char*) = 32 bits. It might work
 *      guarantee it. 
 *
 */
#include <stdio.h>
#include <assert.h>

#ifdef HAVE_STRING_H
# include <string.h>
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_MEMORY_H
# include <memory.h>
#endif

#include <the.h>
#include <proto.h>

#if defined(__BOUNDS_CHECKING_ON) || defined(__USING_EFENCE)
void *get_a_block( int size )
{
   return( malloc( size ) );
}

void give_a_block( void *ptr )
{
   free( ptr );
}

void *resize_a_block( void *ptr, int size )
{
   return( realloc( ptr, size ) );
}

void init_memory_table( void )
{
}

void the_free_flists( void )
{
}
#else

#define FLISTS
#define PATTERN_MEMORY

#ifdef FLISTS

/* 
 * CHUNK_SIZE it the size in which memory is allocated using malloc(), 
 * and that memory is then divided into pieces of the wanted size. 
 * If you increase it, things will work slightly faster, but more 
 * memory is wasted, and vice versa. The 'right' size is dependent on
 * your machine, rexx scripts and your personal taste.
 */
#define CHUNK_SIZE (8192)

/*
 * MAX_INTERNAL_SIZE is the max size of individual pieces of memory 
 * that this system will handle itself. If bigger pieces are requested
 * it will just forward the request to malloc()/free(). Note that 
 * this value should be less than or equal to CHUNK_SIZE.
 */
#define MAX_INTERNAL_SIZE (4096)

/* 
 * MEMINFO_HASHSIZE is the size of the 'hashtable' used to find the size
 * of a chunk of memory, given an address of a byte within that chunk. 
 * Actually, this isn't much of a real hashtable, but still. Allocating
 * large value will not make much harm other than wasting memory. Using 
 * too small value can seriously degrade execution. The optimal size
 * is such that MEMINFO_HASHSIZE * CHUNK_SIZE is only slight bigger 
 * than the actual use of memory in your rexx script (including the 
 * memory that will be wasted)
 */
#define MEMINFO_HASHSIZE (499)

/* 
 * GET_SIZE() is a 'function' that returns a index into the 'hash' 
 * variable, given a specific number. The index returned will be the
 * index of the ptr to the list of free memory entries that is identical
 * or slightly bigger than the parameter in size.
 */
#define GET_SIZE(a) (hash[((a)+3)>>2])

/*
 * This is the hashfunction for use with 'hashtable'. It will effectively
 * just shift away some of the lower bits and fold the result around
 * the 'hashtable'. Note that '13' is corresponent to CHUNK_SIZE, since
 * 8192 == 1<<13, which is the optimal size. If you change one of them
 * be sure to change the other. 
 * 
 * Maybe we could eliminate a division by letting MEMINFO_HASHSIZE have
 * a number equal to a binary 'round' number (e.g. 512). There is no
 * need to keep the size a prime number, since the elements in the 
 * table *will* be well distributed.
 */
#define mem_hash_func(a) (((a)>>13)%MEMINFO_HASHSIZE)

/* 
 * Here are the list of the 'approved' sizes. Memory is only allocatable
 * in these sizes. If you need anything else, use the lowest number that
 * is higher than what you need.
 *
 * Why exactly these numbers? Why not? Note that these are a subset 
 * of the series {8,16,32,64,128...} and {12,24,48,96} mingled together. 
 * Note that you can not allocate memory in smaller sizes than is 
 * possible to fit a pointer (to char and/or void) into. Also take 
 * into consideration that all these sizes should be aligned according
 * to the size of ints and pointers, so don't make them too small.
 */
#define NUMBER_SIZES 19
static const int sizes[NUMBER_SIZES] = 
                     {    8,   12,   16,   24,   32,   48,   64,   96, 
                        128,  192 , 256,  384,  512,  768, 1024, 1536, 
                       2048, 3072, 4096 } ;

/* 
 * The array of pointers to the freelists having memory of the sizes 
 * specified in 'sizes'. I.e. theflists[0] is a pointer to a linked list
 * of free memory chunks of size 8, flist[1] to memory of size 12 etc.
 * The size of this array is the same as the size of 'sizes'.
 */
static char *theflists[NUMBER_SIZES] = { NULL } ;

/*
 * The type meminfo holds the info about the connection between the 
 * address of allocated memory and the size of that memory. When new
 * memory is allocated by malloc(), in size CHUNK_SIZE, a new box of
 * meminfo is created, which holds the address returned from malloc()
 * and the size in which the chunk was divided {8,12,16,24,32...}.
 */
typedef struct meminfo_type 
{
   char *start ;                /* start of memory's address */
   char *last ;                 /* end of memory's address */
   struct meminfo_type *next ;  /* next ptr in linked list */
   int size ;                   /* size of chunks at that address */
} meminfo ;

/* 
 * The 'hashtable'. Used for quick access to the size of a chunk of
 * memory, given its address.
 */
static meminfo *hashtable[ MEMINFO_HASHSIZE ] = { NULL } ;

/* 
 * Array used for rounding a number to an 'approved' size, i.e. a size
 * in which the interpreter will allocate memory. Remember that the 
 * approved sizes are {8,12,16,24,32 ...}? This function will return 
 * 8 for 1 through 8; 12 for 9 through 12; 16 for 13 through 16 etc.
 * It is not initially set, but will be set by init_hash_table().
 *
 * Note: the 'step' in this table (4 as it is defined below) must not
 * be bigger then the smallest gap in between two 'approved' sizes of
 * memory. E.g the smallest gap as defined above is 12-8 = 4.
 *
 * Actually, the name is somewhat misleading, since this is not really
 * a hashtable, it is just a leftover from the time when it actually
 * was a hashtable. 
 *
 * Due to how the hash array is initialized, we have to allocate one 
 * more item than is going to be used. This is really a klugde, and we 
 * really ought to fix it a more clean way.
 */
static short hash[ CHUNK_SIZE/4 + 1 ] ;
#endif

static meminfo *first_chunk=NULL;
static meminfo *curr_chunk=NULL;

# ifdef THE_DEBUG_MEMORY2
static int show_a_free_list(int bin, char *str)
{
   char *ptr;
   int j=0;

   if ( theflists[bin] == NULL )
   {
      if (str)
         fprintf(stderr,"%sbin[%d] Free List unallocated Maximum %d Size: %d\n",str,bin,(CHUNK_SIZE / sizes[bin]), sizes[bin]);
   }
   else
   {
      for (j=1,ptr=theflists[bin]; *(char**)ptr!=NULL && j<5000; ptr=*(char **)ptr,j++ );
      if (str)
         fprintf(stderr,"%sbin[%d] Number in Free List %d Maximum %d Size: %d\n",str,bin,j,(CHUNK_SIZE / sizes[bin]), sizes[bin]);
   }
   return j;
}
#endif
/*
 * This function stores in a singly linked list all chunks of memory
 * that are allocated with malloc(). This is so that they can all be
 * free()ed by the_free_flists().
 */
/******************************************************************************/
#ifdef HAVE_PROTO
int register_mem( void *chunk )
#else
int register_mem( chunk )
void *chunk;
#endif
/******************************************************************************/
{
   meminfo *mem=NULL;

   if ((mem = (meminfo *)malloc( sizeof( meminfo ))) == NULL )
      return(1);
   mem->start = chunk;
   mem->next = NULL;
   if (curr_chunk)
   {
      curr_chunk->next = mem;
   }
   curr_chunk = mem;
   if (first_chunk == NULL)
      first_chunk = curr_chunk;
   return(0);
}

/*
 * This function initiates the variable 'hash'. This might have been 
 * done initially, since the values in this will never change. But 
 * since the size is rather big. it is more efficient to spend some 
 * CPU on initiating it. The startup time might be decreased by swapping
 * this routine for a pre-defined variable. Perhaps it should be 
 * rewritten to use two arrays, one for large pieces of memory and 
 * one for small pieces. That would save space in 'hash'
 *
 * The values put into the array has been described above. 
 */
/******************************************************************************/
#ifdef HAVE_PROTO
void init_memory_table( void )
#else
void init_memory_table()
#endif
/******************************************************************************/
{
   int indeks ;   /* index into current element to be initiated */
   int j ;
   int size ;
   int num ;

   /* 
    * Set the few lowest values manually, since the algoritm breaks
    * down for sufficient small values. 
    */
   indeks = 0 ;
   hash[indeks++] = 0 ;  /* when size equals 0, well ... 8 :-) */
   hash[indeks++] = 0 ;  /* for 1 <= size < 4 */
   hash[indeks++] = 0 ;  /* for 4 <= size < 8 */

   /* 
    * The main loop. How does this algorithm work, well, look at the 
    * following table, in which all numbers should be multiplied with
    * 4 to get the correct numbers. 
    *
    *  bin        sizes
    *   0   (8) :  2
    *   1  (12) :  3
    *   2  (16) :  4  5
    *   3  (24) :  6  7
    *   4  (32) :  8  9 10 11
    *   5  (48) : 12 13 14 15 
    *   6  (64) : 16 17 18 19 20 21 22 23 
    *   7  (96) : 24 25 26 27 28 29 30 31
    *   8 (128) : 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
    *   9 (192) : 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
    * etc
    *
    * The number to the left of the colon is the index into the
    * 'sizes' array, and the number in parenthesis is the size which
    * 'sizes' would return for that index. The numbers to the right of
    * the colon are all the elements in 'hash' that contains that
    * particular index into 'sizes'.  Notice that pairs of lines have
    * equal number of numbers, and that the number of numbers doubles
    * for every second line.
    *
    * Therefore, let size be the number of elements to initialize in
    * each iteration, and double it at the end of the loop. 'size'
    * will then loop through 8, 16, 32, 64, 128 ... For each iteration
    * of that loop, initialize 'size'/2 numbers to 'num' and then the
    * next 'size'/2 numbers to 'num'+1. Increment 'num' by two for
    * each iteration. The 'indeks' is the current number in hash to
    * initialize.
    */
   size = 1 ;
   num = 1 ;
   for (; indeks<(CHUNK_SIZE/4); )
   {
      /* 
       * Initalize first in each pair of bins of same length.
       * I.e  8, 16, 32,  64 ... etc
       */
      for (j=0; j<size; j++ )
         hash[indeks++] = num ;
      num++ ;

      /* 
       * Initialize the second in each pair: 12, 24, 48, 96 ... etc
       */
      for (j=0; j<size; j++ )
         hash[indeks++] = num ;
      num++ ;
      size = size << 1 ;
   }

   /* 
    * Do I need this? I don't think so. It is a kludge to make something
    * work on 64 bit machines, but I don't think it is needed anymore.
    * Just let is be commented out, and the delete it if things seem
    * to work.
    */

   if (sizeof(int)>4 || sizeof(int*)>4)
      hash[3] = 2 ;
   memset( theflists, 0, NUMBER_SIZES * sizeof(char *) );

}



/*
 * Adds information about a chunk of memory to the hashtable memory 
 * addresses and the chunksize at that address. Note that two addresses
 * are sent as parameters, the start of the memory to be registerd, and
 * the address under which the information is to be registered. Why? 
 * Look at the following figure:
 *
 * 0               8K               16K                24K
 * +---------------+-----------------+------------------+-------------+
 * |    AAAAAAAAAAAAAAAAAA   BBBBBBBBBBBBBBBBBB
 * +----+----------------+---+----------------+-------------------------+
 *      3K              11K 13K              21K
 * 
 * Two chunks are allocated: A and B. The chunks are allocated in 8K
 * blocks, but they will in general not follow the 8K boundaries of
 * the machine. The 'hashtable' array have entries that _do_ follow
 * the 8K boundaries of the machine. Therefore, chunk A must be
 * registered under the in the 'hashtable' entries for both the 0-8K
 * segment, and the 8-16K segment. And vice versa, the 8-16K segment
 * may contain parts of chunk A and B.
 *
 * This could be avoided, if the chunks were aligned with the boundaries 
 * of the computer. If you change any of the constants in this part of
 * the program, be sure to tune them to match eachother!
 * 
 * Of course, this routines need memory to be able to register other 
 * memory, so to avoid a deadlock, it calls malloc directly. It will 
 * never release memory, since we can really not be sure that all 
 * memory has been released.
 */
/******************************************************************************/
#ifdef HAVE_PROTO
static int add_entry( char *start, char *addr, int bin_no )
#else
static int add_entry( start, addr, bin_no )
char *start;
char *addr;
int bin_no;
#endif
/******************************************************************************/
{
   meminfo *ptr ;              /* work ptr */
   int tmp ;                   /* tmp storage for mem_hash_func() */
   static meminfo *mem=NULL ;  /* ptr to array, empty at first */
   static int indeks=128 ;      /* force it to allocate at first invocation */

   /*
    * If we have used all free meminfo-boxes, allocate more. This is 
    * forces upon us at the first invocation. Allocate space for 128
    * at a time.
    */
   if (indeks>=128)
   {
      /* Stupid SunOS acc gives incorrect warning for the next line */
      if  (!(mem = (meminfo *)malloc( sizeof( meminfo) * 128 )))
          return(1);
      if (register_mem( (void *)mem ))
         return(1);
      indeks = 0 ;
   }

   /* 
    * Fill in the fields of the box, and put it in the front of the 
    * requested bin in hashtable
    */
   ptr = &mem[ indeks++ ] ;   
   ptr->next = hashtable[tmp=mem_hash_func((unsigned long)addr)] ;
   ptr->size = bin_no ;
   ptr->start = start ;
   hashtable[tmp] = ptr ;
   return(0);
}

/* 
 * Allocate a piece of memory. The size is given as the 'size' parameter.
 * If size is more than MAX_INTERNAL_SIZE, it will call malloc()
 * directly, else, it will return a piece of memory from the freelist,
 * after possibly filling the freelist with more memory if is was
 * empty in the first place.
 */
/******************************************************************************/
#ifdef HAVE_PROTO
void *get_a_block( int size )
#else
void *get_a_block( size )
int size;
#endif
/******************************************************************************/
{
   register int bin ;     /* bin no in array of freelists */
   register char *vptr ;  /* holds the result */
   register void *result ; 
#ifdef THE_DEBUG_MEMORY2
   int before, after;
#endif

   /*
    * If memory is too big, let malloc() handle the problem. 
    */
   if (size>MAX_INTERNAL_SIZE) 
   {
      if ((result=malloc( size )))
      {
         if (register_mem( result ))
            return(NULL);
         return result ;
      }
      else
          return NULL;
   }

   /*
    * Get the first item from the appropriate freelist, and let 'vptr'
    * point to it. Simultaneously set bin to the bin no in 'theflists'
    * to avoid recalculating the number. If the freelist is empty 
    * (i.e vptr==NULL) then allocate more memory.
    */
   if ((vptr=theflists[bin=GET_SIZE(size)])==NULL)
   {
      char *ptr ;      /* work ptr, to loop through the memory */
      char *topaddr ;  /* points to last item in memory */

      /* 
       * Allocate the memory, and set both vptr and initiate the 
       * right element in 'theflists'. Note that the value in 'flists' is
       * 'incremented' later, so it must be set to the value which now
       * is to be allocated. 
       */
      vptr = malloc( CHUNK_SIZE ) ;
      if (!vptr)
          return(NULL);
      theflists[bin] = vptr ;

      /* 
       * Calculate the top address of the memory allocated, and put 
       * the memory into 'topaddr'. Then register the chunk of memory 
       * in both the possible CHUNK_SIZE segments of the machine. In 
       * some rare cases the last registration might not be needed,
       * but do it anyway, to avoid having to determine it.
       */
      topaddr = vptr + CHUNK_SIZE - sizes[bin] ;
      if (register_mem( vptr ))
         return(NULL);
      if (add_entry( vptr, vptr, bin ))
         return(NULL);
      if (add_entry( vptr, vptr + CHUNK_SIZE, bin ))
         return(NULL);

      /*
       * Then loop through the individual pieced of memory within the 
       * newly allocated chunk, and make it a linked list, where the 
       * last ptr in the list is NULL.
       */
      for (ptr=vptr; ptr<topaddr; ptr=ptr+sizes[bin] )
         *(char**)ptr = ptr + sizes[bin] ;
  
      *((char**)(ptr-sizes[bin])) = NULL ;

#ifdef THE_DEBUG_MEMORY2
      show_a_free_list( bin, "get_a_block(): empty freelist for ");
#endif
   }

   /*
    * Update the pointer in 'flist' to point to the next entry in the
    * freelist instead of the one we just allocated, and return to 
    * caller.
    */
#ifdef THE_DEBUG_MEMORY2
   before = show_a_free_list( bin, NULL);
#endif
   theflists[bin] = (*((char**)(vptr))) ;
#ifdef THE_DEBUG_MEMORY2
   after = show_a_free_list( bin, NULL );
   if ( before - 1 != after )
      fprintf(stderr,"****** get_a_block() for bin [%d] failed. Before %d After %d\n",bin,before,after);
#endif
   return (vptr) ;
}


/*
 * The standard interface to freeing memory. The parameter 'ptr' is 
 * a pointer to the memory to be freed, is put first in the freelist
 * pointed to by the appropriate element in 'theflists'.
 * 
 * I am not really sure what cptr do in this, but I think it has 
 * something to do with *void != *char on Crays ... The main consumer
 * of CPU in this routine is the for(;;) loop, it should be rewritten.
 */
/******************************************************************************/
#ifdef HAVE_PROTO
void give_a_block( void *ptr )
#else
void give_a_block( ptr )
void *ptr;
#endif
/******************************************************************************/
{
   char *cptr ;      /* pseudonym for 'ptr' */
   meminfo *mptr ;   /* caches the right element in hashtable */
#ifdef THE_DEBUG_MEMORY2
   int before, after;
#endif

   /*
    * initialize a few values, 'cptr' is easy, while 'mptr' is the
    * list of values for this piece of memory, that is in the 
    * hashtable that returns memory size given a specific address
    */
   cptr = (char*)ptr ;
   mptr = hashtable[ mem_hash_func( ((unsigned long)cptr) ) ] ;

   /* 
    * For each element in the list attached to the specific hashvalue, 
    * loop through the list, and stop at the entry which has a start 
    * address _less_ than 'cptr' and a stop address _higher_ than 
    * 'cptr' (i.e. cptr is within the chunk.)
    */
   for ( ; (mptr) && 
        ((mptr->start+CHUNK_SIZE<=cptr) || (mptr->start>cptr)) ;
        mptr = mptr->next) ;

   /*
    * Now, there are two possibilities, either is mptr==NULL, in which
    * case this piece of memory is never registered in the system, or 
    * then we have more information. In the former case, just give 
    * the address to free(), hoping it knows more. In the latter, put
    * the memory on the appropriate freelist. 
    */
   if (mptr)
   {
      /* 
       * Link it into the first place of the freelist.
       */
#ifdef THE_DEBUG_MEMORY2
      before = show_a_free_list( mptr->size, NULL );
#endif
      *((char**)cptr) = theflists[mptr->size] ;
      theflists[mptr->size] = cptr ;
#ifdef THE_DEBUG_MEMORY2
      after = show_a_free_list( mptr->size, NULL );
      if ( before + 1 != after )
         fprintf(stderr,"****** give_a_block() for bin [%d] failed. Before %d After %d\n",mptr->size,before,after);
#endif
   }
   else
      free( ptr ) ; 
}

/*
 * The interface to resizing memory. The parameter 'ptr' is
 * a pointer to the memory to be resized. First, if the requested size 
 * is within the size of the existing block, just return it. Otherwise
 * the block is put back on the free lists and a new block allocated.
 */
/******************************************************************************/
#ifdef HAVE_PROTO
void *resize_a_block( void *ptr, int size )
#else
void *resize_a_block( ptr, size )
void *ptr;
int size;
#endif
/******************************************************************************/
{
   char *cptr ;      /* pseudonym for 'ptr' */
   meminfo *mptr ;   /* caches the right element in hashtable */
   register void *result ; 

   /*
    * initialize a few values, 'cptr' is easy, while 'mptr' is the
    * list of values for this piece of memory, that is in the 
    * hashtable that returns memory size given a specific address
    */
   cptr = (char*)ptr ;
   mptr = hashtable[ mem_hash_func( ((unsigned long)cptr) ) ] ;

   /* 
    * For each element in the list attached to the specific hashvalue, 
    * loop through the list, and stop at the entry which has a start 
    * address _less_ than 'cptr' and a stop address _higher_ than 
    * 'cptr' (i.e. cptr is within the chunk.)
    */
   for ( ; (mptr) && 
        ((mptr->start+CHUNK_SIZE<=cptr) || (mptr->start>cptr)) ;
        mptr = mptr->next) ;

   /*
    * Now, there are two possibilities, either mptr==NULL, in which
    * case this piece of memory was never registered in the system, or
    * we have more information. In the former case, just give 
    * the address to realloc(), and return. In the latter, if the
    * requested size is still within the currently allocated size
    * return it, or allocate a new chunk, copy the current contents
    * to the new chunk and put the old piece of memory on the 
    * appropriate freelist.
    */
   if (!mptr)
      return ( realloc(ptr, size) ) ;

   /*
    * If the size of the block being resized is within the current
    * block size, simply return the same pointer.
    */
   if (size <= sizes[mptr->size])
      return ptr;

   /*
    * Get the new chunk of memory. This MUST be larger than the
    * previous chunk to have reached here.
    */
   result = get_a_block( size ) ;
   if (result == NULL)
      return NULL;
   /*
    * Copy the current contents into the new chunk.
    */
   memcpy( result, cptr, sizes[mptr->size] ) ;

   /* 
    * Put the old chunk into the first place of the freelist.
    */
   *((char**)cptr) = theflists[mptr->size] ;
   theflists[mptr->size] = cptr ;

   return result;
}

/******************************************************************************/
#ifdef HAVE_PROTO
void the_free_flists( void )
#else
void the_free_flists()
#endif
/******************************************************************************/
{
   meminfo *ptr = first_chunk;
   meminfo *next = NULL;

   while( ptr )
   {
      next = ptr->next;
      free( ptr );
      ptr = next;
   }
   first_chunk = curr_chunk = NULL;
   return;
}
#endif /* __BOUNDS_CHECKING_ON */

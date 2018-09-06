#if 0
#define REGINA_DEBUG_MEMORY
#define REGINA_DEBUG_MEMORY1
#define REGINA_DEBUG_MEMORY2
#define REGINA_DEBUG_MEMORY3
#endif
/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

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
 * tend to use lots of small size memory. If you #define TRACEMEM,
 * memory is traced, which also tend to be slow, since there is a lot
 * of overhead in allocation and deallocation. Also note that in the
 * current implementation you can use malloc()/free() in parallel with
 * the routines defined here.
 *
 * Note that using this metod, the last piece of memory freed will be
 * the first to be used when more memory is needed.
 *
 * The number of calls to malloc() seems to be negligable when using
 * this method (typical less than 100 for medium sized programs). But
 * this is of course dependent on how the program uses memory.
 *
 * The tracing part of this file, (#ifdef TRACEMEM) is an optional
 * extention for debugging purposes. Whenever memory is allocated,
 * mymalloc() allocates 16 bytes more than needed. These bytes are
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
 * An additional option to TRACEMEM is filling allocated and deallocated
 * memory with bitpatterns. If the PATTERN_MEMORY cpp-variable is set,
 * all allocated memory is initated to NOT_USED, and deallocated memory
 * is set BEEN_USED before deallocation.
 *
 * Garbage-collection is not implemented, but listleaked will list out
 * every chunk of allocated memory that are not currently in use. The
 * array markptrs contains a list of functions for marking memory.
 * There is a potential problem with garbage collection, since the
 * interpreter might 'loose' some memory everytime it hits a syntax
 * error, like "say random(.5)". To fix that, memory should either
 * be traced and then garbage collected, or it should have a sort
 * of transaction oriented memory management (yuk!).
 *
 * NOTE that #define'ing TRACEMEM requires that your machine follows
 *      this:  sizeof(int) = sizeof(char*) = 32 bits. It might work
 *      for other machines to (having larger word size), but I don't
 *      guarantee it.
 *
 */

#include "rexx.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#ifdef TRACEMEM
/*
 * If we are tracing memory, each piece of allocated memory gets the
 * following header prepended, which are used to keep track of that
 * piece of memory.
 */
typedef struct memhead
{
   int count ;                    /* Size of this piece of memeory */
   struct memhead *prev, *next ;  /* Ptrs in double linked list */
   unsigned short seqv ;          /* Sequential counter */
   unsigned char flag ;           /* What is this memory used for */
   unsigned char magic ;          /* Not really used */
   void *addr;
} memheader;

# ifdef PATTERN_MEMORY
/*
 * The two byte values NOT_USED and BEEN_USED are patterns which newly
 * allocated dynamic memory will be set to, and memory to be freed
 * will be set to, respectively. This is done to provoke problems  if
 * memory is used but not initialized, or if it used after is has
 * been released.
 */

#  define NOT_USED (0x42)    /* letter 'B' */
#  define BEEN_USED (0x69)   /* letter 'i' */

/*
 * The magic cookie is just a placeholder, it is checked for consistency
 * but could easily be used for something else, if the space is needed.
 */
#  define MAGIC_COOKIE (0xd4)

# endif /* PATTERN_MEMORY */
#endif /* TRACEMEM */


#ifdef FLISTS
/*
 * CHUNK_SIZE it the size in which memory is allocated using malloc(),
 * and that memory is then divided into pieces of the wanted size.
 * If you increase it, things will work slightly faster, but more
 * memory is wasted, and vice versa. The 'right' size is dependent on
 * your machine, rexx scripts and your personal taste.
 */
#define CHUNK_SIZE (8192*4) /* JO - was 8192 */

/*
 * MAX_INTERNAL_SIZE is the max size of individual pieces of memory
 * that this system will handle itself. If bigger pieces are requested
 * it will just forward the request to malloc()/free(). Note that
 * this value should be less than or equal to CHUNK_SIZE.
 */
#define MAX_INTERNAL_SIZE (8192*3) /* JO - was 4096 */

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
 * FLIST_ARRAY_SIZE is the element count of an array which holds meminfo's
 * (see below) while allocating space. Have a look at add_entry() below.
 */
#define FLIST_ARRAY_SIZE 128

/*
 * GET_SIZE() is a 'function' that returns a index into the 'hash'
 * variable, given a specific number. The index returned will be the
 * index of the ptr to the list of free memory entries that is identical
 * or slightly bigger than the parameter in size.
 */
#define GET_SIZE(mt,a) (mt->hash[((a)+3)>>2])

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
 * 8192*4 == 1<<15
 * JO was 13
 */
#define mem_hash_func(a) (((a)>>15)%MEMINFO_HASHSIZE)

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
#define NUMBER_SIZES 25 /* JO - was 19 */
static const int sizes[NUMBER_SIZES] =
                           {    8,   12,   16,   24,   32,   48,   64,   96,
                              128,  192 , 256,  384,  512,  768, 1024, 1536,
                             2048, 3072, 4096,
                             4096+2048, 8192,
                             8192+4096, 16384,
                             16384+8192,32768 };
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
#endif /* FLISTS */

typedef struct { /* mem_tsd: static variables of this module (thread-safe) */
#ifdef FLISTS
   /*
    * The array of pointers to the freelists having memory of the sizes
    * specified in 'sizes'. I.e. flists[0] is a pointer to a linked list
    * of free memory chunks of size 8, flist[1] to memory of size 12 etc.
    * The size of this array is the same as the size of 'sizes'.
    */
   char *flists[NUMBER_SIZES];

   /*
    * The 'hashtable'. Used for quick access to the size of a chunk of
    * memory, given its address.
    */
   meminfo *hashtable[ MEMINFO_HASHSIZE ];

   /*
    * These variables track the allocation of memory allocated by
    * MallocTSD() and used by the internal memory allocation
    * mechanism.
    */
   meminfo *first_entry;
   meminfo *curr_entry;
   /*
    * Array used for rounding a number to an 'approved' size, i.e. a size
    * in which the interpreter will allocate memory. Remember that the
    * approved sizes are {8,12,16,24,32 ...}? This function will return
    * 8 for 1 through 8; 12 for 9 through 12; 16 for 13 through 16 etc.
    * It is not initially set, but will be set by init_memory().
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
   short hash[ CHUNK_SIZE/4 + 1 ] ;

   /* See add_entry() for the following two variables */
   meminfo *mem_ctl ;
   int mem_ctl_idx ;
#endif

#ifdef TRACEMEM
   /*
    * Counter for dynamically memory allocated, in bytes, and ditto for
    * the deallocated memory, dynamic memory currently in use is the
    * difference between these. This is only used for gathering
    * statistics.
    */
   int allocated ;
   int deallocated ;

   /*
    * Sequence number for newly allocated memory, incremented for each
    * new allocation of dynamic memory. Actually, it is stored as a
    * unsigned short in the memhead of each memory allocation. That might
    * be slightly too small, since the number of memory allocation can
    * easily reach 100000, even for relatively small programs.
    *
    * Therefore, the sequence number might be stored as a 24 bit number,
    * (on 32 bit machines). But anyway, who cares, it is only used for
    * debugging purposes.
    */
   int sequence ;

   /*
    * Pointer to last (most newly) allocated memorychunk in double linked
    * list of all allocated dynamic memory.
    */
   struct memhead *header0 ;

#define MAX_MARKERS 100
   void (*(markers[MAX_MARKERS]))(const tsd_t *TSD) ;
   int max_markers_regd ;
#endif
   int FillerForThisStructure;
} mem_tsd_t; /* thread-specific but only needed by this module. see
              * init_memory
              */

#ifdef FLISTS
static void init_hash_table( mem_tsd_t *mt ) ;
# ifdef REGINA_DEBUG_MEMORY
static int show_a_free_list(mem_tsd_t *mt, int bin, char *str );
# endif
#endif

/*
 * This function initiates the 'mem_tsd'. This might have been
 * done initially, since the values in this will never change. But
 * since the size is rather big. it is more efficient to spend some
 * CPU on initiating it. The startup time might be decreased by swapping
 * this routine for a pre-defined variable. Perhaps it should be
 * rewritten to use two arrays, one for large pieces of memory and
 * one for small pieces. That would save space in 'hash'
 *
 * The values put into the array has been described above.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_memory( tsd_t *TSD )
{
   mem_tsd_t *mt;

   if (TSD->mem_tsd != NULL)
      return(1);

   if ( ( TSD->mem_tsd = TSD->MTMalloc( TSD, sizeof(mem_tsd_t) ) ) == NULL )
      return(0);
   mt = (mem_tsd_t *)TSD->mem_tsd;
   memset( mt, 0, sizeof(mem_tsd_t) );
   /*
    * Don't register this chunk of memory! It contains the list of all
    * other blocks of memory to be freed!
    */
#ifdef FLISTS
   init_hash_table( mt ) ;
#endif
   return(1);
}

#ifdef FLISTS
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
static void init_hash_table( mem_tsd_t *mt )
{
   int indeks ;   /* index into current element to be initiated */
   int j ;
   int size ;
   int num ;

   /* Force allocation of the first mem_ctl: */
   mt->mem_ctl_idx = FLIST_ARRAY_SIZE ;

   /*
    * Set the few lowest values manually, since the algoritm breaks
    * down for sufficient small values.
    */
   indeks = 0 ;
   mt->hash[indeks++] = 0 ;  /* when size equals 0, well ... 8 :-) */
   mt->hash[indeks++] = 0 ;  /* for 1 <= size < 4 */
   mt->hash[indeks++] = 0 ;  /* for 4 <= size < 8 */

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
         mt->hash[indeks++] = (short) num ;
      num++ ;

      /*
       * Initialize the second in each pair: 12, 24, 48, 96 ... etc
       */
      for (j=0; j<size; j++ )
         mt->hash[indeks++] = (short) num ;
      num++ ;
      size = size << 1 ;
   }

   /*
    * Do I need this? I don't think so. It is a kludge to make something
    * work on 64 bit machines, but I don't think it is needed anymore.
    * Just let is be commented out, and the delete it if things seem
    * to work.
    */

   /* We need to know if "int" or "int*" ar larger than 4 bytes. Since the
    * sizeof operator is evaluated at compile time and we don't want to
    * get a message "expression is constant" we use a more complex
    * expression. We assume that there is no machine with an "int"-size
    * of 2 and a pointer size of 8 or something else.
    */
   size = sizeof(int) ;
   size += sizeof(int*) ;
   if (size > 8)
      mt->hash[3] = 2 ;
   memset( mt->flists, 0, NUMBER_SIZES * sizeof(char *) );
}
/*
 * This function stores in a singly linked list all chunks of memory
 * that are allocated with malloc(). This is so that they can all be
 * free()ed by the_free_flists().
 */
static int register_mem( const tsd_t *TSD, void *chunk )
{
   meminfo *mem=NULL;
   mem_tsd_t *mt=(mem_tsd_t *)TSD->mem_tsd;

   if ( ( mem = (meminfo *)TSD->MTMalloc( TSD, sizeof(meminfo) ) ) == NULL )
      return(1);
   mem->start = (char *)chunk;
   mem->next = NULL;
   if (mt->curr_entry)
   {
      mt->curr_entry->next = mem;
   }
   mt->curr_entry = mem;
   if (mt->first_entry == NULL)
      mt->first_entry = mt->curr_entry;

#ifdef REGINA_DEBUG_MEMORY3
   fprintf(stderr,"--register_mem(): added %x to malloced memory\n",chunk);
#endif
   return(0);
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
static void add_entry( const tsd_t *TSD, char *start, const char *addr, int bin_no )
{
   mem_tsd_t *mt = (mem_tsd_t *)TSD->mem_tsd;
   meminfo *ptr ;              /* work ptr */
   int tmp ;                   /* tmp storage for mem_hash_func() */

   /*
    * If we have used all free meminfo-boxes, allocate more. This is
    * forces upon us at the first invocation. Allocate space for 128
    * at a time.
    */
   if (mt->mem_ctl_idx>=FLIST_ARRAY_SIZE)
   {
      /* Stupid SunOS acc gives incorrect warning for the next line */
      if  ( ( mt->mem_ctl = (meminfo *)TSD->MTMalloc( TSD, sizeof( meminfo) * FLIST_ARRAY_SIZE ) ) == NULL )
          exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
      mt->mem_ctl_idx = 0 ;
      if ( register_mem( TSD, mt->mem_ctl ) )
          exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
   }

   /*
    * Fill in the fields of the box, and put it in the front of the
    * requested bin in hashtable
    */
   ptr = &mt->mem_ctl[ mt->mem_ctl_idx++ ] ;
   ptr->next = mt->hashtable[tmp=mem_hash_func((unsigned long)addr)] ;
   ptr->size = bin_no ;
   ptr->start = start ;
   mt->hashtable[tmp] = ptr ;
}


/*
 * Allocate a piece of memory. The size is given as the 'size' parameter.
 * If size is more than MAX_INTERNAL_SIZE, it will call malloc()
 * directly, else, it will return a piece of memory from the freelist,
 * after possibly filling the freelist with more memory if is was
 * empty in the first place.
 */
void *get_a_chunk( int size )
{
   return get_a_chunkTSD(__regina_get_tsd(), size) ;
}

void *get_a_chunkTSD( const tsd_t *TSD, int size )
{
   int bin ;     /* bin no in array of freelists */
   char *vptr ;  /* holds the result */
   void *result ;
   mem_tsd_t *mt;
   char *ptr ;      /* work ptr, to loop through the memory */
   char *topaddr ;  /* points to last item in memory */
#ifdef REGINA_DEBUG_MEMORY
   int before, after;
#endif

#ifdef REGINA_DEBUG_MEMORY1
   fprintf(stderr,"get_a_chunkTSD(): want %d bytes...",size);
#endif

   mt = (mem_tsd_t *)TSD->mem_tsd;

   /*
    * If memory is too big, let malloc() handle the problem.
    */
   if (size>MAX_INTERNAL_SIZE)
   {
      if ((result=TSD->MTMalloc( TSD, size )) != NULL)
      {
#ifdef REGINA_DEBUG_MEMORY1
         fprintf(stderr,"got %d at %x (after allocating with malloc)\n",size,result);
#endif
/*
 * removing this fixes memory leak in 908114 - MHES 08-March-2004
         if ( register_mem( TSD, result ) )
            exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
 */
         return result ;
      }
      else
         exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
   }

   /*
    * Get the first item from the appropriate freelist, and let 'vptr'
    * point to it. Simultaneously set bin to the bin no in 'flists'
    * to avoid recalculating the number. If the freelist is empty
    * (i.e vptr==NULL) then allocate more memory.
    */
   if ((vptr=mt->flists[bin=GET_SIZE(mt,size)])==NULL)
   {
      /*
       * Allocate the memory, and set both vptr and initiate the
       * right element in 'flists'. Note that the value in 'flists' is
       * 'incremented' later, so it must be set to the value which now
       * is to be allocated.
       */
      vptr = (char *)TSD->MTMalloc( TSD, CHUNK_SIZE ) ;
      if (!vptr)
          exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;

#ifdef REGINA_DEBUG_MEMORY1
      fprintf(stderr,"got %d at %x in bin %d (after allocating %d bytes)\n",sizes[bin],vptr,bin,CHUNK_SIZE);
#endif
      if ( register_mem( TSD, vptr ) )
          exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
      mt->flists[bin] = vptr ;
#ifdef REGINA_DEBUG_MEMORY3
      memset( vptr, 0xC0+bin, CHUNK_SIZE );
#endif

      /*
       * Calculate the top address of the memory allocated, and put
       * the memory into 'topaddr'. Then register the chunk of memory
       * in both the possible CHUNK_SIZE segments of the machine. In
       * some rare cases the last registration might not be needed,
       * but do it anyway, to avoid having to determine it.
       */
      topaddr = vptr + CHUNK_SIZE - sizes[bin] ;
      add_entry( TSD, vptr, vptr, bin ) ;
      add_entry( TSD, vptr, vptr + CHUNK_SIZE, bin ) ;

      /*
       * Then loop through the individual pieced of memory within the
       * newly allocated chunk, and make it a linked list, where the
       * last ptr in the list is NULL.
       */
      for (ptr=vptr; ptr<topaddr; ptr=ptr+sizes[bin] )
         *(char**)ptr = ptr + sizes[bin] ;

      *((char**)(ptr-sizes[bin])) = NULL ;

#ifdef REGINA_DEBUG_MEMORY
      show_a_free_list( mt, bin, "get_a_chunkTSD(): empty freelist for ");
#endif
   }
#ifdef REGINA_DEBUG_MEMORY1
   else
   {
      fprintf(stderr,"got %d at %x in bin %d\n",sizes[bin],vptr,bin);
   }
#endif

   /*
    * Update the pointer in 'flist' to point to the next entry in the
    * freelist instead of the one we just allocated, and return to
    * caller.
    */
#ifdef REGINA_DEBUG_MEMORY2
   before = show_a_free_list( mt, bin, NULL);
#endif
   mt->flists[bin] = (*((char**)(vptr))) ;
#ifdef REGINA_DEBUG_MEMORY2
   after = show_a_free_list( mt, bin, NULL );
   if ( before - 1 != after )
      fprintf(stderr,"****** get_a_chunkTSD() for bin [%d] failed. Before %d After %d\n",bin,before,after);
#endif
   return (vptr) ;
}


streng *get_a_streng( int size )
{
   return get_a_strengTSD(__regina_get_tsd(), size) ;
}

streng *get_a_strengTSD( const tsd_t *TSD, int size )
{
   register int bin ;     /* bin no in array of freelists */
   register char *vptr ;  /* holds the result */
   register streng *result ;
   mem_tsd_t *mt;
   char *ptr ;      /* work ptr, to loop through the memory */
   char *topaddr ;  /* points to last item in memory */
#ifdef REGINA_DEBUG_MEMORY
   int before, after;
#endif

   size = size + STRHEAD;

   /*
    * If memory is too big, let malloc() handle the problem.
    */
#ifdef REGINA_DEBUG_MEMORY1
   fprintf(stderr,"get_a_strengTSD(): want %d bytes...",size-STRHEAD);
#endif
   if (size>MAX_INTERNAL_SIZE)
   {
      if ( ( result = (streng *)TSD->MTMalloc( TSD, size ) ) != NULL )
      {
         result->len = 0 ;
         result->max = size-STRHEAD ;
/*
 * removing this fixes memory leak in 908114 - MHES 08-Mark-2004
         if ( register_mem( TSD, result ) )
            exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
 */
         return result ;
      }
      else
          exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
   }

   mt = (mem_tsd_t *)TSD->mem_tsd;

   /*
    * Get the first item from the appropriate freelist, and let 'vptr'
    * point to it. Simultaneously set bin to the bin no in 'flists'
    * to avoid recalculating the number. If the freelist is empty
    * (i.e vptr==NULL) then allocate more memory.
    */
   if ((vptr=mt->flists[bin=GET_SIZE(mt,size)])==NULL)
   {
      /*
       * Allocate the memory, and set both vptr and initiate the
       * right element in 'flists'. Note that the value in 'flists' is
       * 'incremented' later, so it must be set to the value which now
       * is to be allocated.
       */
      vptr = (char *)TSD->MTMalloc( TSD, CHUNK_SIZE ) ;
      if (!vptr)
          exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
      if ( register_mem( TSD, vptr ) )
         exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;
      mt->flists[bin] = vptr ;
#ifdef REGINA_DEBUG_MEMORY3
      memset( vptr, 0xA0+bin, CHUNK_SIZE );
#endif

      /*
       * Calculate the top address of the memory allocated, and put
       * the memory into 'topaddr'. Then register the chunk of memory
       * in both the possible CHUNK_SIZE segments of the machine. In
       * some rare cases the last registration might not be needed,
       * but do it anyway, to avoid having to determine it.
       */
      topaddr = vptr + CHUNK_SIZE - sizes[bin] ;
      add_entry( TSD, vptr, vptr, bin ) ;
      add_entry( TSD, vptr, vptr + CHUNK_SIZE, bin ) ;

      /*
       * Then loop through the individual pieced of memory within the
       * newly allocated chunk, and make it a linked list, where the
       * last ptr in the list is NULL.
       */
      for (ptr=vptr; ptr<topaddr; ptr=ptr+sizes[bin] )
         *(char**)ptr = ptr + sizes[bin] ;

      *((char**)(ptr-sizes[bin])) = NULL ;
#ifdef REGINA_DEBUG_MEMORY1
      fprintf(stderr,"got %d at %x (after allocating %d bytes)\n",sizes[bin],vptr,CHUNK_SIZE);
#endif

#ifdef REGINA_DEBUG_MEMORY
      show_a_free_list( mt, bin, "get_a_strengTSD(): empty freelist for ");
#endif
   }
#ifdef REGINA_DEBUG_MEMORY1
   else
   {
      fprintf(stderr,"got %d at %x\n",sizes[bin],vptr);
   }
#endif

   /*
    * Update the pointer in 'flist' to point to the next entry in the
    * freelist instead of the one we just allocated, and return to
    * caller.
    */
#ifdef REGINA_DEBUG_MEMORY2
   before = show_a_free_list( mt, bin, NULL );
#endif
   mt->flists[bin] = (*((char**)(vptr))) ;
   ((streng *)vptr)->len = 0 ;
   ((streng *)vptr)->max = size-STRHEAD ;
#ifdef REGINA_DEBUG_MEMORY2
   after = show_a_free_list( mt, bin, NULL );
   if ( before - 1 != after )
      fprintf(stderr,"****** get_a_strengTSD() for bin [%d] failed. Before %d After %d\n",bin,before,after);
#endif

   return ((streng *)vptr) ;
}

/*
 * Shortcut to deallocate a streng. Since we know the max size of a
 * streng, we don't really need to calculate the size using of the
 * hashtable structure. That saves some time, since a lot of the
 * memorychunks in rexx are strengs.
 *
 * Note that strengs can just as well be deallocated using the
 * 'normal' method, but this interface saves some time. Just a thought:
 * if all allocated string were sure to have a max size that did not
 * waste any memory, we didn't have to expand the GET_SIZE macro,
 * and thereby saving even a few more cycles
 */
void give_a_streng( streng *ptr )
{
   give_a_strengTSD(__regina_get_tsd(), ptr) ;
}

void give_a_strengTSD( const tsd_t *TSD, streng *ptr )
{
   char **tptr ;   /* tmp variable, points to element in flists */
   mem_tsd_t *mt;
   register int bin;
#ifdef REGINA_DEBUG_MEMORY
   int before, after;
#endif

#ifdef REGINA_DEBUG_MEMORY1
   fprintf(stderr,"give_a_strengTSD() going to free %x...", ptr );
#endif
   assert( ptr->len <= ptr->max ) ;
   if ((ptr->max+STRHEAD) > MAX_INTERNAL_SIZE)  /* off-by-one error ? */
   {
      TSD->MTFree(TSD, ptr ) ;
#ifdef REGINA_DEBUG_MEMORY1
      fprintf(stderr,"freed by MTFree()\n" );
#endif
      return ;
   }

   mt = (mem_tsd_t *)TSD->mem_tsd;

   /*
    * First find the right element in flists, then link this piece
    * of memory into the start of the list, clean and simple. 'tptr'
    * is the old first element in the freelist, and 'ptr' is the
    * memory to free.
    */
   bin = GET_SIZE(mt,ptr->max + STRHEAD);
#ifdef REGINA_DEBUG_MEMORY1
   fprintf(stderr,"freed from bin%d\n",bin );
#endif
#ifdef REGINA_DEBUG_MEMORY2
   before = show_a_free_list( mt, bin, NULL );
#endif
   tptr = &mt->flists[bin] ;
   *((char**)ptr) = *tptr ;
   *tptr = (char*)ptr ;
#ifdef REGINA_DEBUG_MEMORY2
   after = show_a_free_list( mt, bin, NULL );
   if ( before + 1 != after )
      fprintf(stderr,"****** give_a_strengTSD() for bin [%d] failed. Before %d After %d\n",bin,before,after);
#endif
}

/*
 * The standard interface to freeing memory. The parameter 'ptr' is
 * a pointer to the memory to be freed, is put first in the freelist
 * pointed to by the appropriate element in 'flists'.
 *
 * I am not really sure what cptr do in this, but I think it has
 * something to do with *void != *char on Crays ... The main consumer
 * of CPU in this routine is the for(;;) loop, it should be rewritten.
 */
void give_a_chunk( void *ptr )
{
   give_a_chunkTSD(__regina_get_tsd(), ptr) ;
}

void give_a_chunkTSD( const tsd_t *TSD, void *ptr )
{
   char *cptr ;      /* pseudonym for 'ptr' */
   meminfo *mptr ;   /* caches the right element in hashtable */
   mem_tsd_t *mt;
#ifdef REGINA_DEBUG_MEMORY
   int before, after;
#endif

   mt = (mem_tsd_t *)TSD->mem_tsd;
   /*
    * initialize a few values, 'cptr' is easy, while 'mptr' is the
    * list of values for this piece of memory, that is in the
    * hashtable that returns memory size given a specific address
    */
   cptr = (char*)ptr ;
   mptr = mt->hashtable[ mem_hash_func( ((unsigned long)cptr) ) ] ;
#ifdef REGINA_DEBUG_MEMORY1
   fprintf(stderr,"give_a_chunkTSD() going to free %x hashtable %d...", ptr, mem_hash_func( ((unsigned int)cptr) ) );
#endif

   /*
    * For each element in the list attached to the specific hashvalue,
    * loop through the list, and stop at the entry which has a start
    * address _less_ than 'cptr' and a stop address _higher_ than
    * 'cptr' (i.e. cptr is within the chunk.)
    */
   for ( ; (mptr) && ((mptr->start+CHUNK_SIZE<=cptr) || (mptr->start>cptr)); mptr = mptr->next) ;

   /*
    * Now, there are two possibilities, either is mptr==NULL, in which
    * case this piece of memory is never registered in the system, or
    * then we have more information. In the former case, just give
    * the address to free(), hoping it knows more. In the latter, put
    * the memory on the appropriate freelist.
    */
   if (mptr)
   {
#ifdef REGINA_DEBUG_MEMORY1
      fprintf(stderr,"freed from bin %d\n", mptr->size );
#endif
      /*
       * Link it into the first place of the freelist.
       */
#ifdef REGINA_DEBUG_MEMORY2
      before = show_a_free_list( mt, mptr->size, NULL );
#endif
      *((char**)cptr) = mt->flists[mptr->size] ;
      mt->flists[mptr->size] = cptr ;
#ifdef REGINA_DEBUG_MEMORY2
      after = show_a_free_list( mt, mptr->size, NULL );
      if ( before + 1 != after )
         fprintf(stderr,"****** give_a_chunkTSD() for bin [%d] failed. Before %d After %d\n",mptr->size,before,after);
#endif
   }
   else
   {
#ifdef REGINA_DEBUG_MEMORY1
      fprintf(stderr,"freed from MTFree()\n" );
#endif
      TSD->MTFree(TSD, ptr ) ;
   }
}

/*
 * This function frees up all memory allocated by the flists internal
 * meomory management routines
 */
void purge_flists( const tsd_t *TSD )
{
   meminfo *ptr;
   meminfo *next;
   mem_tsd_t *mt;

   mt = (mem_tsd_t *)TSD->mem_tsd;
   ptr = mt->first_entry;

   while( ptr )
   {
      next = ptr->next;
      TSD->MTFree(TSD, ptr ) ;
      ptr = next;
   }
   mt->first_entry = mt->curr_entry = NULL;
   return;
}

# ifdef REGINA_DEBUG_MEMORY
static int show_a_free_list(mem_tsd_t *mt, int bin, char *str)
{
   char *ptr;
   int j=0;

   if ( mt->flists[bin] == NULL )
   {
      if (str)
         fprintf(stderr,"%sbin[%d] Free List unallocated Maximum %d Size: %d\n",str,bin,(CHUNK_SIZE / sizes[bin]),sizes[bin]);
   }
   else
   {
      for (j=1,ptr=mt->flists[bin]; *(char**)ptr!=NULL && j<5000; ptr=*(char **)ptr,j++ )
      {
         ;
      }
      if (str)
         fprintf(stderr,"%sbin[%d] Number in Free List %d Maximum %d Size: %d\n",str,bin,j,(CHUNK_SIZE / sizes[bin]), sizes[bin]);
   }
   return j;
}

int show_free_lists(const tsd_t *TSD)
{
   int num_bins = sizeof(sizes)/sizeof(sizes[0]);
   int i;
   mem_tsd_t *mt;

   mt = (mem_tsd_t *)TSD->mem_tsd;
   for ( i = 0; i < num_bins; i++ )
   {
      show_a_free_list( mt, i, "" );
   }
   return 0;
}
# endif /* REGINA_DEBUG_MEMORY */

#endif /* FLISTS */

#ifdef TRACEMEM
/*
 * Strings used to mark chunks of memory when listing dynamically
 * allocated memory in listleaked(). Length is max 8 chars.
 *
 * NOTE: There is a close correspondace between these and the cpp
 *       variables TRC_* in defs.h. If you change one of them, please
 *       change the other too.
 */
static const char *allocs[] = {
   "leaked",     /* status unknown, probably leaked */
   "hashtab",    /* holds hashtable in variable subsystem */
   "procbox",    /* the info local to a single routine */
   "source",     /* a line of source code */
   "srcbox",     /* box in list of source lines */
   "treenode",   /* node in the parse three */
   "var_val",    /* value of a variable */
   "var_nam",    /* name of a variable */
   "var_box",    /* other structure in the variable subsystem */
   "stc_box",    /* box in linked list of the stack lines */
   "stc_line",   /* stack line */
   "sys_info",   /* the common info for a whole program */
   "file_ptr",   /* holds the filetable */
   "proc_arg",   /* holds arguments for internal or builtin functions */
   "label",      /* holds info about labels */
   "static",     /* names of special variables */
   "argcache",   /* the proc argument cache */
   "math",       /* dynamic workarrays in the math funcstion */
   "envirbx",    /* box holding environment definition */
   "envirnm",    /* name in a box holding environment definition */
   "spcvarbx",   /* special variable box */
   "spcvarnm",   /* special variable name */
   "spcnumbx",   /* special number box */
   "spcnumnm",   /* special number contents */
   NULL          /* terminator */
} ;


/*
 * This routine obtains memory, either through get_a_chunk, or through
 * malloc() if we are not running with freelists. The memory requested
 * will be increased with the size of a memhead structure (32 bytes on
 * 'normal' 32 bit machines).
 *
 * The function also updates the statistics, linkes it into the list
 * of currently allocated memory, and might pattern the memory.
 */
void *mymalloc( int bytes )
{
   return mymallocTSD(__regina_get_tsd(), bytes) ;
}

void *mymallocTSD( const tsd_t *TSD, int bytes )
{
   struct memhead *memptr ;  /* holds the result */
   mem_tsd_t *mt;

   mt = (mem_tsd_t *)TSD->mem_tsd;

   /*
    * Increase the size of the memory wanted, so we can put the
    * header into it first. You'd better not have played with the
    * parameters above in such a way that the result is non-aligned.
    */
   mt->allocated += (bytes += sizeof(struct memhead) ) ;

   /*
    * Do the actual allocation of memory, either call get_a_chunk()
    * or be boring and call plain old malloc(). In either case,
    * chicken out if there are not any more memory left. Hmmm, this
    * situation should be handled better. Memory management should
    * be transaction oriented
    */
#ifdef FLISTS
   if ((memptr=get_a_chunkTSD(TSD,bytes)) == NULL)
#else
   if ((memptr = (struct memhead *) TSD->MTMalloc(TSD,bytes)) == NULL)
#endif
       exiterror( ERR_STORAGE_EXHAUSTED, 0 )  ;

#ifdef PATTERN_MEMORY
   /*
    * If the options for memory patterning is set, perform it. This is
    * only useful during debugging, to provoke error due to the use of
    * uninitialized variables. Other than that, it is just a pure waste
    * of CPU.
    */
   memset( memptr, NOT_USED, bytes ) ;
#endif /* PATTERN_MEMORY */

   /*
    * Fill in the fields of the header: the size, the sequence number,
    * the magic number, initialize the flag, and then link it into the
    * linked list of allocated memory, at the start of the list.
    */
   memptr->count = bytes ;
   memptr->flag = 0 ;
   memptr->magic = MAGIC_COOKIE ;
   memptr->seqv = (unsigned short) ++mt->sequence ;
   memptr->prev = NULL ;
#ifdef HAVE_BUILTIN_RETURN_ADDRESS
   {
      void *p;

      p = __builtin_return_address(0);

      if ( ( (unsigned long) p >= (unsigned long) __regina_Str_first ) &&
           ( (unsigned long) p <= (unsigned long) __regina_Str_last ) )
      {
         p = __builtin_return_address(1);

         if ( ( (unsigned long) p >= (unsigned long) __regina_Str_first ) &&
              ( (unsigned long) p <= (unsigned long) __regina_Str_last ) )
         {
            p = __builtin_return_address(2);

            if ( ( (unsigned long) p >= (unsigned long) __regina_Str_first ) &&
                 ( (unsigned long) p <= (unsigned long) __regina_Str_last ) )
            {
               p = __builtin_return_address(3);
            }
         }
      }

      memptr->addr = p;
   }
#else
   memptr->addr = NULL;
#endif
   memptr->next = mt->header0 ;
   if (mt->header0)
      mt->header0->prev = memptr ;

   /*
    * Increment the pointer to the start of the memory that the user
    * is allowed to use, i.e past the header. The return.
    */
   mt->header0 = memptr++ ;
   return memptr ;
}



/*
 * myfree takes a pointer to memory to be deallocated, it is a wrapper
 *    for free(3), and does some housekeeping tasks
 */
void myfree( void *cptr )
{
   myfreeTSD(__regina_get_tsd(), cptr) ;
}

void myfreeTSD( const tsd_t *TSD, void *cptr )
{
   struct memhead *memptr ;   /* ptr to memory to be freed */
   mem_tsd_t *mt;

   mt = (mem_tsd_t *)TSD->mem_tsd;
   /*
    * The header part of the memory is prepended to the part of the
    * memory that the user saw, so move the pointer backwards to the
    * start of the header.
    */
   memptr = ((struct memhead *)cptr) - 1 ;

   /*
    * If the magic cookie is not intact, there must be some serious
    * problems somewhere. Inform the user about it, and exit.
    */
   if (memptr->magic != MAGIC_COOKIE)
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   /*
    * Update the statistics. Remember that we do not decrement the
    * variable 'allocated'. The real number of memory allocated is
    * the difference between those two.
    */
   mt->deallocated -= memptr->count ;

   /*
    * Then unlink the chunk of memory from the linked list of allocated
    * memory. Set the pointers at its neighbors (if any) and set the
    * 'header' variable if it was first in the list.
    */
   if (memptr->next)
      memptr->next->prev = memptr->prev ;

   if (memptr->prev)
      memptr->prev->next = memptr->next ;
   else
      mt->header0 = memptr->next ;

#ifdef PATTERN_MEMORY
   /*
    * If we are to pattern the memory, overwrite the contents of the
    * memory, to provoke errors if parts of the interpreter use
    * memory after it have been deallocated.
    */
   memset( memptr, BEEN_USED, memptr->count ) ;
#endif

   /*
    * Then at last, deallocate the memory, either by giving it to
    * give_a_chunk (to be stored in the freelists) or by giving it
    * it directly to free().
    */
#ifdef FLISTS
   give_a_chunkTSD(TSD,memptr) ;
#else
   TSD->MTFree(TSD,memptr) ;
#endif
}



/* have_allocated returns the amount of dynamic memory that has been
 *    allocated, in bytes.
 */
int have_allocated( tsd_t *TSD, int flag )
{
   int result = -1 ;
   mem_tsd_t *mt;

   mt = (mem_tsd_t *)TSD->mem_tsd;
   switch ( flag )
   {
      case ( MEM_CURRENT ) :
         result = mt->allocated - mt->deallocated ;
         break ;

      case ( MEM_ALLOC ) :
         result = mt->allocated - mt->deallocated - listleaked( TSD, MEMTRC_NONE ) ;
         break ;

      case ( MEM_LEAKED ) :
         result = listleaked( TSD, MEMTRC_NONE ) ;
         break ;

      default :
          exiterror( ERR_INCORRECT_CALL, 0 )  ;
   }

   return result ;
}


void regmarker( const tsd_t *TSD, void (*marker)(const tsd_t *TSD) )
{
   mem_tsd_t *mt;

   mt = (mem_tsd_t *)TSD->mem_tsd;

   if (mt->max_markers_regd>=MAX_MARKERS)
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   mt->markers[mt->max_markers_regd++] = marker ;
}

/*
 * This routine will three distinct things. First it iterates through
 * all the memory currently allocated and mark them as leaked. Then
 * it calls in sequence all the routines that mark the memory that the
 * various parts of the system claims. The pieces of memory that is
 * still marked leaked are then unclaimed. At last it iterates
 * through the list of memory once more, and dumps info about those
 * that are unclaimed by any part of the interpreter.
 *
 * The parameter 'pflag' may have a value which is defined by the
 * macros MEMTRC_ in defs.h. These may be MEMTRC_NONE to not write out
 * anything; MEMTRC_ALL to list all memory; or MEMTRC_LEAKED which
 * only writes out the memory that is actually leaked.
 */
int listleaked( const tsd_t *TSD, int pflag )
{
   /*
    * Array of functions to call for marking all active chunks of dynamic
    * allocated memory. These will mark _all_ dynamically allocated
    * memory. Anything unmarked after all these routines are called,
    * must be leaked memory. Add more functions as you wish
    */
   static void (* const fptr[])( const tsd_t *TSD ) = {
      mark_stack,       /* the lines on the stack */
      mark_systeminfo,  /* the system information box */
      mark_filetable,   /* the file descriptor table */
      mark_param_cache, /* the parameter chache */
      mark_descrs,      /* memory used by sting math routines */
      mark_signals,     /* */
      NULL              /* terminator */
   } ;
   struct memhead *memptr ;  /* ptr that iterates through the memory */
   int i ;                   /* general loop control variable */
   int sum ;                 /* the sum of allocated memory */
   char *string ;            /* ptr to the current allocated memory */
   mem_tsd_t *mt;

   mt = (mem_tsd_t *)TSD->mem_tsd;
   /*
    * First, set the status of all pieces of memory to leaked.
    */
   for (memptr=mt->header0; memptr; memptr=memptr->next)
      memptr->flag = TRC_LEAKED ;

   /*
    * Then, call the functions that claims the memory that belongs to
    * the various parts of the system. These routines are stored in the
    * array 'fptr'. If you ever write anything that uses more memory,
    * be sure to add a function that is able to mark it, and append the
    * name of that function to 'fptr'. If you don't, and garbage
    * collection is implemented, you are in deep trouble.
    *
    * Note the mark_listleaked_params(), that is special, since it marks
    * the parameters that the is in use by during the calling of the
    * builtin function that invokes this function.
    */
   mark_listleaked_params( TSD ) ;
   for (i=0;fptr[i];i++)
      (*(fptr[i]))( TSD ) ;

   for (i=0; i<mt->max_markers_regd; i++)
     (*(mt->markers[i]))( TSD ) ;

   /*
    * Write out a header for the output, but only if we actually are to
    * trace the values.
    */
   if (! pflag==MEMTRC_NONE && TSD->stddump != NULL)
      fprintf(TSD->stddump," Len  Flg Tag      Seqv Address    Caller     Contents\n") ;

   /*
    * Then, loop through the allocated memory, and for each piece of
    * memory in the linked list, check to see if it is leaked. If we
    * were called with the MEMTRC_ALL flag, then list out for every
    * piece of memory.
    */
   for (sum=0,memptr=mt->header0; memptr; memptr=memptr->next)
   {
      if ((memptr->flag==TRC_LEAKED)||(pflag==MEMTRC_ALL))
      {
         /*
          * Keep an account on how much memory is actually in use. If
          * we are not to write anything out, skip the rest of this
          * iteration.
          */
         sum += memptr->count ;
         if (!(pflag==MEMTRC_NONE))
            {
            /*
             * Dump info about the current piece of memory. That includes
             * the size (excl the header), the flag and the string
             * belonging to the flag, and then the sequence number.
             */
            if (TSD->stddump != NULL)
               fprintf(TSD->stddump, "%5d %3d %-8s %4X %-10p %-10p \"",
                       memptr->count - sizeof(struct memhead),
                       memptr->flag, allocs[memptr->flag], memptr->seqv,
                       memptr, memptr->addr );

            /*
             * Dump the contents of the piece of memory. One piece of
             * memory per line in the output.
             */
            string = (char*)(memptr+1) ;
            if (TSD->stddump != NULL)
               for (i=0; i<(int)(memptr->count - sizeof(struct memhead)); i++ )
               {
                  if (i==40) /* bja 20->40 */
                  {
                     /*
                      * If it is more than 40 bytes long, terminate and write - bja 20->40
                      * out "..." to indicate that there are more bytes in
                      * the memory than was possible to write out.
                      */
                     fprintf(TSD->stddump, " ..." ) ;
                     break ;
                  }
                  /*
                   * Write out a byte. If it is not a printable character,
                   * write out a "." instead, to indicate this.
                   */
                  if ( rx_isprint( string[i] ) )
                     putc( string[i], TSD->stddump ) ;
                  else
                     putc( '~', TSD->stddump ) ;
               }
            if (TSD->stddump != NULL)
               fprintf( TSD->stddump, "\"\n" ) ;
         }
      }
   }
   return sum ;
}


/*
 * Marks a chunk of memory pointed to by 'ptr' to be of the kind
 * referenced in 'flag'. Might be defined as a macro, but since memory
 * garbagecollection is just for debugging purposes, there is really
 * no need to worry about that now.
 */
void markmemory( void *ptr, int flag )
{
  struct memhead *memptr ;  /* work pointer to memory to be marked */

  /*
   * It's rather simple, ptr is non-NULL, decrement the memptr pointer
   * to the start of the header, and set the flag. I am not sure
   * whether an internal error should be given if 'ptr' is NULL.
   * Maybe lots of code could be extracted from other parts of the
   * interpreter if they don't have to worry about not sending NULL
   * pointer to markmemory()?
   *
   * That is hardly a problem now, since this is only used for debugging.
   * The only confusing part of this routine might be the casting.
   */
  if ((memptr=((struct memhead *)ptr)) != NULL)
  {
     memptr-- ;
     memptr->flag = (unsigned char) flag ;
  }
  else
      exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
}


/*
 * This is really a simple routine, to write out the values of some
 * of the statistics gathered during (de)allocation of memory. Maybe it
 * should return the answer instead?
 */
void memory_stats(const tsd_t *TSD)
{
   mem_tsd_t *mt;

   mt = (mem_tsd_t *)TSD->mem_tsd;
   if (TSD->stddump == NULL)
      return;
   fprintf(TSD->stddump,
        "Allocated %d bytes in %d chunks, of which %d is deallocated\n",
        mt->allocated, mt->sequence, mt->deallocated ) ; /* bja - variables out of order */
}

#endif /* TRACEMEM */

#ifdef CHECK_MEMORY
#  if defined(TRACEMEM) || defined(FLISTS)
#     error CHECK_MEMORY should only be defined if FLISTS and TRACEMEM are not defined. Please, check the header files.
#  endif

void give_a_streng( streng *ptr )
{
   give_a_strengTSD( __regina_get_tsd(), ptr ) ;
}

void give_a_strengTSD( const tsd_t *TSD, streng *ptr )
{
/*
 * The assert is not really needed if we check for ptr!=NULL for the
 * free(ptr->value). Note, that free(NULL) is allowed in ANSI. But we will
 * check for error free code in case of !defined(CHECK_MEMORY), thus, we
 * assert the freeing. FGC
 */
   assert((ptr != NULL) && (ptr->value != NULL));
   TSD->MTFree(TSD,ptr->value);
   TSD->MTFree(TSD,ptr);
}

#endif /* CHECK_MEMORY */

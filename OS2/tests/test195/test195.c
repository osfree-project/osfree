#define INCL_DOSMEMMGR

#define INCL_DOSERRORS

#include <os2.h>

#include <stdio.h>



#define NUMBLOCKS  128               /* Number of blocks in pool          */

#define POOLSIZE   (NUMBLOCKS*80+64) /* 128 blocks of 80 bytes each.  OS/2

                                        needs 64 bytes to manage the pool */

int main (VOID)

{

  PVOID   pvMemBase       = NULL;       /* Pointer to memory object (pool)*/

  PVOID   apvBlock[NUMBLOCKS] = {NULL}; /* Pointers to memory blocks      */

  ULONG   i               = 0;          /* A loop index                   */

  APIRET  rc              = NO_ERROR;   /* Return code                    */



    /* Allocate the memory pool.  Note the pool is intentionally NOT

       committed here.  OS/2 will commit the memory as needed.           */



  rc = DosAllocMem( &pvMemBase, POOLSIZE, PAG_WRITE );

  if (rc != NO_ERROR) {

    printf("DosAllocMem error:  return code = %u\n", rc);

    return 1;

  }

       /* Make the entire memory object available for Suballocation */



  rc = DosSubSetMem( pvMemBase, DOSSUB_INIT | DOSSUB_SPARSE_OBJ, POOLSIZE );

  if (rc != NO_ERROR) {

    printf("DosSubSetMem error:  return code = %u\n", rc);

    return 1;

  } else { printf("Memory object ready for suballocation.\n"); }



   /* Suballocate the pool into 128 memory blocks, each 80 bytes in size */



  for( i = 0; i < NUMBLOCKS; i++ ) {

    rc = DosSubAllocMem( pvMemBase, &apvBlock[ i ], 80 );

    if( rc != NO_ERROR ) {

      printf( "DosSubAllocMem error: return code = %u  (i = %u)\n", rc, i );

      break;             }

  }



                     /*    USE THE POOL HERE... */

             /* When done, free the 128 memory blocks */



  do {

    rc = DosSubFreeMem(  pvMemBase, apvBlock[ --i ], 80 );

    if (rc != NO_ERROR) {

      printf("DosSubFreeMem error:  return code = %u  (i = %u)\n", rc, i);

      return 1;

    }

  } while( i );



        /* Stop memory block management, and free the memory object */



  rc = DosSubUnsetMem( pvMemBase );

  if (rc != NO_ERROR) {

    printf("DosSubUnsetMem error:  return code = %u\n", rc);

    return 1;

  }



  rc = DosFreeMem( pvMemBase );

  if (rc != NO_ERROR) {

    printf("DosFreeMem error:  return code = %u\n", rc);

    return 1;

  } else {

    printf("Memory freed.\n");

  }



  return NO_ERROR;

}

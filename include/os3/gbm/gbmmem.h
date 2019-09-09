/**
 * gbmmem.h - GBM memory management functions
 */

#ifndef _GBMMEM_H_
#define _GBMMEM_H_

#include <malloc.h>

#ifdef __cplusplus
  extern "C"
  {
#endif

/**
 * Allocates a memory buffer from heap.
 *
 * @param size  The buffer size in bytes.
 *
 * @return The pointer to the buffer or NULL in case of memory error.
 */
void * gbmmem_malloc(size_t size);

/**
 * Changes the size of a previously reserved storage block. The ptr argument points
 * to the beginning of the block. The contents of the block are unchanged up to the
 * shorter of the new and old sizes. gbmmem_realloc allocates the new block from
 * the same heap the original block was in.
 * If ptr is NULL, gbmmem_realloc reserves a block of storage of size bytes from the
 * current thread's default heap (equivalent to calling gbmmem_malloc(size)).
 * If size is 0 and the ptr is not NULL, gbmmem_realloc frees the storage allocated
 * to ptr and returns NULL.
 *
 * @param ptr   The pointer of the block to be resized.
 * @param size  The new size of the block in bytes.
 *
 * @return The pointer to the buffer or NULL in case of memory error.
 *
 * @note gbmmem_realloc returns a pointer to the reallocated storage block.
 * The storage location of the block may be moved by the gbmmem_realloc function.
 * Thus, the ptr argument to gbmmem_realloc is not necessarily the same as the
 * return value.
 * If size is 0, realloc returns NULL. If there is not enough storage to
 * expand the block to the given size, the original block is unchanged
 * and gbmmem_realloc returns NULL.
 */
void * gbmmem_realloc(void *ptr, size_t size);

/**
 * Reserves storage space for an array of num elements, each of length size bytes.
 * It then gives all the bits of each element an initial value of 0.
 *
 * @param num   Number of elements.
 * @param size  Size of one element.
 *
 * @return The pointer to the buffer or NULL in case of memory error.
 */
void * gbmmem_calloc(size_t num, size_t size);

/**
 * Free a memory buffer allocated previously with gbmmem_malloc().
 *
 * @param buffer  The pointer to the buffer.
 */
void gbmmem_free(void * buffer);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif /* _GBMMEM_H_ */


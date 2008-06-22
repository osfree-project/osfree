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


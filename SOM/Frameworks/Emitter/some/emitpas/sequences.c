#include <string.h>
#include <stdlib.h>
#include "sequences.h"

#define LIST_BYTES_INCR 1280

SEQRET seqInit(PIDLSEQUENCE seq, unsigned long max, unsigned long objSize)
{
  if ( !seq ) return SEQ_INVALID_PARAM;
  seq->maximum = max;
  seq->length = 0;
  seq->objects = NULL;
  return SEQ_NO_ERROR;
}

SEQRET seqDone(PIDLSEQUENCE seq, unsigned long objSize,
               ProcessSeqItemFunc itemDestroy)
{
  seqForEach(seq, objSize, itemDestroy);
  return seqClear(seq);
}

PIDLSEQUENCE seqNew(unsigned long max, unsigned long objSize)
{
  PIDLSEQUENCE	seq;

  seq = malloc( sizeof(IDLSEQUENCE) );
  seqInit(seq, max, objSize);
  return seq;
}

SEQRET seqDestroy(PIDLSEQUENCE seq, unsigned long objSize,
                  ProcessSeqItemFunc itemDestroy)
{
  SEQRET	rc;

  rc = seqDone(seq, objSize, itemDestroy);
  if ( rc != SEQ_NO_ERROR )
    return rc;
  free( seq );
  return SEQ_NO_ERROR;
}

static unsigned long objListSize(unsigned long bytes)
{
  unsigned long len;

  len = (bytes / LIST_BYTES_INCR);
  if ( bytes % LIST_BYTES_INCR ) len++;
  return len * LIST_BYTES_INCR;
}

SEQRET seqInsert(PIDLSEQUENCE seq, unsigned long objSize, void *obj)
{
  return seqInsertAt(seq, objSize, seq->length, obj);
}

SEQRET seqIndexOf(PIDLSEQUENCE seq, unsigned long objSize, void *obj,
                  unsigned long *index)
{
  unsigned long	i;
  char		*ptr;

  if ( !seq || !objSize )
    return SEQ_INVALID_PARAM;

  ptr = seq->objects;
  for ( i=0; i<seq->length; i++)
  {
    if ( !memcmp(obj,ptr,objSize) )
    {
      if ( index ) *index = i;
      return SEQ_NO_ERROR;
    }
    ptr += objSize;
  }

  return SEQ_NOT_FOUND;
}

SEQRET seqDelete(PIDLSEQUENCE seq, unsigned long objSize, void *obj)
{
  SEQRET	rc;
  unsigned long	index;

  rc = seqIndexOf(seq, objSize, obj, &index);
  if ( rc != SEQ_NO_ERROR )
    return rc;

  return seqDeleteAt(seq, objSize, index);
}

SEQRET seqInsertAt(PIDLSEQUENCE seq, unsigned long objSize, unsigned long idx, 
                   void *obj)
{
  unsigned long new_list_len;

  if ( !seq || !objSize || idx > seq->length )
    return SEQ_INVALID_PARAM;
  if ( seq->maximum && seq->maximum==seq->length )
    return SEQ_OVERFLOW;

  new_list_len = objListSize((seq->length+1) * objSize);
  if ( objListSize(seq->length * objSize) != new_list_len )
    seq->objects = realloc( seq->objects, new_list_len );
  if ( idx < seq->length )
    memmove( &(seq->objects[ (idx+1) * seq->length ]),
             &(seq->objects[ idx * seq->length ]),
             (seq->length-idx) * objSize );
  memcpy( &(seq->objects[ objSize * idx ]), obj, objSize );
  seq->length++;

  return SEQ_NO_ERROR;
}

SEQRET seqDeleteAt(PIDLSEQUENCE seq, unsigned long objSize, unsigned long idx)
{
  if ( !seq || !objSize || idx >= seq->length )
    return SEQ_INVALID_PARAM;

  memcpy( &(seq->objects[ objSize * idx ]),
          &(seq->objects[ objSize * (idx+1) ]),
          (seq->length-idx-1) * objSize );
  seq->length--;
  return SEQ_NO_ERROR;
}

SEQRET seqSwap(PIDLSEQUENCE seq, unsigned long objSize, unsigned long idx1,
               unsigned long idx2, void *swapBuf)
{
  if ( !seq || idx1 >= seq->length || idx2 >= seq->length )
    return SEQ_INVALID_PARAM;
  if ( idx1 == idx2 )
    return SEQ_NO_ERROR;

  memcpy( swapBuf, &(seq->objects[ objSize * idx1 ]), objSize );
  memcpy( &(seq->objects[ objSize * idx1 ]),
          &(seq->objects[ objSize * idx2 ]), objSize );
  memcpy( &(seq->objects[ objSize * idx2 ]), swapBuf, objSize );
  return SEQ_NO_ERROR;
}

void *seqAt(PIDLSEQUENCE seq, unsigned long objSize, unsigned long idx)
{
  if ( !seq || !objSize || idx >= seq->length )
    return 0;
  return &(seq->objects[ objSize * idx ]);
}

SEQRET seqClear(PIDLSEQUENCE seq)
{
  if ( !seq  )
    return SEQ_INVALID_PARAM;
 
  seq->length = 0;
  free(seq->objects);
  return SEQ_NO_ERROR;
}

SEQRET seqForEach(PIDLSEQUENCE seq, unsigned long objSize, ProcessSeqItemFunc fn)
{
  unsigned long	idx;

  if ( !seq || !fn )
    return SEQ_INVALID_PARAM;

  for (idx=0; idx<seq->length; idx++)
    if ( !fn(idx, &(seq->objects[ objSize * idx ])) )
      break;

  return SEQ_NO_ERROR;
}

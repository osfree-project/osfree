#ifndef __SEQUENCES_H__
#define __SEQUENCES_H__

#pragma pack(1)
typedef struct {
    unsigned long	maximum;
    unsigned long	length;
    char		*objects;
} IDLSEQUENCE, *PIDLSEQUENCE;
#pragma pack()

#define SEQ_NO_ERROR		0
#define SEQ_OVERFLOW		1
#define SEQ_NOT_FOUND		2
#define SEQ_INVALID_PARAM	3

typedef long SEQRET;

typedef long ProcessSeqItem(unsigned long idx, void *item);
typedef ProcessSeqItem *ProcessSeqItemFunc;


SEQRET seqInit(PIDLSEQUENCE seq, unsigned long max, unsigned long objSize);
SEQRET seqDone(PIDLSEQUENCE seq, unsigned long objSize,
               ProcessSeqItemFunc itemDestroy);
PIDLSEQUENCE seqNew(unsigned long max, unsigned long objSize);
SEQRET seqDestroy(PIDLSEQUENCE seq, unsigned long objSize,
                  ProcessSeqItemFunc itemDestroy);
SEQRET seqIndexOf(PIDLSEQUENCE seq, unsigned long objSize, void *obj,
                  unsigned long *index);
SEQRET seqInsert(PIDLSEQUENCE seq, unsigned long objSize, void *obj);
SEQRET seqDelete(PIDLSEQUENCE seq, unsigned long objSize, void *obj);
SEQRET seqInsertAt(PIDLSEQUENCE seq, unsigned long objSize, unsigned long idx, 
                   void *obj);
SEQRET seqDeleteAt(PIDLSEQUENCE seq, unsigned long objSize, unsigned long idx);
SEQRET seqSwap(PIDLSEQUENCE seq, unsigned long objSize, unsigned long idx1,
               unsigned long idx2, void *swapBuf);
void *seqAt(PIDLSEQUENCE seq, unsigned long objSize, unsigned long idx);
SEQRET seqClear(PIDLSEQUENCE seq);
SEQRET seqForEach(PIDLSEQUENCE seq, unsigned long objSize,
		  ProcessSeqItemFunc fn);
#endif

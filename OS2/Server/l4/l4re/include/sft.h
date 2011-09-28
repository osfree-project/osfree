/*  System file table entry structure
 *
 *
 */

#ifndef __SFT_H__
#define __SFT_H__

typedef struct SFT
{
  struct SFT              *sft_next;         /* next SFT entry                */
  ULONG                   sft_flags;         /* flags                         */
  ULONG                   sft_refcount;      /* reference counter             */
  void                    *sft_devhdr;       /* device header ptr for devices */
  /* file system-dependent portion                                            */
  struct sffsd            sft_sffsd;
  /* file system-independent portion                                          */
  struct sffsi            sft_sffsi;
} SFT, *PSFT;

#endif

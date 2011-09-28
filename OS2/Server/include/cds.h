/*  Current directory structure
 *
 *
 */

#ifndef __CDS_H__
#define __CDS_H__

/* Handle table size */
#define MAX_CDS  0x2000

typedef struct CDS
{
  struct CDS        *cds_next;             /* next handle pointer */
  ULONG             cds_pid;               /* PID lookup key      */
  ULONG             cds_flags;             /* flags               */
  ULONG             cds_refcount;          /* reference counter   */
  /* file system-dependent part                                   */
  struct cdfsd      cds_cdfsd;
  /* file system-independent part                                 */
  struct cdfsi      cds_cdfsi;
} CDS, *PCDS;

#endif

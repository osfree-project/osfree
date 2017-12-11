/*  Volume parameter block definitions
 *
 *
 */

#ifndef __VPB_H__
#define __VPB_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <fsd.h>

/* max VPB's limit */
#define MAX_VPBS 0x200

#define VPB_VALID   0xfeedf000
#define VPB_INVAL   0x0badbeef

typedef struct VPB
{
  struct VPB        *vpb_next;    /* next handle in handle table */
  ULONG             vpb_valid;    /* validity signature          */
  ULONG             vpb_refcount; /* reference count             */
  ULONG             vpb_flags;    /* some flags                  */
  /* file system-dependent part                                  */
  struct vpfsd      vpb_vpfsd;
  /* file system-independent part                                */
  struct vpfsi      vpb_vpfsi;
} VPB, *PVPB;

#ifdef __cplusplus
  }
#endif

#endif

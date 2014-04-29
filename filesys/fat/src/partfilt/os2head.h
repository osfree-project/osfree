/*
 * $Header$
 */

#ifndef _OS2HEAD_H_
#define _OS2HEAD_H_

#define INCL_NOBASEAPI
#define INCL_NOPMAPI
#include <os2.h>

#include <devcmd.h>
#include <devclass.h>
#include <dskinit.h>

#define INCL_INITRP_ONLY
#include <reqpkt.h>
#include <iorb.h>

#define ERROR_I24_QUIET_INIT_FAIL       21	/* from bseerr.h */

#define IOERR_NO_SUCH_UNIT		IOERR_UNIT_NOT_ALLOCATED

typedef struct DevClassTableEntry FAR *PDevClassTableEntry;
typedef struct DevClassTableStruc FAR *PDevClassTableStruc;

typedef VOID (FAR* PADDEntryPoint)(PIORB pSomeIORB);
typedef PIORB (FAR* PNotifyAddr)(PIORB pSomeIORB);

#define	MAKE_SEC_CYL(SEC,CYL)		(((SEC)&0x3F)|(((CYL)&0x300)>>2)|\
					 (((CYL)&0xFF)<<8))

/* Modified: original file was wrong. */
typedef struct _PARTITIONENTRY
{
   UCHAR        BootIndicator;
   UCHAR        BegHead;
   USHORT       BegSecCyl;
   UCHAR        SysIndicator;
   UCHAR        EndHead;
   USHORT       EndSecCyl;
   ULONG        RelativeSectors;
   ULONG        NumSectors;
} PARTITIONENTRY;

typedef struct _MBR
{
   UCHAR          Pad[0x1BE];
   PARTITIONENTRY PartitionTable[4];
   USHORT         Signature;
} MBR;

/* 
 * sti and cli are a builtin function in MS Visual C++  (M.Willm, 1995-11-14)
 */
#ifndef __MSC__
#define ENABLE	asm { sti }
#define DISABLE	asm { cli }
#else
#define ENABLE  _enable();
#define DISABLE _disable();
#endif

#endif

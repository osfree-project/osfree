/* $Id: jfs_init1.c,v 1.1 2000/04/21 10:58:11 ktk Exp $ */

static char *SCCSID = "@(#)1.15  9/14/99 12:24:05 src/jfs/ifs/jfs_init1.c, sysjfs, w45.fs32, fixbld";
/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/*
 * Change History :
 *
 */

#define INCL_DOSMISC       /* DOS Miscellaneous values */
#define INCL_DOSERRORS     /* DOS Error values         */
#define INCL_NOPMAPI
#include <os2.h>
#include "jfs_os2.h"
#include "jfs_debug.h"
#include <devcmd.h>

/*
 * This function runs using the kernel stack.  Keep in mind that stack
 * addresses are not flat.
 */

/*
 * Global Data
 */
void * _Seg16	seg16ptr;
struct InfoSegGDT *GIS;
unsigned char * PERF_RMT;       /* MMPH, d201828/emp */
void * _Seg16	pLIS;
void * _Seg16	pDosTable;
uint32	TKSSBase;
struct tcb_s	*pTCBCur;					// F226941
uint16	KernelSS;

/*
 * External references
 */
int16	GetDOSVar(int32, int32, void * _Seg16 *);

/*
 * GetDOSVar Variable Numbers
 */
#define DHGETDOSV_SYSINFOSEG	1
#define DHGETDOSV_LOCINFOSEG	2
#define DHGETDOSV_DOSTABLE	9

void	jfs_init1()
{
	uint32	*DosTable, *DosTable2;
	union {
		uint32	intGIS;
		void * _Seg16 ptrGIS;
	} pGIS;

	if (GetDOSVar(DHGETDOSV_SYSINFOSEG, 0, &seg16ptr))
		brkpoint();
	pGIS.intGIS = (*(uint16 * _Seg16)seg16ptr) << 16;
	GIS = pGIS.ptrGIS;	/* Thunk! */
	if (GetDOSVar(DHGETDOSV_LOCINFOSEG, 0, &seg16ptr))
		brkpoint();
	pLIS = *(void * _Seg16 * _Seg16)seg16ptr;
	if (GetDOSVar(DHGETDOSV_DOSTABLE, 0, &pDosTable))
		brkpoint();
	DosTable = pDosTable;	/* Thunk! */
	DosTable2 = (uint32 *)((uint32)DosTable + ((*(uint8 *)DosTable)<<2) + 2);

	TKSSBase = DosTable2[11];
	pTCBCur = (void *)DosTable2[19];			// F226941

   PERF_RMT = GIS->SIS_perf_mec_table;   /* MMPH, d201828/emp */

	return;
}

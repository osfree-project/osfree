/*
 * $Header$
 */

/************************************************************************/
/*                       Linux partition filter.                        */
/*          (C) Copyright Deon van der Westhuysen, July 1995.           */
/*                                                                      */
/*  Dedicated to Jesus Christ, my Lord and Saviour.                     */
/*                                                                      */
/* This program is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2, or (at your option)  */
/* any later version.                                                   */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this program; if not, write to the Free Software          */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            */
/*                                                                      */
/*  This code is still under development; expect some rough edges.      */
/*                                                                      */
/************************************************************************/

#ifndef _EXTDEFS_H_
#define _EXTDEFS_H_

#include "os2head.h"

/*
 * memset and memcpy are builtin functions in MS Visual C++ 1.5 (M.Willm 1995-11-14)
 */
//#ifndef __MSC__
/* In file e2wrap.asm */
void memset (void FAR *Buffer, UCHAR Value, USHORT Count);
void memcpy (void FAR *Dest, void FAR *Source, USHORT Size);
//#endif

void VirtToPhys (void FAR *pDataSeg,ULONG FAR *ppDataSeg);
void far * PhysToVirt(ULONG ppXferBuf, USHORT XferLength);

VOID FAR E2FilterIORBWrapper (PIORB pIORB);
PIORB FAR FilterNotifyWrapper (PIORB pIORB);
PIORB FAR PartNotifyWrapper (PIORB pIORB);

int FAR ProcRun (PIORB pIORB);
int ProcBlock(PIORB pIORB);

PDevClassTableStruc E2GetADDTable (void);
USHORT RegisterADD (PADDEntryPoint pAddEP, PSZ DriverName);
void E2Print (PSZ Text);


#endif

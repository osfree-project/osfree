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

#ifndef _E2INIT_H_
#define _E2INIT_H_

#include "os2head.h"

void E2Init (PRPINITOUT Req);
void InitScanDrivers (void);
void InitScanAdapter (PADDEntryPoint pADDEntry);
void InitScanUnits (PADDEntryPoint pADDEntry, PADAPTERINFO pAdapterInfo);
PADDEntryPoint InitGetDriverEP (USHORT ADDHandle);
USHORT InitScanPartitions (PADDEntryPoint pAddEP);
int InitIsVirtType(UCHAR PartType);
void ProcessCmdline (char FAR *pCmdLine);
void InitGetUnitFS (void);
void InitSortUnits (void);
void InitPrint (char FAR *Message);
void ErrorPrint (char FAR *Message);
void InitPrintVerbose (char FAR *Message);

#ifdef __MSC__
#pragma alloc_text(InitCode, E2Init)
#pragma alloc_text(InitCode, InitScanDrivers)
#pragma alloc_text(InitCode, InitScanAdapter)
#pragma alloc_text(InitCode, InitScanUnits)
#pragma alloc_text(InitCode, InitGetDriverEP)
#pragma alloc_text(InitCode, InitScanPartitions)
#pragma alloc_text(InitCode, InitIsVirtType)
#pragma alloc_text(InitCode, ProcessCmdline)
#pragma alloc_text(InitCode, InitGetUnitFS)
#pragma alloc_text(InitCode, InitSortUnits)
#pragma alloc_text(InitCode, InitPrint)
#pragma alloc_text(InitCode, ErrorPrint)
#pragma alloc_text(InitCode, InitPrintVerbose)
#endif

#endif

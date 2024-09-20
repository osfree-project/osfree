/*
 * FamilyAPI NE Loader DOS stub
 *
 * Copyright 1993 Robert J. Amstadt
 * Copyright 1995 Alexandre Julliard
 * Copyright 2024 Yuri Prokushev
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos.h>
#include <malloc.h>


#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef HANDLE
#define HANDLE WORD
#endif

#ifndef HGLOBAL
#define HGLOBAL HANDLE
#endif


#include "newexe.h"

#pragma pack( push, 1 )

typedef struct
{
    WORD first; /* ordinal */
    WORD last;  /* ordinal */
    WORD next;  /* bundle */
} ET_BUNDLE;

typedef struct tagOFSTRUCT {
    BYTE    cBytes;
    BYTE    fFixedDisk;
    unsigned int	nErrCode;
    BYTE    reserved[4];
    char    szPathName[128];
} OFSTRUCT;

#pragma pack( pop )



// @todo make emulation of standard windows function via DOS ones
// for sharing of this code with WIN16 KERNEL.EXE
// List of emulated functions
//   GlobalAlloc
//   GlobalLock
//   GlobalUnlock
//   @todo add file functions

#define GMEM_FIXED          0x0000
#define GMEM_ZEROINIT       0x0040

// Emulation of GlobalAlloc. Actually returns segment allocated by int 21h
HGLOBAL GlobalAlloc(WORD flags, DWORD size)
{
	WORD segm;
	WORD s=(size >> 4) + 1;

	__asm
	{
		mov ax,48h
		mov bx, s
		int 21h
		mov segm, ax
	}

    // Zero data
	if (flags & GMEM_ZEROINIT) _fmemset(MK_FP(segm, 0), 0, size);

	return segm;
}

// Produce far pointer from HGLOBAL
char far *  GlobalLock(HGLOBAL h)
{
	return MK_FP(h, 0);
}

// Actually, does nothing
BYTE GlobalUnlock(HGLOBAL h)
{
	return 1;
}

// Global variables
struct new_exe far * mte;		// Module table entry (@todo to be changed via THHOOK)

// @todo fread doesn't work with far pointers?
int main(int argc, char *argv[])
{
  char filename[_MAX_PATH];
  FILE *f;
  struct exe_hdr MZHeader;
  struct new_exe NEHeader;
  int result;
  WORD size;
  int i;
  BYTE far * pData;
  WORD minalloc, pos;

  // We need MS-DOS version > 2.0 to get our filename
  if (_osminor > 2)
  {
  
    // Get our filename
    strcpy(filename, argv[0]);
  
    // Open ourself for read
    if( (f  = fopen( filename, "rb" )) == NULL )
    {
      printf( "Error\n");
      return 1;
    }
  
    // Read old Executable header
    result = fread(&MZHeader, 1, sizeof(MZHeader), f);
	
	//@todo Check is it correct header
  
    // Seek New Executable header
    result = fseek(f, E_LFANEW(MZHeader), SEEK_SET);
  
    // Read New Executable header
    result = fread(&NEHeader, 1, sizeof(NEHeader), f);

	//@todo Check is it correct header
  
    // Calculate in memory size
    size = sizeof(struct new_exe) +								/* NE Header size */
             NE_CSEG(NEHeader) * sizeof(struct new_seg1) +		/* in-memory segment table */
             NE_RESTAB(NEHeader) - NE_RSRCTAB(NEHeader) +			/* resource table */
             NE_MODTAB(NEHeader) - NE_RESTAB(NEHeader) +			/* resident names table */
             NE_CMOD(NEHeader) * sizeof(WORD) +					/* module ref table */
             NE_ENTTAB(NEHeader) - NE_IMPTAB(NEHeader) +			/* imported names table */
             NE_CBENTTAB(NEHeader) +								/* entry table length */
             sizeof(ET_BUNDLE) +									/* ??? */
             2 * (NE_CBENTTAB(NEHeader) - NE_CMOVENT(NEHeader)*6) +	/* entry table extra conversion space */
             sizeof(OFSTRUCT) - 128 + strlen(filename) + 1;		/* loaded file info */
  
    // Allocate memory
    mte=(struct new_exe far *)GlobalLock(GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size));
    
    // Copy header from stack
    _fmemcpy(mte, &NEHeader, sizeof(NEHeader));
    mte->count=0;
    
    /* check programs for default minimal stack size */
    if ((!mte->ne_flags & NE_FFLAGS_LIBMODULE) && (mte->ne_stack < 0x1400))
          mte->ne_stack = 0x1400;
    mte->next=0;
  
    // Move to start of segment table
    result = fseek(f, E_LFANEW(MZHeader)+NE_SEGTAB(NEHeader), SEEK_SET);
  
    // Point to in-memory segment table
    pData=(BYTE *)(mte+1);
    
    // Load segment table, allocate segments
    for (i = NE_CSEG(NEHeader); i > 0; i--)
    {
  	  // Read segment table entry
      result = fread(pData, 1, sizeof(struct new_seg), f);
  
      // Allocate segment
      minalloc = ((struct new_seg1 *)pData)->ns1_minalloc ? ((struct new_seg1 *)pData)->ns1_minalloc : 0x10000;
  	  ((struct new_seg1 *)pData)->ns1_handle=FP_SEG(_fmalloc(minalloc));
  
  	  // Next segment table entry
      pData += sizeof(struct new_seg1);
    }

    // Point to in-memory segment table
    pData=(BYTE *)(mte+1);
	
	// Load segments, relocate segments
    for (i = NE_CSEG(NEHeader); i > 0; i--)
    {
  	  // Read segment into memory
      pos = ((struct new_seg1 *)pData)->ns1_sector << mte->ne_align;
      if (((struct new_seg1 *)pData)->ns1_cbseg) size = ((struct new_seg1 *)pData)->ns1_cbseg;
      else size = minalloc;
      result = fseek(f, pos, SEEK_SET);
      result = fread(MK_FP(((struct new_seg1 *)pData)->ns1_handle,0), 1, size, f);
  	
  	  // Relocate segment

  	  // Next segment table entry
      pData += sizeof(struct new_seg1);
    }
  
    // Load Resource table (will not be implemented yet)
    
    // Load Resident names table
    
    // Load Module reference table
    
    // Load Imported names table
    
    // Load Entry table 
    
    if(f)
    {
      if (fclose(f))
      {
        printf("Error\n");
        return 1;
      }
    }
  
    return 0;
  } else {
      printf("Error\n");
	  return 1;
  }
}

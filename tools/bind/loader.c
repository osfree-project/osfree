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

#include <stdlib.h>
#include <stdio.h>

#include "winemu.h"
#include "newexe.h"

void memcpy(void far * s1, void far * s2, unsigned length)
{	char far * p;
	char far * q;

	if(length) {
		p = s1;
		q = s2;
		do *p++ = *q++;
		while(--length);
	}
}

// Global variables
struct new_exe far * mte;		// Module table entry (@todo to be changed via THHOOK)

int main(int argc, char *argv[])
{
  HFILE f;
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
  
    // Open ourself for read
    if( (f  = _lopen(argv[0], OF_READ)) == -1 )
    {
      printf( "Error\n");
      return 1;
    }

    // Read old Executable header
    result = _lread(f, &MZHeader, sizeof(MZHeader));
	
    //@todo Check is it correct header
  
    // Seek New Executable header
    result = _llseek(f, E_LFANEW(MZHeader), SEEK_SET);
  
    // Read New Executable header
    result = _lread(f, &NEHeader, sizeof(NEHeader));

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
             sizeof(OFSTRUCT) - 128 + lstrlen(argv[0]) + 1;		/* loaded file info */
  
    // Allocate memory
    mte=(struct new_exe far *)GlobalLock(GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size));
    
    // Copy header from stack
    memcpy(mte, &NEHeader, sizeof(NEHeader));
    mte->count=0;
    
    /* check programs for default minimal stack size */
    if ((!mte->ne_flags & NE_FFLAGS_LIBMODULE) && (mte->ne_stack < 0x1400))
          mte->ne_stack = 0x1400;
    mte->next=0;
  
    // Move to start of segment table
    result = _llseek(f, E_LFANEW(MZHeader)+NE_SEGTAB(NEHeader), SEEK_SET);
  
    // Point to in-memory segment table
    pData=(BYTE *)(mte+1);
    
    // Load segment table, allocate segments
    for (i = NE_CSEG(NEHeader); i > 0; i--)
    {
  	  // Read segment table entry
      result = _lread(f, pData, sizeof(struct new_seg));
  
      // Allocate segment
      minalloc = ((struct new_seg1 *)pData)->ns1_minalloc ? ((struct new_seg1 *)pData)->ns1_minalloc : 0x10000;
  	  ((struct new_seg1 *)pData)->ns1_handle=GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, minalloc);
  
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
      result = _llseek(f, pos, SEEK_SET);
      result = _lread(f, GlobalLock(((struct new_seg1 *)pData)->ns1_handle), size);
  	
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
      if (_lclose(f))
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

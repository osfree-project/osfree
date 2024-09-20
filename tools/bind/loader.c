#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos.h>
#include <malloc.h>

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
  
    // Seek New Executable header
    result = fseek(f, E_LFANEW(MZHeader), SEEK_SET);
  
    // Read New Executable header
    result = fread(&NEHeader, 1, sizeof(NEHeader), f);
  
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
    mte=_fmalloc(size);
    
    // Zero data
    _fmemset(mte, 0, size);
    
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
    
    // Load segment table, allocate segments, load segments, relocate segment
    for (i = NE_CSEG(NEHeader); i > 0; i--)
    {
  	  // Read segment table entry
      result = fread(pData, 1, sizeof(struct new_seg), f);
  
      // Allocate segment
      minalloc = ((struct new_seg1 *)pData)->ns1_minalloc ? ((struct new_seg1 *)pData)->ns1_minalloc : 0x10000;
  	  ((struct new_seg1 *)pData)->ns1_handle=FP_SEG(_fmalloc(minalloc));
  
  	  // Read segment into memory
      pos = ((struct new_seg1 *)pData)->ns1_sector << mte->ne_align;
      if (((struct new_seg1 *)pData)->ns1_cbseg) size = ((struct new_seg1 *)pData)->ns1_cbseg;
      else size = minalloc;
      result = fseek(f, pos, SEEK_SET);
      result = fread(MK_FP(((struct new_seg1 *)pData)->ns1_handle,0), 1, size, f);
	  //@todo back seek here
  	
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


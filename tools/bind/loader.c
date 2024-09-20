#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos.h>
#include <malloc.h>

#include "newexe.h"

typedef struct
{
    unsigned short first; /* ordinal */
    unsigned short last;  /* ordinal */
    unsigned short next;  /* bundle */
} ET_BUNDLE;

#pragma pack( push, 1 )
typedef struct tagOFSTRUCT {
    unsigned char    cBytes;
    unsigned char    fFixedDisk;
    unsigned int    nErrCode;
    unsigned char    reserved[4];
    char    szPathName[128];
} OFSTRUCT;

#pragma pack( pop )

// Global variables
struct new_exe far * mte;		// Module table entry

int main( void )
{
  //union REGS aRegs;
  char filename[_MAX_PATH];
  FILE *f;
  struct exe_hdr MZHeader;
  struct new_exe NEHeader;
  int result;
  unsigned short size;
  int i;
  unsigned char * pData;
  unsigned short minalloc;

  //strcpy(filename, argv[0]);

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
           NEHeader.ne_cseg * sizeof(struct new_seg1) +			/* in-memory segment table */
           NEHeader.ne_restab - NEHeader.ne_rsrctab +			/* resource table */
           NEHeader.ne_modtab - NEHeader.ne_restab +			/* resident names table */
           NEHeader.ne_cmod * sizeof(unsigned short) +			/* module ref table */
           NEHeader.ne_enttab - NEHeader.ne_imptab +			/* imported names table */
           NEHeader.ne_cbenttab +								/* entry table length */
           sizeof(ET_BUNDLE) +									/* ??? */
           2 * (NEHeader.ne_cbenttab - NEHeader.ne_cmovent*6) +	/* entry table extra conversion space */
           sizeof(OFSTRUCT) - 128 + strlen(filename) + 1;		/* loaded file info */


  // Allocate memory
  mte=_fmalloc(size);
  
  // Zero data
  _fmemset(mte, 0, size);
  
  // Copy header from stack
  _fmemcpy(mte, &NEHeader, sizeof(NEHeader));
  mte->count=0;
  
  /* check programs for default minimal stack size */
  if (!(mte->ne_flags & 0x8000/*NE_FFLAGS_LIBMODULE*/) && (mte->ne_stack < 0x1400))
        mte->ne_stack = 0x1400;
  mte->next=0;

  // Move to start of segment table
  result = fseek(f, E_LFANEW(MZHeader)+NE_SEGTAB(NEHeader), SEEK_SET);

  // Point to in-memory segment table
  pData=(unsigned char *)(mte+1);
  
  // Load segment table, allocate segments, load segments
  for (i = NEHeader.ne_cseg; i > 0; i--)
  {
	// Read segment table entry
    result = fread(pData, 1, sizeof(struct new_seg), f);

    // Allocate segment
    minalloc = ((struct new_seg1 *)pData)->ns1_minalloc ? ((struct new_seg1 *)pData)->ns1_minalloc : 0x10000;
	((struct new_seg1 *)pData)->ns1_handle=FP_SEG(_fmalloc(minalloc));

	// Read segment
	

	// Next segment table entry
    pData += sizeof(struct new_seg1);
  }
  
  // Load resource table
  
  // Load Resident names table
  
  // Load module reference table
  
  // Load imported names table
  
  // Load entry table
  
  // Load segments
  
  if(f)
  {
    if (fclose(f))
    {
      printf("Error\n");
      return 1;
    }
  }

  return 0;
}


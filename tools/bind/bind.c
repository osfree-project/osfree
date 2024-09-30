/*
 * FamilyAPI Binder
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

// @todo:
// Movable segments support (not required for current tools)
// Import module table (emulate via OMF linking)

#include <stdlib.h>
#include <stdio.h>

#include "winemu.h"
#include "newexe.h"

void memcpy(void FAR * s1, void FAR * s2, unsigned length)
{	char FAR * p;
	char FAR * q;

	if(length) {
		p = s1;
		q = s2;
		do *p++ = *q++;
		while(--length);
	}
}

// Global variables
struct new_exe FAR * mte;		// Module table entry (@todo to be changed via THHOOK)

#define TRACE(...) \
	{ \
		printf(__VA_ARGS__); \
	}

/***********************************************************************
 *           NE_DumpModule
 */
void NE_DumpModule(void)
{
    int i, ordinal;
    struct new_seg1 FAR *pSeg;
    BYTE FAR *pstr;
    WORD FAR *pword;
    struct new_exe FAR *pModule;
    ET_BUNDLE FAR *bundle;
    //ET_ENTRY *entry;

    if (!(pModule = mte))
    {
        //ERR( "**** %04x is not a module handle\n", hModule );
        return;
    }

      /* Dump the module info */
    TRACE( "---\n" );
    //TRACE( "Module %04x:\n", hModule );
    TRACE( "count=%d flags=%04x heap=%d stack=%d\n\r",
             pModule->count, pModule->ne_flags,
             pModule->ne_heap, pModule->ne_stack );
    TRACE( "cs:ip=%04x:%04x ss:sp=%04x:%04x ds=%04x nb seg=%d modrefs=%d\n\r",
             FP_SEG(pModule->ne_csip), FP_OFF(pModule->ne_csip),
             FP_SEG(pModule->ne_sssp), FP_OFF(pModule->ne_sssp),
             pModule->ne_autodata, pModule->ne_cseg, pModule->ne_cmod );
    TRACE( "os_flags=%d swap_area=%d version=%04x\n\r",
             pModule->ne_exetyp, pModule->ne_swaparea, pModule->ne_expver );
//    if (pModule->ne_flags & NE_FFLAGS_WIN32)
        //TRACE( "PE module=%p\n", pModule->module32 );

      /* Dump the file info */
    TRACE( "---\n" );
    //TRACE( "Filename: '%s'\n", NE_MODULE_NAME(pModule) );

      /* Dump the segment table */
    TRACE( "---\n" );
    TRACE( "Segment table:\n" );
    pSeg = ((struct new_seg1 FAR *)((BYTE FAR *)pModule+(pModule)->ne_segtab));
	printf ("%Wp %Wp %d\n", pModule, pSeg, (pModule)->ne_segtab);
    for (i = 0; i < pModule->ne_cseg; i++, pSeg++)
        TRACE( "pSeg=%Wp %02x: pos=%d size=%d flags=%04x minsize=%d hSeg=%04x\n", pSeg,
                 i + 1, pSeg->ns1_sector, pSeg->ns1_cbseg, pSeg->ns1_flags,
                 pSeg->ns1_minalloc, pSeg->ns1_handle );

#if 0

      /* Dump the resource table */
    TRACE( "---\n" );
    TRACE( "Resource table:\n" );
    if (pModule->ne_rsrctab)
    {
        pword = (WORD *)((BYTE *)pModule + pModule->ne_rsrctab);
        TRACE( "Alignment: %d\n", *pword++ );
        while (*pword)
        {
            NE_TYPEINFO *ptr = (NE_TYPEINFO *)pword;
            NE_NAMEINFO *pname = (NE_NAMEINFO *)(ptr + 1);
            TRACE( "id=%04x count=%d\n", ptr->type_id, ptr->count );
            for (i = 0; i < ptr->count; i++, pname++)
                TRACE( "offset=%d len=%d id=%04x\n",
		      pname->offset, pname->length, pname->id );
            pword = (WORD *)pname;
        }
    }
    else TRACE( "None\n" );

      /* Dump the resident name table */
    TRACE( "---\n" );
    TRACE( "Resident-name table:\n" );
    pstr = (BYTE*) pModule + pModule->ne_restab;
    while (*pstr)
    {
        TRACE( "%*.*s: %d\n", *pstr, *pstr, pstr + 1,
                 *(WORD *)(pstr + *pstr + 1) );
        pstr += *pstr + 1 + sizeof(WORD);
    }

      /* Dump the module reference table */
    TRACE( "---\n" );
    TRACE( "Module ref table:\n" );
    if (pModule->ne_modtab)
    {
        pword = (WORD *)((BYTE *)pModule + pModule->ne_modtab);
        for (i = 0; i < pModule->ne_cmod; i++, pword++)
        {
            char name[10];
            GetModuleName16( *pword, name, sizeof(name) );
            TRACE( "%d: %04x -> '%s'\n", i, *pword, name );
        }
    }
    else TRACE( "None\n" );

      /* Dump the entry table */
    TRACE( "---\n" );
    TRACE( "Entry table:\n" );
    bundle = (ET_BUNDLE *)((BYTE *)pModule+pModule->ne_enttab);
    do {
        entry = (ET_ENTRY *)((BYTE *)bundle+6);
        TRACE( "Bundle %d-%d: %02x\n", bundle->first, bundle->last, entry->type);
        ordinal = bundle->first;
        while (ordinal < bundle->last)
        {
            if (entry->type == 0xff)
                TRACE("%d: %02x:%04x (moveable)\n", ordinal++, entry->segnum, entry->offs);
            else
                TRACE("%d: %02x:%04x (fixed)\n", ordinal++, entry->segnum, entry->offs);
            entry++;
        }
    } while ( (bundle->next) && (bundle = ((ET_BUNDLE *)((BYTE *)pModule + bundle->next))) );

    /* Dump the non-resident names table */
    TRACE( "---\n" );
    TRACE( "Non-resident names table:\n" );
    if (pModule->nrname_handle)
    {
        pstr = GlobalLock( pModule->nrname_handle );
        while (*pstr)
        {
            TRACE( "%*.*s: %d\n", *pstr, *pstr, pstr + 1,
                   *(WORD *)(pstr + *pstr + 1) );
            pstr += *pstr + 1 + sizeof(WORD);
        }
    }
    TRACE( "\n" );
#endif
}

/***********************************************************************
 *           apply_relocations
 *
 * Apply relocations to a segment. Helper for NE_LoadSegment.
 */
void apply_relocations( struct new_exe FAR * pModule, struct new_rlc FAR *rep,
                                      int count, int segnum )
{
    BYTE FAR *func_name;
    char buffer[256];
    int i, ordinal;
    WORD offset, FAR * sp;
    //HMODULE module;
    void FAR * address = 0;
    struct new_exe FAR * pModuleTable = (struct new_exe FAR *)((BYTE FAR *)pModule + pModule->ne_modtab);
    struct new_seg1 FAR *pSegTable = (struct new_seg1 FAR *)((BYTE FAR *)pModule+pModule->ne_segtab);
    struct new_seg1 FAR *pSeg = pSegTable + segnum - 1;

    /*
     * Go through the relocation table one entry at a time.
     */
    for (i = 0; i < count; i++, rep++)
    {
        /*
         * Get the target address corresponding to this entry.
         */

        /* If additive, there is no target chain list. Instead, add source
           and target */
        int additive = rep->nr_flags & NRADD;
        switch (rep->nr_flags & NRRTYP)
        {
		case NRRORD:
            // Here is non-standard access to module because FamilyAPI doesn't support
			// DLLs. BIND tool adds object/library files to the end of executable
			// so we need to find corresponding function in such libraries.
			break;
#if 0

        case NRRORD:
            module = pModuleTable[rep->nr_mod-1];
            ordinal = rep->nr_proc;
            address = NE_GetEntryPoint( module, ordinal );
            if (!address)
            {
                struct new_exe FAR *pTarget = NE_GetPtr( module );
                if (!pTarget)
                    WARN_(module)("Module not found: %04x, reference %d of module %*.*s\n",
                             module, rep->target1,
                             *((BYTE *)pModule + pModule->ne_restab),
                             *((BYTE *)pModule + pModule->ne_restab),
                             (char *)pModule + pModule->ne_restab + 1 );
                else
                {
                    ERR("No implementation for %.*s.%d, setting to 0xdeadbeef\n",
                            *((BYTE *)pTarget + pTarget->ne_restab),
                            (char *)pTarget + pTarget->ne_restab + 1,
                            ordinal );
                    address = (FARPROC16)0xdeadbeef;
                }
            }
            if (TRACE_ON(fixup))
            {
                NE_MODULE *pTarget = NE_GetPtr( module );
                TRACE("%d: %.*s.%d=%04x:%04x %s\n", i + 1,
                       *((BYTE *)pTarget + pTarget->ne_restab),
                       (char *)pTarget + pTarget->ne_restab + 1,
                       ordinal, HIWORD(address), LOWORD(address),
                       NE_GetRelocAddrName( rep->nr_stype, additive ) );
            }
            break;
        case NE_RELTYPE_NAME:
            module = pModuleTable[rep->target1-1];
            func_name = (BYTE *)pModule + pModule->ne_imptab + rep->target2;
            memcpy( buffer, func_name+1, *func_name );
            buffer[*func_name] = '\0';
            ordinal = NE_GetOrdinal( module, buffer );
            address = NE_GetEntryPoint( module, ordinal );

            if (ERR_ON(fixup) && !address)
            {
                NE_MODULE *pTarget = NE_GetPtr( module );
                ERR("No implementation for %.*s.%s, setting to 0xdeadbeef\n",
                    *((BYTE *)pTarget + pTarget->ne_restab),
                    (char *)pTarget + pTarget->ne_restab + 1, buffer );
            }
            if (!address) address = (FARPROC16) 0xdeadbeef;
            if (TRACE_ON(fixup))
            {
                NE_MODULE *pTarget = NE_GetPtr( module );
                TRACE("%d: %.*s.%s=%04x:%04x %s\n", i + 1,
                       *((BYTE *)pTarget + pTarget->ne_restab),
                       (char *)pTarget + pTarget->ne_restab + 1,
                       buffer, HIWORD(address), LOWORD(address),
                       NE_GetRelocAddrName( rep->nr_stype, additive ) );
            }
            break;
#endif
        case NRRINT:
            if ((rep->nr_union.nr_intref.nr_segno) == 0xff) /* Movable segment */
            {
                //address  = NE_GetEntryPoint( pModule->self, rep->target2 );
				// @todo: get data from movable entry point (nr_entry), not direct offset
				printf("Movable!\n");
				address= MK_FP(FP_SEG(pModule), rep->nr_union.nr_intref.nr_entry); 
            }
            else /* Fixed segment */
            {
				address=GlobalLock(pSegTable[rep->nr_union.nr_intref.nr_segno-1].ns1_handle);
                address = MK_FP(FP_SEG(address), rep->nr_union.nr_intref.nr_entry);
            }

            TRACE("INT %d: %Wp \n",
                  i + 1, address//,
                  //NE_GetRelocAddrName( rep->nr_stype, additive ) 
				  );
            break;
#if 0
        case NRROSF:
            /* Relocation type 7:
             *
             *    These appear to be used as fixups for the Windows
             * floating point emulator.  Let's just ignore them and
             * try to use the hardware floating point.  Linux should
             * successfully emulate the coprocessor if it doesn't
             * exist.
             */
            TRACE("%d: TYPE %d, OFFSET %04x, TARGET %04x %04x %s\n",
                  i + 1, rep->nr_flags, rep->nr_soff,
                  rep->target1, rep->target2,
                  NE_GetRelocAddrName( rep->nr_stype, additive ) );
            continue;
#endif

        }
        offset  = rep->nr_soff;

        /* Apparently, high bit of nr_stype is sometimes set; */
        /* we ignore it for now */
        if (rep->nr_stype > 13 /*NE_RADDR_OFFSET32*/)
        {
            //char module[10];
            //GetModuleName16( pModule->self, module, sizeof(module) );
            //ERR("WARNING: module %s: unknown reloc addr type = 0x%02x. Please report.\n",
                 //module, rep->nr_stype );
        }

        if (additive)
        {
            sp =  MK_FP( FP_SEG(GlobalLock(pSeg->ns1_handle)), offset ) ;
            TRACE("    %04x:%04x\n", offset, *sp );
            switch (rep->nr_stype & NRSTYP)
            {
            case NRSBYT:
                *(BYTE FAR *)sp += rep->nr_union.nr_intref.nr_entry&0xff;
                break;
            case NRSOFF:
                *sp += rep->nr_union.nr_intref.nr_entry;
                break;
            case NRSPTR:
                *sp += FP_OFF(address);
                *(sp+1) = FP_SEG(address);
                break;
            case NRSSEG:
                /* Borland creates additive records with offset zero. Strange, but OK */
                if (*sp)
                    ;//ERR("Additive selector to %04x.Please report\n",*sp);
                else
                    *sp = FP_SEG(address);
                break;
            default:
                goto unknown;
            }
        }
        else  /* non-additive fixup */
        {
            do
            {
                WORD next_offset;

                sp = MK_FP( FP_SEG(GlobalLock(pSeg->ns1_handle)), offset ) ;
                next_offset = *sp;
                TRACE("    %04x:%04x\n", offset, *sp );
                switch (rep->nr_stype & NRSTYP)
                {
                case NRSBYT:
                    *(BYTE FAR *)sp = rep->nr_union.nr_intref.nr_entry&0xff;
                    break;
                case NRSOFF:
                    *sp = rep->nr_union.nr_intref.nr_entry;
                    break;
                case NRSPTR:
                    *sp = FP_OFF(address);
                    *(sp+1) = FP_SEG(address);
                    break;
                case NRSSEG:
                    *sp = FP_SEG(address);
                    break;
                default:
                    goto unknown;
                }
                if (next_offset == offset) break;  /* avoid infinite loop */
                //@todo if (next_offset >= GlobalSize(pSeg->hs1_handle)) break;
                offset = next_offset;
            } while (offset != 0xffff);
        }
    }
    return ;

unknown:
    TRACE("WARNING: %d: unknown ADDR TYPE %d,  "
         "TYPE %d,  OFFSET %04x,  TARGET %04x %04x\n",
         i + 1, rep->nr_stype, rep->nr_flags,
         rep->nr_soff, rep->nr_union.nr_osfix.nr_ostype, rep->nr_union.nr_osfix.nr_osres);
    return ;
}

int main(int argc, char *argv[])
{
  HFILE f;
  struct exe_hdr MZHeader;
  struct new_exe NEHeader;
  int result;
  WORD size;
  WORD count;
  int i;
  BYTE FAR * pData;
  BYTE FAR * pSeg;
  WORD minalloc;
  DWORD pos;
  HGLOBAL t;

  printf("osFree FamilyAPI Binder v.0.1\n");
  // We need MS-DOS version > 2.0 to get our filename
  if (_osmajor > 2)
  {
	  printf("x1\n");
    // Open ourself for read
    if( (f  = _lopen("attrib.exe"/*argv[0]*/, OF_READ)) != HFILE_ERROR )
    {
	  printf("x2\n");
      // Read old Executable header
      if ( (result = _lread(f, &MZHeader, sizeof(MZHeader))) == sizeof(MZHeader) )
	  {
	  printf("x3\n");
        // Check MZ Header magic
        if (E_MAGIC(MZHeader) == EMAGIC)
        {
	  printf("x4\n");
          // Seek New Executable header
          if ( (result = _llseek(f, E_LFANEW(MZHeader), SEEK_SET)) != HFILE_ERROR )
          {
	  printf("x5\n");
            // Read New Executable header
            if ( (result = _lread(f, &NEHeader, sizeof(NEHeader))) == sizeof(NEHeader) )
            {
	  printf("x6\n");
              // Check NE Header magic
              if (NE_MAGIC(NEHeader) == NEMAGIC)
              {
	  printf("x7\n");
                /* check for OS/2 program */
                if (NE_EXETYP(NEHeader) == NE_OS2)
                {
                  // Calculate in memory size
                  size = sizeof(struct new_exe) +							/* NE Header size */
                     NE_CSEG(NEHeader) * sizeof(struct new_seg1) +		/* in-memory segment table */
                     NE_RESTAB(NEHeader) - NE_RSRCTAB(NEHeader) +			/* resource table */
                     NE_MODTAB(NEHeader) - NE_RESTAB(NEHeader) +			/* resident names table */
                     NE_CMOD(NEHeader) * sizeof(WORD) +					/* module ref table */
                     NE_ENTTAB(NEHeader) - NE_IMPTAB(NEHeader) +			/* imported names table */
                     NE_CBENTTAB(NEHeader) +								/* entry table length */
                     sizeof(ET_BUNDLE) +									/* ??? */
                     2 * (NE_CBENTTAB(NEHeader) - NE_CMOVENT(NEHeader)*6) +	/* entry table extra conversion space */
                     sizeof(OFSTRUCT) - 128 + lstrlen("attrib.exe") + 1;		/* loaded file info */
	  printf("x8\n");
		      
                  // Allocate memory
                  if (t=GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size))
                  {
	  printf("x9\n");
                    if (mte=(struct new_exe FAR *)GlobalLock(t))
                    {
	  printf("x10\n");
                      // Copy header from stack
                      memcpy(mte, &NEHeader, sizeof(NEHeader));
                      mte->count=0;
				    
                      /* check programs for default minimal stack size */
                      if ((!mte->ne_flags & NE_FFLAGS_LIBMODULE) && (mte->ne_stack < 0x1400))
                        mte->ne_stack = 0x1400;
                      mte->next=0;
				    
                      // Move to start of segment table
                      if ( (result = _llseek(f, E_LFANEW(MZHeader)+NE_SEGTAB(NEHeader), SEEK_SET) != HFILE_ERROR) )
                      {
	  printf("x11\n");
                        // Point to in-memory segment table
                        pData=(BYTE FAR *)(mte+1);
                        mte->ne_segtab = FP_OFF(pData);
					    
                        // Load segment table, allocate segments
                        for (i = NE_CSEG(NEHeader); i > 0; i--)
                        {
                          // Read segment table entry
                          if ( result = _lread(f, pData, sizeof(struct new_seg)) != HFILE_ERROR)
                          {
                            // Allocate segment
                            minalloc = ((struct new_seg1 FAR *)pData)->ns1_minalloc ? ((struct new_seg1 FAR *)pData)->ns1_minalloc : 0x10000;

							if (((struct new_seg1 FAR *)pData)->ns1_handle=GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, minalloc))
                            {					    
                              // Next segment table entry
                              pData += sizeof(struct new_seg1);
							} else {
                              printf( "Error: Allocate segment entry\n");
                              return 1;
                            }
                          } else {
                            printf( "Error: Read segment entry\n");
                            return 1;
                          }
                        }
					    
                        // Point to in-memory segment table
                        pData=(BYTE FAR *)(mte+1);
                        printf("pData=%Wp\n\r", pData);
					    
	  printf("x12\n");

                        // Load segments, relocate segments
                        for (i = NE_CSEG(NEHeader); i > 0; i--)
                        {
                          // Read segment into memory
                          pos = (LONG)((struct new_seg1 FAR *)pData)->ns1_sector << mte->ne_align;
                          printf("pos=%ld\n\r", pos);
                          size = ((struct new_seg1 FAR *)pData)->ns1_cbseg ? ((struct new_seg1 FAR *)pData)->ns1_cbseg : 0x10000;

                          if ( (result = _llseek(f, pos, SEEK_SET)) != HFILE_ERROR )
                          {
                            if (pSeg=GlobalLock(((struct new_seg1 FAR *)pData)->ns1_handle))
                            {
                              if ( ( result = _lread(f, pSeg, size) ) != HFILE_ERROR )
                              {
                                // Relocate segment
                                if (((struct new_seg1 FAR *)pData)->ns1_flags & NSRELOC)
                                {
                                  BYTE FAR * pRLC;
                                  HGLOBAL hRLC;
					    
                                  if ( ( result = _lread(f, &count, sizeof(count)) ) != HFILE_ERROR )
                                  {
                                    if (hRLC=GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, count * sizeof(struct new_rlc)))
                                    {
                                      if (pRLC=GlobalLock(hRLC))
                                      {
                                        if ( (result = _lread(f, pRLC, count * sizeof(struct new_rlc)) ) != HFILE_ERROR)
                                        {
								  
                                          printf("Segment %d pos %ld Number of relocations: %d\n\r", NE_CSEG(NEHeader)-i+1, pos, count);
		      		    	
                                          apply_relocations(mte, (struct new_rlc FAR *)pRLC, count, NE_CSEG(NEHeader)-i+1);
		      		    	              // GlobalUnlock/GlobalFree
									
                                          // Next segment table entry
                                          pData += sizeof(struct new_seg1);
                                          printf("pData=%Wp\n\r", pData);
							            } else {
                                          printf( "Error: Read segment relocation entires\n");
                                          return 1;
                                        }
							          } else {
                                        printf( "Error: Lock segment relocation count\n");
                                        return 1;
                                      }
							        } else {
                                      printf( "Error: Allocate segment relocation count\n");
                                      return 1;
                                    }
							      } else {
                                    printf( "Error: Read segment relocation count\n");
                                    return 1;
                                  }
                                }
							  } else {
                                printf( "Error: Read segment data\n");
                                return 1;
                              }
							} else {
                              printf( "Error: Lock segment data\n");
                              return 1;
                            }
                          } else {
                            printf( "Error: Seek segment data\n");
                            return 1;
                          }
                        }
					    
                        NE_DumpModule();
					    
                        // Load Resource table (will not be implemented yet)
					    
                        // Load Resident names table
					    
                        // Load Module reference table
					    
                        // Load Imported names table
					    
                        // Load Entry table
					    
                        if (_lclose(f)!= HFILE_ERROR)
                        {
                          // Execute program
					    
                          __asm {
                            ;jmp NEHeader.csip
                          } 
                          return 0;
                        } else {
                          printf("Error: Close file\n");
                          return 1;
                        }
                      } else {
                        printf("Error: Seek to segment table\n");
                        return 1;
                      }
                    } else {
                      printf( "Error: Can't lock memory\n");
                      return 1;
                    }
                  } else {
                    printf( "Error: Can't allocate memory\n");
                    return 1;
                  }
                } else {
                  printf( "Error: Target OS not OS/2\n");
                  return 1;
                }
              } else {
                printf( "Error: Bad NE header\n");
                return 1;
              }
            } else {
              printf( "Error: Read NE Header\n");
              return 1;
            }
          } else {
            printf( "Error: Seek to NE header\n");
            return 1;
          }
        } else {
          printf( "Error: Bad MZ header\n");
          return 1;
        }
      } else {
        printf( "Error: Read MZ Header result=%d %d\n", result, sizeof(MZHeader));
        return 1;
      }
    } else {
      printf( "Error: File open\n");
      return 1;
    }
  } else {
      printf("Error: Unsupported DOS version\n");
	  return 1;
  }
}

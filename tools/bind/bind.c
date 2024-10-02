/*
 *  FamilyAPI Binder
 *
 *  Copyright 2024 Yuri Prokushev
 *
 *  (c) osFree Project 2024, <https://www.osfree.org>
 *  for licence see licence.txt in root directory, or project website
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
#include "omf.h"

char func[255];

char * findfunctionname(char * module, WORD ordinal)
{
	FILE * f;
	omf_record_header head;
	int result;
	char buf[512];
	int i;
	long pos;
	char mod[255];
	WORD ord;
	
	// Open os2.lib for read
    if(f=fopen("os2.lib", "rb"))
    {
      for (;1;)
	  {
      if ( (result = fread(&head, 1, sizeof(head), f)) == sizeof(head) )
	  {
		  //printf ("type=%02x length=%d\n", head.type, head.length);
          fread(&buf, 1, head.length, f);

		  if (head.type==0x88) // comment
		  {
			  if (buf[1]==0xa0) // comment class
			  {
				  if (buf[2]==0x01) // impdef
				  {
					  if (buf[3]!=0)
					  {
						  memcpy(&func, &buf[5], buf[4]);	// function name
						  func[buf[4]]=0;
						  memcpy(&mod, &buf[5+buf[4]+1], buf[5+buf[4]]); // module name
						  mod[buf[5+buf[4]]]=0;
						  ord=buf[5+buf[4]+1+buf[5+buf[4]]]+0xff*buf[5+buf[4]+1+buf[5+buf[4]]+1];						// ordinal
						  if ((!strcmp(mod, module)) && (ord==ordinal)) return func;//printf("%s %s %d\n", func, mod, ord);
					  } else {
						  // panic!
					  }
				  }
			  }
		  }

  		  if (head.type==0xf1) break; // End of lib

		  if (head.type==0x8a) // end of obj
		  {
			  fgetpos(f, &pos);
//			  printf("%d %d\n", pos, 512-pos%512);
			  fread(&buf, 1, 512-pos%512, f);
		  }
	  }
	  }
          fclose(f);
	}
}

int main(int argc, char *argv[])
{
  FILE * f;
  struct exe_hdr MZHeader;
  struct new_exe NEHeader;
  int result;
  int i;
  int MouAPI = 0;
  int KbdAPI = 0;
  int VioAPI = 0;
  int DLLAPI = 0;
  char ** mods = NULL;

  printf("osFree FamilyAPI Binder v.0.1\n");

    // Open ourself for read
    if(f=fopen("attrib.exe"/*argv[0]*/, "rb"))
    {
      // Read old Executable header
      if ( (result = fread(&MZHeader, 1, sizeof(MZHeader), f)) == sizeof(MZHeader) )
	  {
        // Check MZ Header magic
        if (E_MAGIC(MZHeader) == EMAGIC)
        {
          // Seek New Executable header
          if ( !fseek(f, E_LFANEW(MZHeader), SEEK_SET))
          {
            // Read New Executable header
            if ( (result = fread(&NEHeader, 1, sizeof(NEHeader), f)) == sizeof(NEHeader) )
            {
              // Check NE Header magic
              if (NE_MAGIC(NEHeader) == NEMAGIC)
              {
                /* check for OS/2 program */
                if (NE_EXETYP(NEHeader) == NE_OS2)
                {
    			  // seek to Module table
                  if (!fseek(f, E_LFANEW(MZHeader)+NE_MODTAB(NEHeader), SEEK_SET))
                  {
                    //Allocate memory for mod table
					WORD offset;
					BYTE len;
					WORD count;
					struct new_seg seg;
					int j;
					struct new_rlc rlc;
					
                    mods=(char**)malloc(NE_CMOD(NEHeader)*sizeof(char*));
					
					// Read mod table
					for (i=NE_CMOD(NEHeader); i >0 ; i-- )
					{
                      fseek(f, E_LFANEW(MZHeader)+NE_MODTAB(NEHeader)+2*(NE_CMOD(NEHeader)-i), SEEK_SET);
					  fread(&offset, 1, sizeof(offset), f);
					  fseek(f, E_LFANEW(MZHeader)+NE_IMPTAB(NEHeader)+offset, SEEK_SET);
					  fread(&len, 1, sizeof(len), f);
					  mods[NE_CMOD(NEHeader)-i]=malloc(len+1);
					  fread(mods[NE_CMOD(NEHeader)-i], 1, len, f);
					  mods[NE_CMOD(NEHeader)-i][len]=0;
					  //printf("%d %s\n", i, mods[NE_CMOD(NEHeader)-i]);
					}
					  
					// Now read segments fixup tables and build list of imported functions
                    for (i = NE_CSEG(NEHeader); i > 0; i--)
                    {
                      // Read segment table entry
					  fseek(f, E_LFANEW(MZHeader)+NE_SEGTAB(NEHeader)+(NE_CSEG(NEHeader)-i)*sizeof(struct new_seg), SEEK_SET);
                      fread(&seg, 1, sizeof(struct new_seg), f);
					  // Read relocation table
					  fseek(f, (seg.ns_sector<<NEHeader.ne_align)+(seg.ns_cbseg?seg.ns_cbseg:0x10000), SEEK_SET);
					  fread(&count, 1, sizeof(count), f);
					  //printf("%d pos=%d %d\n", i, (seg.ns_sector<<NEHeader.ne_align)+(seg.ns_cbseg?seg.ns_cbseg:0x10000), count);
                      for (j = 0; j < count; j++)
                      {
						fread(&rlc, 1, sizeof(struct new_rlc), f);
						if (((rlc.nr_flags & NRRTYP)==NRRORD) || ((rlc.nr_flags & NRRTYP)==NRRNAM))
						{
							if ((rlc.nr_flags & NRRTYP)==NRRNAM)
							{
							  // not supported yet...
  							  printf("Panic!\n");
							  return 1;
							} else {
  						      printf("%s.%d %s\n", mods[rlc.nr_union.nr_import.nr_mod-1], rlc.nr_union.nr_import.nr_proc, findfunctionname(mods[rlc.nr_union.nr_import.nr_mod-1],rlc.nr_union.nr_import.nr_proc));
							}
						}		
					  }
                    }

					
                        if (!fclose(f))
                        {
                          // Exit
                          return 0;
                        } else {
                          printf("Error: Close file\n");
                          return 1;
                        }
                  } else {
                    printf( "Error: Seek module reference table\n");
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
}

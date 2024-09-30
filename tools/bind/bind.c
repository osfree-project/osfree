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
					  
					// Now read segment fixup table and build list of imported functions
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

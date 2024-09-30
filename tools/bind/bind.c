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

#include "winemu.h"
#include "newexe.h"

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

    // Open ourself for read
    if( (f  = _lopen("attrib.exe"/*argv[0]*/, OF_READ)) != (HFILE)HFILE_ERROR )
    {
      // Read old Executable header
      if ( (result = _lread(f, &MZHeader, sizeof(MZHeader))) == sizeof(MZHeader) )
	  {
        // Check MZ Header magic
        if (E_MAGIC(MZHeader) == EMAGIC)
        {
          // Seek New Executable header
          if ( (result = _llseek(f, E_LFANEW(MZHeader), SEEK_SET)) != HFILE_ERROR )
          {
            // Read New Executable header
            if ( (result = _lread(f, &NEHeader, sizeof(NEHeader))) == sizeof(NEHeader) )
            {
              // Check NE Header magic
              if (NE_MAGIC(NEHeader) == NEMAGIC)
              {
                /* check for OS/2 program */
                if (NE_EXETYP(NEHeader) == NE_OS2)
                {
    			  // seek to Module table
                  if ( (result = _llseek(f, E_LFANEW(MZHeader)+NEHeader.ne_modtab, SEEK_SET)) != HFILE_ERROR )
                  {
					  //Allocate memory for mod table
					  
                        if (_lclose(f)!= (HFILE)HFILE_ERROR)
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

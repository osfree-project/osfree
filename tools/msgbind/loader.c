#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos.h>
#include <os2\newexe.h>

int main( void )
{
  union REGS aRegs;
  char filename[_MAX_PATH];
  FILE *f;
  struct exe_hdr MZHeader;
  struct new_exe NEHeader;
  int result;


  if( (f  = fopen( filename, "rb" )) == NULL )
  {
    printf( "Error\n");
    return 1;
  }

  result = fread(&MZHeader, 1, sizeof(MZHeader), f);

  result = fseek(f, E_LFANEW(MZHeader), SEEK_SET);

  result = fread(&NEHeader, 1, sizeof(NEHeader), f);

  result = fseek(f, E_LFANEW(MZHeader)+NE_SEGTAB(NEHeader), SEEK_SET);

//NE_CSEG(x)

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


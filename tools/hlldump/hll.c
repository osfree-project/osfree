/*
   HLL debug info dumper

   for a moment we support limited range of HLL format internals, if you
   need other, feel free to add them. Btw: we support only LX files...

   Bartosz Tomasik <bart2@asua.org.pl>

   note: the whole thing is based on
       - sd386 (by IBM)
       - dbgLxDumper (Knut St.Osmundsen, Project Odin)

*/
#include <os2.h>


#include <stdlib.h>
#include <stdio.h>

#include "hllfuncs.h"

#pragma pack(push, 1)
#include "hll.h"
#pragma pack(pop)


// -----


/* modules */
ULONG ulModules;
MODULE *pModules;

/* directory */
ULONG ulDirEntries=0;
extern ULONG ulDirOffset;


/* general */
void * pvFile; /* pointer to buffer cointaing file */
BYTE bDebugFormat; /* debug format */
ULONG ulDebugOff;

void main(int argc, char *argv[])
{
 if (argc==1)
 {
   printf("Usuage: hlldump file");
   return;
 };

 pvFile=NULL;
 pModules=NULL;

 HllLoadFile(argv[1]);

 if (pvFile==NULL) free(pvFile);

 if (!ProcessLx()) goto error;

 /* these two are required for rest to work properly */
 ProcessDirectory();
 ProcessModules();
 /* all below can be commented out to turn off dumps generation */
   ProcessPublics();
   ProcessLines();
   ProcessSymbols();
   ProcessTypes();

  goto ok;
error:
 printf("*** Error: last function did not complete sucessfully\n");
ok:

 if (pvFile!=NULL) free(pvFile);

};


void HllLoadFile(PSZ szFileName)
{
  FILE *pFile;
  ULONG ulFileSize=0;

  pvFile=NULL; /*we have no buffer yet */

  printf("+++ Loading %s file into memory\n",szFileName);
  pFile=fopen(szFileName,"rb");
  if (pFile==NULL)
  {
    printf("*** Error while opening: %s\n",szFileName);
    return;
  };

  rewind(pFile);
  fseek(pFile,0,SEEK_END);
  ulFileSize=ftell(pFile);

  printf("    - file size: %u\n",ulFileSize);
  if (ulFileSize==0)
  {
    printf("*** Error: file size 0 bytes\n");
    fclose(pFile);
    return;
  };

  pvFile=calloc(ulFileSize,1);
  rewind(pFile);

  if (fread(pvFile,1,ulFileSize,pFile)!=ulFileSize)
  {
    printf("*** Error: could not load whole file...\n");
    free(pvFile);
    fclose(pFile);
    return;
  };

  fclose(pFile);
  printf("--- Loaded sucessfully\n");

};

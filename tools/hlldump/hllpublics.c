/* public symbols loading and dumping */
#include <os2.h>

#include <stdio.h>

#include "hllfuncs.h"

#pragma pack(push, 1)
#include "hll.h"
#pragma pack(pop)

extern void *pvFile;
extern BYTE bDebugFormat; /* debug format */
extern ULONG ulDebugOff;
extern ULONG ulModules;
extern MODULE *pModules;

BOOL ProcessPublics()
{
  ULONG i;
  FILE *dmp;
  printf("+++ Processing public symbols\n");

  dmp=fopen("publics.dat","a"); // !!

  for (i=0;i<ulModules;i++)
  {
   PHLLPUBLICSYM32  pPubSym = (PHLLPUBLICSYM32)((PBYTE)pvFile + ulDebugOff +pModules[i].Publics);
   PBYTE test;
   if (pModules[i].Publics==0) continue;

   fprintf(dmp,"\n\nmodule number: 0x%08x (%u)\n",i+1,i+1);

  while ( (UCHAR *)(pPubSym)- (PBYTE)pvFile - ulDebugOff -pModules[i].Publics < pModules[i].PubLen)
  {
    fprintf(dmp,"    %#03x:%#08x iType=%#2x  name=%.*s\n",
                            pPubSym->usObject,
                            pPubSym->ulOffset,
                            pPubSym->usType,
                            pPubSym->cchNameLen,
                            pPubSym->achName);

    /* next */
    pPubSym = (PHLLPUBLICSYM32)&pPubSym->achName[pPubSym->cchNameLen];
  };

  }; //end: for (i=0;i<ulModules;i++)

  fclose(dmp);

  printf("--- Processed\n");
  return TRUE;
};


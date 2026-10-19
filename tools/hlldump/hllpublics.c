/*!
 * @file hllpublics.c
 * @brief Loading and dumping of HLL public symbols.
 */

/* public symbols loading and dumping */
#include <os2.h>

#include <stdio.h>

#include "hllfuncs.h"

#pragma pack(push, 1)
#include "hll.h"
#pragma pack(pop)

/*!
 * @brief Buffer holding the whole loaded executable file.
 */
extern void *pvFile;
/*!
 * @brief Detected debug information format.
 */
extern BYTE bDebugFormat; /* debug format */
/*!
 * @brief Offset of the debug information within the file.
 */
extern ULONG ulDebugOff;
/*!
 * @brief Number of modules described in the HLL debug directory.
 */
extern ULONG ulModules;
/*!
 * @brief Per-module information table.
 */
extern MODULE *pModules;

/*!
 * @brief Dump public symbols for all modules.
 * @retval TRUE Always returns TRUE.
 * @return TRUE on success.
 */
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

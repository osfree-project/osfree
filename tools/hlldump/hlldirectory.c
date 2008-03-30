/* functions related to HLL directory loading and dumping */

#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hllfuncs.h"

#pragma pack(push, 1)
#include "hll.h"
#pragma pack(pop)

extern void *pvFile;
extern BYTE bDebugFormat; /* debug format */
extern ULONG ulDebugOff;
extern ULONG ulModules;
extern MODULE *pModules;
/*extern ULONG ulDirOffset;
extern ULONG ulDirEntries;*/

BOOL ProcessDirectory()
{
 const PHLLHDR pHdr= (PHLLHDR)((PBYTE)pvFile + ulDebugOff);
 PHLLDIR pHllDir;
 POLDHLLDIR pOldHllDir;
 ULONG ulDirSize;
 ULONG ulDirEntries;
 ULONG i;
 FILE *dmp;

 HLLDIRENTRY tmpHllDirEntry;

 printf("+++ Processing HLL directory\n");

 printf("    - directory offset: 0x%08x (%d)\n",pHdr->ulDirOffset,
        pHdr->ulDirOffset);

 if (bDebugFormat!=DBGTYPE_32IBM)
 {
  pOldHllDir=(POLDHLLDIR)((PBYTE)pHdr+pHdr->ulDirOffset);
  printf("    - number of entries in directory 0x%08x (%d)\n",pOldHllDir->cEntries,pOldHllDir->cEntries);
  ulDirEntries=pOldHllDir->cEntries;
  ulDirSize=ulDirEntries*sizeof(OLDHLLDIRENTRY);
 } else
 {
  pHllDir=(PHLLDIR)((PBYTE)pHdr+pHdr->ulDirOffset);
  printf("    - directory header size: 0x%02x (%d)\n",pHllDir->cb,pHllDir->cb);
  printf("    - number of entries in directory: 0x%08x (%d)\n",pHllDir->cEntries,pHllDir->cEntries);
  printf("    - size of each entry: 0x%02x (%d)\n",pHllDir->cbEntry,pHllDir->cbEntry);
  ulDirEntries=pHllDir->cEntries;
  if (pHllDir->cbEntry!=sizeof(HLLDIRENTRY))
   printf("    * warning: Hll directory entries (reported and built-in) don't match!\n");
  ulDirSize=ulDirEntries*sizeof(HLLDIRENTRY);
 }; //END: if (bDebugFormat!=DBGTYPE_32IBM)

 printf("    - total directory size: 0x%08x (%u)\n",ulDirSize);

 /* TODO: check if that does not exceed pvFile buffer size!); */
 ulModules=0;

 /* first, get the number of different modules out there */
 for (i = 0; i < ulDirEntries; i++)
 {
  if (bDebugFormat!=DBGTYPE_32IBM)
   {
   if (pOldHllDir->aEntries[i].ModIndex>ulModules)
           ulModules=pOldHllDir->aEntries[i].ModIndex;
   }
   else
   {
   if (pHllDir->aEntries[i].ModIndex>ulModules)
           ulModules=pHllDir->aEntries[i].ModIndex;
   };
 }; // for (i = 0; i < ulDirEntries; i++)

 printf("    - number of different modules described: 0x%08x (%u)\n",ulModules,ulModules);

 pModules=(MODULE *)calloc(sizeof(MODULE),ulModules);

 dmp=fopen("directory.dat","a"); // !!

 /* for each entry in directory */
 for (i = 0; i < ulDirEntries; i++)
 {
  USHORT usSubSect=((bDebugFormat!=DBGTYPE_32IBM)
                    ? pOldHllDir->aEntries[i].SubSectType
                    : pHllDir->aEntries[i].SubSectType);

        static const char * apsz[] =
        {
            "HLL_DE_MODULES",
            "HLL_DE_PUBLICS",
            "HLL_DE_TYPES",
            "HLL_DE_SYMBOLS",
            "HLL_DE_SRCLINES",
            "HLL_DE_LIBRARIES",
            "unknown",
            "unknown",
            "HLL_DE_SRCLNSEG",
            "unknown",
            "HLL_DE_IBMSRC"
        };

       const char *pszType = usSubSect >= HLL_DE_MODULES
                              && usSubSect <= HLL_DE_IBMSRC
                              ? apsz[usSubSect - HLL_DE_MODULES]
                              : "unknown";

  fprintf(dmp,"\n\ndirectory entry: 0x%08x (%u)\n",i+1,i+1);

  if (bDebugFormat!=DBGTYPE_32IBM)
  {
   tmpHllDirEntry.SubSectType=usSubSect;
   tmpHllDirEntry.ModIndex=pOldHllDir->aEntries[i].ModIndex;
   tmpHllDirEntry.SubSectOff=pOldHllDir->aEntries[i].SubSectOff;
   tmpHllDirEntry.SubSectLen=pOldHllDir->aEntries[i].SubSectLen;
  } else
  {
    memcpy(&tmpHllDirEntry,&(pHllDir->aEntries[i]),sizeof(HLLDIRENTRY));
  };

        fprintf(dmp, "    usType   0x%08x (%d) %s\n"
                       "    ModIndex 0x%08x (%d)\n"
                       "    offset   0x%08x (%d)\n"
                       "    length   0x%08x (%d)\n",
                       usSubSect,
                       usSubSect,
                       pszType,
                       tmpHllDirEntry.ModIndex,
                       tmpHllDirEntry.ModIndex,
                       tmpHllDirEntry.SubSectOff,
                       tmpHllDirEntry.SubSectOff,
                       tmpHllDirEntry.SubSectLen,
                       tmpHllDirEntry.SubSectLen);

  /* we skip modules with id 0 */
  if (tmpHllDirEntry.ModIndex==0) continue;

  switch (usSubSect)
  {
    case HLL_DE_MODULES:
      pModules[tmpHllDirEntry.ModIndex-1].FileName    =
                                                  tmpHllDirEntry.SubSectOff;
      pModules[tmpHllDirEntry.ModIndex-1].FileNameLen =
                                                  tmpHllDirEntry.SubSectLen;

    break;

    case HLL_DE_PUBLICS:
      pModules[tmpHllDirEntry.ModIndex-1].Publics =
                                                  tmpHllDirEntry.SubSectOff;
      pModules[tmpHllDirEntry.ModIndex-1].PubLen  =
                                                  tmpHllDirEntry.SubSectLen;
    break;

    case HLL_DE_TYPES  :
      pModules[tmpHllDirEntry.ModIndex-1].TypeDefs =
                                                  tmpHllDirEntry.SubSectOff;
      pModules[tmpHllDirEntry.ModIndex-1].TypeLen  =
                                                  tmpHllDirEntry.SubSectLen;
    break;

    case HLL_DE_SYMBOLS:
      pModules[tmpHllDirEntry.ModIndex-1].Symbols =
                                                  tmpHllDirEntry.SubSectOff;
      pModules[tmpHllDirEntry.ModIndex-1].SymLen  =
                                                  tmpHllDirEntry.SubSectLen;
    break;

    case HLL_DE_SRCLINES:
    case HLL_DE_SRCLNSEG:
    case HLL_DE_IBMSRC:
      pModules[tmpHllDirEntry.ModIndex-1].LineNums =
                                                  tmpHllDirEntry.SubSectOff;
      pModules[tmpHllDirEntry.ModIndex-1].LineNumsLen  =
                                                  tmpHllDirEntry.SubSectLen;
    break;

/*  case HLL_DE_LIBRARIES: // dunno what to do with these...
    break; */
  }; // end: switch (usSubSect)
 }; // end: for (i = 0; i < ulDirEntries; i++)

 fclose(dmp);

 /* dump created, and module table too, now fill it up with additional info */
 for (i = 0; i < ulDirEntries; i++)
 {

  if (bDebugFormat!=DBGTYPE_32IBM)
  {
   tmpHllDirEntry.SubSectType=pOldHllDir->aEntries[i].SubSectType;
   tmpHllDirEntry.ModIndex=pOldHllDir->aEntries[i].ModIndex;
   tmpHllDirEntry.SubSectOff=pOldHllDir->aEntries[i].SubSectOff;
   tmpHllDirEntry.SubSectLen=pOldHllDir->aEntries[i].SubSectLen;
  } else
  {
    memcpy(&tmpHllDirEntry,&(pHllDir->aEntries[i]),sizeof(HLLDIRENTRY));
  };

  /* TODO: recognize other debug formats then BN04... */
  switch (tmpHllDirEntry.SubSectType)
  {
    case HLL_DE_PUBLICS:
      pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Pubs = TYPE_PUB_32;
    break;

    case HLL_DE_TYPES:
     if (pModules[tmpHllDirEntry.ModIndex-1].FileName!=0)
     {
       PHLL04MODULE pHllModule=(PHLL04MODULE) (pHdr+pModules[tmpHllDirEntry.ModIndex-1].FileName);
       if (pHllModule->usDebugStyle==HLL)
       {
        switch( pHllModule->chVerMajor )
        {
         case 0x1:
          pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Typs = TYPE103_HL01;
         break;

         case 0x2:
          pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Typs = TYPE103_HL02;
         break;

         case 0x3:
          pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Typs = TYPE103_HL03;
         break;

         case 0x4:
          pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Typs = TYPE103_HL04;
         break;
        };
       } else
       {
        /* we assume 32bit CodeView format, but it may be 16 bit as well!!!
           TODO: add checking routine */
        pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Typs = TYPE103_CL386;

      /*ModuleType = GetBitness( pModules[pDirectory[i].ModIndex-1]pCsects->CsectLo );
      if( ModuleType == BIT16 )
       pModules[pDirectory[i].ModIndex-1].DbgFormatFlags.Typs = TYPE103_C600;*/

       };
     }; //END: if (pModules[tmpHllDirEntry.ModIndex-1].FileName!=0)
    break;

    case HLL_DE_SYMBOLS:
     if (pModules[tmpHllDirEntry.ModIndex-1].FileName!=0)
     {
       PHLL04MODULE pHllModule=(PHLL04MODULE)(pHdr+pModules[tmpHllDirEntry.ModIndex-1].FileName);
       if (pHllModule->usDebugStyle==HLL)
       {
         switch (pHllModule->chVerMajor)
         {
          case 0x1:
            pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Syms = TYPE104_HL01;
          break;

          case 0x2:
           pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Syms = TYPE104_HL02;
          break;

          case 0x3:
           pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Syms = TYPE104_HL03;
          break;

          case 0x4:
           pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Syms = TYPE104_HL04;
          break;
         };
       } else
       {
        /* we assume 32bit CodeView format, but it may be 16 bit as well!!!
           TODO: add checking routine */
      pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Syms = TYPE104_CL386;
/*      ModuleType = GetBitness( pModules[pDirectory[i].ModIndex-1].pCsects->CsectLo );
      if( ModuleType == BIT16 )
       pModules[pDirectory[i].ModIndex-1].DbgFormatFlags.Syms = TYPE104_C600;*/
       };
     };//if (pModules[tmpHllDirEntry.ModIndex-1].FileName!=0)
    break;

    case HLL_DE_SRCLINES:
          pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Lins = TYPE105;
    break;

    case HLL_DE_SRCLNSEG:
          pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Lins = TYPE109_32;
    break;

    case HLL_DE_IBMSRC:
     if (pModules[tmpHllDirEntry.ModIndex-1].FileName!=0)
     {
       PHLL04MODULE pHllModule=(PHLL04MODULE)((PBYTE)pHdr+pModules[tmpHllDirEntry.ModIndex-1].FileName);
       switch (pHllModule->chVerMajor)
       {
      case 0x1:
       pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Lins = TYPE10B_HL01;
       break;

      case 0x2:
       pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Lins = TYPE10B_HL02;
       break;

      case 0x3:
       pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Lins = TYPE10B_HL03;
       break;

      case 0x4:
       pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Lins = TYPE10B_HL04;
       break;
      default:
       /* this should never happen but if.. well it's bad for us :> */
       pModules[tmpHllDirEntry.ModIndex-1].DbgFormatFlags.Lins = 0;
       };
     };
    break;
  }; //end: switch (usSubSect)

 }; //end: for (i = 0; i < ulDirEntries; i++)

 printf("--- Processed\n");

 return TRUE;
};

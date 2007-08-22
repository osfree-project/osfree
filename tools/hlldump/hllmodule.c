/* module structure dumping */

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

BOOL ProcessModules()
{
  ULONG i;
  FILE *dmp;
  printf("+++ Processing modules\n");

  dmp=fopen("modules.dat","a"); // !!

  for (i=0;i<ulModules;i++)
  {
   PHLL04MODULE pHllModule= (PHLL04MODULE)((PBYTE)pvFile + ulDebugOff +pModules[i].FileName);
   ULONG ulSegCount;
   ULONG ulCurrSeg;
   PHLL04SEGINFO pHllSeg;


   if (pModules[i].FileName==0) continue;

   fprintf(dmp,"\n\nmodule number: 0x%08x (%u)\n",i+1,i+1);

                fprintf(dmp,
                        "    Modulename:   %.*s\n"
                        "    overlay       %d\n"
                        "    ilib          %d\n"
                        "    pad           %d\n"
                        "    cSegInfo      %d\n"
                        "    usDebugStyle  %#04x %c%c\n"
                        "    HLL Version   %d.%d\n"
                        "    cchName       %d\n"
                        ,
                        pHllModule->cchName,
                        &pHllModule->achName[0],
                        pHllModule->overlay,
                        pHllModule->iLib,
                        pHllModule->pad,
                        pHllModule->cSegInfo,
                        pHllModule->usDebugStyle,
                        pHllModule->usDebugStyle & 0xFF,
                        pHllModule->usDebugStyle >> 8,
                        pHllModule->chVerMajor,
                        pHllModule->chVerMinor,
                        pHllModule->cchName
                        );

          fprintf(dmp,"    Module Info Offset:  0x%08x (%u)\n"
                      "    Module Info Lenght:  0x%08x (%u)\n"
                      "    Publics Info Offset: 0x%08x (%u)\n"
                      "    Publics Info Lenght: 0x%08x (%u)\n"
                      "    Types Info Offset:   0x%08x (%u)\n"
                      "    Types Info Lenght:   0x%08x (%u)\n"
                      "    Lines Info Offset:   0x%08x (%u)\n"
                      "    Lines Info Lenght:   0x%08x (%u)\n"
                      "    Symbols Info Offset: 0x%08x (%u)\n"
                      "    Symbols Info Lenght: 0x%08x (%u)\n",
                      pModules[i].FileName,pModules[i].FileName,
                      pModules[i].FileNameLen,pModules[i].FileNameLen, 
                      pModules[i].Publics,pModules[i].Publics,
                      pModules[i].PubLen,pModules[i].PubLen,
                      pModules[i].TypeDefs,pModules[i].TypeDefs,
                      pModules[i].TypeLen,pModules[i].TypeLen,
                      pModules[i].LineNums,pModules[i].LineNums,
                      pModules[i].LineNumsLen,pModules[i].LineNumsLen,
                      pModules[i].Symbols,pModules[i].Symbols,
                      pModules[i].SymLen,pModules[i].SymLen);

                /*
                 * Dump Segment info
                 */
                fprintf(dmp,
                        "     SegmentInfo %d\n"
                        "       Object    %#x\n"
                        "       Offset    %#x\n"
                        "       Length    %#x\n",
                        0,
                        pHllModule->SegInfo0.usObject,
                        pHllModule->SegInfo0.ulOffset,
                        pHllModule->SegInfo0.ulLength);

        ulSegCount = pHllModule->cSegInfo > 0 ? pHllModule->cSegInfo : 0;
        pHllSeg = (PHLL04SEGINFO)((void*)&pHllModule->achName[pHllModule->cchName]);

        for (ulCurrSeg = 0; ulCurrSeg + 1 < ulSegCount; ulCurrSeg++)
        {
                    fprintf(dmp,
                        "     SegmentInfo %d\n"
                        "       Object    %#x\n"
                        "       Offset    %#x\n"
                        "       Length    %#x\n",
                        ulCurrSeg + 1,
                        pHllSeg[ulCurrSeg].usObject,
                        pHllSeg[ulCurrSeg].ulOffset,
                        pHllSeg[ulCurrSeg].ulLength);
         };


  }; //end: for (i=0;i<ulModules;i++)

  fclose(dmp);

  printf("--- Processed\n");
  return TRUE;
};


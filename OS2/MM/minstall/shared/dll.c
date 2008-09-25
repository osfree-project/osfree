
// щ Д ДДДДНН = Д  щ  Д = ННДДДД Д щ
// і                               і
//    ЬЫЫЫЫЫЫЫЬ   ЬЫЬ  ЬЫЫЫЫЫЫЫЫЬ          ъ  ъДДДНДДНДННДДННННДНННННННННОД
// і ЫЫЫЫЯЯЯЫЫЫЫ ЫЫЫЫЫ ЫЫЫЯ   ЯЫЫЫ і             MINSTALL Front-End      є
// є ЫЫЫЫЬЬЬЫЫЫЫ ЫЫЫЫЫ ЫЫЫЬ   ЬЫЫЫ є      ъ ДДДДНДННДДННННДННННННННДНННННОД
// є ЫЫЫЫЫЫЫЫЫЫЫ ЫЫЫЫЫ ЫЫЫЫЫЫЫЫЫЯ  є       Section: MMOS/2 for eCS       є
// є ЫЫЫЫ   ЫЫЫЫ ЫЫЫЫЫ ЫЫЫЫ ЯЫЫЫЫЬ є     і Created: 28/10/02             є
// і ЯЫЫЯ   ЯЫЫЯ  ЯЫЯ  ЯЫЫЯ   ЯЫЫЯ і     і Last Modified:                і
//                  ЬЬЬ                  і Number Of Modifications: 000  і
// щ              ЬЫЫЯ             щ     і INCs required: *none*         і
//      ДДДДДДД ЬЫЫЯ                     є Written By: Martin Kiewitz    і
// і     ЪїЪїіЬЫЫЫЬЬЫЫЫЬ           і     є (c) Copyright by              і
// є     АЩіАЩЯЫЫЫЯЯЬЫЫЯ           є     є      AiR ON-Line Software '02 ъ
// є    ДДДДДДД    ЬЫЫЭ            є     є All rights reserved.
// є              ЬЫЫЫДДДДДДДДД    є    ДОНННДНННННДННННДННДДНДДНДДДъДД  ъ
// є             ЬЫЫЫЭі іЪїііД     є
// і            ЬЫЫЫЫ АДііАЩіД     і
//             ЯЫЫЫЫЭДДДДДДДДДД     
// і             ЯЯ                і
// щ Дґ-=’iз йп-Liпо SйџвW’зо=-ГДД щ

#define INCL_NOPMAPI
#define INCL_BASE
#define INCL_DOSMODULEMGR
#include <os2.h>

#include <global.h>

HMODULE DLL_Load (PSZ DLLName) {
   HMODULE DLLHandle = 0;
   if (DosLoadModule (NULL, 0, DLLName, &DLLHandle))
      return 0;
   return DLLHandle;
 }

VOID DLL_UnLoad (HMODULE DLLNameHandle) {
   DosFreeModule (DLLNameHandle);
 }

BOOL DLL_GetDataResource (HMODULE DLLHandle, ULONG ResourceID, PPVOID ResourcePtrPtr, PULONG ResourceSizePtr) {
   if (DosGetResource(DLLHandle, RT_RCDATA, ResourceID, ResourcePtrPtr))
      return FALSE;
   if (DosQueryResourceSize(DLLHandle, RT_RCDATA, ResourceID, ResourceSizePtr))
      return FALSE;
   return TRUE;
 }

PFN DLL_GetEntryPoint (HMODULE DLLHandle, PSZ EntryPointNamePtr) {
   PFN ResultCodePtr = 0;
   if (DosQueryProcAddr(DLLHandle, 0, EntryPointNamePtr, &ResultCodePtr))
      return NULL;
   return ResultCodePtr;
 }

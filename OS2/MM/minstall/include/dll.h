#ifndef _DLL_H_
#define _DLL_H_

#include <os2.h>

HMODULE DLL_Load (PSZ DLLName);
VOID DLL_UnLoad (HMODULE DLLNameHandle);
BOOL DLL_GetDataResource (HMODULE DLLHandle, ULONG ResourceID, PPVOID ResourcePtrPtr, PULONG ResourceSizePtr);
PFN DLL_GetEntryPoint (HMODULE DLLHandle, PSZ EntryPointNamePtr);

#endif

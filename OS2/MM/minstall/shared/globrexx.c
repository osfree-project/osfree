
#define INCL_NOPMAPI
#define INCL_BASE
#define INCL_DOSMODULEMGR
#include <os2.h>
#define INCL_REXXSAA
#include <rexxsaa.h>                        /* needed for RexxStart()     */

#include <global.h>
#include <globstr.h>
#include <globrexx.h>

BOOL REXX_SetVariable (PSZ VarName, PCHAR StringPtr, ULONG StringLen) {
   SHVBLOCK SharedVarBlock;

   SharedVarBlock.shvnext            = NULL;
   SharedVarBlock.shvname.strptr     = VarName;
   SharedVarBlock.shvname.strlength  = strlen(VarName);
   SharedVarBlock.shvvalue.strptr    = StringPtr;
   SharedVarBlock.shvvalue.strlength = StringLen;
   SharedVarBlock.shvcode            = RXSHV_SET;
   if ((RexxVariablePool(&SharedVarBlock)) && (SharedVarBlock.shvret==0))
      return FALSE;
   return TRUE;
 }

BOOL REXX_SetVariableViaPSZ (PSZ VarName, PSZ PSZPtr) {
   ULONG  PSZLength = strlen(PSZPtr);
   return REXX_SetVariable (VarName, PSZPtr, PSZLength);
 }

BOOL REXX_SetVariableViaULONG (PSZ VarName, ULONG Value) {
   CHAR TempBuffer[12];
   ultoa (Value, TempBuffer, 10);
   return REXX_SetVariableViaPSZ (VarName, TempBuffer);
 }

BOOL REXX_SetVariableViaULONGHex (PSZ VarName, ULONG Value) {
   CHAR TempBuffer[12];
   ultoa (Value, TempBuffer, 16);
   return REXX_SetVariableViaPSZ (VarName, TempBuffer);
 }

LONG REXX_GetStemVariable (PCHAR DestPtr, ULONG DestMaxLength, PSZ VarNameBase, USHORT VarNameID) {
   ULONG    VarNameBaseLen = strlen(VarNameBase);
   CHAR     VarWorker[256];
   PCHAR    TmpPos         = (PCHAR)((ULONG)&VarWorker+VarNameBaseLen);
   SHVBLOCK SharedVarBlock;

   if (DestMaxLength==0)
      return -1;                            // No destination space...

   *DestPtr = 0;                            // Set NUL-string

   if ((VarNameBaseLen==0) || (VarNameBaseLen>250))
      return -1;                            // Buffer-overflow or no variable

   memcpy(&VarWorker, VarNameBase, VarNameBaseLen);
   if (*(TmpPos-1)!=0x2E)
      return -1;                            // No stem variable
   itoa(VarNameID, TmpPos, 10);

   SharedVarBlock.shvnext            = NULL;
   SharedVarBlock.shvname.strptr     = (PCHAR)&VarWorker;
   SharedVarBlock.shvname.strlength  = strlen(VarWorker);
   SharedVarBlock.shvvalue.strptr    = DestPtr;
   SharedVarBlock.shvvalue.strlength = DestMaxLength-1;
   SharedVarBlock.shvcode            = RXSHV_FETCH;
   if ((RexxVariablePool(&SharedVarBlock)) && (SharedVarBlock.shvret==0))
      return -1;
   TmpPos  = DestPtr+SharedVarBlock.shvvaluelen;
   *TmpPos = 0;                             // Set terminating NUL
   return SharedVarBlock.shvvaluelen;
 }

BOOL REXX_SetStemVariable (PSZ VarNameBase, USHORT VarNameID, PCHAR StringPtr, ULONG StringLen) {
   ULONG    VarNameBaseLen = strlen(VarNameBase);
   CHAR     VarWorker[256];
   PCHAR    TmpPos         = (PCHAR)((ULONG)&VarWorker+VarNameBaseLen);
   SHVBLOCK SharedVarBlock;

   if ((VarNameBaseLen==0) || (VarNameBaseLen>250))
      return FALSE;                         // Buffer-overflow or no variable

   memcpy(&VarWorker, VarNameBase, VarNameBaseLen);
   if (*(TmpPos-1)!=0x2E)
      return FALSE;                         // No stem variable
   itoa(VarNameID, TmpPos, 10);

   SharedVarBlock.shvnext            = NULL;
   SharedVarBlock.shvname.strptr     = (PCHAR)&VarWorker;
   SharedVarBlock.shvname.strlength  = strlen(VarWorker);
   SharedVarBlock.shvvalue.strptr    = StringPtr;
   SharedVarBlock.shvvalue.strlength = StringLen;
   SharedVarBlock.shvcode            = RXSHV_SET;
   if ((RexxVariablePool(&SharedVarBlock)) && (SharedVarBlock.shvret==0))
      return FALSE;
   return TRUE;
 }

BOOL REXX_SetStemVariableViaPSZ (PSZ VarNameBase, USHORT VarNameID, PSZ PSZPtr) {
   ULONG  PSZLength = strlen(PSZPtr);
   return REXX_SetStemVariable (VarNameBase, VarNameID, PSZPtr, PSZLength);
 }

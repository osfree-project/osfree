#ifndef _GLOBSTR_H_
#define _GLOBSTR_H_

#define INCL_NOPMAPI
#define INCL_BASE
#define INCL_DOSMODULEMGR
#include <os2.h>

PCHAR STRING_GetCmdLinePtr (void);
BOOL STRING_Copy (PSZ DestString, ULONG DestMaxLength, PCHAR StringBegin, PCHAR StringEnd);
BOOL STRING_CopyPSZ (PSZ DestString, ULONG DestMaxLength, PSZ String);
// Combines 2 Strings and also checks for buffer-overflow
//  if returns false, DestString buffer is too small
BOOL STRING_CombinePSZ (PSZ DestString, ULONG DestMaxLength, PSZ String1, PSZ String2);
// If EndPos==0 -> Won't do buffer overflow checking... only used on KNOWN data
PCHAR STRING_SkipASCIIZ (PCHAR StartPos, PCHAR EndPos);
PCHAR STRING_GetASCIIZString (PCHAR DestPtr, ULONG DestMaxSize, PCHAR StartPos, PCHAR EndPos);
PCHAR STRING_GetASCIIZNumericValue (PULONG DestPtr, PCHAR StartPos, PCHAR EndPos);
// Generates a String and escapes '\' and '"' chars and puts result in DestString
//  Replies 0 on buffer-overflow (or NUL variable) otherwise length of result
PCHAR STRING_BuildEscaped (PCHAR DestPtr, PULONG DestMaxSizePtr, PSZ StringPtr);
PCHAR STRING_GetEndOfLinePtr (PCHAR CurPos, PCHAR EndPos);
CHAR STRING_GetValidChar (PCHAR *CurPosPtr, PCHAR EndPos, PULONG CurLineNo);
PCHAR STRING_SkipString (PCHAR StartPos, PCHAR EndPos);
PCHAR STRING_SkipNumericValue (PCHAR StartPos, PCHAR EndPos);
// Extracts a string from an ASCII string using the \ escape mechanismn
PCHAR STRING_GetString (PCHAR DestPtr, ULONG DestMaxSize, PCHAR StartPos, PCHAR EndPos);
// Extracts a value (e.g. '0', '0L', '0x0' and '0x0L') including error checking
//  Needs to be called using CurPos == First digit of value
PCHAR STRING_GetNumericValue (PULONG DestPtr, PCHAR StartPos, PCHAR EndPos);
// Isolates one word of a string. Replies with next StartPos, if successful
//  otherwise NULL. Will set WordBeginPos and WordLength accordingly.
//  Example: 'A brown fox jumped over an assembler'
//            IsolateWord -> 'A'
//            IsolateWord -> 'brown'
//            etc.
PCHAR STRING_IsolateWord (PCHAR *WordBeginPosPtr, PULONG WordLengthPtr, PCHAR StartPos, PCHAR EndPos);

#endif


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
#include <globstr.h>

PCHAR STRING_GetCmdLinePtr (void) {
   PTIB  ptib;
   PPIB  ppib;
   PCHAR CmdLinePtr;

   DosGetInfoBlocks(&ptib, &ppib);
   CmdLinePtr = STRING_SkipASCIIZ(ppib->pib_pchcmd, NULL);
   while (*CmdLinePtr==' ')
      CmdLinePtr++;
   return CmdLinePtr;
 }

BOOL STRING_Copy (PSZ DestString, ULONG DestMaxLength, PCHAR StringBegin, PCHAR StringEnd) {
   ULONG BytesToCopy = StringEnd-StringBegin;

   if (BytesToCopy>=DestMaxLength)
      return FALSE;
   memcpy (DestString, StringBegin, BytesToCopy);
   DestString += BytesToCopy;
   *DestString = 0;
   return TRUE;
 }

BOOL STRING_CopyPSZ (PSZ DestString, ULONG DestMaxLength, PSZ String) {
   ULONG StringLen = strlen(String);

   if (StringLen>=DestMaxLength)
      return FALSE;
   memcpy (DestString, String, StringLen);
   DestString += StringLen;
   *DestString = 0;                      // Terminating NUL
   return TRUE;
 }

// Combines 2 Strings and also checks for buffer-overflow
//  if returns false, DestString buffer is too small
BOOL STRING_CombinePSZ (PSZ DestString, ULONG DestMaxLength, PSZ String1, PSZ String2) {
   ULONG String1Len = strlen(String1);
   ULONG String2Len = strlen(String2);

   DestMaxLength--;                      // We need one char as NUL...
   if ((String1Len+String2Len+1)>DestMaxLength)
      return FALSE;
   memcpy (DestString, String1, String1Len);
   DestString += String1Len;
   memcpy (DestString, String2, String2Len);
   DestString += String2Len;
   *DestString = 0;                      // Terminating NUL
   return TRUE;
 }

// If EndPos==0 -> Won't do buffer overflow checking... only used on KNOWN data
PCHAR STRING_SkipASCIIZ (PCHAR StartPos, PCHAR EndPos) {
   PCHAR CurPos = StartPos;
   if (!EndPos) {
      while (*CurPos!=0)                    // <-- Badass code!
         CurPos++;
      return CurPos+1;
    }
   while (CurPos<EndPos) {
      if (*CurPos==0)
         return CurPos+1;
      CurPos++;
    }
   return NULL;
 }

PCHAR STRING_GetASCIIZString (PCHAR DestPtr, ULONG DestMaxSize, PCHAR StartPos, PCHAR EndPos) {
   PCHAR CurPos = StartPos;
   CHAR  CurChar;

   while ((CurPos<EndPos) && (DestMaxSize>0)) {
      CurChar = *CurPos; CurPos++;
      *DestPtr = CurChar; DestPtr++; DestMaxSize--;
      if (CurChar==0) {                  // Copy till and including NUL
         return CurPos;
       }
    }
   return NULL;                          // EOB encountered instead of NUL
 }

PCHAR STRING_GetASCIIZNumericValue (PULONG DestPtr, PCHAR StartPos, PCHAR EndPos) {
   PCHAR CurPos     = StartPos;
   CHAR  CurChar;
   ULONG DigitChars = 0;
   CHAR  TempBuffer[12];

   // First we check, if hexadecimal escape is found '0x'
   if (((StartPos+2)<EndPos) && (*(PUSHORT)CurPos==0x7830)) {
      CurPos += 2;
      while (CurPos<EndPos) {
         CurChar = *CurPos;
         if (CurChar==0) break;          // NUL ends the string...
         if (!(((CurChar>=0x30) && (CurChar<=0x39)) || ((CurChar>=0x41) && (CurChar<=0x46)) || (CurChar=='L')))
            return NULL;                 // Invalid value
         CurPos++; DigitChars++;
       }
    } else {
      while (CurPos<EndPos) {
         CurChar = *CurPos;
         if (CurChar==0) break;          // NUL ends the string...
         if (!((CurChar>=0x30) && (CurChar<=0x39) || (CurChar=='L')))
            return NULL;                 // Invalid value
         CurPos++; DigitChars++;
       }
    }
   if ((DigitChars) && (CurPos<EndPos)) {   // We found any valid chars?
      *DestPtr   = strtoul(StartPos, NULL, 0);
      return CurPos+1;
    }
   return NULL;
 }

// Generates a String and escapes '\' and '"' chars and puts result in DestString
//  Replies 0 on buffer-overflow (or NUL variable) otherwise length of result
PCHAR STRING_BuildEscaped (PCHAR DestPtr, PULONG DestMaxSizePtr, PSZ StringPtr) {
   CHAR  CurChar;
   PCHAR CurDestPos    = DestPtr;
   ULONG DestMaxSize   = *DestMaxSizePtr;
   PCHAR MaxDestPos    = DestPtr+DestMaxSize;
   
   while ((CurDestPos<MaxDestPos)) {
      CurChar = *StringPtr;
      if (CurChar==0) {                     // End-of-String?
         *CurDestPos      = 0;
         *DestMaxSizePtr -= CurDestPos-DestPtr;
         return CurDestPos;
       } else if (CurChar==0x0D) {          // CR? -> '\n'
         *CurDestPos = 0x5C; CurDestPos++;
         if (CurDestPos<MaxDestPos) {
            *CurDestPos = 'n'; CurDestPos++;
          }
       } else {
         if ((CurChar==0x5C) | (CurChar==0x22)) { // Escape '\' and '"'
            *CurDestPos = 0x5C; CurDestPos++;
          }
         *CurDestPos = CurChar; CurDestPos++;
       }
      StringPtr++;
    }
   CurDestPos--;
   *CurDestPos     = 0;                     // Set terminating NUL anyway
   *DestMaxSizePtr = 0;                     // No bytes left
   return NULL;                             //  for security
 }

PCHAR STRING_GetEndOfLinePtr (PCHAR CurPos, PCHAR EndPos) {
   PCHAR  LineEndPos = CurPos;

   while ((LineEndPos<EndPos) && (*LineEndPos!=0x0D))
      LineEndPos++;                         // Look for End-Of-Line...
   return LineEndPos;
 }

CHAR STRING_GetValidChar (PCHAR *CurPosPtr, PCHAR EndPos, PULONG CurLineNo) {
   PCHAR CurPos  = *CurPosPtr;
   CHAR  CurChar;

   while (CurPos<EndPos) {
      CurChar = *CurPos;
      if (CurChar==0x0D) *CurLineNo += 1;
      if ((CurChar!=0x0D) && (CurChar!=' ')) { // Search for not CR nor space
         *CurPosPtr = CurPos;
         return CurChar;
       }
      CurPos++;
    }
   *CurPosPtr = CurPos;
   return 0;
 }

PCHAR STRING_SkipString (PCHAR StartPos, PCHAR EndPos) {
   PCHAR CurPos  = StartPos;
   CHAR  CurChar;

   if (*CurPos=='"') {
      CurPos++;
      while (CurPos<EndPos) {
         CurChar = *CurPos;
         switch (CurChar) {
          case 0x0D:
            return NULL;                    // Incomplete string, but CR
          case '"':
            return CurPos+1;                // Found End-Of-String
          case 0x5C:
            CurPos++; break;
          }
         CurPos++;
       }
    } else {
      while ((CurPos<EndPos) && (*CurPos!=' ') && (*CurPos!='='))
         CurPos++;
      return CurPos;
    }
   return NULL;
 }

PCHAR STRING_SkipNumericValue (PCHAR StartPos, PCHAR EndPos) {
   PCHAR CurPos     = StartPos;
   CHAR  CurChar;

   while (CurPos<EndPos) {
      CurChar = toupper(*CurPos);
      if (CurChar=='L') {
         CurPos++; break;             // 'L' is value terminator and is skipped
       }
      if ((CurChar==0x0D) || (CurChar=='"') || (CurChar==';') || (CurChar==',') || (CurChar==' ') || (CurChar==']') || (CurChar==')'))
         break;                       // "normal" ending characters...
      CurPos++;
    }
   return CurPos;
 }

// Extracts a string from an ASCII string using the \ escape mechanismn
PCHAR STRING_GetString (PCHAR DestPtr, ULONG DestMaxSize, PCHAR StartPos, PCHAR EndPos) {
   PCHAR  CurPos     = StartPos;
   ULONG  TmpLen     = 0;
   PCHAR  CurDestPtr = DestPtr;
   CHAR   CurChar;

   // One less, because of terminating Zero
   DestMaxSize--;

   if (*StartPos=='"') {
      CurPos++;                             // Skip over '"'
      // Now take over any chars till '"' including escape mechanismn
      while (CurPos<EndPos) {
         CurChar = *CurPos;
         if (CurChar=='"') {
            *CurDestPtr = 0; return CurPos+1;
          }
         if ((CurChar==0x0D) || (CurChar==0)) break;
         if (CurChar==0x5C) {               // is escape mechanismn
            CurPos++;
            if (CurPos>=EndPos) break;
          }
         if (DestMaxSize>0) {
            *CurDestPtr++ = CurChar; DestMaxSize--;
          }
         CurPos++;
       }
      return 0;                             // End-Of-Line reached during String
    } else {
      while (CurPos<EndPos) {
         CurChar = *CurPos;
         if ((CurChar==' ') || (CurChar=='=') || (CurChar==0x0D) || (CurChar==0))
            break;                          // End-Of-String
         if (DestMaxSize>0) {
            *CurDestPtr++ = CurChar; DestMaxSize--;
          }
         CurPos++;
       }
      *CurDestPtr = 0;
      return CurPos;
    }
 }

// Extracts a value (e.g. '0', '0L', '0x0' and '0x0L') including error checking
//  Needs to be called using CurPos == First digit of value
PCHAR STRING_GetNumericValue (PULONG DestPtr, PCHAR StartPos, PCHAR EndPos) {
   PCHAR  CurPos      = StartPos;
   CHAR   CurChar     = 0;
   ULONG  DigitChars  = 0;

   // First we check, if hexadecimal escape is found '0x'
   if (((StartPos+2)<EndPos) && (*(PUSHORT)CurPos==0x7830)) {
      CurPos += 2;
      while (CurPos<EndPos) {
         CurChar = toupper(*CurPos);
         if (CurChar=='L') {
            CurPos++; break;             // 'L' is value terminator and is skipped
          }
         if ((CurChar==0) || (CurChar=='=') || (CurChar==0x0D) || (CurChar=='"') || (CurChar==';') || (CurChar==',') || (CurChar==' ') || (CurChar==']') || (CurChar==')'))
            break;                       // "normal" ending characters...
         if (!(((CurChar>=0x30) && (CurChar<=0x39)) || ((CurChar>=0x41) && (CurChar<=0x46))))
            return NULL;                 // Invalid value
         CurPos++; DigitChars++;
       }
    } else {
      while (CurPos<EndPos) {
         CurChar = toupper(*CurPos);
         if (CurChar=='L') {
            CurPos++; break;             // 'L' is value terminator and is skipped
          }
         if ((CurChar==0) || (CurChar=='=') || (CurChar==0x0D) || (CurChar=='"') || (CurChar==';') || (CurChar==',') || (CurChar==' ') || (CurChar==']') || (CurChar==')'))
            break;                       // "normal" ending characters...
         if (!((CurChar>=0x30) && (CurChar<=0x39)))
            return NULL;                 // Invalid value
         CurPos++; DigitChars++;
       }
    }
   if (DigitChars) {                     // We found any valid chars?
      *DestPtr   = strtoul(StartPos, NULL, 0);
      return CurPos;
    }
   return NULL;
 }

// Isolates one word of a string. Replies with next StartPos, if successful
//  otherwise NULL. Will set WordBeginPos and WordLength accordingly.
//  Example: 'A brown fox jumped over an assembler'
//            IsolateWord -> 'A'
//            IsolateWord -> 'brown'
//            etc.
PCHAR STRING_IsolateWord (PCHAR *WordBeginPosPtr, PULONG WordLengthPtr, PCHAR StartPos, PCHAR EndPos) {
   while ((StartPos<EndPos) && ((*StartPos==' ') || (*StartPos=='-') || (*StartPos=='.') || (*StartPos==',')))
      StartPos++;
   if (StartPos<EndPos) {
      *WordBeginPosPtr = StartPos;
      *WordLengthPtr   = 0;
      while ((StartPos<EndPos) && (*StartPos!=' ') && (*StartPos!='-') && (*StartPos!='.') && (*StartPos!=',')) {
         StartPos++; (*WordLengthPtr)++; }
      return StartPos;
    }
   return NULL;
 }

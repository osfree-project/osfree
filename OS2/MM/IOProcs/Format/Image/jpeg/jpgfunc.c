/*
 * Copyright (c) Chris Wohlgemuth 2002
 * All rights reserved.
 *
 * http://www.geocities.com/SiliconValley/Sector/5785/
 * http://www.os2world.com/cdwriting
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The authors name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/************************************************************************/
/* Put all #defines here                                                */
/************************************************************************/

#define INCL_32                          * force 32 bit compile
#define INCL_GPIBITMAPS
#define INCL_DOSFILEMGR
#define INCL_DOSRESOURCES
#define INCL_DOSMODULEMGR
#define INCL_WIN
#define INCL_GPI
#define INCL_PM                         /* force 32 bit compile */

/************************************************************************/
/* Put all #includes here                                               */
/************************************************************************/

#include <limits.h>
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <os2medef.h>
#include <mmioos2.h>
#include "jpgproc.h"



/*************************************************************************************
 * Routine is called on each DLL instantiation and termination.
 * Caller assures that no two instances execute this code
 * at the same time.
 * 00) Change name to _DLL_InitTerm and use the linkage pragma.
 *************************************************************************************/
/*************************************************************************************
 * Variable holds DLL module handle.  Gets set at initialization.
 * Data area is separate for each using process, so this value
 * could be different (or same) per process.
 *************************************************************************************/
HMODULE hModuleHandle;

//#pragma linkage (_DLL_InitTerm, system)
//#pragma checkout (suspend)       /* Prevent unreferenced parameter messages */

ULONG APIENTRY LibMain(ULONG hModHandle, ULONG fTerm)
{
   hModuleHandle = hModHandle;  /* Remember for NLS lookup */

#ifdef DEBUG
      writeLog("Handle: %d\n", hModuleHandle);
#endif

   return (1L);                 /* Success */
} /* DLL_InitTerm */
//#pragma checkout (resume)


/**************************************************************************
 *   GetFormatString                                                     **
 **************************************************************************
 *
 * ARGUMENTS:
 *
 *     lSearchId         - Table search key
 *     pszFormatString   - Address where to return string.
 *                         If null, then string is not returned.
 *     lBytes            - Number of bytes to copy.
 *
 * RETURN:
 *
 *     Returns 0 if string not found, or the number of characters (bytes)
 *     copied to the callers buffer.
 *     Note, returned string is not ASCII nul terminated
 *
 * DESCRIPTION:
 *
 *     This function will retrieve the format string for the specified
 *     IOProc from the resource file that contains the strings.
 *
 ***************************************************************************/
LONG GetFormatString (LONG lSearchId,
                      PSZ  pszFormatString,   /* Null, or dest address     */
                      LONG lBytes)            /* Caller provided maximum   */
{
   PVOID   pResourceData;
   CHAR    *pTemp;
   LONG    lStringLen;      /* Length of format string  */
   LONG    lRetVal = 0;     /* Function return value    */
   LONG    lSearchTemp;

   if (DosGetResource(hModuleHandle,
                      RT_RCDATA,
                      MMOTION_IOPROC_NAME_TABLE,
                      &pResourceData))
      {
      return (MMIO_ERROR);
      }

   /*
    * The resource table is of the form : FOURCC String\0
    * Loop until a match is found, then return the string.
    * Copy up to the number of bytes specified.
    */

   lStringLen = 0;
   pTemp = (CHAR *)pResourceData;

   while (pTemp)
      {
      memmove(&lSearchTemp, pTemp, sizeof(LONG));

      if (lSearchTemp == 0L)
         {
         break;  /* End of table, search item not found */
         }

      if (lSearchTemp == lSearchId)   /* Search item found?               */
         {
         pTemp += sizeof(LONG);       /* Bypass numeric search id         */
         lStringLen = strlen(pTemp);  /* Determine length of found string */
         if (lStringLen >= lBytes)    /* Truncate, if necessary           */
            {
            if (lBytes > 0)
               lStringLen = lBytes;   /* Number of characters to return   */
            else
               {
               /* Callers buffer has zero size.  Cannot return any data   */
               lRetVal = 0;           /* Size of returned data            */
               break;                 /* Done searching                   */
               }
            }
         if (pszFormatString != NULL)
            {
            memcpy(pszFormatString, pTemp, lStringLen); /* Copy string to caller */
            }
         lRetVal = lStringLen;        /* Function return value            */
         break;                       /* We're done searching             */
         }

      pTemp += sizeof(FOURCC);
      pTemp += (strlen(pTemp) + 1);   /* Advance to next search key       */
      }

   DosFreeResource( pResourceData );

   return (lRetVal);  /* Zero or strlen */
}


/**************************************************************************
 *   GetFormatStringLength                                               **
 **************************************************************************
 *
 * ARGUMENTS:
 *
 *     lSearchId         - Table search key
 *     plNameLength      - Address where to return string length.
 *
 * RETURN:
 *
 *     Length of the format string not including the terminating '\0'.
 *     That is, the same value as would be returned from strlen().
 *
 * DESCRIPTION:
 *
 *     This function will retrieve the length of the format string
 *     for the specified IOProc from the resource file that contains
 *     the strings.
 *
 ***************************************************************************/
LONG GetFormatStringLength (LONG  lSearchId,
                            PLONG plNameLength)
{
   LONG  lStringSize;
   LONG  lRetVal;

   lStringSize = GetFormatString (lSearchId, NULL, LONG_MAX);
   if (lStringSize > 0)             /* String found?                      */
      {
      *plNameLength = lStringSize;  /* yes, return strlen                 */
      lRetVal = 0;                  /* and indicate success to caller     */
      }
   else
      {
      *plNameLength = 0;            /* no, error.  Return zero for length */
      lRetVal = lStringSize;        /* and error code from called routine */
      }
   return (lRetVal);
}


/**************************************************************************
 *   GetNLSData                                                          **
 **************************************************************************
 *
 * ARGUMENTS:
 *
 *     pulCodePage       - Address where to return the code page/country.
 *     pulLanguage       - Address where to return the language/dialect.
 *
 * RETURN:
 *
 *     Error code or 0.
 *
 * DESCRIPTION:
 *
 *     This function will retrieve the NLS information for the IOProc
 *     strings contained in the resource file.
 *
 ***************************************************************************/

ULONG APIENTRY GetNLSData( PULONG pulCodePage,
                           PULONG pulLanguage )
{
   PVOID   pResourceData;
   CHAR    *pTemp;

   if (DosGetResource( hModuleHandle,
                       RT_RCDATA,
                       MMOTION_NLS_CHARSET_INFO,
                       &pResourceData ))
      {
      return (MMIO_ERROR);
      }

   /*
    * The resource table is of the form :
    *   usCodePage     Low
    *   usCountryCode  High
    *   usLanguage     Low
    *   usDialect      High
    */

   pTemp = (CHAR *)pResourceData;

   while (pTemp)
      {
      memmove( pulCodePage, pTemp, sizeof(ULONG) );
      pTemp += sizeof(ULONG);

      if (pTemp == NULL)
         {
         break;
         }

      memmove( pulLanguage, pTemp, sizeof(ULONG) );

      break;
      }

   DosFreeResource( pResourceData );

   return (MMIO_SUCCESS);
}

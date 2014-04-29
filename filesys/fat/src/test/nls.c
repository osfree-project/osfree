# all needed includes.

#include <uconv.h>


BOOL LoadTranslateTable(VOID)
{
APIRET rc;
ULONG ulParmSize;
BYTE   rgData[256];
PBYTE  pIn;
USHORT rgTranslate[256];
PUSHORT pOut;
UconvObject  uconv_object = NULL;
INT iIndex;
size_t       in_bytes_left;
size_t       uni_chars_left;
size_t       num_subs;
ULONG rgCP[3];
ULONG cbCP;

   rc = DosLoadModule(rgData, sizeof rgData, "UCONV.DLL", &hModLang);
   if (rc)
      {
      printf("No NLS support found (%s does not load).\n", rgData);
      printf("No UNICODE translate table loaded!\n");
      return TRUE;
      }
   rc = DosQueryProcAddr(hModLang, 0L,
      "UniCreateUconvObject", (PFN *)&pUniCreateUconvObject);
   if (rc)
      {
      printf("ERROR: Could not find address of UniCreateUconvObject.\n");
      return FALSE;
      }
   rc = DosQueryProcAddr(hModLang, 0L,
      "UniUconvToUcs", (PFN *)&pUniUconvToUcs);
   if (rc)
      {
      printf("ERROR: Could not find address of UniUconvToUcs.\n");
      return FALSE;
      }

   rc = DosQueryCp(sizeof rgCP, rgCP, &cbCP);
   if (f32Parms.ulCurCP == rgCP[0])
      return FALSE;

   if (f32Parms.ulCurCP)
      {
      BYTE chChar;
      printf("Loaded unicode translate table is for CP %lu\n", f32Parms.ulCurCP);
      printf("Current CP is %lu\n", rgCP[0]);
      printf("Would you like to reload the translate table for this CP [Y/N]? ");
      fflush(stdout);

      for (;;)
         {
         chChar = getch();
         switch (chChar)
            {
            case 'y':
            case 'Y':
               chChar = 'Y';
               break;
            case 'n':
            case 'N':
               chChar = 'N';
               break;
            default :
               DosBeep(660, 10);
               continue;
            }
         printf("%c\n", chChar);
         break;
         }
      if (chChar == 'N')
         return FALSE;
      }

   for (iIndex = 0; iIndex < 256; iIndex++)
      rgData[iIndex] = iIndex;

   rc = pUniCreateUconvObject((UniChar *)L"", &uconv_object);
   if (rc != ULS_SUCCESS)
      {
      printf("UniCreateUconvObject error: return code = %u\n", rc);
      return FALSE;
      }

   pIn  = rgData;
   in_bytes_left = sizeof rgData;
   pOut = rgTranslate;
   uni_chars_left = sizeof rgTranslate / sizeof (USHORT);

   rc = pUniUconvToUcs(uconv_object,
      (PVOID *)&pIn,
      &in_bytes_left,
      &pOut,
      &uni_chars_left,
      &num_subs);

   if (rc != ULS_SUCCESS)
      {
      printf("UniUconvToUcs failed, rc = %u\n", rc);
      return FALSE;
      }


   ulParmSize = sizeof rgTranslate;
   rc = DosFSCtl(NULL, 0, NULL,
               rgTranslate, ulParmSize, &ulParmSize,
               FAT32_SETTRANSTABLE, "FAT32", -1, FSCTL_FSDNAME);
   if (rc)
      {
      printf("Unable to set translate table for current Codepage.\n");
      return FALSE;
      }

   f32Parms.ulCurCP = rgCP[0];
   printf("Unicode translate table for CP %lu loaded.\n", rgCP[0]);
   DosFreeModule(hModLang);
   return TRUE;
}


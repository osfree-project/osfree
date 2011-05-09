/*  Getting variables, like environment,
 *  InfoBlocks etc.
 */

#define  INCL_DOS
#include <os2.h>

APIRET __cdecl   KalScanEnv(PCSZ  pszName,
                            PCSZ  *ppszValue);

APIRET __cdecl   KalGetInfoBlocks(PTIB *pptib,
                                  PPIB *pppib);

APIRET APIENTRY  DosScanEnv(PCSZ  pszName,
                            PCSZ  *ppszValue)
{
  return KalScanEnv(pszName, ppszValue);
}

APIRET APIENTRY DosGetInfoBlocks(PTIB *pptib,
                                 PPIB *pppib)
{
  return KalGetInfoBlocks(pptib, pppib);
}

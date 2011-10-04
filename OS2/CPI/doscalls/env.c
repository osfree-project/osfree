/*  Getting variables, like environment,
 *  InfoBlocks etc.
 */

#define  INCL_DOS
#include <os2.h>

#include <string.h>

//APIRET __cdecl   KalScanEnv(PCSZ  pszName,
//                            PCSZ  *ppszValue);

APIRET __cdecl   KalGetInfoBlocks(PTIB *pptib,
                                  PPIB *pppib);

APIRET APIENTRY DosGetInfoBlocks(PTIB *pptib,
                                 PPIB *pppib)
{
  return KalGetInfoBlocks(pptib, pppib);
}

APIRET APIENTRY  DosScanEnv(PCSZ  pszName,
                            PCSZ  *ppszValue)
{
  char varname[0x100];
  PPIB pib;
  PTIB tib;
  int  i;
  char *p, *q, *env;

  /* Get application info blocks */
  DosGetInfoBlocks(&tib, &pib);

  /* get the environment */
  env = pib->pib_pchenv;

  /* search for needed env variable */
  for (p = env; *p; p += strlen(p) + 1)
  {
    // move until '=' sign is encountered
    for (i = 0, q = p; *q && *q != '=' && i < 256; q++, i++) ;

    /* copy to name buffer  */
    strncpy(varname, p, i);
    /* add ending zero byte */
    varname[i] = '\0';

    if (!strcasecmp(varname, pszName))
    {
      /* variable found */
      strcpy(*ppszValue, q + 1);
      return NO_ERROR;
    }
  }

  return ERROR_ENVVAR_NOT_FOUND;
}

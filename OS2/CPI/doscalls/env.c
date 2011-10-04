/*  Getting variables, like environment,
 *  InfoBlocks etc.
 */

#define  INCL_DOSERRORS
#define  INCL_DOS
#include <os2.h>

#include <string.h>


APIRET __cdecl   KalGetInfoBlocks(PTIB *pptib,
                                  PPIB *pppib);

APIRET APIENTRY DosGetInfoBlocks(PTIB *pptib,
                                 PPIB *pppib)
{
  return KalGetInfoBlocks(pptib, pppib);
}

/*!
  @brief         Gets an environment variable by name

  @param         pszName       variable name
  @param         ppszValue     pointer to returned pointer variable (to an environment variable)

  @return
    NO_ERROR                   if env. var found successfully
    ERROR_ENVVAR_NOT_FOUND     env. var. not found

  API
    DosGetInfoBlocks
*/

APIRET APIENTRY  DosScanEnv(PCSZ  pszName,
                            PSZ  *ppszValue)
{
  char varname[CCHMAXPATH];
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
    for (i = 0, q = p; *q && *q != '=' && i < CCHMAXPATH - 1; q++, i++) ;

    /* copy to name buffer  */
    strncpy(varname, p, i);
    /* add ending zero byte */
    varname[i] = '\0';

    if (!strcasecmp(varname, pszName))
    {
      /* variable found */
      *ppszValue = q + 1;

      return NO_ERROR;
    }
  }

  return ERROR_ENVVAR_NOT_FOUND;
}

/*  Getting variables, like environment,
 *  InfoBlocks etc.
 */

#include <string.h>

#include "kal.h"

APIRET APIENTRY DosGetInfoBlocks(PTIB *pptib,
                                 PPIB *pppib)
{
  APIRET rc;
  log("%s enter\n", __FUNCTION__);
  rc = KalGetInfoBlocks(pptib, pppib);
  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
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
  APIRET rc;

  log("%s enter\n", __FUNCTION__);
  log("pszName=%s\n", pszName);

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

      log("pszValue=%s\n", *ppszValue);
      rc = NO_ERROR;
      break;
    }
  }

  if (rc)
  {
    log("not found\n");
    rc = ERROR_ENVVAR_NOT_FOUND;
  }

  log("%s exit => %lx\n", __FUNCTION__, rc);
  return rc;
}

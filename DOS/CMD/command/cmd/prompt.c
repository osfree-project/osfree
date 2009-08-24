/* $Id: prompt.c 771 2004-02-01 13:55:39Z skaus $
 *  PROMPT.C - prompt handling.
 *
 */

#include "../config.h"


#include "../include/cmdline.h"
#include "../include/command.h"
#include "../include/context.h"
#include "../include/misc.h"

int cmd_prompt(char *param)
{
  if(param && *param == '=')    /* skip '=' & spaces */
    param = ltrimcl(param + 1);

  /* NO rtrim()! */
  return chgEnvRemove(PROMPTVAR, param);
}

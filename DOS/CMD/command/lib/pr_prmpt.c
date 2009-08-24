/* $Id: pr_prmpt.c 528 2003-03-11 21:02:03Z skaus $
 * print the command-line prompt
 *
 */

#include "../config.h"


#include "../include/command.h"
#include "../include/misc.h"

#include "tcc2wat.h"

void printprompt(void)
{       char *pr;

        dbg_printmem();

        pr = getEnv(PROMPTVAR);        /* get PROMPT environment var. */

        displayPrompt(pr? pr: DEFAULT_PROMPT);
}

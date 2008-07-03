/* $Id: pr_prmpt.c,v 1.1 2001/04/12 00:33:53 skaus Exp $
 * print the command-line prompt
 *
 */

#include "../config.h"


#include "../include/command.h"
#include "../include/misc.h"

void printprompt(void)
{	char *pr;

	dbg_printmem();

	pr = getEnv(PROMPTVAR);        /* get PROMPT environment var. */

	displayPrompt(pr? pr: DEFAULT_PROMPT);
}


/* $Id: cdd.c 771 2004-02-01 13:55:39Z skaus $
 * CDD - changes drive and directory
 */

#include "../config.h"

#include "../include/command.h"

int cmd_cdd(char *param)
{
	return cd_dir(param, 1, "CDD");
}

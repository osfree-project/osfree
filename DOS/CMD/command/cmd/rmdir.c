/* $Id: rmdir.c 1247 2006-07-02 16:46:29Z blairdude $
 * RD / RMDIR - makes a call to directory_handler to do its work
 */

#include "../config.h"

//#include <dir.h>

#include "../include/command.h"
#include "../include/misc.h"

int cmd_rmdir(char *param)
{
#ifdef FEATURE_LONG_FILENAMES
    return mk_rd_dir( param, 0x3A00, "RMDIR" );
#else
        return mk_rd_dir(param, rmdir, "RMDIR");
#endif
}

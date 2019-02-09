/*  OS/2 fs server API: misc functions
 *  osFree OS/2 personality
 *
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/MountReg.h>
#include <os3/globals.h>
#include <os3/io.h>

/* libc includes */
#include <ctype.h>

/* local includes */
#include "api.h"

static ULONG drivemap = -1;

void setdrivemap(ULONG *map);

void setdrivemap(ULONG *map)
{
    struct I_Fs_srv *fsrv;
    int  i, diskno;
    char *drive;
    char drv;

    *map = 0;

    for (i = 0; i < fsrouter.srv_num_; i++)
    {
        fsrv = fsrouter.fs_srv_arr_[i];
        drive = fsrv->drive;
        drv = tolower(*(drive));
        diskno = drv - 'a';
        *map |= (1 << diskno);
    }

    io_log("map=%lu", *map);
}
 
APIRET FSGetDriveMap(ULONG *map)
{
    if (drivemap == -1)
    {
        setdrivemap(&drivemap);
    }

    *map = drivemap;
    return 0;
}

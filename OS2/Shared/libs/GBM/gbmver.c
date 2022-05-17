/*

gbmver.c - Shows the library version info of GBM (Generalized Bitmap Module).
                   Simple Generic Version without checking exporting function prototypes.

Author: Heiko Nitzsche

History
-------
26-Aug-2008: Initial version
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gbm.h"

/********************************************/

/** Show version info and found GBM.DLL */
static gbm_boolean show_info(void)
{
    int version = 0;

    /* Standard handling for non-OS/2 operating systems */
    gbm_init();

    /* get library version */
    version = gbm_version();

    gbm_deinit();

    printf("GBM version: %d.%d\n", version/100, version-100);
    return GBM_TRUE;
}

/********************************************/
/********************************************/

int main(int argc, char * argv[])
{
  int retCode = 0;

  if (! show_info())
  {
    retCode = 1;
  }

  return retCode;
}



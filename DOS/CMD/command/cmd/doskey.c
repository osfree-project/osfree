/* $Id: doskey.c 1291 2006-09-05 01:44:33Z blairdude $
 *  DOSKEY -- command line editor extender <-> incorporated, but
 *		not loadable
 */

#include "../config.h"

#include "../include/command.h"
#include "../strings.h"

int cmd_doskey (char * param) {
    (void)param;
    displayString (TEXT_MSG_DOSKEY);
    return 0;
}

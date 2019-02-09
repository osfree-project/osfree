/* os2exec server API, Genode-specific */

#include <os3/modmgr.h>

/* Share dataspaces of executable with given hmod
   to a given client */
extern "C"
long excShare(struct mod_list_t **mods, unsigned long hmod, void *client_id)
{
    /* Does nothing on Genode */
    return 0;
}

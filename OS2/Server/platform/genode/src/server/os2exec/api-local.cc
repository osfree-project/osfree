/* os2exec server API, Genode-specific */

/* Share dataspaces of executable with given hmod
   to a given client */
extern "C"
long excShare(unsigned long hmod, void *client_id)
{
    /* Does nothing on Genode */
    return 0;
}

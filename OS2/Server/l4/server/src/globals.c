
#include "globals.h"
#include <stdlib.h>
#include <stdio.h>
/* Global variables. */

struct FSRouter fsrouter;

void init_globals(void) {
    FSRouter_init(&fsrouter);
    /*FSRouter_test(&fsrouter);*/
    
    /* Assign file provider server to device letter. */
    struct I_Fs_srv  *fs_srv = malloc(sizeof(struct I_Fs_srv));
    I_Fs_srv_init2(fs_srv, "BMODFS", "c:");
    //printf("Info i server_drv(glob): %s, %s\n", fs_srv->server, fs_srv->drive );
    
    FSRouter_add(&fsrouter, fs_srv);
}

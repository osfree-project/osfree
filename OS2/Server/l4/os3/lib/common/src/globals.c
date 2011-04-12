
#include <l4/os3/globals.h>
#include <stdlib.h>
#include <stdio.h>
/* Global variables. */

struct FSRouter fsrouter;

void init_globals(void) {
    FSRouter_init(&fsrouter);
    
    /* Assign file provider server to device letter. */
    struct I_Fs_srv  *fs_srv = malloc(sizeof(struct I_Fs_srv));
    I_Fs_srv_init2(fs_srv, "/file/system", "c:");
    
    FSRouter_add(&fsrouter, fs_srv);
}

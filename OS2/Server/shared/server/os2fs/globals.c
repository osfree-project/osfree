/* osFree internal */
#include <os3/globals.h>

/* libc includes */
#include <stdlib.h>
#include <stdio.h>
/* Global variables. */

struct FSRouter fsrouter;

void init_globals(void) {
    struct I_Fs_srv  *fs_srv;

    FSRouter_init(&fsrouter);

    /* Assign file provider server to device letter. */
    fs_srv = malloc(sizeof(struct I_Fs_srv));
    I_Fs_srv_init2(fs_srv, "/file/system", "c:");
    FSRouter_add(&fsrouter, fs_srv);

    fs_srv = malloc(sizeof(struct I_Fs_srv));
    I_Fs_srv_init2(fs_srv, "/", "d:");
    FSRouter_add(&fsrouter, fs_srv);

    fs_srv = malloc(sizeof(struct I_Fs_srv));
    I_Fs_srv_init2(fs_srv, "/file", "f:");
    FSRouter_add(&fsrouter, fs_srv);
}

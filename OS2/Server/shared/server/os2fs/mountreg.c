/* osFree internal; */
#include <os3/MountReg.h>
#include <os3/globals.h>

/* libc includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Initierar en I_Fs_srv till noll. */
void I_Fs_srv_init(struct I_Fs_srv *s) {
    s->mountpoint='\0'; 
    s->drive='\0';
}

/* Initierar en I_Fs_srv till server och enhetsbokstav. */
void I_Fs_srv_init2(struct I_Fs_srv *s, char *strsrv, char *drv) {
    s->mountpoint=strsrv; 
    s->drive=drv;
}

/* Kopplar en enhetsbokstav till en server och returnerar den.*/
struct I_Fs_srv* FSRouter_route(struct FSRouter *s, char drv) {
    /* I_Fs_srv_t *fs_srv_arr_;   */
    int i;
    for(i=0; i< s->srv_num_; i++) {
        I_Fs_srv_t * server_drv = s->fs_srv_arr_[i];
        if(server_drv) {
            if(tolower(server_drv->drive[0]) == tolower(drv)) {
                return server_drv;
            }
        }
    }
    return 0;
}

/* Get a drive letter by path prefix (if any) */
struct I_Fs_srv* FSRouter_route_back(struct FSRouter *s, char *prefix) {
    int i;
    for (i = 0; i < s->srv_num_; i++) {
        I_Fs_srv_t *server_drv = s->fs_srv_arr_[i];
        if (server_drv) {
            if (prefix && ! strcasecmp(server_drv->mountpoint, prefix))
            {
                return server_drv;
            }
        }
    }

    return NULL;
}

/* Lägger till en serverkoppling, I_Fs_srv, till listan fs_srv_arr_. */
void FSRouter_add(struct FSRouter *s, struct I_Fs_srv *fs_srv_) {
   s->fs_srv_arr_[s->srv_num_] = fs_srv_;
   int *t_ptr =&(s->srv_num_);
   (*t_ptr)++;
}

void FSRouter_test(struct FSRouter *s) { printf("test\n"); }


void FSRouter_init(struct FSRouter *s) {
    s->fs_srv_arr_ = (I_Fs_srv_t**) malloc(sizeof(I_Fs_srv_t)*25);
    s->max_drv = 24;
    s->srv_num_ = 0;
}


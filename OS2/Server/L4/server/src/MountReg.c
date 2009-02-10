
#include "MountReg.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Initierar en I_Fs_srv till noll. */
void I_Fs_srv_init(struct I_Fs_srv *s) {
    s->server='\0'; 
    s->drive='\0';
}

/* Initierar en I_Fs_srv till server och enhetsbokstav. */
void I_Fs_srv_init2(struct I_Fs_srv *s, char *strsrv, char *drv) {
    s->server=strsrv; 
    s->drive=drv;
}

/* Kopplar en enhetsbokstav till en server och returnerar den.*/
struct I_Fs_srv* FSRouter_route(struct FSRouter *s, char drv) {
    /* I_Fs_srv_t *fs_srv_arr_;   */
    int i;
    for(i=0; i< s->srv_num_; i++) {
        I_Fs_srv_t * server_drv = s->fs_srv_arr_[i];
        if(server_drv) {
            if(server_drv->drive[0] == drv) {
                return server_drv;
            }
        }
    }
    return 0;
}

/* Hämta en enhetsbokstav från en sökväg. */
char get_drv(const char *s_path) {
    if(s_path[1] == ':')
        return s_path[0];
    return '\0';
}

/* ?  Kovertera en sökväg 'c:\config.sys' till '/config.sys' ??? */
void os2_fname_to_fileprov_fname(const char *fname,char *filename) {
    int l = strlen(fname);  /*strnlen(fname, 254); #ifdef _GNU_SOURCE */
    strcpy(filename, (const char*)&fname[2]);
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

/**
 * Return a new dataspace with the requested file in it
 *
 * \param obj           our client
 * \param name		requested file name
 * \param dm		dataspace manager for allocating the dataspace
 * \param flags		flags used for creating the dataspace
 * \retval ds		dataspace including the file image
 * \retial size		file size
 * \retval _ev		dice-generated environment
 *
 * \return		- 0 on success
 *			- -L4_ENOMEM    memory allocation failed
 *			- -L4_ENOTFOUND file not found
 *			- -L4_EIO       error reading file
 *			- -L4_EINVAL    error passing the dataspace to client
 */
/*long
l4fprov_file_open_component (CORBA_Object _dice_corba_obj, "BMODFS" 
                             const char* fname,
                             const l4_threadid_t *dm_id,
                             unsigned long flags,
                             l4dm_dataspace_t *ds,
                             l4_size_t *size,
                             CORBA_Server_Environment *_dice_corba_env)
*/                             

/*int I_Fs_srv::open_f_prov(char *s_file, void *corba_obj, int *dm_id, 
                          unsigned long flags, ); */


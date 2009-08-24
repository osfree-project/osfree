#include <l4/names/libnames.h>
#include <l4/env/errno.h>
#include <l4/sys/consts.h>
#include <l4/os2/os2server.h>

static l4_threadid_t server_id = L4_INVALID_ID;
static CORBA_Object server = &server_id;
            
//! Request netserver id at nameserver
static int check_server(void){
    if (l4_is_invalid_id(server_id)){
        if (!names_waitfor_name(os2server_name,&server_id,10000)) return 1;
    }
    return 0;
}

//! print the string
void os2server_VioWrtTTY(char *buf, int count, int handle){
    CORBA_Environment env = dice_default_environment;

    if (check_server()) ;//return -L4_EINVAL;
    os2server_VioWrtTTY_call(server, buf, count, handle, &env);
    //return -env._p.ipc_error;
}

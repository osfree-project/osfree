/**
 * \file   l4vfs/term_server/server/common/basic_io.c
 * 
 * \brief  interface functions from basic_io, common_io etc.
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/l4vfs/basic_io.h>
#include <l4/l4vfs/basic_name_server.h>
#include <l4/l4vfs/name_space_provider.h>
#include <l4/l4vfs/name_server.h>

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <termios.h>

#include <l4/l4vfs/types.h>
#include <l4/l4vfs/object_server-server.h>
#include <l4/l4vfs/common_io-server.h>
#include <l4/l4vfs/common_io_notify-server.h>
#include <l4/term_server/internal-client.h>
#include <l4/term_server/main-server.h>

#include <l4/sys/types.h>
#include <l4/sys/syscalls.h>
#include <l4/log/l4log.h>
#include <l4/util/l4_macros.h>

#include <basic_io.h>
#include <dirs.h>
#include <term.h>

#define MIN(a, b) ((a)<(b)?(a):(b))

extern int _DEBUG;
volume_id_t myvolume;

object_handle_t
l4vfs_basic_io_open_component(CORBA_Object _dice_corba_obj,
                              const object_id_t *object_id,
                              l4_int32_t flags,
                              CORBA_Server_Environment *_dice_corba_env)
{
    int ret;

    // am I responsible for this volume?
    LOGd(_DEBUG, "volume id: %d", object_id->volume_id);
    if (object_id->volume_id != myvolume)
        return -ENOENT;
    LOGd(_DEBUG, "entry belongs to vc_server");

    // prevent someone from opening root for writing
    if ((object_id->object_id == 0) &&
        ((flags & O_WRONLY) || (flags & O_RDWR)))
        return -EACCES;

    // find free client state and fill in values
    ret = clientstate_open(flags, *_dice_corba_obj, object_id->object_id);
    if (ret < 0)
    {
        LOG_Error("error on clientstate_open()");
        return ret;
    }

    // open a terminal if the client does not open root
    if (object_id->object_id > 0)
    {
        int term = object_id->object_id - 1;
        LOGd(_DEBUG,"opening window");
        if (terms[term].refcount > 0)
        {
            // FIXME: access control - only the terms owner should
            //        be able to re-open it.
            terms[term].refcount++;
            LOGd(_DEBUG, "refcount: %d", terms[term].refcount );
        }
        else
        {
            int i = term_open(*_dice_corba_obj, object_id);
            if (i)
            {
                LOG("error on term open");
                // XXX: hmm, we should clean up the mess and return a
                //      nice error code here
                exit(1);
            }
        }
    }

    return ret;
}

l4_int32_t
l4vfs_common_io_close_component(CORBA_Object _dice_corba_obj,
                                object_handle_t handle,
                                CORBA_Server_Environment *_dice_corba_env)
{
    // do not care for state of client sem here as we do
    // not want to wait for other threads when closing
    int ret;
    int term = clients[handle].object_id - 1;

    if (--terms[term].refcount == 0)
        // close terminal window
        term_close(clients[handle].object_id);

    // free clientstate
    ret = clientstate_close(handle, *_dice_corba_obj);
    return ret;
}

l4vfs_ssize_t
l4vfs_common_io_read_component(CORBA_Object _dice_corba_obj,
                               object_handle_t h,
                               char **buf,
                               l4vfs_size_t *count,
                               short *_dice_reply,
                               CORBA_Server_Environment *_dice_corba_env)
{
    l4_threadid_t myself, partner;

    CORBA_Environment env = dice_default_environment;
    env.malloc            = (dice_malloc_func)malloc;
    env.free              = (dice_free_func)free;

    l4semaphore_down(&(clients[h].client_sem));
    // no one may read from an invalid object
    if (clients[h].object_id < 0)
    {
        LOG("invalid file descriptor");
        *_dice_reply = DICE_REPLY;
        l4semaphore_up(&(clients[h].client_sem));
        return -EBADF;
    }

    // no one may read from the root object
    if (clients[h].object_id == 0)
    {
        LOG("no read to root allowed");
        *_dice_reply = DICE_REPLY;
        l4semaphore_up(&(clients[h].client_sem));
        return -EISDIR;
    }

    // only read from open objects
    if (! clients[h].open)
    {
        LOG("handle not open");
        *_dice_reply = DICE_REPLY;
        l4semaphore_up(&(clients[h].client_sem));
        return -EINVAL;
    }

    // no reads from write only objects
    if (clients[h].rw_mode & O_WRONLY)
    {
        LOG("handle open only for write");
        *_dice_reply = DICE_REPLY;
        l4semaphore_up(&(clients[h].client_sem));
        return -EINVAL;
    }

    // no one may write to an object he is not owning
    if (! l4_task_equal(clients[h].client, *_dice_corba_obj))
    {
        LOG("invalid read request - not owning object");
        LOGd(_DEBUG,"owner="l4util_idfmt", requester="l4util_idfmt")",
                l4util_idstr(clients[h].client), l4util_idstr(*_dice_corba_obj));
        *_dice_reply = DICE_REPLY;
        l4semaphore_up(&(clients[h].client_sem));
        return -EBADF;
    }

    // should not happen
    if (l4_is_invalid_id(clients[h].worker))
    {
        LOG("no worker thread for read, client");
        *_dice_reply = DICE_REPLY;
        l4semaphore_up(&(clients[h].client_sem));
        return -EINVAL;
    }

    // we do not reply directly
    *_dice_reply = DICE_NO_REPLY;

    LOGd(_DEBUG,"set DICE_NO_REPLY");

    partner = clients[h].worker;
    myself  = l4_myself();
    l4semaphore_up(&(clients[h].client_sem));

    LOGd(_DEBUG,"sending read to "l4util_idfmt"", l4util_idstr(partner));
    l4vfs_term_server_internal_start_read_send(&partner, h, count, &myself,
                                               _dice_corba_obj, &env);

    // return whatever you like - doesn't matter
    return 0;
}

void l4vfs_common_io_notify_read_notify_component(
    CORBA_Object _dice_corba_obj,
    object_handle_t fd,
    l4vfs_ssize_t retval,
    const char *buf,
    l4vfs_size_t *count,
    const l4_threadid_t *source,
    CORBA_Server_Environment *_dice_corba_env)
{
    CORBA_Server_Environment env = dice_default_server_environment;

    l4semaphore_down(&(clients[fd].client_sem));
    // we do not handle notify-calls from other tasks as a matter of security
    if (!l4_task_equal( l4_myself(), *_dice_corba_obj))
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return;
    }

    if (l4_is_invalid_id(clients[fd].client))
    {
        LOG("cannot reply to client anymore - seems to have shut down");
        l4semaphore_up(&(clients[fd].client_sem));
        return;
    }

    l4semaphore_up(&(clients[fd].client_sem));
    l4vfs_common_io_read_reply((l4_threadid_t *)source, retval,
                               (char **)&buf, count, &env);
}


l4vfs_ssize_t
l4vfs_common_io_write_component(CORBA_Object _dice_corba_obj,
                                object_handle_t fd,
                                const char *buf,
                                l4vfs_size_t *count,
                                short *_dice_reply,
                                CORBA_Server_Environment *_dice_corba_env)
{
    l4semaphore_down(&(clients[fd].client_sem));
    // no one may write to an invalid object
    if (clients[fd].object_id < 0)
    {
        LOG("invalid object");
        l4semaphore_up(&(clients[fd].client_sem));
        return -EBADF;
    }

    // no one may write to the root object
    if (clients[fd].object_id == 0)
    {
        LOG("writing to root now allowed");
        l4semaphore_up(&(clients[fd].client_sem));
        return -EINVAL;
    }

    // only write to opened objects
    if (!clients[fd].open)
    {
        LOG("fd not open");
        l4semaphore_up(&(clients[fd].client_sem));
        return -EINVAL;
    }

    // no writes to read only objects
    if (clients[fd].rw_mode & O_RDONLY)
    {
        LOG("fd open only for reading");
        l4semaphore_up(&(clients[fd].client_sem));
        return -EINVAL;
    }

    // no one may write to an object he is not owning
    if (!l4_task_equal( clients[fd].client, *_dice_corba_obj))
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return -EBADF;
    }

    l4semaphore_up(&(clients[fd].client_sem));

    return term_write(clients[fd].object_id, (l4_int8_t*)buf, *count);
}

l4vfs_off_t
l4vfs_basic_io_lseek_component(CORBA_Object _dice_corba_obj,
                               object_handle_t fd,
                               l4vfs_off_t offset,
                               l4_int32_t whence,
                               CORBA_Server_Environment *_dice_corba_env)
{
    return -ESPIPE;
}

l4_int32_t
l4vfs_basic_io_fsync_component(CORBA_Object _dice_corba_obj,
                               object_handle_t fd,
                               CORBA_Server_Environment *_dice_corba_env)
{
    // just do nothing
    return 0;
}

int l4vfs_basic_io_getdents_component(CORBA_Object _dice_corba_obj,
                                      object_handle_t fd,
                                      l4vfs_dirent_t **dirp,
                                      unsigned int *count,
                                      CORBA_Server_Environment *_dice_corba_env)
{
    dir_t mydir;
    l4vfs_dirent_t *actual;
    void *array_ptr;
    char str_buf[10] = "\0";  // used for ".", "..", "vc0" .. "vc3"
    int i, entry_size, array_size;

    mydir.name = str_buf;

    l4semaphore_down(&(clients[fd].client_sem));
    // reading root ?
    if (clients[fd].object_id != 0)
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return -ENOTDIR;
    }

    // am i open?
    if (!clients[fd].open)
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return -EBADF;
    }

    // is it the same thread that opened me?
    if (!l4_task_equal(clients[fd].client, *_dice_corba_obj))
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return -EBADF;
    }

    // if the current seekpos is at the end of dir yet,
    // return 0
    if (clients[fd].seekpos > MAX_TERMS + 1)
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return 0;
    }

    // initialization
    array_size = 0;
    array_ptr  = NULL;

    // now build up the array
    // (MAX_TERMS-1) terminals + "." + ".." = MAX_TERMS+1)
    // start from the current seekpos
    for (i = clients[fd].seekpos ; i < MAX_TERMS + 2; i++)
    {
        switch(i)
        {
            // 1st case - the "." dir
            case 0:
                strcpy(mydir.name, ".");
                mydir.object_id = 0;
                break;
            // 2nd case - the ".." dir --> we`ve got a problem
            // here. The vc_server must not know and must not
            // care about, where it is mounted. This means it
            // must not know the .. directory. Therefore we
            // need to return an invalid object_id here. If the
            // client really wants to know the object_id of ..,
            // it has to resolve this from the name_server
            case 1:
                strcpy(mydir.name, "..");
                mydir.object_id = -1;
                break;
            // the final MAX_TERMS-1 cases: our virtual consoles
            default:
                sprintf(mydir.name, "vc%d", i - 2);
                mydir.object_id = i-1;
                break;
        }
        LOGd(_DEBUG, "name = %s, id = %d", mydir.name, mydir.object_id);

        // calculate the size of the new entry we have to malloc
        entry_size = sizeof(local_object_id_t) // inode-size == object_id-size
                     + sizeof(off_t)           // the offset to next dirent
                     + sizeof(unsigned short)  // will store entry_size there
                     + strlen(mydir.name) + 1; // length of the name + 1 for the
                     // \0 character

        // round up to align to word size
        // (got it frome the name_server...)
        entry_size = entry_size + sizeof(int) - 1;
        entry_size = (entry_size/sizeof(int)) * sizeof(int);

        // check if the new entry would make the array
        // larger than the receive buffer
        if (array_size + entry_size > *count)
        {
            LOGd(_DEBUG, "entry does not fit in array anymore.");
            break;
        }

        // add entry_size bytes to the array
        LOGd(_DEBUG, "new entry needs %d bytes.", entry_size);
        array_ptr        = realloc(array_ptr, (array_size + entry_size));
        LOGd(_DEBUG, "realloced: %p", array_ptr);
        // now go to the point where we fill in the new values
        actual           = (l4vfs_dirent_t *)(array_ptr + array_size);
        LOGd(_DEBUG, "actual dirent: %p", actual);

        LOGd(_DEBUG, "filling in the values");
        actual->d_ino    = mydir.object_id;
        actual->d_off    = -1;
        actual->d_reclen = entry_size;
        strncpy( actual->d_name, mydir.name, L4VFS_MAX_NAME);

        // calculate new array size
        array_size += entry_size;
        clients[fd].seekpos ++;
    }

    l4semaphore_up(&(clients[fd].client_sem));

    // check if the receive buffer was too small
        // for even one entry - return EINVAL then
    if (array_ptr == NULL)
        return -EINVAL;

    // finally set the dirp return value
    // - as array_ptr is located on the local stack, we have
    // to memcpy instead of simply setting dirp = array_ptr;
    memcpy(*dirp, array_ptr, array_size);

    // if everything worked, we return the size of the
    // new array
    return array_size;
}

object_id_t l4vfs_basic_name_server_resolve_component(
    CORBA_Object _dice_corba_obj,
    const object_id_t *base,
    const char* pathname,
    CORBA_Server_Environment *_dice_corba_env)
{
    object_id_t ret;
    int term_number;

    LOGd(_DEBUG, "check root (%d.%d) '%s'",
         base->volume_id, base->object_id, pathname);

    // if anyone wants to check something below root
    // (base->object_id > 0), we return invalid values as 
    // vc_server does not support any directories
    if (base->object_id > 0)
    {
        LOGd(_DEBUG, "illegal resolve request.");
        ret.volume_id = L4VFS_ILLEGAL_VOLUME_ID;
        ret.object_id = L4VFS_ILLEGAL_OBJECT_ID;
        return ret;
    }

    // root may be returned if the request is / or if it is
    // empty because then it is part of a //// etc. pattern
    if ((pathname[0]=='/') || (strlen(pathname)==0))
    {
        LOGd(_DEBUG, "found root");
        ret.volume_id = myvolume;
            ret.object_id = 0;
            return ret;
    }

    // should never get here
    if (!strcmp( pathname, ".." ))
    {
        LOGd(_DEBUG,"found ..");
        ret.volume_id = L4VFS_ILLEGAL_VOLUME_ID;
        ret.object_id = L4VFS_ILLEGAL_OBJECT_ID;
        return ret;
    }

    // check .
    LOGd(_DEBUG, "check .");
    if (pathname[0] ==  '.')
        {
        LOGd(_DEBUG, "found .");
        ret.volume_id = myvolume;
        ret.object_id = 0;
        return ret;
    }

    LOGd(_DEBUG, "check normal file: %s", pathname );

    // now check the files
    if (pathname != NULL)
    {
        // filenames always start with "vc" followed
        // by a number, which we translate into a
        // terminal index by adding 1.
        if ((pathname[0] == 'v') && pathname[1] == 'c')
            {
        term_number = atoi( &pathname[2] );
        // is it a valid terminal?
        if (term_number < 0 || term_number > MAX_TERMS)
        {
            LOGd(_DEBUG, "term number not supported");
            ret.object_id = L4VFS_ILLEGAL_OBJECT_ID;
            ret.volume_id = L4VFS_ILLEGAL_VOLUME_ID;
        }
        else
        {
            LOGd(_DEBUG, "found normal file");
            ret.object_id = term_number + 1;
            ret.volume_id = myvolume;
        }
                return ret;
           }
    }

    // nothing found
    ret.volume_id = L4VFS_ILLEGAL_VOLUME_ID;
    ret.object_id = L4VFS_ILLEGAL_OBJECT_ID;
    return ret;
}

// Martin: this function will most probably never be called!
// I think it is superflous, or am I wrong?
//
// Bjoern: It should not be a mistake to provide this function. At least
//     rev_resolving works - maybe someone wants to use it later.
char* l4vfs_basic_name_server_rev_resolve_component(
    CORBA_Object _dice_corba_obj,
    const object_id_t *dest,
    object_id_t *parent,
    CORBA_Server_Environment *_dice_corba_env)
{
    char *ret = NULL, *no = NULL;

    // if it is not my volume id, i cannot resolve it
    if (dest->volume_id != myvolume)
    {
        LOGd(_DEBUG, "invalid volume id found");
        return NULL;
    }

    // check for valid object id
    if (dest->object_id < 0 || dest->object_id > MAX_TERMS )
    {
        LOGd(_DEBUG, "invalid object id");
        return NULL; 
    }

    switch( dest->object_id )
    {
        // 0 means root
    case 0:  ret = strdup("/"); 
        LOGd(_DEBUG, "resolving root.");
        break;
        // else return vc[0-MAX_TERMS-1]
    default: no = malloc(4*sizeof(char));
        sprintf(no, "%d", dest->object_id-1);
        ret = strdup("vc");
        strcat(ret, no);
        LOGd(_DEBUG, "rev_resolved: %s", ret);
        break;
    }

    //LOGd(_DEBUG, "ret = %s", ret);

    return strdup(ret);
}

l4_threadid_t
l4vfs_basic_name_server_thread_for_volume_component(
    CORBA_Object _dice_corba_obj,
    volume_id_t volume_id,
    CORBA_Server_Environment *_dice_corba_env)
{
    if (volume_id == myvolume)
    {
        LOGd(_DEBUG, "server-Thread for volume called.");
        return l4_myself();
    }
    return L4_INVALID_ID;
}

l4_int32_t
l4vfs_common_io_ioctl_component(CORBA_Object _dice_corba_obj,
                                object_handle_t fd,
                                int cmd,
                                char **arg,
                                l4vfs_size_t *count,
                                CORBA_Server_Environment *_dice_corba_env)
{
    struct winsize *win;
    struct termios *termios;
    termstate_t *term;
    int termno;

    l4semaphore_down(&(clients[fd].client_sem));
    // reading root ?
    if (clients[fd].object_id == 0)
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return -EBADF;
    }

    // am i open?
    if (!clients[fd].open)
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return -EBADF;
    }

    // is it the same thread that opened me?
    if ( !l4_task_equal(clients[fd].client, *_dice_corba_obj ) )
    {
        l4semaphore_up(&(clients[fd].client_sem));
        return -EBADF;
    }

    // calculate term number
    termno = clients[fd].object_id - 1;
    term = terms[termno].terminal;

    l4semaphore_up(&(clients[fd].client_sem));

    // handle ioctl command
    switch(cmd)
    {
        // get win size
        case TIOCGWINSZ:
            win = (struct winsize *)*arg;
            vt100_getwinsize( term, win );
            LOGd(_DEBUG, "ioctl: TIOCGWINSZ (%d)", cmd);
            return 0;

        // get term attributes
        case TCGETS:
            termios = (struct termios *)*arg;
            vt100_tcgetattr( term, termios );
            LOGd(_DEBUG, "ioctl: TCGETS (%d)", cmd);
            return 0;

        // set term attributes - we do not support different 
        // validation times for the attributes - changes become
        // valid at the moment of execution
        case TCSETS:
        case TCSETSW:
        case TCSETSF:
            termios = (struct termios *)*arg;
            vt100_tcsetattr( term, termios );
            LOGd(_DEBUG, "ioctl: TCSETS/SW/SF (%d)", cmd);
            return 0;

        default:    LOG("\033[34m;unknown ioctl: %d\033[37m;", cmd);
                    break;
    }

    return -ENOTTY;
}


l4_int32_t
l4vfs_common_io_fcntl_component(CORBA_Object _dice_corba_obj,
                                object_handle_t fd,
                                int cmd,
                                long *arg,
                                CORBA_Server_Environment *_dice_corba_env)
{
    l4semaphore_down(&(clients[fd].client_sem));
    // no one may read from an invalid object
    if (clients[fd].object_id < 0)
    {
        LOG("invalid file descriptor");
        l4semaphore_up(&(clients[fd].client_sem));
        return -EBADF;
    }

    // no one may read from the root object
    if (clients[fd].object_id == 0)
    {
        LOG("no read to root allowed");
        l4semaphore_up(&(clients[fd].client_sem));
        return -EISDIR;
    }

    // only read from open objects
    if (!clients[fd].open)
    {
        LOG("handle not open");
        l4semaphore_up(&(clients[fd].client_sem));
        return -EINVAL;
    }

	switch(cmd)
	{
		case F_GETFL:
			l4semaphore_up(&(clients[fd].client_sem));
			return clients[fd].rw_mode;
			break;
		case F_SETFL:
			clients[fd].rw_mode = *arg;
			break;
		default:
			LOG("unimplemented fcntl: 0x%x", cmd);
			break;
	}

	l4semaphore_up(&(clients[fd].client_sem));
	return 0;
}


l4_int32_t
l4vfs_basic_io_stat_component(CORBA_Object _dice_corba_obj,
                              const object_id_t *object_id,
                              l4vfs_stat_t *buf,
                              CORBA_Server_Environment *_dice_corba_env)
{
    // am I responsible for this volume?
    LOGd(_DEBUG, "volume id: %d", object_id->volume_id);
    if (object_id->volume_id != myvolume)
        return -ENOENT;
    LOGd(_DEBUG, "entry belongs to vc_server");

    // check for legal local-object_id
    if (object_id->object_id < 0 || object_id->object_id > MAX_TERMS )
    {
        LOGd(_DEBUG, "invalid object id");
        return -ENOENT;
    }

    buf->st_dev   = myvolume;
    buf->st_ino   = object_id->object_id;
    buf->st_nlink = 1;
    buf->st_size  = 0;  // useless but defined ...
    // fixme: other fields are undefined for now ...

    if (object_id->object_id == 0)
    {
        buf->st_mode  = S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO;
    }
    else
    {  // maybe pipe is better than reg here?
        buf->st_mode  = S_IFREG | S_IRWXU | S_IRWXG | S_IRWXO;
    }

    return 0;
}

void
l4vfs_select_notify_clear_component(CORBA_Object _dice_corba_obj,
                                    object_handle_t handle,
                                    int mode,
                                    const l4_threadid_t *notif_tid,
                                    CORBA_Server_Environment *_dice_corba_env)
{
    int termno = clients[handle].object_id - 1;

    l4semaphore_down(&clients[handle].client_sem);
    /* Select cancelled, erase all info. */
    vt100_unset_select_info(terms[termno].terminal, handle, mode, notif_tid);
    l4semaphore_up(&clients[handle].client_sem);
}


void
l4vfs_select_notify_request_component (CORBA_Object _dice_corba_obj,
                                       object_handle_t handle,
                                       int mode,
                                       const l4_threadid_t *notif_tid,
                                       CORBA_Server_Environment *_dice_corba_env)
{
    int termno = clients[handle].object_id - 1;

    l4semaphore_down(&clients[handle].client_sem);

    /* Store select info. */
    vt100_set_select_info(terms[termno].terminal, handle, mode, notif_tid);

    /* Immediately send notification if possible. */
    if (vt100_data_avail(terms[termno].terminal)) 
        vt100_select_notify(terms[termno].terminal);

    l4semaphore_up(&clients[handle].client_sem);
}


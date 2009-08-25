#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include "MountReg.h"
#include "globals.h"

// L4 headers
#include <l4/sys/types.h>
#include <l4/env/errno.h>
#include <l4/sys/syscalls.h>
#include <l4/log/l4log.h>
#include <l4/l4rm/l4rm.h>
#include <l4/util/util.h>
#include <l4/env/env.h>
#include <l4/names/libnames.h>
#include <l4/generic_fprov/generic_fprov-client.h>

void io_printf(const char* chrFormat, ...)
{
    va_list arg_ptr;
    /*void *tb;*/

    va_start (arg_ptr, chrFormat);
    vprintf(chrFormat, arg_ptr);
    va_end (arg_ptr);
}

int io_load_file33(const char * filename, void ** addr, unsigned long * size)
{
  FILE *f=0;

  //f = fopen(filename, "rb");
  if(f) {
//     fseek(f, 0, SEEK_END);
//     *size = ftell(f);  /* Extract the size of the file and reads it into a buffer.*/
//     rewind(f);
//     *addr = (void *)malloc(*size+1);
//     fread(*addr, *size, 1, f);
//     fclose(f);
//     return 0; /*NO_ERROR;*/
  }

  return 2; /* ERROR_FILE_NOT_FOUND; */
}

//int io_load_file(const char * filename, void ** addr, unsigned long * size);

/* OBS! For fsrouter to work it must be initialized from globals.c:init_globals() */
int io_load_file(const char * filename, void ** addr, unsigned long * size) {

  static l4_threadid_t        fs_drvc_id;
  l4_threadid_t        dm_id;
  l4dm_dataspace_t     ds;
  CORBA_Environment   _env = dice_default_environment;
  char drv = get_drv(filename);

  if(drv == '\0') {
    //io_printf("io.c Warning, no drive in filename, assume c: for now.\n");
    drv = 'c';
  }
  /* How to find the working directory? From this thread ( l4thread_myself() ) find the thread's 
     process (our own prcess structure) and lookup the working directory there.
     L4 has a task- and thread number, as seen in output by [x.y] */
  
  struct I_Fs_srv *target_fs_srv = FSRouter_route(&fsrouter, drv );
  //io_printf("io_load_file: '%s'\n", filename);
  
  if(target_fs_srv) {
      if (!names_waitfor_name(target_fs_srv->server, &fs_drvc_id, 10000))
      {
        io_printf("File provider %s doesn't answer\n", target_fs_srv->server);
        return 2;
      };

      dm_id = l4env_get_default_dsm();
     if (l4_is_invalid_id(dm_id))
      {
        io_printf("Bad dataspace!\n");
        return 1;
      }
      int f_status = l4fprov_file_open_call(&fs_drvc_id,
                           filename, /* OBS! Ta bort enheten från sökvägen! */
                           &dm_id,
                           0, &ds, size, &_env);
      if(f_status)
        return 2;
      f_status=l4rm_attach(&ds, *size, 0,
                  L4DM_RO | L4RM_MAP, addr);
      if(f_status)
      {
        io_printf("error %s\n", l4env_errstr(f_status));
        return 2;
      }
      //io_printf("io_load_file(: '%s', 0x%x, %d )\n", filename, *addr, *size);
      return 0;
  } else {
    io_printf("target_fs_srv: 0x%x\n", target_fs_srv);
    return 2; /* ERROR_FILE_NOT_FOUND; */
  }
}

/*
NAME
       fcntl - manipulate file descriptor

SYNOPSIS
       #include <unistd.h>
       #include <fcntl.h>

       int fcntl(int fd, int cmd);
       int fcntl(int fd, int cmd, long arg);
       int fcntl(int fd, int cmd, struct flock *lock);

DESCRIPTION
       fcntl()  performs  one  of the operations described below on the open file descriptor fd. The
       operation is determined by cmd.
       
/pub/L4_Fiasco/tudos2/my_user_build_dir/pkg/uclibc/lib/uclibc/ARCH-all/libc/stdio/_fopen.c:124: undefined reference to `fcntl'
/pub/L4_Fiasco/tudos2/my_user_build_dir/pkg/uclibc/lib/uclibc/ARCH-all/libc/stdio/_fopen.c:135: undefined reference to `open

:124-126				&& fcntl(filedes, F_SETFL, O_APPEND))	// Need O_APPEND. 
:136 		if ((stream->__filedes = open(((const char *) fname_or_mode),
									  open_mode, 0666)) < 0) {
*/ 

int fcntl(int fd, int cmd, long arg) {
    printf("io.c fcntl(%d\n", fd);
    return 0;
}

/*
NAME
       open, creat - open and possibly create a file or device

SYNOPSIS
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

       int open(const char *pathname, int flags);
       int open(const char *pathname, int flags, mode_t mode);
       int creat(const char *pathname, mode_t mode);

DESCRIPTION
       Given  a  pathname  for a file, open() returns a file descriptor, a small,
       non-negative  integer  for  use  in  subsequent  system  calls   (read(2),
       write(2),  lseek(2),  fcntl(2),  etc.).  The file descriptor returned by a
       successful call will be the lowest-numbered file descriptor not  currently
       open for the process.

       The  new  file descriptor is set to remain open across an execve(2) (i.e.,
       the FD_CLOEXEC file descriptor flag described  in  fcntl(2)  is  initially
       disabled).   The  file  offset  is  set  to the beginning of the file (see
       lseek(2)).

       A call to open() creates a new open file description, an entry in the sys-
       tem-wide  table of open files.  This entry records the file offset and the
       file status flags (modifiable via the fcntl() F_SETFL operation).  A  file
       descriptor is a reference to one of these entries; this reference is unaf-
       fected if pathname is subsequently removed or modified to refer to a  dif-
       ferent  file.   The new open file description is initially not shared with
       any other process, but sharing may arise via fork(2).


       The parameter flags must  include  one  of  the  following  access  modes:
       O_RDONLY,  O_WRONLY, or O_RDWR.  These request opening the file read-only,
       write-only, or read/write, respectively.

       In addition, zero or more file creation flags and file status flags can be
       bitwise-or'd  in  flags.   The  file  creation  flags are O_CREAT, O_EXCL,
       O_NOCTTY, and O_TRUNC.  The file status flags are  all  of  the  remaining
       flags  listed below.  The distinction between these two groups of flags is
       that the file status flags can be retrieved and (in some  cases)  modified
       using  fcntl(2).   The  full  list  of file creation flags and file status
       flags is as follows:

       O_APPEND
              The file is opened in append mode. Before each  write(),  the  file
              offset  is  positioned  at the end of the file, as if with lseek().
              O_APPEND may lead to corrupted files on NFS file  systems  if  more
              than  one  process appends data to a file at once.  This is because
              NFS does not support appending to a file, so the client kernel  has
              to simulate it, which can't be done without a race condition.

       O_ASYNC
              Enable  signal-driven I/O: generate a signal (SIGIO by default, but
              this can be changed via fcntl(2)) when input or output becomes pos-
              sible  on this file descriptor.  This feature is only available for
              terminals, pseudo-terminals, sockets, and (since Linux  2.6)  pipes
              and FIFOs.  See fcntl(2) for further details.

       O_CREAT
              If the file does not exist it will be created.  The owner (user ID)
              of the file is set to the effective user ID  of  the  process.  The
              group  ownership (group ID) is set either to the effective group ID
              of the process or to the group ID of the parent directory  (depend-
              ing  on filesystem type and mount options, and the mode of the par-
              ent directory, see, e.g., the mount  options  bsdgroups  and  sysv-
              groups of the ext2 filesystem, as described in mount(8)).

       O_DIRECT
              Try to minimize cache effects of the I/O to and from this file.  In
              general this will degrade performance, but it is useful in  special
              situations,  such  as when applications do their own caching.  File
              I/O is done directly to/from user space buffers.  The I/O  is  syn-
              chronous, i.e., at the completion of a read(2) or write(2), data is
              guaranteed to have been  transferred.   Under  Linux  2.4  transfer
              sizes, and the alignment of user buffer and file offset must all be
              multiples of the logical block size of the file system. Under Linux
              2.6 alignment to 512-byte boundaries suffices.

              A semantically similar (but deprecated) interface for block devices
              is described in raw(8).

       O_DIRECTORY
              If pathname is not a directory, cause the open to fail.  This  flag
              is  Linux-specific,  and  was  added  in kernel version 2.1.126, to
              avoid denial-of-service problems if opendir(3) is called on a  FIFO
              or  tape  device, but should not be used outside of the implementa-
              tion of opendir.

       O_EXCL When used with O_CREAT, if the file already exists it is  an  error
              and  the open() will fail. In this context, a symbolic link exists,
              regardless of where it points to.  O_EXCL is  broken  on  NFS  file
              systems;  programs  which  rely  on it for performing locking tasks
              will contain a race condition.  The solution for performing  atomic
              file  locking  using  a  lockfile is to create a unique file on the
              same file  system  (e.g.,  incorporating  hostname  and  pid),  use
              link(2)  to  make  a link to the lockfile. If link() returns 0, the
              lock is successful.  Otherwise, use stat(2) on the unique  file  to
              check  if its link count has increased to 2, in which case the lock
              is also successful.

       O_LARGEFILE
              (LFS) Allow files whose sizes cannot be  represented  in  an  off_t
              (but can be represented in an off64_t) to be opened.

       O_NOATIME
              (Since  Linux  2.6.8)  Do  not  update  the  file  last access time
              (st_atime in the inode) when the file is  read(2).   This  flag  is
              intended  for use by indexing or backup programs, where its use can
              significantly reduce the amount of disk activity.   This  flag  may
              not be effective on all filesystems.  One example is NFS, where the
              server maintains the access time.


*/
typedef  int mode_t;

int open(const char *pathname, int flags, mode_t mode) {
    //printf("io.c open(%s\n", pathname);
    return 0;
}


                                     
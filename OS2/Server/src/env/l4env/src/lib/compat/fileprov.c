#include <os3/io.h>
#include <dirent.h>
#include <os3/MountReg.h>
#include <os3/globals.h>

// L4 headers
#include <l4/sys/types.h>
#include <l4/env/env.h>
#include <l4/sys/syscalls.h>
#include <l4/l4rm/l4rm.h>
#include <l4/util/util.h>
#include <l4/env/env.h>
#include <l4/names/libnames.h>

#include <l4/generic_fprov/generic_fprov-client.h>

//extern l4_threadid_t fs;
extern l4_threadid_t fprov_id;

l4_threadid_t dsm = L4_INVALID_ID;

int io_load_file(const char *filename, void **addr, unsigned long *size)
{
  CORBA_Environment env = dice_default_environment;
  l4dm_dataspace_t ds;
  int  rc;

  io_log("filename=%s\n", filename);
  io_log("fileprov=%lu.%lu\n", fprov_id.id.task, fprov_id.id.lthread);

  /* query default dataspace manager id */
  if (l4_is_invalid_id(dsm))
    dsm = l4env_get_default_dsm();

  if (l4_is_invalid_id(dsm))
  {
    io_log("No dataspace manager found\n");
    return 2;
  }

  if (!filename || !*filename)
    return 2;

  /* get a file from a file provider */
  rc = l4fprov_file_open_call(&fprov_id, filename, &dsm, 0,
                       &ds, (l4_size_t *)size, &env);

  if (rc == 2)
    return rc; /* ERROR_FILE_NOT_FOUND */

  io_log("size=%lu\n", *size);

  /* attach the created dataspace to our address space */
  rc = l4rm_attach(&ds, *size, 0, L4DM_RW | L4RM_MAP, addr);

  io_log("addr=%lu\n", *addr);

  if (rc < 0)
    return 8; /* What to return? */

  return 0;
}

int io_close_file(void *address)
{
  int rc;
  l4dm_dataspace_t ds;
  l4_addr_t addr;
  l4_size_t size;
  l4_offs_t offset;
  l4_threadid_t pager;

  rc = l4rm_lookup_region(address, &addr, &size, &ds,
                          &offset, &pager);

  if (rc < 0)
    return -rc;

  if (rc == L4RM_REGION_DATASPACE)
  {  
    l4rm_detach(addr);
    l4dm_close(&ds);
  }

  return 0;
}

#if 0

int io_load_file(const char * filename, void ** addr, unsigned long * size)
{
  char ch;
  FILE *f=0;
  char * newfilename;
  char * newdirectory;
  struct dirent *diren;
  char buf[256];
  int  len, i;

  io_log("filename=%s\n", filename);
  char drv = get_drv(filename);

  io_log("drv=%c:\n", drv);

  if(drv == '\0') 
  {
    return 2; /* ERROR_FILE_NOT_FOUND */
  }

  char * directory = get_directory(filename);
  io_log("directory=%s\n", directory);
  if (directory==NULL)
  {
    return 2; /* ERROR_FILE_NOT_FOUND */
  }
  char * name = get_name(filename);
  io_log("name=%s\n", name);

  DosNameConversion(directory);
  DosNameConversion(name);

  io_log("directory=%s\n", directory);
  io_log("name=%s\n", name);
#if 0
  io_log("srv_num_=%d\n", fsrouter.srv_num_);
  for(i=0; i< fsrouter.srv_num_; i++)
  {
    I_Fs_srv_t *srv = fsrouter.fs_srv_arr_[i];
    if (srv)
    {
      io_log("srv->drive=%s, srv->mountpoint=%s\n", srv->drive, srv->mountpoint);
    }
  }    
#endif
  struct I_Fs_srv *target_fs_srv = FSRouter_route(&fsrouter, drv);

  newfilename=malloc(strlen(target_fs_srv->mountpoint)+
                     strlen(directory)+
                     strlen(name)+1);
  newdirectory=malloc(strlen(target_fs_srv->mountpoint)+
                     strlen(directory)+1);
  strcpy(newdirectory, target_fs_srv->mountpoint);
  newdirectory=strcat(newdirectory, directory);
  strcpy(newfilename, newdirectory);
  newfilename=strcat(newfilename, name);

//  io_log("%s => %s", filename, newfilename);

  //if (newdirectory[strlen(newdirectory) - 1] == '/')
  //  newdirectory[strlen(newdirectory) - 1] = '\0';

  //io_log("%s", newdirectory); // If I remove this then next line return NULL. Why?
  //enter_kdebug("stop"); // break into debugger
  DIR *dir = opendir(newdirectory);

  if (dir==NULL)
  {
    io_log("Error opening directory\n");
    return 2; /* ERROR_FILE_NOT_FOUND */
  }
  else
    io_log("opendir() successful\n");


  io_log("name=%s\n", name);
  while(diren = readdir(dir)) 
  {
        len = strlen(name);
        strncpy(buf, diren->d_name, len);
	buf[len] = '\0';
        io_log("diren->d_name=%s\n", diren->d_name);
	io_log("buf=%s\n", buf);
        if(!diren)
            break;
        if(strcasecmp(buf, name)==0) {
            break;
        }
  }
  
  if (!diren)
  {
    io_log("diren=0\n");
    closedir(dir);
    return 2;
  }
 
  //io_log("directory read");
  //io_log("newdirectory=%s", newdirectory);
  //io_log("diren->d_name=%s", diren->d_name);
  strcpy(newfilename, newdirectory);
  newfilename=strcat(newfilename, buf);

  closedir(dir);

  //io_log("newfilename=%s", newfilename);
  f = fopen(newfilename, "rb");
  io_log("file opened\n");
  if(f) {
     fseek(f, 0, SEEK_END);
     *size = ftell(f);  /* Extract the size of the file and reads it into a buffer.*/
     rewind(f);
     *addr = (void *)malloc(*size+1);
     fread(*addr, *size, 1, f);
     fclose(f);
     io_log("successful return\n");
     return 0; /*NO_ERROR;*/
  }

  return 2; /* ERROR_FILE_NOT_FOUND; */
}

/* OBS! For fsrouter to work it must be initialized from globals.c:init_globals() */
int io_load_file33(const char * filename, void ** addr, unsigned long * size) {

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
        io_printf("File provider %s doesn't answer\n", target_fs_srv->mountpoint);
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
#endif

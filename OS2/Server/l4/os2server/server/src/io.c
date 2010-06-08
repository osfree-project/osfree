#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>
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
#include <l4/l4con/l4con.h>
#include <l4/l4con/l4con-client.h>
#include <l4/l4con/l4contxt.h>

void io_printf(const char* chrFormat, ...)
{
    va_list arg_ptr;

    va_start (arg_ptr, chrFormat);
    vprintf(chrFormat, arg_ptr);
    va_end (arg_ptr);
}

int io_load_file(const char * filename, void ** addr, unsigned long * size)
{
  char ch;
  FILE *f=0;
  char * newfilename;
  char * newdirectory;
  struct dirent *diren;

  char drv = get_drv(filename);

  LOG("drv=%c:", drv);

  if(drv == '\0') 
  {
    return 2; /* ERROR_FILE_NOT_FOUND */
  }

  char * directory = get_directory(filename);
  LOG("directory=%s", directory);
  if (directory==NULL)
  {
    return 2; /* ERROR_FILE_NOT_FOUND */
  }
  char * name = get_name(filename);
  LOG("name=%s", name);

  DosNameConversion(directory);
  DosNameConversion(name);

  LOG("directory=%s", directory);
  LOG("name=%s", name);

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

//  LOG("%s => %s", filename, newfilename);

  //if (newdirectory[strlen(newdirectory) - 1] == '/')
  //  newdirectory[strlen(newdirectory) - 1] = '\0';

  if (!strcmp(name, "sub32.dll")) LOG("%s", newdirectory); // If I remove this then next line return NULL. Why?
  //enter_kdebug("stop"); // break into debugger
  DIR *dir = opendir(newdirectory);

  if (dir==NULL)
  {
    LOG("Error opening directory");
    return 2; /* ERROR_FILE_NOT_FOUND */
  }
  else
    LOG("opendir() successful");

  while(diren = readdir(dir)) 
  {
        if(!diren)
            break;
        if(strcasecmp(diren->d_name, name)==0) {
            break;
        }
  }
  
  if (!diren)
  {
    LOG("diren=0");
    return 2;
  }
  
  LOG("directory read");
  LOG("newdirectory=%s", newdirectory);
  LOG("diren->d_name=%s", diren->d_name);
  strcpy(newfilename, newdirectory);
  newfilename=strcat(newfilename, diren->d_name);

  closedir(dir);

  LOG("newfilename=%s", newfilename);
  f = fopen(newfilename, "rb");
  LOG("file opened");
  if(f) {
     fseek(f, 0, SEEK_END);
     *size = ftell(f);  /* Extract the size of the file and reads it into a buffer.*/
     rewind(f);
     *addr = (void *)malloc(*size+1);
     fread(*addr, *size, 1, f);
     fclose(f);
     LOG("successful return");
     return 0; /*NO_ERROR;*/
  }

  return 2; /* ERROR_FILE_NOT_FOUND; */
}

#if 0
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

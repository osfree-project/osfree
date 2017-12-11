/* osFree internal */
#include <os3/token.h>
#include <os3/io.h>
#include <os3/cfgparser.h>

/* l4env includes */
#include <l4/sys/types.h>
#include <l4/env/env.h>
#include <l4/l4rm/l4rm.h>
#include <l4/dm_mem/dm_mem.h>
#include <l4/dm_generic/consts.h>
#include <l4/generic_fprov/generic_fprov-client.h>

/* libc includes */
#include <stdlib.h>

extern l4_threadid_t fs;
extern l4_threadid_t dsm;
extern l4_threadid_t os2srv;

char get_drv(const char *s_path);
char *get_directory(const char *s_path);
char *get_name(const char *s_path);
char *get_fname(const char *s_path);

unsigned int find_module_path(const char * name, char * full_path_name);
unsigned int find_path(const char *name, char *full_path_name);

void cfg_Getenv(const char *path, char **value);

/* extract drive letter from path */
char get_drv(const char *s_path)
{
    if(s_path[1] == ':')
        return s_path[0];
    return '\0';
}

/* extract directory from path */
char *get_directory(const char *s_path) 
{
  int s=0;
  int e=0;
  int i=0;

  if(s_path[1] == ':')
  {
    s=2;
  }

  for (i=s; i<=strlen(s_path); i++)
  {
    if (s_path[i]=='\\')
    {
      e=i;
    }
  }
  
  if (e==0) return '\0';
  char *dir=malloc(e-s+2);
  strncpy(dir, &s_path[s], e-s+1);
  dir[e - s + 1] = '\0';
  return dir;
}

/* extract name from path */
char *get_name(const char *s_path) 
{
  int s=0;
  int e=0;
  int i=0;

  for (i=0; i<=strlen(s_path); i++)
  {
    if (s_path[i]=='\\')
    {
      s=i+1;
    }
  }

  e=strlen(s_path);
  char *name=malloc(e-s+1);
  strncpy(name, &s_path[s], e-s+1);
  return name;
}

#if 1
char *get_fname(const char *s_path) 
{
  //int s=0;
  int e=-1;
  int i=0;
  //char name[1024];

  for (i = 0; i < strlen(s_path); i++)
  {
    if (s_path[i]=='.')
    {
      e=i;
    }
  }
  
  if (e == -1)
    e = strlen(s_path);

  //strcpy(name, s_path);
  char *xname = malloc(e + 1); // no free!
  strncpy(xname, s_path, e + 1);
  xname[e]='\0';
  
  return xname;
}

#else

char * get_fname(const char *s_path) 
{
  int s=0;
  int e=0;
  int i=0;
  char name[1024];

  for (i=0; i<=strlen(s_path); i++)
  {
    if (s_path[i]=='.')
    {
      e=i;
    }
  }
  
  if (e==0)
  {
    e=strlen(s_path)-1;
  }
  strcpy(name, s_path);
  char *xname=malloc(e-s+1);
  strncpy(xname, &name[s], e-s+1);
  xname[e]='\0';
  return xname;
}
#endif

#if 0

char * os2_fname_to_vfs_fname(const char *filename) 
{
  char * newfilename;
  char * newdirectory;
  struct dirent *diren;

  char drv = get_drv(filename);

  if(drv == '\0') 
  {
    return NULL; 
  }

  char * directory = get_directory(filename);
  if (directory==NULL)
  {
    return NULL;
  }
  char * name = get_name(filename);

  DosNameConversion(directory);
  DosNameConversion(name);

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

  return newfilename;
}

#endif

// This function searches LIBPATH option. Only OS/2 path formats are supported.

unsigned int find_module_path(const char * name, char * full_path_name)
{
  CORBA_Environment env = dice_default_environment;
  l4dm_dataspace_t ds;
  l4_size_t size;
  int  rc;

  /* query default dataspace manager id */
  if (l4_is_invalid_id(dsm))
    dsm = l4env_get_default_dsm();

  if (l4_is_invalid_id(dsm))
  {
    io_log("No dataspace manager found\n");
    return 2;
  }
  //FILE *f;
  char *p = options.libpath - 1;
  STR_SAVED_TOKENS st;
  char * p_buf = full_path_name;
  //char *sep="\\";
#ifdef __LINUX__
  char *hostsep="/";
#else
  char *hostsep="\\";
#endif
  char *psep=";";

  StrTokSave(&st);
  if((p = StrTokenize((char*)options.libpath, psep)) != 0)
  do if(*p)
  {
    //int B_LEN = 250;
    char buf[251];
    //char *str_buf=(char*) &buf;
    char buf2[251];
    char * file_to_find=(char*) &buf2;
    long i=0;

    p_buf = full_path_name;
    p_buf[0] = '\0';
    while (p[i]!='\0')
    {
      if (p[i]=='\\') p[i]=*hostsep; ++i;
    }

    strcat(p_buf, p);
    if (p[strlen(p)-1]!=*hostsep)
    {
      strcat(p_buf, hostsep);
    }

    strcat(p_buf, name);

    buf[0] = '\0';
    buf2[0] = '\0';
    strcat(file_to_find, name);
    #ifndef L4API_l4v2
      if(find_case_file(file_to_find, p, str_buf, B_LEN))
      {
        p_buf[0] = 0;
        strcat(p_buf, p);
        strcat(p_buf, hostsep);
        strcat(p_buf, str_buf); /* Case corrected for file, Needed on Linux. */
      }
    #endif

    io_log(p_buf);
    //io_log(os2_fname_to_vfs_fname(p_buf));

    //f = fopen(os2_fname_to_vfs_fname(p_buf), "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
    /* try opening a file from a file provider */
    rc = l4fprov_file_open_call(&fs, p_buf, &dsm, 0,
                       &ds, &size, &env);
    
    if(!rc)
    {
      l4dm_close(&ds);
      StrTokStop();
      return 0; /* NO_ERROR */
    }
  } while((p = StrTokenize(0, psep)) != 0);
  StrTokRestore(&st);

  return 2 /*ERROR_FILE_NOT_FOUND*/;
}

unsigned int find_path(const char *name, char *full_path_name)
{
  CORBA_Environment env = dice_default_environment;
  l4dm_dataspace_t ds;
  l4_size_t size;
  int  rc;

  //FILE *f;
  char *p;
  #define buf_size 4096
  char buf[buf_size+1];
  char *path = (char *) &buf;
  STR_SAVED_TOKENS st;
  char * p_buf = full_path_name;
  char *sep="\\";
  char *psep=";";

  /* query default dataspace manager id */
  if (l4_is_invalid_id(dsm))
    dsm = l4env_get_default_dsm();

  if (l4_is_invalid_id(dsm))
  {
    io_log("No dataspace manager found\n");
    return 2;
  }

  io_log("name=%s", name);
  io_log("full_path_name=%s", full_path_name);

  cfg_Getenv("PATH", &path);
  io_log("PATH: %s", path);

  p = path - 1;

  StrTokSave(&st);
  io_log("0");
  if((p = StrTokenize((char*)path, psep)) != 0) do if(*p)
  {
    io_log("1");
    p_buf = full_path_name;
    p_buf[0] = 0;
    io_log(p);
    if (!strcmp(p,"."))
    {
      strcat(p_buf, options.bootdrive);
      strcat(p_buf, "\\"); // For OS2Server current directory is always root
    } else {
      strcat(p_buf, p);
      strcat(p_buf, sep);
    }
    strcat(p_buf, name);
    io_log("2");
    io_log("p_buf=%s", p_buf);
    //f = fopen(os2_fname_to_vfs_fname(p_buf), "rb"); /* Tries to open the file, if it works f is a valid pointer.*/
    //if(f)
    /* try opening a file from a file provider */
    io_log("call fprov");
    io_log("fs: %x.%x", fs.id.task, fs.id.lthread);
    rc = l4fprov_file_open_call(&fs, p_buf, &dsm, 0,
                       &ds, &size, &env);
    io_log("called, rc=%d", rc);
    io_log("3");
    if(!rc)
    {
      io_log("30");
      l4dm_close(&ds);
      io_log("31");
      StrTokStop();
      io_log("32");
      return 0; /* NO_ERROR */
    }
    io_log("4");
  } while((p = StrTokenize(0, psep)) != 0);
  StrTokRestore(&st);

  io_log("5");
  return 2/*ERROR_FILE_NOT_FOUND*/;
}

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/token.h>
#include <os3/io.h>
#include <os3/cfgparser.h>
#include <os3/path.h>

/* libc includes */
#include <string.h>
#include <stdlib.h>

char get_drv(const char *s_path);
char *get_directory(const char *s_path);
char *get_name(const char *s_path);

void cfg_Getenv(char *path, char **value);

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
  unsigned int s=0;
  unsigned int e=0;
  unsigned int i=0;

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
  char *dir=(char *)malloc(e-s+2);
  strncpy(dir, &s_path[s], e-s+1);
  dir[e - s + 1] = '\0';
  return dir;
}

/* extract name from path */
char *get_name(const char *s_path) 
{
  unsigned int s=0;
  unsigned int e=0;
  unsigned int i=0;

  for (i=0; i<=strlen(s_path); i++)
  {
    if (s_path[i]=='\\')
    {
      s=i+1;
    }
  }

  e=strlen(s_path);
  char *name=(char *)malloc(e-s+1);
  strncpy(name, &s_path[s], e-s+1);
  return name;
}

#if 1
char *get_fname(const char *s_path) 
{
  //int s=0;
  int e=-1;
  unsigned int i=0;
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
  char *xname = (char *)malloc(e + 1); // no free!
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
  void *addr;
  unsigned long size;
  int  rc;

  char *p = options.libpath - 1;
  STR_SAVED_TOKENS st;
  char * p_buf = full_path_name;

#ifdef __LINUX__
  const char *hostsep="/";
#else
  const char *hostsep="\\";
#endif
  const char *psep=";";

  io_log("options.libpath=%s\n", options.libpath);
  StrTokSave(&st);
  if((p = StrTokenize((char*)options.libpath, psep)) != 0)
  do if(*p)
  {
    //char buf[251];
    char buf2[251];
    char * file_to_find=(char*) &buf2;
    long i=0;

    p_buf = full_path_name;
    p_buf[0] = '\0';
    while (p[i]!='\0')
    {
      if (p[i]=='\\')
          p[i]=*hostsep;

      ++i;
    }

    strcat(p_buf, p);
    if (p[strlen(p)-1]!=*hostsep)
    {
      strcat(p_buf, hostsep);
    }

    strcat(p_buf, name);

    //buf[0] = '\0';
    buf2[0] = '\0';
    strcat(file_to_find, name);

    io_log(p_buf);

    rc = io_load_file(p_buf, &addr, &size);

    if(!rc)
    {
      io_close_file(addr);
      StrTokStop();
      return NO_ERROR;
    }
  } while((p = StrTokenize(0, psep)) != 0);
  StrTokRestore(&st);

  return ERROR_FILE_NOT_FOUND;
}

unsigned int find_path(const char *name, char *full_path_name)
{
  void *addr;
  unsigned long size;
  int  rc;

  char *p;
  #define buf_size 4096
  char buf[buf_size+1] = {0};
  char *path = (char *) &buf;
  STR_SAVED_TOKENS st;
  char * p_buf = full_path_name;
  const char *sep="\\";
  const char *psep=";";

  cfg_Getenv((char *)"PATH", &path);

  p = path - 1;

  io_log("path=%s\n", path);
  StrTokSave(&st);

  if((p = StrTokenize((char*)path, psep)) != 0) do if(*p)
  {
    p_buf = full_path_name;
    p_buf[0] = 0;

    if (!strcmp(p,"."))
    {
      strcat(p_buf, options.bootdrive);
      strcat(p_buf, "\\"); // For OS2Server current directory is always root
    } else {
      strcat(p_buf, p);
      strcat(p_buf, sep);
    }

    strcat(p_buf, name);

    /* try opening a file from a file provider */
    rc = io_load_file(p_buf, &addr, &size);

    if(!rc)
    {
      io_close_file(addr);
      StrTokStop();
      return NO_ERROR;
    }
  } while((p = StrTokenize(0, psep)) != 0);
  StrTokRestore(&st);

  return ERROR_FILE_NOT_FOUND;
}

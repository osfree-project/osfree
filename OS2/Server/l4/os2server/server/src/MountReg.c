
#include "MountReg.h"
#include "globals.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <l4/log/l4log.h>

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

/* extract drive letter from path */
char get_drv(const char *s_path) {
    if(s_path[1] == ':')
        return s_path[0];
    return '\0';
}

/* extract directory from path */
char * get_directory(const char *s_path) 
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
  
  if (e==0) return NULL;
  char *dir=malloc(e-s+2);
  strncpy(dir, &s_path[s], e-s+1);
  dir[e - s + 1] = '\0';
  return dir;
}

/* extract name from path */
char * get_name(const char *s_path) 
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


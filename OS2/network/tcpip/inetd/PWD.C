#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fix_env.h"

#include "pwd.h"

static struct passwd *_getpwd(char *buffer)
{
  static struct passwd pw;
  char *ptr;

  if (buffer[strlen(buffer) - 1] == '\n')
    buffer[strlen(buffer) - 1] = 0;

  if ( buffer[0] == '#' )
    return NULL;

  pw.pw_name = buffer;

  if ( (ptr = strchr(buffer, ':')) != NULL )
    *ptr++ = 0;
  else
    return NULL;

  pw.pw_passwd = ptr;

  if ( (ptr = strchr(ptr, ':')) != NULL )
    *ptr++ = 0;
  else
    return NULL;

  pw.pw_uid = atoi(ptr);

  if ( (ptr = strchr(ptr, ':')) != NULL )
    *ptr++ = 0;
  else
    return NULL;

  pw.pw_gid = atoi(ptr);

  if ( (ptr = strchr(ptr, ':')) != NULL )
    *ptr++ = 0;
  else
    return NULL;

  pw.pw_gecos = ptr;

  if ( ptr[0] && ptr[1] && (ptr = strchr(ptr + 2, ':')) != NULL )
    *ptr++ = 0;   /* skip drive: */
  else
    return NULL;

  pw.pw_dir = ptr;

  if ( ptr[0] && ptr[1] && (ptr = strchr(ptr + 2, ':')) != NULL )
    *ptr++ = 0;   /* skip drive: */
  else
    return NULL;

  pw.pw_shell = ptr;

  return &pw;
}

struct passwd *getpwnam(const char *name)
{
  struct passwd *pw;
  static char buffer[256];
  FILE *passwd;

  strcpy(buffer,"c:\\etc\\passwd");

  if ( (passwd = fopen(fix_env(buffer,"ETC"), "r")) == NULL )
    return NULL;

  while ( fgets(buffer, sizeof(buffer), passwd) != NULL )
  {
    if (buffer[strlen(buffer) - 1] == '\n')
      buffer[strlen(buffer) - 1] = 0;

    if ( (pw=_getpwd(buffer))==NULL )
      continue;

    if ( strcmp(name, pw->pw_name) == 0 )
    {
      fclose(passwd);
      return pw;
    }
  }

  fclose(passwd);
  return NULL;
}

struct passwd *getpwuid(uid_t uid)
{
  struct passwd *pw;
  static char buffer[256];
  FILE *passwd;

  strcpy(buffer,"c:\\etc\\passwd");

  if ( (passwd = fopen(fix_env(buffer,"ETC"), "r")) == NULL )
    return NULL;

  while ( fgets(buffer, sizeof(buffer), passwd) != NULL )
  {
    if (buffer[strlen(buffer) - 1] == '\n')
      buffer[strlen(buffer) - 1] = 0;

    if ( (pw=_getpwd(buffer))==NULL )
      continue;

    if ( uid == pw->pw_uid)
    {
      fclose(passwd);
      return pw;
    }
  }

  fclose(passwd);
  return NULL;
}

int getuid(void)
{
  char *name;
  struct passwd *pw;

  name=getenv("LOGNAME");
  if (!name) return 0;

  pw=getpwnam(name);
  if (!pw) return 0;

  return pw->pw_uid;
}

int setpwnam(const char *name, const char *crypted)
{
  struct passwd *pw;
  static char old[256], new[256], bak[256], line[256], buffer[256];
  char *ptr;
  FILE *passwd, *newpasswd;
  int found = 0;

  strcpy(old, "c:\\etcp\\passwd");
  fix_env(old, "ETC");
  strcpy(new, old);
  strcat(new, ".new");
  strcpy(bak, old);
  strcat(bak, ".bak");

  if ( (passwd = fopen(old, "r")) == NULL )
    return -1;

  if ( (newpasswd = fopen(new, "w")) == NULL )
    return -1;

  while ( fgets(line, sizeof(line), passwd) != NULL )
  {
    strcpy(buffer, line);

    if (buffer[strlen(buffer) - 1] == '\n')
      buffer[strlen(buffer) - 1] = 0;

    if ( (pw=_getpwd(buffer))==NULL )
      goto done;

    if (strcmp(name, pw->pw_name) == 0)
    {
      sprintf(line, "%s:%s:%d:%d:%s:%s:%s\n", pw->pw_name, crypted,
              pw->pw_uid, pw->pw_gid, pw->pw_gecos, pw->pw_dir, pw->pw_shell);
      found=1;
    }

  done:
    fputs(line, newpasswd);
  }

  fclose(passwd);
  fclose(newpasswd);

  if ( !found )
  {
    unlink(new);
    return -1;
  }

  unlink(bak);
  if (rename(old, bak))
    return -1;
  if (rename(new, old))
    return -1;

  return 0;
}

void setpwent(void)
{
}

void endpwent(void)
{
}



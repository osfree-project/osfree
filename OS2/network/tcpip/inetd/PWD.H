#ifndef _PWD_INCLUDED /* allow multiple inclusions */
#define _PWD_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#  ifndef __WATCOMC__
#  ifndef _GID_T
#    define _GID_T
     typedef long gid_t;
#  endif /* _GID_T */

#  ifndef _UID_T
#    define _UID_T
     typedef long uid_t;
#  endif /* _UID_T */
#endif

   struct passwd {
        char    *pw_name;
        char    *pw_passwd;
        uid_t   pw_uid;
        gid_t   pw_gid;
        char    *pw_age;
        char    *pw_comment;
        char    *pw_gecos;
        char    *pw_dir;
        char    *pw_shell;
        long    pw_audid;
        int     pw_audflg;
   };

     extern struct passwd *getpwuid(uid_t);
     extern struct passwd *getpwnam(const char *);
     extern int setpwnam(const char *, const char *);
     extern int getuid(void);
     extern void endpwent(void);
     extern void setpwent(void);

   struct s_passwd {
       char    *pw_name;
       char    *pw_passwd;
       char    *pw_age;
       long     pw_audid;
       int     pw_audflg;
   };

/*
     extern struct passwd *getpwent(void);
     extern struct passwd *fgetpwent(FILE *);
     extern struct s_passwd *getspwent(void);
     extern struct s_passwd *getspwuid(int);
     extern struct s_passwd *getspwaid(int);
     extern struct s_passwd *getspwnam(char *);
     extern struct s_passwd *fgetspwent(FILE *);
*/

#  ifndef UID_NOBODY            /* Uid of NFS "nobody". */
#     define UID_NOBODY ((unsigned short) 0xfffe)
#  endif

#ifdef __cplusplus
}
#endif

#endif /* _PWD_INCLUDED */


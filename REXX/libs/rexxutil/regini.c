/* System information functions for regutil
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is regutil.
 *
 * The Initial Developer of the Original Code is Patrick TJ McPhee.
 * Portions created by Patrick McPhee are Copyright © 1998, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/regini.c,v 1.3 2004/08/21 14:48:39 prokushev Exp $
 */
#include "regutil.h"

#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
# include <sys/utsname.h>
# include <sys/param.h>
# include <sys/time.h>
# include <sys/resource.h>
# include <poll.h>
# include <fcntl.h>
# include <errno.h>
# ifdef USE_STATFS
#  if defined(INCL_MOUNT)
#   include <sys/mount.h>
#  elif defined(INCL_STATVFS)
#   include <sys/statvfs.h>
#   define statfs statvfs
#   define f_bsize f_frsize
#  elif defined(INCL_VFS)
#   include <sys/vfs.h>
#  endif
# endif
#endif

/* ******************************************************************** */
/* ***************************** Registry ***************************** */
/* ******************************************************************** */

static const char notimp[] = "not implemented",
                   error[] = "ERROR:";
#ifdef _WIN32
static const int list_keys(const char * const inifile, const char * const app,
                           chararray * ca)
{
   char buf[32768], *cp;
   int l, rc;

   rc = !GetPrivateProfileSection(app, buf, sizeof(buf), inifile);
   for (cp = buf; cp; cp += l+1)
      cha_addstr(ca, cp, l = strlen(cp));

   return rc;
}

static const int list_apps(const char * const inifile, chararray * ca)
{
   char buf[32768], *cp;
   int l, rc;

   rc = !GetPrivateProfileSectionNames(buf, sizeof(buf), inifile);
   for (cp = buf; cp; cp += l+1)
      cha_addstr(ca, cp, l = strlen(cp));

   return rc;
}

static const int delete_key(const char * const inifile, const char * const app,
                            const char * key)
{
   int rc = !WritePrivateProfileString(app, key, NULL, inifile);
   return rc;
}

static const int delete_app(const char * const inifile, const char * const app)
{
   int rc = !WritePrivateProfileSection(app, NULL, inifile);
   return rc;
}

static const int set_key(const char * const inifile, const char * const app,
                         const char * key, const char * const val)
{
   int rc = !WritePrivateProfileString(app, key, val, inifile);

   return rc;
}

static const int get_key(const char * const inifile, const char * const app,
                         const char * key, PRXSTRING result)
{
   result->strlength = GetPrivateProfileString(app, key, error, result->strptr,
                                               DEFAULTSTRINGSIZE, inifile);

   return 0;
}
#else
#include "inifile.h"


static int list_keys(const char * const inifile, const char * const app,
                           chararray * ca)
{
   char ** keys;
   inif_t fit = ini_open(inifile);
   int count;
   register int i;

   if (fit) {
      keys = ini_enum_val(fit, app, &count);
      for (i = 0; i < count; i++) {
         cha_addstr(ca, keys[i], strlen(keys[i]));
      }
      free(keys);

      /* don't ini_close(fit), since it's mostly harmless and much faster */
   }

   return 0;
}

static int list_apps(const char * const inifile, chararray * ca)
{
   char ** apps;
   inif_t fit = ini_open(inifile);
   int count;
   register int i;

   if (fit) {
      apps = ini_enum_sec(fit, &count);
      for (i = 0; i < count; i++) {
         cha_addstr(ca, apps[i], strlen(apps[i]));
      }
      free(apps);
      /* don't ini_close(fit), since it's mostly harmless and much faster */
   }

   return 0;
}

static int delete_key(const char * const inifile, const char * const app,
                            const char * key)
{
   inif_t fit = ini_open(inifile);

   if (fit) {
      ini_del_val(fit, app, key);
      /* don't ini_close(fit), since it's mostly harmless and much faster */
   }

   return 0;
}

static int delete_app(const char * const inifile, const char * const app)
{
   inif_t fit = ini_open(inifile);

   if (fit) {
      ini_del_sec(fit, app);
      /* don't ini_close(fit), since it's mostly harmless and much faster */
   }

   return 0;
}

static int set_key(const char * const inifile, const char * const app,
                         const char * key, const char * const val)
{
   inif_t fit = ini_open(inifile);

   if (fit) {
      ini_set_val(fit, app, key, val);
      /* don't ini_close(fit), since it's mostly harmless and much faster */
   }

   return 0;
}

static int get_key(const char * const inifile, const char * const app,
                         const char * key, PRXSTRING result)
{
   inif_t fit = ini_open(inifile);
   char * s;

   if (fit) {
      s = ini_get_val(fit, app, key);
      /* don't ini_close(fit), since it's mostly harmless and much faster */

      if (!s) {
         result->strlength = sizeof(error)-1;
         memcpy(result->strptr, error, sizeof(error)-1);
      }
      else {
         rxresize(result, strlen(s));
         memcpy(result->strptr, s, result->strlength);
      }
   }

   return 0;
}
#endif




/* sysini([inifile],app,key,val,stem--)
 *  allowable combinations:
 *    app, key, and value -- set app.key to value
 *    app, key, value='DELETE:' -- delete app.key
 *    app and key -- return the value of app.key
 *    app, key='DELETE:' -- delete the app
 *    app, key='ALL:', stem -- query key names for app
 *    app = 'ALL:', stem -- query app names for ini file
 */
rxfunc(sysini)
{
   static const char all[] = "ALL:", delete[] = "DELETE:";
   char * inifile, *app, *key, *val;
   chararray * ca;
   PRXSTRING stem = NULL;
   int rc = 0, rcc = 0;

   checkparam(2, 4);

   if (argv[1].strlength == 0)
      return BADARGS;

   if (argv[0].strlength > 0)
      rxstrdup(inifile, argv[0]);
   else
      inifile = NULL;

   rxstrdup(app, argv[1]);
   strupr(app);
   
   if (argc > 2 && argv[2].strlength > 0)
   {
      rxstrdup(key, argv[2]);
      strupr(key);
   }
   else
      key = NULL;

   if (argc > 3 && argv[3].strlength > 0)
      rxstrdup(val, argv[3]);
   else
      val = NULL;

   result->strlength = 0;

   /* test for enumeration */
   if (key && !strcmp(key, all)) {
      /* must have a stem */
      if (argc < 4)
         rcc = 1;

      else {
         ca = new_chararray();
         rcc = list_keys(inifile, app, ca);
         stem = argv + 3;
      }
   }
   else if (!strcmp(app,all)) {
      if (argc != 3)
         rcc = 1;
      else {
         ca = new_chararray();
         rcc = list_apps(inifile, ca);
         stem = argv + 2;
      }
   }

   /* set or delete a value */
   else if (argc == 4) {
      if (!strcasecmp(val, delete))
         rcc = delete_key(inifile, app, key);
      else
         rcc = set_key(inifile, app, key, val);
   }

   /* set or delete an app */
   else if (!key || !strcmp(key, delete)) {
      rcc = delete_app(inifile, app);
   }


   /* the only thing left is to retrieve a value */
   else  {
      rcc = get_key(inifile, app, key, result);
   }

   if (stem) {
      setastem(stem, ca);
      delete_chararray(ca);
   }

   if (rcc) {
      memcpy(result->strptr, error, sizeof(error));
      result->strlength = sizeof(error)-1;
   }

   return rc;
} 


/* ******************************************************************** */
/* ************************ System Parameters ************************* */
/* ******************************************************************** */


#if _WIN32

/* for Win32, return the drive with the windows directory.
 * for Unix, return the name of the kernel file */
rxfunc(sysbootdrive)
{
   GetSystemDirectory(result->strptr, DEFAULTSTRINGSIZE);
   result->strlength = 2;
   return 0;
}


/* syswinver() */
rxfunc(syswinver)
{
   OSVERSIONINFO osvi;

   memset(&osvi, 0, sizeof(osvi));
   osvi.dwOSVersionInfoSize = sizeof(osvi);

   if (!GetVersionEx(&osvi))
   result->strlength = sprintf(result->strptr, "error %d", GetLastError());
   else
   result->strlength = sprintf(result->strptr, "%s %d.%02d", osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ? "Windows95" : "WindowsNT",
                               osvi.dwMajorVersion, osvi.dwMinorVersion);
   return 0;
} 
#else

#ifdef HAS_GETBOOTFILE
# include <paths.h>
#endif

/* for Win32, return the drive with the windows directory.
 * for Unix, return the name of the kernel file */
rxfunc(sysbootdrive)
{
#ifdef HAS_GETBOOTFILE
   strcpy(result->strptr, getbootfile());
   result->strlength = strlen(result->strptr);
#else
   memcpy(result->strptr, "/vmunix", 7);
   result->strlength = 7;
#endif
   return 0;
}


/* syswinver() */
rxfunc(syswinver)
{
   struct utsname un;

   uname(&un);
   
   result->strlength = sprintf(result->strptr, "%s %s.%s", un.sysname,
                               un.version, un.release);
    return 0;
} 
#endif

rxfunc(sysos2ver)
{
  return syswinver(fname, argc, argv, pSomething, result);
} 

rxfunc(syslinver)
{
  return syswinver(fname, argc, argv, pSomething, result);
} 

rxfunc(sysversion)
{
  return syswinver(fname, argc, argv, pSomething, result);
} 


rxfunc(sysutilversion)
{
   /* not sure what number to return. I'm not strictly compatible with IBM
    * 2.00, so I say 1.26. I'll always make my version numbers different
    * from IBM's (unless I actually catch up on the implementation) */
   static const char version[] = "1.26";

   memcpy(result->strptr, version, sizeof(version)-1);
   result->strlength = sizeof(version)-1;
   return 0;
}


/* sysqueryprocess('PID'|'TID'|'PPRIO'|'TPRIO'|'PTIME'|'TTIME') */
rxfunc(sysqueryprocess)
{
   char * arg;

   checkparam(1,1);

   rxstrdup(arg, argv[0]);
   strupr(arg);
#ifdef _WIN32
   if (!strcmp(arg, "PID")) {
      result->strlength = sprintf(result->strptr, "%u", GetCurrentProcessId());
   }
   /* for now, assume no threads ... */
   else if (!strcmp(arg, "TID")) {
      result->strlength = sprintf(result->strptr, "%u", GetCurrentThreadId());
   }
#else
   if (!strcmp(arg, "PID")) {
      result->strlength = sprintf(result->strptr, "%u", getpid());
   }
   /* for now, assume no threads ... */
   else if (!strcmp(arg, "TID")) {
      result_zero();
   }
#endif
   /* I don't want to give the impression that I don't care, but if this
    * matters, why are you writing your application in an interpreted
    * language? */
   else if (!strcmp(arg, "PPRIO")) {
      memcpy(result->strptr, "NORMAL", 6);
      result->strlength = 6;
   }
   else if (!strcmp(arg, "TPRIO")) {
      memcpy(result->strptr, "NORMAL", 6);
      result->strlength = 6;
   }
   else if (!strcmp(arg, "PTIME")) {
      result->strlength = sprintf(result->strptr, "%lu", clock());
   }
   else if (!strcmp(arg, "TTIME")) {
      result->strlength = sprintf(result->strptr, "%lu", clock());
   }
   else
      return BADARGS;

   return 0;
}


/* sysdriveinfo(drive)
 * Under Unix, drive can be any directory name
 */
#ifdef _WIN32
rxfunc(sysdriveinfo)
{
    char dirname[4], freespace[20], label[50];
    int rc = 1;
    unsigned long spc, bps, fc, c;

    checkparam(1,1);

    if (argv[0].strlength >= 3) {
       memcpy(dirname, argv[0].strptr, 3);
    }
    else if (argv[0].strlength > 0) {
       dirname[0] = argv[0].strptr[0];
       dirname[1] = ':';
       dirname[2] = '\\';
    }
    else {
       return BADARGS;
    }

    dirname[3] = 0;

    GetVolumeInformation(dirname, label, sizeof(label), &spc, &bps, &rc, freespace, sizeof(freespace));

    /* query drive info */
    rc = GetDiskFreeSpace(dirname, &spc, &bps, &fc, &c);

    if (!rc) {
	result->strlength = sprintf(result->strptr, "%d", GetLastError());
    }

    /* we round to the nearest 1k to avoid overflows of 32-bit integers */
    else {
#ifndef SYSDI_RETURNS_BYTES

       bps *= spc;

       /* possibly faster version of bps % 1024 */
       if (!(bps & 0x3ff)) {
          bps /= 1024;
       }
       /* bps % 512 */
       else if (!(bps & 0x1ff)) {
          bps /= 512;
          fc /= 2;
          c /= 2;
       }
       else if (!(bps & 0xff)) {
          bps /= 256;
          fc /= 4;
          c /= 4;
       }

       result->strlength = sprintf(result->strptr, "%s %lu %lu %s", dirname, fc*bps, c*bps, label);
#else
       result->strlength = sprintf(result->strptr, "%s %.0f %.0f %s", dirname, (double)fc*bps*spc, (double)c*bps*spc, label);
#endif

    }

    return 0;
} 

/*
NT: Returns drive information in the form: drive:  free total label. 
Unix: Returns drive information in the form: mount-point free total device.
 
free & total disk space are given in kb
*/

/* sysdrivemap([drive],[opt]) */
rxfunc(sysdrivemap)
{
   char buf[1024];
   int bufl, type, options;
   char * opt;
   register int i,j,c = 'C';

   checkparam(0,2);

   if (argc > 0 && argv[0].strlength > 0) {
      c = toupper(argv[0].strptr[0]);
   }

   if (argc > 1 && argv[1].strlength > 0) {
      rxstrdup(opt, argv[1]);
      strupr(opt);

      if (!strcmp(opt, "USED"))
         options = -1;
      else if (!strcmp(opt, "FREE"))
         options = -2;
      else if (!strcmp(opt, "LOCAL"))
         options = -3;
      else if (!strcmp(opt, "REMOTE"))
         options = DRIVE_REMOTE;
      else if (!strcmp(opt, "REMOVABLE"))
         options = DRIVE_REMOVABLE;
      else if (!strcmp(opt, "CDROM"))
         options = DRIVE_CDROM;
      else if (!strcmp(opt, "RAMDISK"))
         options = DRIVE_RAMDISK;
      else
         return BADARGS;
   }

   /* get this -- there are probably 800,000 functions that take a buffer
    * and its size as arguments, in that order, and then there's this one */
   bufl = GetLogicalDriveStrings(sizeof(buf), buf);

   memset(result->strptr, ' ', DEFAULTSTRINGSIZE);

   if (options == -2) {
      for (i = j = 0; i < bufl; i += 4, c++) {
         while (c < buf[i]) {
            result->strptr[j++] = c;
            result->strptr[j] = ':';
            j += 2;
            c++;
         }
      }

      while (c <= 'Z') {
         result->strptr[j++] = c;
         result->strptr[j] = ':';
         j += 2;
         c++;
      }
   }

   else
      for (i = j = 0; i < bufl; i += 4) {
         /* ignore drives which are below the map */
         if (buf[i] < c)
            continue;

         type = GetDriveType(buf+i);

         if ((options == -1) ||
             (options == -3 && type != DRIVE_REMOTE) ||
             (options == type)) {
            memcpy(result->strptr+j, buf+i, 2);
            j += 3;
         }
      }

   result->strlength = j - (j != 0);
   return 0;
} 

#else
rxfunc(sysdriveinfo)
{
   char * dirname;
   register unsigned free, total, bsize;
#ifdef USE_STATFS
   struct statfs sfs;

#ifdef HAS_F_MNTFROMNAME
# define MNTFRMNAME sfs.f_mntfromname
# define MNTONNAME sfs.f_mntonname
#else
# define MNTFRMNAME dirname
# define MNTONNAME dirname
#endif
#endif

   checkparam(1,1);

#ifdef USE_STATFS
   rxstrdup(dirname, argv[0]);

   /* query drive info */
    if (statfs(dirname, &sfs) == -1) {
	result->strlength = 0;
    }
    else {
       free = sfs.f_bavail;   /* report space available to ordinary users */
       total = sfs.f_blocks;
       bsize = sfs.f_bsize;

#ifndef SYSDI_RETURNS_BYTES
       /* report space in 1k blocks to avoid blowing up the integer */
       if (!(bsize % 1024))
          bsize /= 1024;
       else if (!(bsize % 512)) {
          bsize /= 512;
          free /= 2;
          total /= 2;
       }
       else if (!(bsize % 256)) {
          bsize /= 256;
          free /= 4;
          total /= 4;
       }
       else {
          free /= 1024;
          total /= 1024;
       }

       /* 24: 10x2 for the disk space + 3 for spaces and 1 for a null */
       rxresize(result, strlen(MNTONNAME)+strlen(MNTFRMNAME)+24);

       result->strlength = sprintf(result->strptr, "%s %u %u %s",
                MNTONNAME, free*bsize, total*bsize, MNTFRMNAME);

#else
       /* 44: 20x2 for the disk space + 3 for spaces and 1 for a null */
       rxresize(result, strlen(MNTONNAME)+strlen(MNTFRMNAME)+44);

       result->strlength = sprintf(result->strptr, "%s %.0f %.0f %s",
                MNTONNAME, (double)free*bsize, (double)total*bsize, MNTFRMNAME);
#endif
    }
#else
    result->strlength = 0;
#endif

    return 0;
} 

/*
NT: Returns drive information in the form: drive:  free total label. 
Unix: Returns drive information in the form: mount-point free total device.
 
free & total disk space are given in kb
*/

/* sysdrivemap([drive],[opt]) */
rxfunc(sysdrivemap)
{
   strcpy(result->strptr, notimp);
   result->strlength = sizeof(notimp)-1;
   return 0;
} 


#endif

#ifndef PRIO_MIN
# define PRIO_MIN -20
# define PRIO_MAX 20
#endif

/* syssetpriority(class,delta)
 * class indicates the priority class: 0 means no change, 1 means idle time
 * only, 2 means normal, 3 means time critical, 4 means `real time'
 * anyway, none of that has a clear analog in Unix, so it doesn't do anything.
 * delta is the nice level, from -20 to 20. In Unix, -20 is the highest
 * priority, so the nice level is inverted.
 */
rxfunc(syssetpriority)
{
   char * sclass, *sprio;
   int class, prio;
   int rc;

   checkparam(2, 2);
   rxstrdup(sclass, argv[0]);
   rxstrdup(sprio, argv[1]);
   class = atoi(sclass);
   prio = atoi(sprio);

   if (class < 0 || class > 5 || prio < PRIO_MIN || prio > PRIO_MAX)
      return BADARGS;

#ifndef _WIN32
   rc = setpriority(PRIO_PROCESS, 0, -prio);
#else
   switch (class) {
      case 1: class = IDLE_PRIORITY_CLASS; break;
      case 2: class = NORMAL_PRIORITY_CLASS; break;
      case 3: class = REALTIME_PRIORITY_CLASS; break;
      case 4: class = HIGH_PRIORITY_CLASS; break;
   }

   if (class) {
      rc = SetPriorityClass(GetCurrentProcess(), class);
   }

   rc = SetThreadPriority(GetCurrentThread(), prio);
#endif

   result->strlength = sprintf(result->strptr, "%d", rc);
   return 0;
} 


#ifdef _WIN32
int sleep(unsigned int secs)
{
   Sleep(secs*1000);
   return 0;
}


/* sleep for a certain number of microseconds */
int usleep(unsigned int usecs)
{
   /* win32 doesn't let you usleep -- but it lets you millisleep, which is
    * still pretty good. */
   Sleep(usecs/1000);
   return 0;
}
#endif

/* syssleep(secs) -- go to sleep for secs seconds. */
rxfunc(syssleep)
{
    int secs;
    int rc = 0;

    checkparam(1,1);

    secs = rxint(argv);

    if (secs)
	rc = sleep(secs);

    /* handle microseconds and call usleep */
    if (!rc) {
	secs = rxuint(argv);
	if (secs)
	    usleep(secs);
    }

    if (!rc) {
	result_zero();
    }
    else {
	/* do something with the errno */
	result->strlength = sprintf(result->strptr, "%d", rc);
    }

    return 0;
} 
 

#ifdef _WIN32

/* sysswitchsession(name) */
rxfunc(sysswitchsession)
{
   char * name;
   HWND hwnd;

   checkparam(1,1);

   rxstrdup(name, argv[0]);

   hwnd = FindWindow(NULL, name);

   if (hwnd == INVALID_HANDLE_VALUE) {
      result_zero();
   }
   else {
      result->strlength = sprintf(result->strptr, "%d", SetForegroundWindow(hwnd));
   }

   return 0;
} 
#else
rxfunc(sysswitchsession)
{
   strcpy(result->strptr, notimp);
   result->strlength = sizeof(notimp)-1;
   return 0;
} 
#endif


/* syssystemdirectory() */
rxfunc(syssystemdirectory)
{
#ifdef _WIN32
   result->strlength = GetSystemDirectory(result->strptr, DEFAULTSTRINGSIZE);
#else
   memcpy(result->strptr, "/etc", 4);
   result->strlength = 4;
#endif

   return 0;
} 

#ifdef _WIN32

/* sysvolumelabel("drive") */
rxfunc(sysvolumelabel)
{
   char dirname[3], label[50], sysname[50];
   int serial, maxlen, flags;

   checkparam(0,1);

   if (argc > 0 && argv[0].strlength) {
      dirname[0] = argv[0].strptr[0];
   }
   else {
      GetCurrentDirectory(sizeof(label), label);
      dirname[0] = label[0];
   }

   dirname[1] = ':';
   dirname[2] = 0;

   label[0] = 0;
   GetVolumeInformation(dirname, label, sizeof(label), &serial, &maxlen, &flags, sysname, sizeof(sysname));

   strcpy(result->strptr, label);
   result->strlength = strlen(label);
   return 0;
} 

/* SysWaitNamedPipe(name, [timeout]) */
rxfunc(syswaitnamedpipe)
{
   char * name, *tos;
   int timeout;

   checkparam(1,2);

   rxstrdup(name, argv[0]);
   if (argc > 1) {
      rxstrdup(tos, argv[1]);
      timeout = atoi(tos);
   }
   else {
      timeout = INFINITE;
   }

   result->strlength = sprintf(result->strptr, "%d", WaitNamedPipe(name, timeout));
   return 0;
} 
#else
/* sysvolumelabel("drive") */
rxfunc(sysvolumelabel)
{
   strcpy(result->strptr, notimp);
   result->strlength = sizeof(notimp)-1;
   return 0;
} 


rxfunc(syswaitnamedpipe)
{
   char * name, *tos;
   int rc, timeout;
   struct pollfd pfd;

   checkparam(1,2);

   rxstrdup(name, argv[0]);
   if (argc > 1) {
      rxstrdup(tos, argv[1]);
      timeout = atoi(tos);
   }
   else {
      timeout = -1;
   }

   pfd.fd = open(name, O_RDONLY);

   if (pfd.fd == -1) {
      rc = errno;
   }
   else {
      pfd.events = POLLIN;
      pfd.revents = 0;
      
      rc = poll(&pfd, 1, timeout);

      if (rc == -1) {
         rc = errno;
      }
      else {
         /* either it timed out or it's readable */
         if (pfd.revents & POLLIN) {
            rc = 0;
         }
         else {
            rc = 1460;  /* defined as ERROR_TIMEOUT in winerror.h */
         }
      }

      close(pfd.fd);
   }


   result->strlength = sprintf(result->strptr, "%d", rc);
   return 0;
} 

/* tack these in here for now. I want to implement rxmessagebox for Unix,
 * but I probably need a configure script to do it without requiring the
 * X windows headers, so I ignore it for now */
rxfunc(rxmessagebox)
{
   strcpy(result->strptr, notimp);
   result->strlength = sizeof(notimp)-1;
   return 0;
}


rxfunc(rxwinexec)
{
   strcpy(result->strptr, notimp);
   result->strlength = sizeof(notimp)-1;
   return 0;
}


#endif

/* sysgeterrortext(error_number) */
rxfunc(sysgeterrortext)
{
   int rc;
   char * rcs;

   checkparam(1,1);

   rxstrdup(rcs, argv[0]);

   rc = atoi(rcs);

#if defined(_WIN32)
   result->strlength = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, rc,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *)result->strptr,
                 DEFAULTSTRINGSIZE, NULL);
#elif defined(THREAD_SAFE)
   if (!strerror_r(rc, result->strptr, DEFAULTSTRINGSIZE)) {
      result->strlength = strlen(result->strptr);
   }
   else {
      result->strlength = 0;
   }
#else
   if (rcs = strerror(rc)) {
      result->strlength = strlen(rcs);
      memcpy(result->strptr, rcs, result->strlength);
   }
   else {
      result->strlength = 0;
   }
#endif

   return 0;
}

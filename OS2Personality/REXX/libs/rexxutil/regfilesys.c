/* File system functions for regutil
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
 * Portions created by Patrick McPhee are Copyright © 1999, 2001
 * Patrick TJ McPhee. All Rights Reserved.
 *
 * Contributors:
 *
 * $Header: /netlabs.cvs/osfree/src/REXX/libs/rexxutil/regfilesys.c,v 1.3 2004/08/21 14:48:39 prokushev Exp $
 */
#include "regutil.h"
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#ifndef _WIN32
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/param.h>
# ifndef AIX
#  include <sys/mount.h>
# endif
# include <sys/stat.h>
# include <dirent.h>
# include <fnmatch.h>
# include <limits.h>
#else
# include <sys/utime.h>
# include <io.h>
# define MAX_USHORT 65535 /* or so they say */
# define MAXPATHLEN _MAX_PATH
# define F_OK 0
#endif

/* ******************************************************************** */
/* ********************* File System Interaction ********************** */
/* ******************************************************************** */

static int errnotorc(const int eno)
{
   int rc;

   switch (eno) {
      case ENOTDIR: rc = 3; break;
      case EINVAL: rc = 87; break;
      case ENAMETOOLONG: rc = 206; break;
      case ENOENT: rc = 2; break;
      case ENOSPC:
      case EROFS: rc = 108; break;
#ifdef EDQUOT
      case EDQUOT:
#endif
      case EEXIST:
#ifdef EACCESS
      case EACCESS:
#endif
      case EPERM: rc = 5; break;
#ifdef ELOOP
      case ELOOP: rc = 36; break;
#endif
      case EBUSY: rc = 32; break;
      /* if we get these, something is really wrong */
      case EIO:
      case EFAULT: rc = -1; break;
      case 0: return 0; break;
      default: rc = 1;
   }

   return rc;
}

/* SysFileDelete(file) */
rxfunc(sysfiledelete)
{
    char * filename;
    int rc;

    checkparam(1,1);

    rxstrdup(filename, argv[0]);

    rc = remove(filename);

    if (!rc) {
	result_zero();
    }
    else {
	/* do something with the errno */
	rc = errnotorc(errno);
        if (rc < 0)
           return BADGENERAL;
	result->strlength = sprintf(result->strptr, "%d", rc);
    }

    return 0;
} 


char * mapfile(const char * name, int * len);
int unmapfile(char *buf, int size);
/* SysFileSearch(target,file,stem, [options]) */
rxfunc(sysfilesearch)
{
    unsigned char *buf, * dptr, *bol, *eol, *eof, *filename, *options,
                  *lbuf = NULL;
    PRXSTRING stem;
    RXSTRING target;
    register int i, lines = 0, matched;
    rxbool obeycase = false, shownumbers = false;
    int rc = 0, len;
    chararray *array;

    checkparam(3,4);

    target = argv[0];
    rxstrdup(filename, argv[1]);
    stem = argv+2;

    /* options are c: case sensitive search, and n: show line numbers */
    if (argc > 3) {
	rxstrdup(options, argv[3]);
        strlwr(options);
	if (strchr(options, 'c'))
	    obeycase = true;
	if (strchr(options, 'n'))
	    shownumbers = true;
    }

    /* map the file into memory. Note that if the OS doesn't support memory-
     * mapped I/O, this allocates a buffer and reads the file into it. */
    if (!(buf = mapfile(filename, &len))) {
	rc = 3;
    }
    else {
        eof = buf+len;

	if (!obeycase) {
           rxstrdup(target.strptr, argv[0]);
           strlwr(target.strptr);
	}

	array = new_chararray();
	if (array == NULL)
	    rc = 2;

	/* pick off each line and search it. lines are terminated with
         * new-line. don't worry about carriage returns, since we're not
	 * trying to map lines exactly. */
	for (lines = 1, bol = buf; bol < eof && !rc;
	     lines++, bol = eol + 1) {
           eol = memchr(bol, '\n', eof-bol);
           if (!eol) {
              eol = eof;
           }

           if (obeycase) {
              for (dptr = memchr(bol, target.strptr[0], eol-bol), matched = 0;
                   dptr && dptr < (eol - target.strlength) && matched < target.strlength;
                   dptr = memchr(dptr, target.strptr[0], eol-dptr)) {
                 if (!memcmp(dptr, target.strptr, target.strlength)) {
                    matched = target.strlength;
                 }
                 else {
                    dptr++;
                 }
              }
           }
           else {
              for (dptr = bol, matched = 0; matched < target.strlength && dptr <= eol; dptr++) {
                 if (tolower(*dptr) == target.strptr[matched]) {
                    matched++;
                 }
                 else if (matched) {
                    matched = 0;
                 }
              }              
           }

	    /* if the target is matched on this line, add it to the array */
	    if (matched == target.strlength) {
		if (!shownumbers) {
                    i = eol - bol;
                    if (i && bol[i-1] == '\r')
                       i--;
		    if (cha_adddummy(array, bol, i))
			rc = 2;
		}
		else {
		    lbuf = realloc(lbuf, eol-bol+15);
		    if (!lbuf)
			rc = 2;
		    else {
			i = sprintf(lbuf, "%d:", lines);
                        memcpy(lbuf+i, bol, eol-bol);
                        i += eol - bol;
                        if (lbuf[i-1] == '\r')
                           i--;
			if (cha_addstr(array, lbuf, i))
			    rc = 2;
		    }
		}
	    }
	}

        setastem(stem, array);

        delete_chararray(array);

        if (lbuf)
           free(lbuf);

        unmapfile(buf, len);
    }

    result->strlength = sprintf(result->strptr, "%d", rc);

    return 0;
} 


#ifndef _WIN32
# ifndef INITMOUNTNAMES
/* if initmountnames doesn't exist, we'll call the file system ufs and hope
 * for the best */
rxfunc(sysfilesystemtype)
{
   memcpy(result->strptr, "UFS", 3);
   result->strlength = 3;

   return 0;
}

# else
static char * mountnames[] = INITMOUNTNAMES;

/* SysFileSystemType(drive) */
rxfunc(sysfilesystemtype)
{
    char * drive;
    int rc;
    struct statfs sfs;

    checkparam(1, 1);
    rxstrdup(drive, argv[0]);

    /* drive is really any file name here */
    rc = statfs(drive, &sfs);

    if (rc == -1)
    	result->strlength = 0;

    /* this might not be too terribly portable. we may have to do some
     * decoding ourselves. */
    else if (sfs.f_type < DIM(mountnames) && mountnames[sfs.f_type]) {
	strcpy(result->strptr, mountnames[sfs.f_type]);
	result->strlength = strlen(result->strptr);
    }
    else {
	result->strlength = 0;
    }

    return 0;
} 
# endif  /* INITMOUNTNAMES */
#else
rxfunc(sysfilesystemtype)
{
    char drive[4];
    int rc;
    char name[100], fsname[100];
    long flags;
    unsigned long serial, pathlen;

    checkparam(1, 1);

    if (argv[0].strlength < 1 || argv[0].strptr == NULL)
	return BADARGS;

    /* drive must be the root (eg c:\) */
    drive[0] = argv[0].strptr[0];
    memcpy(drive+1, ":\\", 3);

    rc = GetVolumeInformation(drive, name, sizeof(name), &serial, &pathlen,
                              &flags, fsname, sizeof(fsname));

    if (rc) {
	strcpy(result->strptr, fsname);
	result->strlength = strlen(result->strptr);
    }
    else {
	result->strlength = 0;
    }

    return 0;
} 
#endif /* _WIN32 */


#define CR_FILES 1
#define CR_DIRS  2

#define TF_SORTABLE 1
#define TF_SENSIBLE 2


/* return all the files under dir which match the pattern and the criterion.
 * if do_subdirs is true, recurse through subdirectories.
 */

#ifndef _WIN32

static void get_matched_files(chararray * ca, const char * dir,
			      int criterion, const char * const pattern,
                              const char * attrs,
			      rxbool do_subdirs, rxbool name_only, int time_format)
{
    DIR * dirp = opendir(dir);
    struct dirent * thede;
    struct stat st;
    struct tm * tm;
    rxbool may_have_subdirs, filename_matches;
    char pth[MAXPATHLEN], buf[MAXPATHLEN+40], *slash = "/";
    int l, dl, matchattr = 0, skipattr = 0, system_files = 0, check_links = 0;

    if (dirp == NULL) return;

    /* strip out the directory name in the special case where we start in
     * the current directory */
    if (strcmp(dir, ".")) {
	dl = strlen(dir) + 1;
        if (dir[dl-2] == '/')
           slash = "";
    }
    else
	dl = 0;

    /* clever trick: if a directory has 2 links to it, it has no
     * sub-directories. This is because the .. of each sub-directory shows
     * up as a link, as does the . in the directory itself, and the name in
     * the parent directory. Trust me on this. */
    stat(dir, &st);
    may_have_subdirs = st.st_nlink > 2;

    /* if there are no sub-directories & we don't want files, get out */
    if (!may_have_subdirs && !(criterion&CR_FILES)) {
       closedir(dirp);
       return;
    }

    if (attrs) {
       /* The attributes are archive, directory, readonly, hidden,
        * and system. I map them to these meanings:
        *  _A_SYSTEM means the file owns to a process with pid < 10
        *  _A_RDONLY means the write bit is not set for this process
        *  _A_HIDDEN means the read bit is not set for this process
        *  _A_SUBDIR means execute permission is set  (!)
        *  _A_ARCH means a file has more than one link
        *
        * This might change if IBM did something sensible with its Linux
        * version (need to check).
        */

       if (attrs[0] == '+') check_links = 1;
       else if (attrs[0] == '-') check_links = -1;
       if (attrs[1] == '+') matchattr |= X_OK;
       else if (attrs[1] == '-') skipattr |= X_OK;
       if (attrs[2] == '+') skipattr |= R_OK;
       else if (attrs[2] == '-') matchattr |= R_OK;
       if (attrs[3] == '+') skipattr |= W_OK;
       else if (attrs[3] == '-') matchattr |= W_OK;
       if (attrs[4] == '+') system_files = 1;
       else if (attrs[4] == '-') system_files = -1;
    }

    for (thede = readdir(dirp); thede; thede = readdir(dirp)) {

        /* skip the current directory and parent directory */
        if (thede->d_name[0] == '.' &&
            (thede->d_name[1] == 0 ||
             (thede->d_name[1] == '.' && thede->d_name[2] == 0)))
           continue;

        filename_matches = !fnmatch(pattern, thede->d_name, 0);

        /* no need to continue if this doesn't match and we aren't
         * recursing */
        if ((!do_subdirs || !may_have_subdirs) && !filename_matches)
           continue;

	if (dl) {
	    l = sprintf(pth, "%s%s%s", dir, slash, thede->d_name);
	}
	else {
            l = strlen(thede->d_name);
	    memcpy(pth, thede->d_name, l+1);
	}

        /* filter out files which don't match the required permissions
         * we do it here to avoid the overhead of calling stat on
         * non-matches. */
        if (matchattr && access(pth, matchattr))
           continue;
        if (skipattr && !access(pth, skipattr))
           continue;

        /* no need to stat if we don't need to the information */
        if (
#ifndef DT_DIR
            may_have_subdirs ||
#endif
            !name_only || check_links || system_files) {
           /* this is a bit expensive, but it's the only way to portably find
            * the size, date and whether the file is a directory */
           if (lstat(pth, &st) == -1)
               continue;
       }
       else {
#ifdef DT_DIR
          /* also no need to stat when dirent returns the information,
           * as in BSD */
          if (thede->d_type & DT_DIR)
             st.st_mode = S_IFDIR;
          else
             st.st_mode = S_IFREG;
#else
          /* if dirent doesn't return the information, we know at this point
           * that we don't have a directory, and we don't care beyond that
            */
          st.st_mode = S_IFREG;
#endif
       }

        if (check_links > 0 && st.st_nlink < 2)
            continue;
        else if (check_links < 0 && st.st_nlink > 1)
            continue;
        if (system_files > 0 && st.st_uid >= 10)
            continue;
        else if (system_files < 0 && st.st_uid <= 10)
            continue;

        if (!name_only)
           tm = localtime(&st.st_mtime);

	/* we match the pattern only against the file name in the directory
	 * being searched. */
	if (filename_matches && (!S_ISDIR(st.st_mode)) && (criterion&CR_FILES)) {
	    if (name_only) {
		cha_addstr(ca, pth, l);
	    }
	    else {
               char dbuf[30], ftype = '-';

               if (time_format == TF_SORTABLE)
                  sprintf(dbuf, "%4d/%02d/%02d/%02d/%02d", tm->tm_year+1900,
                          tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min);
               else if (time_format == TF_SENSIBLE)
                  sprintf(dbuf, "%4d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900,
                          tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
               else
                  strftime(dbuf, sizeof(dbuf), "%c", tm);

               /* check for selected special files */
               switch (st.st_mode&S_IFMT) {
                  case S_IFIFO: ftype = 'p'; break;
                  case S_IFCHR: ftype = 'c'; break;
                  case S_IFBLK: ftype = 'b'; break;
                  case S_IFLNK: ftype = 'l'; break;
                  case S_IFSOCK: ftype = 's'; break;
               }

	       l = sprintf(buf, "%s %10d %c%c%c%c%c%c%c%c%c%c %s", dbuf, (int)st.st_size, ftype,
		       (S_IRUSR&st.st_mode) ? 'r' : '-', (S_IWUSR&st.st_mode) ? 'w' : '-',
		       (S_IXUSR&st.st_mode) ? 'x' : '-',
		       (S_IRGRP&st.st_mode) ? 'r' : '-', (S_IWGRP&st.st_mode) ? 'w' : '-',
		       (S_IXGRP&st.st_mode) ? 'x' : '-',
		       (S_IROTH&st.st_mode) ? 'r' : '-', (S_IWOTH&st.st_mode) ? 'w' : '-',
		       (S_IXOTH&st.st_mode) ? 'x' : '-', pth);
	       cha_addstr(ca, buf, l);
	   }
	}
	else if (may_have_subdirs && S_ISDIR(st.st_mode)) {
           if (filename_matches && (criterion&CR_DIRS))  {
              if (name_only) {
                 cha_addstr(ca, pth, l);
              }
              else {
                 char dbuf[30];

                 if (time_format == TF_SORTABLE)
                    sprintf(dbuf, "%4d/%02d/%02d/%02d/%02d", tm->tm_year+1900,
                            tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min);
                 else if (time_format == TF_SENSIBLE)
                    sprintf(dbuf, "%4d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900,
                            tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                 else
                    strftime(dbuf, sizeof(dbuf), "%c", tm);

                 l = sprintf(buf, "%s %10d d%c%c%c%c%c%c%c%c%c %s", dbuf, (int)st.st_size,
                             (S_IRUSR&st.st_mode) ? 'r' : '-', (S_IWUSR&st.st_mode) ? 'w' : '-',
			     (S_IXUSR&st.st_mode) ? 'x' : '-',
			     (S_IRGRP&st.st_mode) ? 'r' : '-', (S_IWGRP&st.st_mode) ? 'w' : '-',
			     (S_IXGRP&st.st_mode) ? 'x' : '-',
			     (S_IROTH&st.st_mode) ? 'r' : '-', (S_IWOTH&st.st_mode) ? 'w' : '-',
			     (S_IXOTH&st.st_mode) ? 'x' : '-', pth);

                 cha_addstr(ca, buf, l);
              }
	    }
            if (do_subdirs)
               get_matched_files(ca, pth, criterion, pattern, attrs, do_subdirs, name_only, time_format);
	}
    }

    closedir(dirp);
}
#else
static void get_matched_files(chararray * ca, const char * dir,
			      int criterion, const char * const pattern,
                              const char * attrs,
			      rxbool do_subdirs, rxbool name_only, int time_format)
{
   WIN32_FIND_DATA fd;
   HANDLE sh;
   FILETIME lft;
   SYSTEMTIME syst;
   char pth[MAXPATHLEN], buf[MAXPATHLEN+40], pbuf[10], tdir[MAXPATHLEN], *slash = "";
   BOOL rc = TRUE;
   rxbool really_do_subdirs = false;
   int l, dl, matchattr = 0, skipattr = 0;

   if (!strcmp(dir, ".")) {
      _getcwd(tdir, sizeof(tdir));
      dir = tdir;
      dl = strlen(dir);
      if (dir[dl-1] != '\\' && dir[dl-1] != '/')
         slash = "\\";

      strcpy(pth, pattern);
   }
   else {
      dl = strlen(dir);

      if (dir[dl-1] != '\\' && dir[dl-1] != '/')
         slash = "\\";
      sprintf(pth, "%s%s%s", dir, slash, pattern);
   }

   if (attrs) {
      int aflags[] = {_A_ARCH,_A_SUBDIR,_A_HIDDEN,_A_RDONLY,_A_SYSTEM };

      for (l = 0; l < 5 && attrs[l]; l++) {
         if (attrs[l] == '+') matchattr |= aflags[l];
         else if (attrs[l] == '-') skipattr |= aflags[l];
      }
   }

    /* if the pattern is anything other than *, and sub-directory searching
     * is on, we will miss sub-directories which don't miss the pattern. So,
     * if this happens, we pretend we don't really want to do
     * sub-directories, then later on we get all the sub-directories and do
     * each one in turn. This means that if the search is for *, we do a
     * depth-first search, but if it's for something else, we do breadth-
     * first */
   if (do_subdirs && memcmp(pattern, "*", 2)) {
      really_do_subdirs = true;
      do_subdirs = false;
   }

   for (sh = FindFirstFile(pth, &fd);
        rc && sh != INVALID_HANDLE_VALUE;
        rc = FindNextFile(sh, &fd)) {
      if (!strcmp(".", fd.cFileName) || !strcmp("..", fd.cFileName))
         continue;

      /* set up the pth variable now, in case it's a directory &
       * do_subdirs is true */
      if (((criterion&CR_FILES) && !(fd.dwFileAttributes&_A_SUBDIR)) ||
          (((criterion&CR_DIRS) || do_subdirs) && (fd.dwFileAttributes&_A_SUBDIR))) {

         l = sprintf(pth, "%s%s%s", dir, slash, fd.cFileName);
      }

      if ( (((criterion&CR_FILES) && !(fd.dwFileAttributes&_A_SUBDIR)) ||
           ((criterion&CR_DIRS) && (fd.dwFileAttributes&_A_SUBDIR))) &&
           (!matchattr || (matchattr&fd.dwFileAttributes)) &&
           (!(skipattr&fd.dwFileAttributes)) ) {

         if (name_only) {
             cha_addstr(ca, pth, l);
         }
         else {
            FileTimeToLocalFileTime(&fd.ftLastWriteTime, &lft);
            FileTimeToSystemTime(&lft, &syst);
            if (time_format == TF_SORTABLE)
            l = sprintf(buf, "%4d/%02d/%02d/%02d/%02d  %10d  %c%c%c%c%c %s", syst.wYear,
                          syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, (int)fd.nFileSizeLow,
                          (fd.dwFileAttributes&_A_ARCH) ? 'A' : '-',
                          (fd.dwFileAttributes&_A_SUBDIR) ? 'D' : '-',
                          (fd.dwFileAttributes&_A_HIDDEN) ? 'H' : '-',
                          (fd.dwFileAttributes&_A_RDONLY) ? 'R' : '-',
                          (fd.dwFileAttributes&_A_SYSTEM) ? 'S' : '-',
                          pth);
            else if (time_format == TF_SENSIBLE)
            l = sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d  %10d  %c%c%c%c%c %s", syst.wYear,
                          syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond, (int)fd.nFileSizeLow,
                          (fd.dwFileAttributes&_A_ARCH) ? 'A' : '-',
                          (fd.dwFileAttributes&_A_SUBDIR) ? 'D' : '-',
                          (fd.dwFileAttributes&_A_HIDDEN) ? 'H' : '-',
                          (fd.dwFileAttributes&_A_RDONLY) ? 'R' : '-',
                          (fd.dwFileAttributes&_A_SYSTEM) ? 'S' : '-',
                          pth);
            else {
            int hour = syst.wHour%12;
            if (!hour) hour = 12;
            l = sprintf(buf, "%2d/%02d/%02d  %2d:%02d%c  %10d  %c%c%c%c%c  %s", syst.wMonth,
                          syst.wDay, syst.wYear%100, hour, syst.wMinute, syst.wHour >=12 ? 'p' : 'a',
                          (int)fd.nFileSizeLow,
                          (fd.dwFileAttributes&_A_ARCH) ? 'A' : '-',
                          (fd.dwFileAttributes&_A_SUBDIR) ? 'D' : '-',
                          (fd.dwFileAttributes&_A_HIDDEN) ? 'H' : '-',
                          (fd.dwFileAttributes&_A_RDONLY) ? 'R' : '-',
                          (fd.dwFileAttributes&_A_SYSTEM) ? 'S' : '-',
                          pth);
            }
            cha_addstr(ca, buf, l);
         }
     }
     if (do_subdirs && (fd.dwFileAttributes&_A_SUBDIR))
        get_matched_files(ca, pth, criterion, pattern, attrs, do_subdirs, name_only, time_format);

  }

  if (sh != INVALID_HANDLE_VALUE)
      FindClose(sh);

  /* need to recurse with a different pattern to get the sub-directories,
   *  then search each of them with the real pattern */
  if (really_do_subdirs) {
     chararray * dirs = new_chararray();
     register int i;

     if (!dirs)
        return;

     get_matched_files(dirs, dir, CR_DIRS, "*", "", true, true, true);
     for (i = 0; i < dirs->count; i++) {
        get_matched_files(ca, dirs->array[i].strptr, criterion, pattern, attrs, false, name_only, time_format);
     }
     delete_chararray(dirs);
  }
}

#endif

/* SysFileTree(filespec,stem, [options], [tattrib], nattrib]) */
rxfunc(sysfiletree)
{
    char * pattern, *dir, *options, *attrs = NULL;
    int criterion = 0;
    rxbool do_subdirs=false, name_only=false;
    int time_format = 0;
    chararray * files;
    int rc = 0;
# ifdef _WIN32
    char * bpattern;
# else
    char * cp1, *cp2;
# endif

    checkparam(2, 5);

    rxstrdup(pattern, argv[0]);

    if (argc > 2) {
	rxstrdup(options, argv[2]);
	strupr(options);
	while (*options) {
	    switch (*options) {
		case 'F': criterion |= CR_FILES; break;
		case 'D': criterion |= CR_DIRS; break;
		case 'B': criterion = CR_FILES|CR_DIRS; break;
		case 'S': do_subdirs = true; break;
		case 'T': time_format = TF_SORTABLE; break;
		case 'L': time_format = TF_SENSIBLE; break;
		case 'O': name_only = true;
		default: /* ignore invalid options */ ;
	    }
	options++;
	}
    }

    if (!criterion)
	criterion = CR_FILES|CR_DIRS;

    /* file attributes -- these are supported for matching only, not
     * setting */
    if (argc > 3) {
        if (argv[3].strlength == 5)
           rxstrdup(attrs, argv[3]);
        else if (argv[3].strlength < 5) {
           attrs = alloca(6);
           memcpy(attrs, argv[3].strptr, argv[3].strlength);
           memset(attrs+argv[3].strlength, '*', 5 - argv[3].strlength);
           attrs[5] = 0;
        }
        else {
           attrs = alloca(6);
           memcpy(attrs, argv[3].strptr, 5);
           attrs[5] = 0;
        }
     }

    files = new_chararray();

    if (files == NULL)
	rc = 2;

    else {
	/* break the list into directory & pattern, so the input can be of
	 * the form /usr/home/ptjm/*.c */

	dir = pattern;
	pattern = strrchr(dir, '/');
#ifdef _WIN32  /* win32 allows slashes in both directions */
	bpattern = strrchr(dir, '\\');
        if (pattern < bpattern) pattern = bpattern;
#endif
	
	if (pattern) {
            /* handle things like /rgb.txt */
            if (pattern == dir)
               dir = "/";

#ifdef _WIN32
            else if (pattern == (dir+2) && dir[1] == ':') {
               dir = alloca(4);
               memcpy(dir, pattern-2, 3);
               dir[3] = 0;
            }
#endif

            *pattern++ = 0;
	}
	else {
	    pattern = dir;
	    dir = ".";
	}

        /* OS/2 rexxutil treats \dir\ as if it were \dir\* (reported by Ruhsam Bernhard) */
        if (!*pattern)
           pattern = "*";

#ifndef _WIN32
        /* Unix systems require normalisation of the directory */
        if (*dir != '/') {
           cp1 = alloca(MAXPATHLEN+1);
           getcwd(cp1, MAXPATHLEN+1);
           /* don't append `dir' if it's "." */
           if (dir[0] != '.' || dir[1] != 0) {
              strcat(cp1, "/");
              strcat(cp1, dir);
           }

           dir = cp1;
        }

#ifdef HASNT_REALPATH
        /* now get rid of doubled /, embedded .. and . -- this will cause a problem
         * on apollo systems, which treat paths starting with // specially (but
         * I don't think those systems are used much any more) */
        while (cp1 = strstr(dir, "//")) {
           strcpy(cp1, cp1+1);
        }

        while (cp1 = strstr(dir, "/./")) {
           strcpy(cp1, cp1+2);
        }

        /* dir+1 to avoid paths which start with /.. (which are invalid and
         * ought to fail later) */
        while (cp1 = strstr(dir+1, "/../")) {
           cp2 = strrchr(cp1-1, '/');
           strcpy(cp2, cp1+3);
        }

        /* trailing . and .. */
        cp1 = strrchr(dir, '/');
        if (cp1[1] == '.' && cp1[2] == 0) {
           if (cp1 == dir) cp1[1] = 0;
           else cp1[0] = 0;
        }
        else if (cp1[1] == '.' && cp1[2] == '.' && cp1[3] == 0) {
           if (cp1 != dir) {
              for (cp2= cp1 - 1; *cp2 != '/'; cp2--)
                  ;

              if (cp2 != dir) {
                 cp2[0] = 0;
              }
              else {
                 cp2[1] = 0;
              }
           }
        }
#else
        cp1 = alloca(MAXPATHLEN+1);
        dir = realpath(dir, cp1);
        if (!dir) {
           rc = errnotorc(errno);
        }
#endif
#endif

	/* get a list of all files which match the name and file type */
        if (dir)
           get_matched_files(files, dir, criterion, pattern, attrs, do_subdirs, name_only, time_format);
        setastem(argv+1, files);
        delete_chararray(files);
    }

    result->strlength = sprintf(result->strptr, "%d", rc);

    return 0;
}


/* SysMkDir(dirspec) */
rxfunc(sysmkdir)
{
    char * dirname;
    int rc;

    checkparam(1,1);

    rxstrdup(dirname, argv[0]);

#ifdef _WIN32
    rc = mkdir(dirname);
#else
    rc = mkdir(dirname, 0755);
#endif

    if (!rc) {
	result_zero();
    }
    else {
       rc = errnotorc(errno);
       if (rc < 0)
          return BADGENERAL;

       result->strlength = sprintf(result->strptr, "%d", rc);
    }

    return 0;
} 

/* SysRmDir(dirspec) */
rxfunc(sysrmdir)
{
    char * dirname;
    int rc;

    checkparam(1,1);

    rxstrdup(dirname, argv[0]);

    rc = rmdir(dirname);

    if (!rc) {
	result_zero();
    }
    else {
        rc = errnotorc(errno);
        if (rc < 0)
           return BADGENERAL;
        /* the specified RC for a non-existent dir is different for a
         * non-existent file. grr */
        else if (rc == 32)
           rc = 16;

	result->strlength = sprintf(result->strptr, "%d", rc);
    }

    return 0;
} 

 
/* SysSearchPath(path,filename) */
rxfunc(syssearchpath)
{
#ifdef _WIN32
   const char delim[] = ";", pathpat[] = "%s\\%s";
#else
   const char delim[] = ":", pathpat[] = "%s/%s";
#endif
   char *pathseg, *pseg, *rpath, *path, *pathname, *filename;
   int len;

   checkparam(2, 2);

   rxstrdup(pathname, argv[0]);
   rxstrdup(filename, argv[1]);

   /* return empty string if not found */
   result ->strlength = 0;

   rpath = getenv(pathname);

   if (rpath != NULL) {
      len = strlen(rpath) + 1;
      path = alloca(len);
      memcpy(path, rpath, len);

      pathseg = alloca(len+argv[1].strlength+sizeof(pathpat));

      for (pseg = strtok(path, delim); pseg; pseg = strtok(NULL, delim)) {
          len = sprintf(pathseg, pathpat, pseg, filename);

          if (!access(pathseg, F_OK)) {
              rxresize(result, len);
              memcpy(result->strptr, pathseg, len);
              break;
          }
      }
   }

   return 0;
} 
 

/* SysTempFileName(template, [filter]) */
rxfunc(systempfilename)
{
   register int i, j;
   unsigned long rno;
   char rbuf[20];
   char template, * tp, *origname = NULL;
   int len;

   checkparam(1,2);

   init_random();

   rno = random();

   if (argc > 1)
      template =  RXSTRPTR(argv[1])[0];
   else
      template = '?';

   memcpy(result->strptr, argv[0].strptr, argv[0].strlength);
   result->strlength = argv[0].strlength;
   /* null terminate so access() will work */
   result->strptr[result->strlength] = 0;


   do {
      len = sprintf(rbuf, "%05lu", rno);

      /* walk backwards through rbuf so that the rno++ is guaranteed to have
       * an effect. */
      for (tp = memchr(argv[0].strptr, template, argv[0].strlength), j = len - 1;
           tp && j > len - 6;
           tp = memchr(tp+1, template, argv[0].strlength - i-1), j--) {
         i = tp - argv[0].strptr;
         result->strptr[i] = rbuf[j];
      }

      if (!origname) {
         rxstrdup(origname, *result);
      }
      else {
         /* give up when we cycle around to the original name again */
         if (!memcmp(origname, result->strptr, result->strlength)) {
            result->strlength = 0;
            break;
         }
      }

      rno++;
   } while (!access(result->strptr, 0));

   return 0;
} 


/* copy a file from one location to another */
static int copy(const char * from, const char * to)
{
   FILE * in, *out;
   char buf[32768];
   int rc, br;
   struct stat st;
   struct utimbuf utb;

   if ((in = fopen(from, "rb")) == NULL) {
      return errno;
   }
   if ((out = fopen(to, "wb")) == NULL) {
      rc = errno;
      fclose(in);
      return rc;
   }

   fstat(fileno(in), &st);
   utb.actime = st.st_atime;
   utb.modtime = st.st_mtime;

   /* my first plan was to read the input file using memory mapped I/O, but
    * this is more portable and about as fast, without using a lot of memory
    */
   for (br = fread(buf, 1, sizeof(buf), in); br > 0; br = fread(buf, 1, sizeof(buf), in))
      fwrite(buf, 1, br, out);

   fclose(in);
   fclose(out);

   /* set the access & modification times to be the same as the from file */
   utime(to, &utb);

   return 0;
}


/* SysCopyObject(from, to) */
rxfunc(syscopyobject)
{
   char * from, * to;
   int rc;

   checkparam(2,2);

   rxstrdup(from, argv[0]);
   rxstrdup(to, argv[1]);

   rc = copy(from, to);

   if (!rc) {
      result_zero();
   }
   else {
      /* do something with the errno */
      rc = errnotorc(rc);
      if (rc < 0)
         return BADGENERAL;

      result->strlength = sprintf(result->strptr, "%d", rc);
   }

   return 0;
}

/* SysMoveObject(from, to) */
rxfunc(sysmoveobject)
{
   char * from, * to;
   int rc;

   checkparam(2,2);

   rxstrdup(from, argv[0]);
   rxstrdup(to, argv[1]);

   rc = rename(from, to);

   if (rc == -1 && errno == EXDEV) {
      rc = copy(from, to);
      if (!rc)
         rc = remove(from);
   }

   /* copy() returns the errno */
   if (rc == -1)
      rc = errno;


   if (!rc) {
      result_zero();
   }
   else {
      /* do something with the errno */
      rc = errnotorc(rc);
      if (rc < 0)
         return BADGENERAL;

      result->strlength = sprintf(result->strptr, "%d", rc);
   }

   return 0;
}

/* SysChreateShadow(from, to) */
rxfunc(syscreateshadow)
{
   char * from, * to;
   int rc = 1;

   checkparam(2,2);

   rxstrdup(from, argv[0]);
   rxstrdup(to, argv[1]);

#ifndef _WIN32
   /* hard link if possible, symlink if necessary */
   rc = link(from, to);
   if (rc == -1 && errno == EXDEV) {
      rc = symlink(from, to);
   }
#endif

   if (!rc) {
      result_zero();
   }
   else {
      rc = errnotorc(errno);
      if (rc < 0)
         return BADGENERAL;

      result->strlength = sprintf(result->strptr, "%d", rc);
   }

   return 0;
}


/* SysGetFileDateTime(filename [, which]). Gets the timestamp
 * from the file. which tells which timestamp (access, create, modify)
 * the return format is yyyy-mm-dd hh:mm:ss */
rxfunc(sysgetfiledatetime)
{
   char * filename, * which;
   struct stat st;
   struct tm * tm;

   checkparam(1,2);

   rxstrdup(filename, argv[0]);

   if (argc == 2) {
      rxstrdup(which, argv[1]);
      strlwr(which);
   }
   else {
      which = "modify";
   }

   if (stat(filename, &st) == -1) {
      result->strlength = sprintf(result->strptr, "%d", errnotorc(errno));
   }
   else {
      if (*which == 'm')
         tm = localtime(&st.st_mtime);
      else if (*which == 'a')
         tm = localtime(&st.st_atime);
      else if (*which == 'c')
         tm = localtime(&st.st_ctime);
      else
         return BADARGS;

      result->strlength = sprintf(result->strptr, "%04d-%02d-%02d %02d:%02d:%02d",
         tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min,
         tm->tm_sec);
   }

   return 0;
}

/* SysSetFileDateTime(filename [, date [, time]]). This will
 * be used to set the modification time only. */
rxfunc(syssetfiledatetime)
{
   char * filename, * thedate, * thetime, buf[10];
   struct utimbuf utb;
   struct stat st;
   struct tm then;
   rxbool hasdate, hastime;

   checkparam(1,3);

   rxstrdup(filename, argv[0]);

   if (argc > 1) {
      if (argv[1].strlength == 10) {
         hasdate = true;
         rxstrdup(thedate, argv[1]);
      }
      else if (argv[1].strlength) {
         return BADARGS;
      }
      else {
         hasdate = false;
      }

      if (argc == 3) {
         if (argv[2].strlength && argv[2].strlength == 8) {
            hastime = true;
            rxstrdup(thetime, argv[2]);
         }
         else if (argv[2].strlength) {
            return BADARGS;
         }
         else {
            hastime = false;
         }
      }
      else {
         hastime = false;
      }
   }
   else {
      hasdate = hastime = false;
   }


   if (stat(filename, &st) == -1) {
      result->strlength = sprintf(result->strptr, "%d", errnotorc(errno));
   }
   else {
      utb.actime = st.st_atime;

      if (hasdate) {
         memset(&then, 0, sizeof(then));
         memcpy(buf, thedate, 4);
         buf[4] = 0;
         /* struct tm is so y2k compliant. It was planned this way from the
          * start... */
         then.tm_year = atoi(buf) - 1900;
         memcpy(buf, thedate+5, 2);
         buf[2] = 0;
         then.tm_mon = atoi(buf) - 1;
         memcpy(buf, thedate+8, 2);
         then.tm_mday = atoi(buf);
         /* do this now since I don't know whether to set the tm_isdst flag,
          * but the system will deal with it. This probably doesn't work
          * correctly on the day of the cut-over */
         utb.modtime = mktime(&then);

         if (then.tm_isdst) {
            then.tm_isdst = 1;
            then.tm_hour = 0;
            utb.modtime = mktime(&then);
         }
      }
      else {
         time(&utb.modtime);
      }

      if (hastime) {
         then = *localtime(&utb.modtime);
         memcpy(buf, thetime, 2);
         buf[2] = 0;
         then.tm_hour = atoi(buf);
         memcpy(buf, thetime+3, 2);
         then.tm_min = atoi(buf);
         memcpy(buf, thetime+6, 2);
         then.tm_sec = atoi(buf);

         utb.modtime = mktime(&then);
      }
   }

   if (utime(filename, &utb) == -1)
      result->strlength = sprintf(result->strptr, "%d", errnotorc(errno));
   else
      result_zero();

   return 0;
}

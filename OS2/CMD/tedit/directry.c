/***********************************************************************/
/* DIRECTRY.C - Directory routines                                     */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

static char RCSid[] = "$Id: directry.c,v 1.4 2002/04/17 07:39:48 mark Exp $";

#if defined(__OS2__) && !defined(__EMX__)
#   define INCL_DOS
#endif

#include <the.h>
#include <proto.h>

#include <directry.h>
#include <thematch.h>

#ifdef HAVE_PROTO
int is_a_dir(ATTR_TYPE);
#else
int is_a_dir();
#endif

#define NUM_DIRTYPE 5
static ATTR_TYPE curr_dirtype =
       (F_DI | F_AR | F_RO | F_HI | F_SY);
static ATTR_TYPE all_dirtype =
       (F_DI | F_AR | F_RO | F_HI | F_SY);
static CHARTYPE _THE_FAR *dirtype[NUM_DIRTYPE] =
       {(CHARTYPE *)"normal",
       (CHARTYPE *)"readonly",
       (CHARTYPE *)"system",
       (CHARTYPE *)"hidden",
       (CHARTYPE *)"directory"};
static ATTR_TYPE att[NUM_DIRTYPE] =
       {0,F_RO,F_SY,F_HI,F_DI};

/*********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *make_full(CHARTYPE *path, CHARTYPE *file)
#else
CHARTYPE *make_full(path, file)
CHARTYPE *path, *file;
#endif
/*********************************************************************/
{
 static CHARTYPE _THE_FAR filebuf[BUFSIZ];
 short pathlen=strlen((DEFCHAR *)path);

 if (pathlen+1+strlen((DEFCHAR *)file)+1 > BUFSIZ)
    return(NULL);
 if (!strcmp((DEFCHAR *)path, "") || !strcmp((DEFCHAR *)path, "."))
   {
    (void) strcpy((DEFCHAR *)filebuf, (DEFCHAR *)file);
    return(filebuf);
   }
 (void) strcpy((DEFCHAR *)filebuf, (DEFCHAR *)path);
 if (*(path+(pathlen - 1)) != ISLASH && *file != ISLASH)
    (void) strcat((DEFCHAR *)filebuf, (DEFCHAR *)ISTR_SLASH);
 (void) strcat((DEFCHAR *)filebuf, (DEFCHAR *)file);
 return(filebuf);
}
/*********************************************************************/
#if defined(UNIX) || defined(EMX) || (defined(AMIGA) && defined(GCC))
#ifdef HAVE_PROTO
short getfiles(CHARTYPE *path,CHARTYPE *files,struct dirfile **dpfirst,
                                    struct dirfile **dplast)
#else
short getfiles(path,files,dpfirst,dplast)
CHARTYPE *path;
CHARTYPE *files;
struct dirfile **dpfirst;
struct dirfile **dplast;
#endif
/*********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 DIR *dirp=NULL;
 struct stat sp;
 struct dirent *direntp=NULL;
 struct dirfile *dp=NULL;
 CHARTYPE *full_name=NULL;
 short entries = 10;
 struct tm *timp=NULL;
/*--------------------------- processing ------------------------------*/
 dirp = opendir((DEFCHAR *)path);
 if (dirp == NULL)
    return(10);

 dp = *dpfirst = (struct dirfile *)(*the_malloc)(entries * sizeof(struct dirfile));
 if (dp == NULL)
    return(RC_OUT_OF_MEMORY);
 memset(dp, 0, entries*sizeof(struct dirfile) );
 memset(*dpfirst, 0, entries*sizeof(struct dirfile) );
 *dplast = *dpfirst + entries;

 for (direntp = readdir(dirp);direntp != NULL;direntp = readdir(dirp))
    {
     if (thematch(files,(CHARTYPE *)direntp->d_name,0) == 0)
       {
        if ((full_name = make_full(path,(CHARTYPE *)direntp->d_name)) == NULL)
           return(RC_OUT_OF_MEMORY);
#if defined(HAVE_LSTAT)
        if (lstat((DEFCHAR *)full_name,&sp) != 0)
#else
        if (stat((DEFCHAR *)full_name,&sp) != 0)
#endif
           continue;
        dp->fname_length = strlen(direntp->d_name)+1;
        if ((dp->fname = (CHARTYPE *)(*the_malloc)(dp->fname_length*sizeof(CHARTYPE))) == NULL)
           return(RC_OUT_OF_MEMORY);
        strcpy((DEFCHAR *)dp->fname,direntp->d_name);
#if defined(EMX)
        dp->fattr = sp.st_attr;
#else
        dp->fattr = sp.st_mode;
#endif
#if defined(_HPUX_SOURCE)
        dp->facl  = sp.st_acl;
#else
        dp->facl  = 0;
#endif
        timp = localtime(&(sp.st_mtime));
        dp->f_hh = HH_MASK(timp);
        dp->f_mi = MI_MASK(timp);
        dp->f_ss = SS_MASK(timp);
        dp->f_dd = DD_MASK(timp);
        dp->f_mm = MM_MASK(timp);
        dp->f_yy = YY_MASK(timp);
        dp->fsize = sp.st_size;
        dp->lname = NULL;
        /*
         * If we have the lstat() function, and the current file
         * is a symbolic link, go and get the filename the symbolic
         * link points to...
         */
#if defined(S_ISLNK) && defined(HAVE_READLINK)
        if (S_ISLNK(dp->fattr))
          {
           char buf[MAX_FILE_NAME+1];
           int rc=0;

           rc = readlink((DEFCHAR *)full_name,buf,sizeof(buf));
           if (rc != (-1))
             {
              if ((dp->lname = (CHARTYPE *)(*the_malloc)((rc+1)*sizeof(CHARTYPE))) == NULL)
                 return(RC_OUT_OF_MEMORY);
              memcpy((DEFCHAR *)dp->lname,buf,rc);
              dp->lname[rc] = '\0';
             }
          }
#endif
        dp++;
        if (dp == *dplast)
          {
           *dpfirst = (struct dirfile *)(*the_realloc)((CHARTYPE *)*dpfirst,
                      2 * entries * sizeof (struct dirfile));
           if (*dpfirst == NULL)
              return(RC_OUT_OF_MEMORY);
           dp = *dpfirst + entries;
           *dplast = dp + entries;
           entries *= 2;
          }
       }
    }
 closedir(dirp);
 *dplast = dp;
 return(0);
}
#else
/*********************************************************************/
#ifdef HAVE_PROTO
short getfiles(CHARTYPE *path,CHARTYPE *files,struct dirfile **dpfirst,
                                    struct dirfile **dplast)
#else
short getfiles(path,files,dpfirst,dplast)
CHARTYPE *path;
CHARTYPE *files;
struct dirfile **dpfirst;
struct dirfile **dplast;
#endif
/*********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 struct dirfile *dp=NULL;
#ifdef OS2
#  if defined(__32BIT__) || defined(__386__)
 ULONG matches=1L;
 ULONG rsvrd=FIL_STANDARD;
 FILEFINDBUF3 ffblk;
#  else
 USHORT matches=1;
 ULONG rsvrd=0;
 FILEFINDBUF ffblk;
#  endif
 HDIR hdir=HDIR_SYSTEM;
#else
 FSTR_TYPE ffblk;
#endif
 ATTR_TYPE attrs=curr_dirtype;
 DONE_TYPE done=0;
#if defined(WIN32) && defined(_MSC_VER)
 DONE_TYPE handle=0;
 struct tm *local;
#endif
 CHARTYPE *full_path=NULL;
 CHARTYPE str_attr[12];
 CHARTYPE str_date[10];
 CHARTYPE str_time[6];
 short entries = 10;
/*--------------------------- processing ------------------------------*/
 if ((full_path = make_full(path,"*.*")) == NULL)
    return(RC_FILE_NOT_FOUND);

#if defined(DOS) && defined(TC)
 done = findfirst(full_path,&ffblk,attrs);
#endif
#if defined (DOS) && defined(MSC)
 done = _dos_findfirst(full_path,attrs,&ffblk);
#endif
#if defined(DOS) && defined(GO32)
 done = findfirst(full_path,&ffblk,attrs);
#endif
#if defined (WIN32) && defined(__WATCOMC__)
 done = _dos_findfirst(full_path,attrs,&ffblk);
#endif
#if defined (WIN32) && defined(_MSC_VER)
 ffblk.attrib = attrs;
 handle = _findfirst(full_path,&ffblk);
 if (handle == (-1))
    done = (-1);
 else
    local = localtime(&ffblk.time_write);
#endif
#ifdef OS2
#  if defined(__32BIT__) || defined(__386__)
 done = DosFindFirst((PSZ) full_path, (PHDIR)&hdir, (ULONG)attrs,
                     (PVOID)&ffblk, (ULONG)sizeof(ffblk), (PULONG)&matches,
                     (ULONG)rsvrd);
#  else
 done = DosFindFirst((PSZ) full_path, (PHDIR)&hdir, (USHORT)attrs,
                     (PFILEFINDBUF)&ffblk, (USHORT)sizeof(ffblk), (PUSHORT)&matches,
                     (ULONG)rsvrd);
#  endif
#endif
 if (done != 0)
    return(RC_FILE_NOT_FOUND);

 dp = *dpfirst = (struct dirfile *)(*the_malloc)(entries * sizeof (struct dirfile));
 if (dp == NULL)
    return(RC_OUT_OF_MEMORY);
 memset(*dpfirst, 0, entries*sizeof(struct dirfile) );
 *dplast = *dpfirst + entries;

 while(!done)
    {
     if (thematch(files,ffblk.NAME_NAME,FNM_IGNORECASE) == 0)
       {
        dp->fname_length = strlen(ffblk.NAME_NAME)+1;
        if ((dp->fname = (CHARTYPE *)(*the_malloc)(dp->fname_length*sizeof(CHARTYPE))) == NULL)
           return(RC_OUT_OF_MEMORY);
        strcpy(dp->fname,ffblk.NAME_NAME);
        dp->fattr = ffblk.ATTR_NAME;
        dp->f_hh = HH_MASK(ffblk.TIME_NAME);
        dp->f_mi = MI_MASK(ffblk.TIME_NAME);
        dp->f_ss = SS_MASK(ffblk.TIME_NAME);
        dp->f_dd = DD_MASK(ffblk.DATE_NAME);
        dp->f_mm = MM_MASK(ffblk.DATE_NAME);
        dp->f_yy = YY_MASK(ffblk.DATE_NAME);
        dp->fsize = ffblk.SIZE_NAME;
        dp->lname = NULL;
        dp++;
        if (dp == *dplast)
          {
           *dpfirst = (struct dirfile *)(*the_realloc)((CHARTYPE *)*dpfirst,
                      2 * entries * sizeof (struct dirfile));
           if (*dpfirst == NULL)
              return(RC_FILE_NOT_FOUND);
           dp = *dpfirst + entries;
           *dplast = dp + entries;
           entries *= 2;
          }
       }
#if defined(DOS) && defined(TC)
     done = findnext(&ffblk);
#endif
#if defined(DOS) && defined(MSC)
     done = _dos_findnext(&ffblk);
#endif
#if defined(DOS) && defined(GO32)
     done = findnext(&ffblk);
#endif
#if defined(WIN32) && defined(__WATCOMC__)
     done = _dos_findnext(&ffblk);
#endif
#if defined (WIN32) && defined(_MSC_VER)
     done = _findnext(handle,&ffblk);
     local = localtime(&ffblk.time_write);
#endif
#ifdef OS2
#  if defined(__32BIT__) || defined(__386__)
     done = DosFindNext((HDIR)hdir, (PVOID)&ffblk, (ULONG)sizeof(ffblk),
            (PULONG)&matches);
#  else
     done = DosFindNext((HDIR)hdir, (PFILEFINDBUF)&ffblk, (USHORT)sizeof(ffblk),
            (PUSHORT)&matches);
#  endif
#endif
    }
 *dplast = dp;
 return(RC_OK);
}
#endif

/*********************************************************************/
#ifdef HAVE_PROTO
int date_compare(struct dirfile *first,struct dirfile *next)
#else
int date_compare(first,next)
struct dirfile *first;
struct dirfile *next;
#endif
/*********************************************************************/
{
 if (first->f_yy > next->f_yy)
    return(1);
 else if (first->f_yy < next->f_yy)
    return(-1);
 if (first->f_mm > next->f_mm)
    return(1);
 else if (first->f_mm < next->f_mm)
    return(-1);
 if (first->f_dd > next->f_dd)
    return(1);
 else if (first->f_dd < next->f_dd)
    return(-1);
 return(0);
}
/*********************************************************************/
#ifdef HAVE_PROTO
int time_compare(struct dirfile *first,struct dirfile *next)
#else
int time_compare(first,next)
struct dirfile *first;
struct dirfile *next;
#endif
/*********************************************************************/
{
 if (first->f_hh > next->f_hh)
    return(1);
 else if (first->f_hh < next->f_hh)
    return(-1);
 if (first->f_mi > next->f_mi)
    return(1);
 else if (first->f_mi < next->f_mi)
    return(-1);
 if (first->f_ss > next->f_ss)
    return(1);
 else if (first->f_ss < next->f_ss)
    return(-1);
 return(0);
}

/*********************************************************************/
#ifdef HAVE_PROTO
int date_comp(struct dirfile *first,struct dirfile* next)
#else
int date_comp(first,next)
struct dirfile *first;
struct dirfile* next;
#endif
/*********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 int rc=0;
/*--------------------------- processing ------------------------------*/
 rc = date_compare(first,next);
 if (rc == 0)
    rc = time_compare(first,next);
 if (rc == 0)
#if defined(OS2) || defined(WIN32) || defined(DOS)
    rc = stricmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#else
    rc = strcmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#endif
 if (rc == 0)
    return(0);
 if (DIRORDERx == DIRSORT_DESC)
    rc = (rc == 1) ? (-1) : 1;
 return(rc);
}

/*********************************************************************/
#ifdef HAVE_PROTO
int time_comp(struct dirfile *first,struct dirfile* next)
#else
int time_comp(first,next)
struct dirfile *first;
struct dirfile* next;
#endif
/*********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 int rc=0;
/*--------------------------- processing ------------------------------*/
 rc = time_compare(first,next);
 if (rc == 0)
#if defined(OS2) || defined(WIN32) || defined(DOS)
    rc = stricmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#else
    rc = strcmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#endif
 if (rc == 0)
    return(0);
 if (DIRORDERx == DIRSORT_DESC)
    rc = (rc == 1) ? (-1) : 1;
 return(rc);
}

/*********************************************************************/
#ifdef HAVE_PROTO
int dir_comp(struct dirfile *first,struct dirfile* next)
#else
int dir_comp(first,next)
struct dirfile *first;
struct dirfile* next;
#endif
/*********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 int first_dir=0;
 int next_dir=0;
 int rc=0;
/*--------------------------- processing ------------------------------*/
 first_dir=is_a_dir(first->fattr);
 next_dir=is_a_dir(next->fattr);
 if (first_dir && !next_dir)
    rc = (-1);
 if (!first_dir && next_dir)
    rc = 1;
 if (rc == 0)
#if defined(OS2) || defined(WIN32) || defined(DOS)
    rc = stricmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#else
    rc = strcmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#endif
 if (rc == 0)
    return(0);
 if (DIRORDERx == DIRSORT_DESC)
    rc = (rc == 1) ? (-1) : 1;
 return(rc);
}

/*********************************************************************/
#ifdef HAVE_PROTO
int size_comp(struct dirfile *first,struct dirfile* next)
#else
int size_comp(first,next)
struct dirfile *first;
struct dirfile* next;
#endif
/*********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 int rc=0;
/*--------------------------- processing ------------------------------*/
 if (first->fsize > next->fsize)
    rc = 1;
 else
   {
    if (first->fsize < next->fsize)
       rc = -1;
   }
 if (rc == 0)
#if defined(OS2) || defined(WIN32) || defined(DOS)
    rc = stricmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#else
    rc = strcmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#endif
 if (rc == 0)
    return(0);
 if (DIRORDERx == DIRSORT_DESC)
    rc = (rc == 1) ? (-1) : 1;
 return(rc);
}

/*********************************************************************/
#ifdef HAVE_PROTO
int name_comp(struct dirfile *first,struct dirfile* next)
#else
int name_comp(first,next)
struct dirfile *first;
struct dirfile* next;
#endif
/*********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 int rc=0;
/*--------------------------- processing ------------------------------*/
#if defined(OS2) || defined(WIN32) || defined(DOS)
 rc = stricmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#else
 rc = strcmp((DEFCHAR *)first->fname,(DEFCHAR *)next->fname);
#endif
 if (rc == 0)
    return(0);
 if (DIRORDERx == DIRSORT_DESC)
    rc = (rc == 1) ? (-1) : 1;
 return(rc);
}
/*********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *file_date(struct dirfile *date,CHARTYPE *str_date)
#else
CHARTYPE *file_date(date,str_date)
struct dirfile *date;
CHARTYPE *str_date;
#endif
/*********************************************************************/
{
 static CHARTYPE _THE_FAR mon[12][4] = {
 "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
 sprintf((DEFCHAR *)str_date,"%2d-%3.3s-%4.4d",date->f_dd,mon[date->f_mm],date->f_yy);
 return(str_date);
}
/*********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *file_time(struct dirfile *time,CHARTYPE *str_time)
#else
CHARTYPE *file_time(time,str_time)
struct dirfile *time;
CHARTYPE *str_time;
#endif
/*********************************************************************/
{
 sprintf((DEFCHAR *)str_time,"%2d:%2.2d",time->f_hh,time->f_mi);
 return(str_time);
}
/*********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *file_attrs(ATTR_TYPE attrs,CHARTYPE *str_attr,int facl)
#else
CHARTYPE *file_attrs(attrs,str_attr,facl)
ATTR_TYPE attrs;
CHARTYPE *str_attr;
int facl;
#endif
/*********************************************************************/
{
#if defined(UNIX) || defined(AMIGA)
 ATTR_TYPE ftype=(attrs & S_IFMT);

 str_attr[11] = '\0';
 str_attr[10] = ' ';
 if (facl)
    str_attr[10] = '+';
 switch(ftype)
   {
#if defined(S_IFLNK)
    case S_IFLNK:  str_attr[0] = 'l'; break;
#endif
    case S_IFDIR:  str_attr[0] = 'd'; break;
    case S_IFCHR:  str_attr[0] = 'c'; break;
#if defined(S_IFIFO)
    case S_IFIFO:  str_attr[0] = 'p'; break;
#else
# if defined(S_IFFIFO)
    case S_IFFIFO:  str_attr[0] = 'p'; break;
# endif
#endif
#if defined(S_IFSOCK)
    case S_IFSOCK: str_attr[0] = 's'; break;
#endif
    default:       str_attr[0] = '-'; break;
   }
 str_attr[1] = (attrs & S_IRUSR) ? 'r' : '-';
 str_attr[2] = (attrs & S_IWUSR) ? 'w' : '-';
 str_attr[3] = (attrs & S_IXUSR) ? 'x' : '-';
 str_attr[3] = (attrs & S_ISUID) ? 's' : str_attr[3];
 str_attr[4] = (attrs & S_IRGRP) ? 'r' : '-';
 str_attr[5] = (attrs & S_IWGRP) ? 'w' : '-';
 str_attr[6] = (attrs & S_IXGRP) ? 'x' : '-';
 str_attr[6] = (attrs & S_ISGID) ? 's' : str_attr[6];
 str_attr[7] = (attrs & S_IROTH) ? 'r' : '-';
 str_attr[8] = (attrs & S_IWOTH) ? 'w' : '-';
 str_attr[9] = (attrs & S_IXOTH) ? 'x' : '-';
#else
 strcpy(str_attr,".... ");
 if ((attrs & F_RO) == F_RO)
   str_attr[0] = 'r';
 if ((attrs & F_AR) == F_AR)
   str_attr[1] = 'a';
 if ((attrs & F_SY) == F_SY)
   str_attr[2] = 's';
 if ((attrs & F_HI) == F_HI)
   str_attr[3] = 'h';
 str_attr[4] = '\0';
 if ((attrs & F_DI) == F_DI)
   strcat(str_attr,"(dir)  ");
 else
   strcat(str_attr,"       ");
#endif
 return(str_attr);
}
/*********************************************************************/
#ifdef HAVE_PROTO
int is_a_dir(ATTR_TYPE attrs)
#else
int is_a_dir(attrs)
ATTR_TYPE attrs;
#endif
/*********************************************************************/
{
#if defined(UNIX) || defined(AMIGA)
 ATTR_TYPE ftype=(attrs & S_IFMT);

 if (ftype == S_IFDIR)
    return(1);
#else
 if ((attrs & F_DI) == F_DI)
    return(1);
#endif
 return(0);
}
/*********************************************************************/
#ifdef HAVE_PROTO
short set_dirtype(CHARTYPE *params)
#else
short set_dirtype(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE *p=NULL;
 register short i=0;
 ATTR_TYPE attribs=0;
 bool found=FALSE;
/*--------------------------- processing ------------------------------*/
 if (strcmp((DEFCHAR *)params,"") == 0)      /* error if no paramaters */
   {
    display_error(3,(CHARTYPE *)"",FALSE);
    return(RC_INVALID_OPERAND);
   }
 if (strcmp((DEFCHAR *)params,"*") == 0)                 /* set to ALL */
   {
    curr_dirtype = (F_RO | F_HI | F_SY | F_DI | F_AR);
    return(RC_OK);
   }
 p = (CHARTYPE *)strtok((DEFCHAR *)params," ");
 while(p != NULL)
   {
    found = FALSE;
    for (i=0;i<NUM_DIRTYPE;i++)
       {
        if (equal(dirtype[i],p,1))
          {
           found = TRUE;
           attribs |= att[i];
          }
       }
    if (!found)
      {
       display_error(1,(CHARTYPE *)p,FALSE);
       return(RC_INVALID_OPERAND);
      }
    p = (CHARTYPE *)strtok(NULL," ");
   }
 curr_dirtype = attribs;
 return(RC_OK);
}

/*********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *get_dirtype(CHARTYPE *buf)
#else
CHARTYPE *get_dirtype(buf)
CHARTYPE *buf;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 if (curr_dirtype == all_dirtype)                 /* all masks enabled */
   {
    strcpy((DEFCHAR*)buf,"*");
   }
 else
   {
    strcpy((DEFCHAR*)buf,"normal");
    if (curr_dirtype & F_DI)
       strcat((DEFCHAR*)buf," directory");
    if (curr_dirtype & F_RO)
       strcat((DEFCHAR*)buf," readonly");
    if (curr_dirtype & F_HI)
       strcat((DEFCHAR*)buf," hidden");
    if (curr_dirtype & F_SY)
       strcat((DEFCHAR*)buf," system");
   }
 return(buf);
}

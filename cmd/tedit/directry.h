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

/*
$Id: directry.h,v 1.3 2001/12/18 08:23:27 mark Exp $
*/

#if defined(DOS) && defined(TC)
# define F_RO FA_RDONLY
# define F_HI FA_HIDDEN
# define F_SY FA_SYSTEM
# define F_DI FA_DIREC
# define F_AR FA_ARCH

# define FSTR_TYPE struct ffblk

# define ATTR_TYPE CHARTYPE
# define DATE_TYPE short
# define TIME_TYPE short
# define SIZE_TYPE long
# define D_TYPE    DATE_TYPE
# define T_TYPE    TIME_TYPE
# define DONE_TYPE short

# define ATTR_NAME ff_attrib
# define DATE_NAME ff_fdate
# define TIME_NAME ff_ftime
# define SIZE_NAME ff_fsize
# define NAME_NAME ff_name

# define HOUR_MASK ((time & 0xf800) >> 11)
# define MINU_MASK ((time & 0x07e0) >> 5)
# define DAYS_MASK (date & 0x001f)
# define MONT_MASK (mon[((date & 0x01e0) >> 5)-1])
# define YEAR_MASK (((date & 0xfe00) >> 9)+1980)

# define HH_MASK(a) ((a & 0xf800) >> 11)
# define MI_MASK(a) ((a & 0x07e0) >> 5)
# define SS_MASK(a) ((a & 0x001f) << 1)
# define DD_MASK(a) (a & 0x001f)
# define MM_MASK(a) (((a & 0x01e0) >> 5)-1)
# define YY_MASK(a) (((a & 0xfe00) >> 9)+1980)

#endif

#if defined(DOS) && defined(MSC)
# include <dos.h>
# define F_RO _A_RDONLY
# define F_HI _A_HIDDEN
# define F_SY _A_SYSTEM
# define F_DI _A_SUBDIR
# define F_AR _A_ARCH

# define FSTR_TYPE struct find_t

# define ATTR_TYPE CHARTYPE
# define DATE_TYPE unsigned short
# define TIME_TYPE unsigned short
# define SIZE_TYPE long
# define D_TYPE    DATE_TYPE
# define T_TYPE    TIME_TYPE
# define DONE_TYPE short

# define ATTR_NAME attrib
# define DATE_NAME wr_date
# define TIME_NAME wr_time
# define SIZE_NAME size
# define NAME_NAME name

# define HOUR_MASK ((time & 0xf800) >> 11)
# define MINU_MASK ((time & 0x07e0) >> 5)
# define DAYS_MASK (date & 0x001f)
# define MONT_MASK (mon[((date & 0x01e0) >> 5)-1])
# define YEAR_MASK (((date & 0xfe00) >> 9)+1980)

# define HH_MASK(a) ((a & 0xf800) >> 11)
# define MI_MASK(a) ((a & 0x07e0) >> 5)
# define SS_MASK(a) ((a & 0x001f) << 1)
# define DD_MASK(a) (a & 0x001f)
# define MM_MASK(a) (((a & 0x01e0) >> 5)-1)
# define YY_MASK(a) (((a & 0xfe00) >> 9)+1980)
#endif

#if defined(DOS) && defined(GO32)
# define F_RO FA_RDONLY
# define F_HI FA_HIDDEN
# define F_SY FA_SYSTEM
# define F_DI FA_DIREC
# define F_AR FA_ARCH

# define FSTR_TYPE struct ffblk

# define ATTR_TYPE CHARTYPE
# define DATE_TYPE short
# define TIME_TYPE short
# define SIZE_TYPE long
# define D_TYPE    DATE_TYPE
# define T_TYPE    TIME_TYPE
# define DONE_TYPE short

# define ATTR_NAME ff_attrib
# define DATE_NAME ff_fdate
# define TIME_NAME ff_ftime
# define SIZE_NAME ff_fsize
# define NAME_NAME ff_name

# define HOUR_MASK ((time & 0xf800) >> 11)
# define MINU_MASK ((time & 0x07e0) >> 5)
# define DAYS_MASK (date & 0x001f)
# define MONT_MASK (mon[((date & 0x01e0) >> 5)-1])
# define YEAR_MASK (((date & 0xfe00) >> 9)+1980)

# define HH_MASK(a) ((a & 0xf800) >> 11)
# define MI_MASK(a) ((a & 0x07e0) >> 5)
# define SS_MASK(a) ((a & 0x001f) << 1)
# define DD_MASK(a) (a & 0x001f)
# define MM_MASK(a) (((a & 0x01e0) >> 5)-1)
# define YY_MASK(a) (((a & 0xfe00) >> 9)+1980)
#endif

#if defined(EMX)
# include <dirent.h>
# include <time.h>

# define F_RO A_RONLY
# define F_HI A_HIDDEN
# define F_SY A_SYSTEM
# define F_DI A_DIR
# define F_AR A_ARCHIVE

# define ATTR_TYPE long
# define SIZE_TYPE long
# define TIME_TYPE time_t
# define DATE_TYPE CHARTYPE
# define D_TYPE    struct tm *
# define T_TYPE    struct tm *
# define DONE_TYPE short

# define HOUR_MASK (time->tm_hour)
# define MINU_MASK (time->tm_min)
# define DAYS_MASK (date->tm_mday)
# define MONT_MASK (mon[date->tm_mon])
# define YEAR_MASK (date->tm_year)

# define HH_MASK(a) (a->tm_hour)
# define MI_MASK(a) (a->tm_min)
# define SS_MASK(a) (a->tm_sec)
# define DD_MASK(a) (a->tm_mday)
# define MM_MASK(a) (a->tm_mon)
# define YY_MASK(a) ((a->tm_year)+1900)
#endif

#if defined(OS2) && !defined(EMX)
# define F_RO FILE_READONLY
# define F_HI FILE_HIDDEN
# define F_SY FILE_SYSTEM
# define F_DI FILE_DIRECTORY
# define F_AR FILE_ARCHIVED

# define FSTR_TYPE FILEFINDBUF

# define ATTR_TYPE USHORT
# define DATE_TYPE FDATE
# define TIME_TYPE FTIME
# define SIZE_TYPE ULONG
# define D_TYPE    DATE_TYPE
# define T_TYPE    TIME_TYPE
# define DONE_TYPE USHORT

# define ATTR_NAME attrFile
# define DATE_NAME fdateLastWrite
# define TIME_NAME ftimeLastWrite
# define SIZE_NAME cbFile
# define NAME_NAME achName

# define HOUR_MASK (time.hours)
# define MINU_MASK (time.minutes)
# define DAYS_MASK (date.day)
# define MONT_MASK (mon[date.month-1])
# define YEAR_MASK (date.year+1980)

# define HH_MASK(a) (a.hours)
# define MI_MASK(a) (a.minutes)
# define SS_MASK(a) (a.twosecs/2)
# define DD_MASK(a) (a.day)
# define MM_MASK(a) (a.month-1)
# define YY_MASK(a) (a.year+1980)
#endif

#if defined(WIN32) && defined(__WATCOMC__)
# include <dos.h>
# define F_RO _A_RDONLY
# define F_HI _A_HIDDEN
# define F_SY _A_SYSTEM
# define F_DI _A_SUBDIR
# define F_AR _A_ARCH

# define FSTR_TYPE struct find_t

# define ATTR_TYPE unsigned short
# define DATE_TYPE unsigned short
# define TIME_TYPE unsigned short
# define SIZE_TYPE long
# define D_TYPE    DATE_TYPE
# define T_TYPE    TIME_TYPE
# define DONE_TYPE short

# define ATTR_NAME attrib
# define DATE_NAME wr_date
# define TIME_NAME wr_time
# define SIZE_NAME size
# define NAME_NAME name

# define HOUR_MASK ((time & 0xf800) >> 11)
# define MINU_MASK ((time & 0x07e0) >> 5)
# define DAYS_MASK (date & 0x001f)
# define MONT_MASK (mon[((date & 0x01e0) >> 5)-1])
# define YEAR_MASK (((date & 0xfe00) >> 9)+1980)

# define HH_MASK(a) ((a & 0xf800) >> 11)
# define MI_MASK(a) ((a & 0x07e0) >> 5)
# define SS_MASK(a) ((a & 0x001f) << 1)
# define DD_MASK(a) (a & 0x001f)
# define MM_MASK(a) (((a & 0x01e0) >> 5)-1)
# define YY_MASK(a) (((a & 0xfe00) >> 9)+1980)
#endif

#if defined(WIN32) && defined(_MSC_VER)
# include <dos.h>
# include <io.h>
# include <time.h>
# define F_RO _A_RDONLY
# define F_HI _A_HIDDEN
# define F_SY _A_SYSTEM
# define F_DI _A_SUBDIR
# define F_AR _A_ARCH

# define FSTR_TYPE struct _finddata_t

# define ATTR_TYPE unsigned
# define DATE_TYPE time_t
# define TIME_TYPE time_t
# define SIZE_TYPE long
# define D_TYPE    DATE_TYPE
# define T_TYPE    TIME_TYPE
# define DONE_TYPE long

# define ATTR_NAME attrib
# define DATE_NAME time_write
# define TIME_NAME time_write
# define SIZE_NAME size
# define NAME_NAME name

# define HOUR_MASK ((local==NULL)?0:local->tm_hour)
# define MINU_MASK ((local==NULL)?0:local->tm_min)
# define DAYS_MASK ((local==NULL)?0:local->tm_mday)
# define MONT_MASK 
# define YEAR_MASK ((local==NULL)?0:local->tm_year+1900)

# define HH_MASK(a) ((local==NULL)?0:local->tm_hour)
# define MI_MASK(a) ((local==NULL)?0:local->tm_min)
# define SS_MASK(a) ((local==NULL)?0:local->tm_sec)
# define DD_MASK(a) ((local==NULL)?0:local->tm_mday)
# define MM_MASK(a) ((local==NULL)?0:local->tm_mon)
# define YY_MASK(a) ((local==NULL)?0:local->tm_year+1900)
#endif

#if defined(UNIX) || (defined(__GNUC__) && !defined(__EMX__)) && !defined(GO32)
# if defined(M_XENIX)
#   include <sys/dirent.h>
#   include <sys/ndir.h>
# else
#   if defined(HAVE_DIRENT_H)
#     include <dirent.h>
#   endif
#   if defined(HAVE_SYS_STAT_H)
#     include <sys/stat.h>
#   endif
#   if defined(HAVE_SYS_MODE_H)
#     include <sys/mode.h>
#   endif
# endif
# if defined(HAVE_TIME_H)
#  include <time.h>
# endif

# define F_RO 0
# define F_HI 0
# define F_SY 0
# define F_DI 0
# define F_AR 0

# if defined(ATT) || defined(M_XENIX)
#  define mode_t ushort
# endif

# define ATTR_TYPE mode_t
# define SIZE_TYPE long
# define TIME_TYPE time_t
# define DATE_TYPE CHARTYPE
# define D_TYPE    struct tm *
# define T_TYPE    struct tm *
# define DONE_TYPE short

# define HOUR_MASK (time->tm_hour)
# define MINU_MASK (time->tm_min)
# define DAYS_MASK (date->tm_mday)
# define MONT_MASK (mon[date->tm_mon])
# define YEAR_MASK (date->tm_year)

# define HH_MASK(a) (a->tm_hour)
# define MI_MASK(a) (a->tm_min)
# define SS_MASK(a) (a->tm_sec)
# define DD_MASK(a) (a->tm_mday)
# define MM_MASK(a) (a->tm_mon)
# define YY_MASK(a) ((a->tm_year)+1900)
#endif

#if defined(AMIGA) && defined(GCC)
# if defined(HAVE_DIRENT_H)
#   include <dirent.h>
# endif
# if defined(HAVE_SYS_STAT_H)
#   include <sys/stat.h>
# endif
# if defined(HAVE_SYS_MODE_H)
#   include <sys/mode.h>
# endif
# if defined(HAVE_TIME_H)
#  include <time.h>
# endif

# define F_RO 0
# define F_HI 0
# define F_SY 0
# define F_DI 0
# define F_AR 0

# define ATTR_TYPE mode_t
# define SIZE_TYPE long
# define TIME_TYPE time_t
# define DATE_TYPE CHARTYPE
# define D_TYPE    struct tm *
# define T_TYPE    struct tm *
# define DONE_TYPE short

# define HOUR_MASK (time->tm_hour)
# define MINU_MASK (time->tm_min)
# define DAYS_MASK (date->tm_mday)
# define MONT_MASK (mon[date->tm_mon])
# define YEAR_MASK (date->tm_year)

# define HH_MASK(a) (a->tm_hour)
# define MI_MASK(a) (a->tm_min)
# define SS_MASK(a) (a->tm_sec)
# define DD_MASK(a) (a->tm_mday)
# define MM_MASK(a) (a->tm_mon)
# define YY_MASK(a) ((a->tm_year)+1900)
#endif

struct dirfile {
	CHARTYPE	*fname;		/* file name */
	CHARTYPE	*lname;		/* link name */
	ATTR_TYPE	fattr;		/* file attributes */
	SIZE_TYPE	fsize;		/* size of file */
	CHARTYPE	f_hh;		/* hour */
	CHARTYPE	f_mi;		/* minute */
	CHARTYPE	f_ss;		/* second */
	CHARTYPE	f_dd;		/* day */
	CHARTYPE	f_mm;		/* month */
	int	f_yy;		/* year */
	int	facl;		/* acl */
	int	fname_length;		/* length of filename */
};

#ifdef HAVE_PROTO
int date_comp();           /* this has been deliberatly left undefined */
int time_comp();           /* this has been deliberatly left undefined */
int size_comp();           /* this has been deliberatly left undefined */
int name_comp();           /* this has been deliberatly left undefined */
int dir_comp();            /* this has been deliberatly left undefined */
CHARTYPE *make_full(CHARTYPE *,CHARTYPE *);
short getfiles(CHARTYPE *,CHARTYPE *,struct dirfile **,struct dirfile **);
CHARTYPE *file_attrs(ATTR_TYPE,CHARTYPE *,int);
CHARTYPE *file_date(struct dirfile *,CHARTYPE *);
CHARTYPE *file_time(struct dirfile *,CHARTYPE *);
short set_dirtype(CHARTYPE *);
#else
int date_comp();
int time_comp();
int size_comp();
int name_comp();
int dir_comp();
CHARTYPE *make_full();
short getfiles();
CHARTYPE *file_attrs();
CHARTYPE *file_date();
CHARTYPE *file_time();
short set_dirtype();
#endif

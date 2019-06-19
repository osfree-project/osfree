/*    
	msclib.c	1.25
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*	standard c library file interface functions

 */

#include "platform.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "windows.h"

#include "io.h" 
#include "mfs_config.h"
#include "mfs_core.h"
#include "dos.h"
#include "ctype.h"
#include "kerndef.h"
#include "Log.h"

#define is_slash(c)  ( ((c)=='/' || (c)=='\\') ? TRUE : FALSE )

extern LPSTR strpbrkr(LPCSTR, LPCSTR);
extern BOOL TWIN_GetCurrentDate(LPSTR, UINT);

#ifdef sun 
#include "limits.h"
#define RAND_MAX INT_MAX
#endif /*sun*/

/****************************************************************************
**
**  Functions below have name conflicts with MSCLIB functions. 
**
*****************************************************************************/
#ifndef HAVE_MSCLIB /* If MSCLIB not available on Platform. */


/****************************************************************************/
long
_filelength(int handle)
{
	LPMFSFILE fp;
        fp =  (LPMFSFILE) mfs_finfo (handle, 0L, 0L, 0L);
	if ( fp ) {
		return fp->fsize;
	}
        return EBADF;
}

/****************************************************************************/
long
filelength(int handle)
{
	/*
	 * Borland uses a different name for this function.  Just
	 * turn around and call the MS equivalent.  This could be done
	 * with a #define for a slight speedup, but that would damage
	 * the namespace, disallowing the use of any variable or element
	 * with this name, so we just do it as a call.  Besides, a good
	 * optimizing compiler will make the two entry points equivalent,
	 * and remove the extraneous call.
	 */
	return(_filelength(handle));
}

/****************************************************************************/
void
_splitpath(LPCSTR path, LPSTR drive, LPSTR dir, LPSTR fname, LPSTR ext)
{
    char buffer[_MAX_PATH];
    LPSTR ptr = buffer;

    if (path == NULL)
	return;

    strcpy(buffer,path);
    if (drive)
	*drive = '\0';
    if (buffer[1] == ':') {
	if (drive != NULL) {
	    drive[0] = buffer[0];
	    drive[1] = ':';
	    drive[2] = '\0';
	}
	ptr += 2;
    }
    if (dir != NULL)
	strcpy(dir,ptr);

    ptr += strlen(ptr);
    if (ext)
	*ext = '\0';
    if (fname)
	*fname = '\0';

    while ((ptr = strpbrkr((dir != NULL && dir[0]) ? dir : buffer,"/:\\."))) {
	if (*ptr == '.' ) {
	    if (ext != NULL)
		strcpy(ext,ptr+1);
	    *ptr = '\0';
	    continue;
	}
	if (is_slash(*ptr) || *ptr == ':' ) {
	    ptr++;
	    strcpy(fname,ptr);
	    *ptr = '\0';
	    break;
	}
    }
    if (!ptr)
	ptr = (dir && dir[0]) ? dir : buffer;
    if (*ptr == ':')
	ptr++;

    if (*ptr)
	strcpy(fname,ptr);
}

/****************************************************************************/
void
_makepath(LPSTR path, LPCSTR drive, LPCSTR dir, LPCSTR fname, LPCSTR ext)
{
    LPSTR ptr = path;
    int cnt = 0,len;
    char slash;

    if (drive && drive[0]) {
	/* if we've got a drive letter, presumably we are dealing */
	/* with DOS-style filename, so use mostly backslashes;	  */
	/* if no drive letter is used, assume UNIX style	  */
	slash = '\\';
	*ptr++ = drive[0];
	*ptr++ = ':';
	cnt += 2;
    }
    else 
	slash = '/';

    if (dir && dir[0]) {
	if (!is_slash(dir[0])) {
	    *ptr++ = slash;
	    cnt++;
	}
	len = strlen(dir);
	if ((cnt + len) >= _MAX_PATH)
	    len = _MAX_PATH - cnt - 1;
	strncpy(ptr,dir,len);
	ptr += len;
	cnt += len;
	if (cnt == _MAX_PATH - 1) {
	    *ptr = '\0';
	    return;
	}
	if (!is_slash(*(ptr-1))) {
	    *ptr++ = slash;
	    cnt++;
	}
    }
    if (fname && fname[0]) {
	len = strlen(fname);
	if ((cnt + len) >= _MAX_PATH)
	    len = _MAX_PATH - cnt - 1;
	strncpy(ptr,fname,len);
	ptr += len;
	cnt += len;
	if (cnt++ >= _MAX_PATH) {
	    *ptr = '\0';
	    return;
	}
	*ptr++ = '.';
    }
    if (ext && ext[0]) {
	len = strlen(ext);
	if ((cnt + len) >= _MAX_PATH)
	    len = _MAX_PATH - cnt - 1;
	strncpy(ptr,ext,len);
	ptr += len;
    }

    *ptr = '\0';
}

LPSTR
_fullpath(LPSTR buffer, LPCSTR pathname, size_t maxlen)
{
   char	altname[_MAX_PATH];

   mfs_config(XMFS_CFG_ALTPATH, 0L, (DWORD)altname, (DWORD)pathname);

   if ( strlen(altname) > maxlen )
	return NULL;

   return strcpy(buffer, altname);
}

#define	DATE_US	2

LPSTR
_strdate(LPSTR datestr)
{
    TWIN_GetCurrentDate(datestr, DATE_US);
    return datestr;
}

char *
strlwr(char *s)
{
    char *p;

    p = s;
    while (*p != 0)
    {
        if (isupper(*p))
            *p = tolower(*p);
        p++;
    }

    return(s);
}


char *
strupr(char *s)
{
    char *p;

    p = s;
    while (*p != 0)
    {
        if (islower(*p))
            *p = toupper(*p);
        p++;
    }

    return(s);
}


char *
strrev(char *str)
{
    char *s1;
    char *s2;
    char tmp;

    /*
     *  Reverse a string in place.
     */

    /*
     *  If no string to reverse, return.
     */
    if ((str == NULL) || (*str == 0))
        return(str);

    /*
     *  Setup pointers to end and beginning of string, and then work
     *  in from both ends, swapping until we meet in the middle.
     */
    s1 = str;
    s2 = str + (strlen(str) - 1);
    while (s1 < s2)
    {
        tmp = *s1;
        *s1 = *s2;
        *s2 = tmp;
        s1++;
        s2--;
    }

    return (str);
}

unsigned long
lrotl(unsigned long value, int shift)
{
    unsigned long upper = value << shift;
    unsigned long lower = value >> (32-shift);

    return (upper | lower);
}

unsigned long
lrotr(unsigned long value, int shift)
{
    unsigned long lower = value >> shift;
    unsigned long upper = value << (32-shift);

    return (upper | lower);
}

#endif /* HAVE_MSCLIB - If MSCLIB not available on Platform. */



/****************************************************************************
**
**  Functions below have no name conflicts with MSCLIB functions. 
**
*****************************************************************************/

/****************************************************************************/
unsigned
_dos_getfileattr(char *pathname, unsigned short *attrib)
{
        unsigned int localattr;
        unsigned rc;
        rc =  (unsigned) mfs_access (0L, (DWORD)pathname, (DWORD)&localattr, 0L);
        *attrib = (short) localattr;
        return rc;
}

/****************************************************************************/
unsigned
_dos_setfileattr(char *pathname, unsigned short attrib)
{
        unsigned int localattr;
        unsigned rc;

	localattr = attrib;
        rc =  (unsigned) mfs_access (1L, (DWORD)pathname, (DWORD)&localattr, 0L);
        return rc;
}

/****************************************************************************/
unsigned
_dos_getftime(int handle, unsigned *date, unsigned *time)
{
	LPMFSFILE fp;
        fp =  (LPMFSFILE) mfs_finfo (handle, 0L, 0L, 0L);
	if ( fp ) {
		*date = fp->adate;
		*time = fp->atime;
		return 0;
	}
        return EBADF;
}

/****************************************************************************/
int
getftime(int handle, struct ftime *ftimep)
{
	memset(ftimep, 0, sizeof(*ftimep));
	return(0);
}


/*
 *  Borland's version of random() conflicts with Unix.  Create ours
 *  here.  We will use #defines to change uses of "random()" to
 *  "borland_random()".
 */
int
borland_random(int num)
{
    int i;
    int j;

    i = RAND_MAX / num;
    i *= num;
    while ((j = rand()) >= i)
        continue;

    return(j%num);
}

long
dostounix(struct date *d, struct time *t)
{
    return(0);
}

void
unixtodos(long time, struct date *d, struct time *t)
{
    return;
}



/*    
	mfs_config.c	1.62
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

*	mfs configuration definitions

 */

#include "platform.h"

#include <string.h>
#include <ctype.h> /* toupper/tolower */
#include <errno.h>

#ifdef NETWARE
#define _SIZE_T_DEFINED_
#define _MAX_PATH 255
#include <direct.h>
#include <nwdir.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#include "windows.h"
#include "BinTypes.h"
#include "Log.h"
#include "WinConfig.h"
#include "compat.h"
#include "dos_error.h"
#include "mfs_config.h"
#include "mfs_core.h"

CONFIG xdos;
/*** #define SEVERE  1 ***/
/*** #define NETWARE 1 ***/

#define LAST_CHAR(s) ((s)[strlen(s) - 1])
#define MFS_MODE_TOUPPER	1	/* convert DOS filenames to uppercase */
#define MFS_MODE_TOLOWER	2	/* convert DOS filenames to lowercase */
#define MFS_MODE_RETRY	4	/* ignore case in filenames, retry any*/
#define MFS_MODE_DEFAULT	(MFS_MODE_TOUPPER|MFS_MODE_TOLOWER|MFS_MODE_RETRY)

#define NFILES		100

unsigned long error_code(unsigned int);
LPARAM GetCompatibilityFlags(UINT);
/*
**   LOCAL FUNCTIONS
*/
void dos_setup();
int  get_path(int, char *, int);

static char *normalize(LPSTR,LPSTR);
static DWORD mfs_dosname(BOOL,LPSTR,LPSTR);
static DWORD mfs_altname(BOOL,LPSTR,LPSTR);
static BOOL already_mapped(int, LPSTR);
static int  drive_type(int);

extern int GetPathType(LPSTR);
extern int GetDriveFromPath(LPSTR);
extern int GetDriveFromHandle(DWORD);
 
/****************************************************************************/
DWORD
mfs_config(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
    UINT	message;   /* p1 */
    UINT	handle;    /* p2 */
    LPVOID      lpvParam1; /* p3 */
    LPVOID	lpvParam2; /* p4 */
    DWORD retcode;

    message = (UINT)p1;
    handle = (UINT)p2;
    lpvParam1 = (LPVOID)p3;
    lpvParam2 = (LPVOID)p4;

    switch (message) {
	case XMFS_CFG_DOSNAME:
		return mfs_dosname(FALSE, (LPSTR)lpvParam1, (LPSTR)lpvParam2);

	case XMFS_CFG_DOSPATH:
		return mfs_dosname(TRUE, (LPSTR)lpvParam1, (LPSTR)lpvParam2);

	case XMFS_CFG_ALTNAME:
		return mfs_altname(FALSE, (LPSTR)lpvParam1, (LPSTR)lpvParam2);

	case XMFS_CFG_ALTPATH:
		return mfs_altname(TRUE,( LPSTR)lpvParam1, (LPSTR)lpvParam2);

	case XMFS_CFG_SETDRIVE: /* 1- A:, 2- B:, etc. as in _dos_setdrive */
                if (xdos.drivemap[handle] != NULL)
                   {
		   xdos.currentdrive = handle;
                   xdos.drivemap[0] = xdos.drivemap[handle];
                   }
	        return (DWORD)xdos.logicaldrives;

	case XMFS_CFG_GETDRIVE: /* 1- A:, 2- B:, etc. as in _dos_getdrive */
    if (xdos.drivemap[xdos.currentdrive])
       return xdos.drivemap[xdos.currentdrive]->drive;
		else return (DWORD)xdos.currentdrive;

	case XMFS_CFG_GETBOOT:
		return (DWORD)xdos.bootdrive;

	case XMFS_CFG_NFILES:
		retcode = xdos.nfiles;
		if (handle)
		   xdos.nfiles = (int)lpvParam2;
		return retcode;

	case XMFS_CFG_GETCONFIG:
		return (DWORD)&xdos;

	case XMFS_CFG_DRIVETYPE:
		return (DWORD)drive_type((int)p2);

	case XMFS_CFG_SETMAXFH:
		xdos.nfiles = (int)p2;
		return 0L;

	case XMFS_CFG_GETDRIVEMAP:
		{
		int   nDrive = (int)p2;
		LPSTR lpszBuf = (LPSTR)p3;
		int   nLength = (int)p4;

		if (nDrive < 0 || nDrive > 32 || xdos.drivemap[nDrive] == NULL)
		   return (DWORD)-1;
		else {
		     strncpy(lpszBuf, xdos.drivemap[nDrive]->name, min(nLength-1, strlen(xdos.drivemap[nDrive]->name)));
		     lpszBuf[min(nLength, strlen(xdos.drivemap[nDrive]->name))] = '\0';	
		     return 0L;
		     }
		}
	case XMFS_CFG_GETROOTDRIVE:
		{
		register i;

		for (i = 0;  i < 32; i++)
		    if (xdos.drivemap[i] != (LPDEVICE)NULL)
                       if (xdos.drivemap[i]->name != NULL)
			  if (strcmp(xdos.drivemap[i]->name, "/") == 0)
		             return (DWORD)i;

	        return 0L;
		}
	
	default:
		break;
    }
    return (DWORD)-1;
}

/****************************************************************************/
int
convert_error(int ErrorNum )
{
	switch(ErrorNum) {
	case EACCES:
		return error_code(ACCESS_DENIED);

	case ENOENT:
		return error_code(FILE_NOT_FOUND);

#ifndef NETWARE
	case ENAMETOOLONG:
		return error_code(FILE_NOT_FOUND);
                         
     	case ENOTDIR:
		return error_code(PATH_NOT_FOUND);
#endif
	}

	return 0;
}

/****************************************************************************/
static LPSTR
normalize(char *new,char *dir)
{
	char cwd[256];	

#ifndef NETWARE
	getcwd(cwd,256);	/* save current directory */
	if (chdir(dir))		/* try to change to destination */
	   *new = 0;	
	else getcwd(new,256);	/* if it's ok, get the full dir spec */
	chdir(cwd);		/* and switch back to saved directory */
#else
	strcpy(new, dir);	/* for NetWare just copy whatever the mapping is... */
#endif
	return new;
}

/****************************************************************************/
LPDEVICE
AllocateDrive(char *device, char *name)
{
        char    dir[256];
	struct stat st;
	DEVICE *dev;

	if (name == NULL || strlen(name) == 0)
	   return 0;

	dev = (DEVICE *) WinMalloc(sizeof(DEVICE));
	dev->device = (LPSTR)WinMalloc(strlen(device)+1);  strcpy(dev->device, device);
	/*
	**   Validate the device letter, and convert the
	**   mapping to type and path
        **
	**   MFS_DRIVE_NATIVE
	**   MFS_DRIVE_FAT
	**   MFS_DRIVE_PSEUDO
	*/
        if (mfs_stat((DWORD)name, (DWORD)&st, 0L, 0L) != 0)
           return NULL;
	if (S_ISDIR(st.st_mode))
	   {
           dev->type = MFS_DRIVE_NATIVE;
           normalize(dir, name); /* get full path */
	   dev->name = (LPSTR)WinMalloc(strlen(dir) +1);  strcpy(dev->name, dir);
           }
        else if (S_ISBLK(st.st_mode))
                {
                dev->type = MFS_DRIVE_FAT;
	        dev->name = (LPSTR)WinMalloc(strlen(name) +1);  strcpy(dev->name, name);
                }
             else if (S_ISREG(st.st_mode))
                     {
                     dev->type = MFS_DRIVE_PSEUDO;
	             dev->name = (LPSTR)WinMalloc(strlen(name) +1);  strcpy(dev->name, name);
                     }
                  else {
		       logstr(LF_ERROR,"Unsupported drive type for '%s'\n",name);
		       return 0;
                       }
	dev->drive = (toupper(*dev->device) - 'A' + 1) & 0x1F;
        dev->cwd = (LPSTR)WinMalloc(2);  
        strcpy(dev->cwd, "/");
	/*
	**   Figure out the mapping... 
	*/
	if (xdos.drivemap[dev->drive]) 
	   logstr(LF_ERROR,"duplicate drive mapping for %s\n",dev->device);
	else 
           xdos.drivemap[dev->drive] = dev;

	return dev;
}

/****************************************************************************/
static int
AllocateEntry(char *name)
{
	int  drive;
	char device[8];

	/*
	**   Find first available entry in xdos.drivemap and
	**   fill it in with the given name.
	*/
	for (drive = 3;   drive < 32;   drive++) 
	    {
	    if (xdos.drivemap[drive])
	       continue;	
	    sprintf(device, "%c", drive + 'A' - 1);
	    AllocateDrive(device, name);
	    break;
	    }
	return drive;
}

/****************************************************************************
**	
**	setup DOS drive mapping
**	
*****************************************************************************/
void
dos_setup()
{
	char devnames[1024];
	char device[256];
	char *lpszIniFile;
	char *key;
	int  n;

	/* 
	**   Go get all the strings in the xdos section... 
	*/
	lpszIniFile = GetTwinFilename();
	GetPrivateProfileString("xdos",0,0,devnames,1024,lpszIniFile);
	/* 
	**   How many drives, and which is last... 
	*/
	xdos.logicaldrives = 0;
	xdos.lastdrive     = 0;
	/* 
	**   Go through each string, and look it up... 
	*/
	for (key = devnames;   *key != 0;   key += strlen(key) + 1) 
	    {
	    GetPrivateProfileString(
			(LPSTR)"xdos",
			(LPSTR) key,
			(LPSTR) 0,		/* no default value... */
			device,
			128,
			(LPSTR)lpszIniFile);

	    if (*key == '#' || *key == ';')
		continue;

	    AllocateDrive(key, device);
	    }
	/*
	**   Add option for setting current drive to higher directory 
	**   ie. rather than current directory, it might be ../.. 
	*/
	if (GetCompatibilityFlags(0) & WD_MAPCURDIR) 
	   {
	        /*   Map the current directory and then the root */
	     	getcwd(device, 256);
	     	xdos.currentdrive = AllocateEntry(device);
	     	xdos.bootdrive    = AllocateEntry("/");
	   }
	else {
	   	/*   Map root directory only */
	   	xdos.bootdrive    = AllocateEntry("/");
	   	xdos.currentdrive = xdos.bootdrive;
	     }
	/*
	*/
	xdos.dosmode   = GetTwinInt(WCP_DOSMODE);
	xdos.memory    = GetTwinInt(WCP_MEMORY);
	xdos.extended  = GetTwinInt(WCP_EXTENDED);
	xdos.nfiles    = NFILES;

           xdos.fileio  = (LPMFSFILE)WinMalloc(sizeof(MFSFILE) * xdos.nfiles);
           xdos.fileptr = (LPMFSFILE *)WinMalloc(sizeof(LPMFSFILE) * xdos.nfiles);
           memset((LPSTR)xdos.fileio,0,sizeof(MFSFILE) * xdos.nfiles);
           memset((LPSTR)xdos.fileptr,0,sizeof(LPMFSFILE) * xdos.nfiles);
 
           /* reserve 0-4 */
           xdos.fileio[0].usage = -1;
           xdos.fileio[1].usage = -1;
           xdos.fileio[2].usage = -1;
           xdos.fileio[3].usage = -1;
           xdos.fileio[4].usage = -1;
 
           xdos.fileptr[0] = &xdos.fileio[0];
           xdos.fileptr[1] = &xdos.fileio[1];
           xdos.fileptr[2] = &xdos.fileio[2];
           xdos.fileptr[3] = &xdos.fileio[3];
           xdos.fileptr[4] = &xdos.fileio[4];

	   logstr(LF_SYSTEM,"XDOS Configuration\n------------------\n");
	   for (n = 1;   n < 32;   n++) {
	     if (xdos.drivemap[n]) {
	       logstr(LF_SYSTEM,"%c:\t%s\n", 
			n +'A' - 1,xdos.drivemap[n]->name);
	       
	       xdos.logicaldrives++;
	       if (xdos.drivemap[n]->drive > xdos.lastdrive)
		  xdos.lastdrive = xdos.drivemap[n]->drive;

	       if (xdos.drivemap[n]->type == MFS_DRIVE_PSEUDO)
                  xdos.drivemap[n]->lpfnDispatch = FATProc;
	       else if (xdos.drivemap[n]->type == MFS_DRIVE_FAT)
                       xdos.drivemap[n]->lpfnDispatch = FATProc;
                    else xdos.drivemap[n]->lpfnDispatch = MFSProc;
	       }		
	    }
	
	   logstr(LF_VERBOSE,"LASTDRIVE = %c\n",xdos.lastdrive+'A'-1);
	   logstr(LF_VERBOSE,"DRIVES    = %d\n",xdos.logicaldrives);
	   logstr(LF_VERBOSE,"BOOTDRIVE = %c\n",xdos.bootdrive+'A'-1);
	   logstr(LF_VERBOSE,"ROOTDRIVE = %d\n",xdos.currentdrive);
	   logstr(LF_VERBOSE,"MEMORY    = %dKb\n",xdos.memory);
	   logstr(LF_VERBOSE,"EXTENDED  = %dKb\n",xdos.extended);
	   logstr(LF_VERBOSE,"FILES     = %d\n",xdos.nfiles);
        getcwd(device, sizeof(device));
	if (xdos.drivemap[xdos.currentdrive])
	{
	    if (xdos.drivemap[xdos.currentdrive]->cwd)
		WinFree(xdos.drivemap[xdos.currentdrive]->cwd);
	    xdos.drivemap[xdos.currentdrive]->cwd = (LPSTR)WinMalloc(strlen(device)+1);
            strcpy(xdos.drivemap[xdos.currentdrive]->cwd, device);
	    xdos.drivemap[0] = xdos.drivemap[xdos.currentdrive];
	}
	else
	    xdos.drivemap[0] = xdos.drivemap[xdos.lastdrive];
}

/****************************************************************************/
static BOOL already_mapped(int drive, LPSTR source)
{
    char *pn, *ps;
    BOOL  ynMapped = 1;

    if (strcmp(source, "/") == 0 || strcmp(source, "\\") == 0)
    {
        /* C:\ is definitely NOT mapped */
        return 0;
    }

    for (pn = xdos.drivemap[drive]->name,ps = source;  *pn && *ps;  pn++, ps++)
    {
        if ((*pn == '\\' && *ps == '/') || (*pn == '/' && *ps == '\\'))
            continue;
        else
        {
            if (GetCompatibilityFlags(0) & WD_NOCHANGECASE)
                ynMapped = ((*ps) == (*pn));
	    else
                ynMapped = (tolower(*ps) == tolower(*pn));
            if (!ynMapped)
               return 0;
            }
       }
    return 1;
}


/****************************************************************************
**
**   Convert 'source' to NATIVE file specification (i.e. Unix)
**   and stores it in 'target'. fExpand specifies, whether to 
**   return full "normalized" specification.
**
**   MiD  27-NOV-1995  Don't expect colon in source[1], since NetWare
**                     drives may be longer than 1 letter. Support
**                     NetWare as NATIVE file system, i.e.
**
**                     server/volume:/directory/file.ext
**
*****************************************************************************/
static DWORD
mfs_altname(BOOL fExpand, LPSTR target, LPSTR source)
{
    char   buf[_MAX_PATH];
    LPSTR  t,s,lpTemp;
    int    drive;

    t = buf;		/* setup pointers for copy... */
    s = source;
    
    if (GetCompatibilityFlags(0) & WD_MAPCURDIR)
    	drive = xdos.bootdrive;
    else    	
	drive = xdos.currentdrive;
	
#ifndef NETWARE
    /* 
    **   This is a case of chdir('\') going to "/" 
    */
    if (source[1] == ':' || source[0] == '\\') 
       {
       /* do we have a drive spec? 
       */
       if (source[1] == ':') 
          {
	  drive = GetDriveFromPath(s);
	  s += 2;  		/* skip over the drive... */ 
	  if (*(s+1) == '\\' || *(s+1) == '/') 	/* as in D:\\WINAPPS... */
	     s++;
          }	
#else
    lpTemp = strchr(source, ':');
    if (lpTemp != NULL || source[0] == '\\' || source[0] == '/') 
       {
       /* do we have a drive spec? 
       */
       if (lpTemp && *lpTemp == ':') 
          {	
	  /* get drive number checking the mapping in xdos.drivemap[] */
	  drive = GetDriveFromPath(s);	

	  /* skip over the drive... */ 
	  s += (lpTemp - &source[0] + 1);  	

          /* now 's' points to 1st path character...  */
	  if (*(s+1) == '/' || *(s+1) == '\\')	
	     s++;				
          }	
#endif

       if (drive > 0 && drive <= xdos.lastdrive && xdos.drivemap[drive]) 
          {
          if (!already_mapped(drive, s))
             {
	     strcpy(buf, xdos.drivemap[drive]->name); /* substitute drive with Unix mapping */
	     if (*s != '\\')                          /* add leading /... */
	        strcat(buf,"/");
	     else if (LAST_CHAR(buf) == '/' || LAST_CHAR(buf) == '\\')
	  	     s++;
	     t += strlen(buf);
             }
          else buf[0] = '\0';
	  }
       }
    /*
    **   At this point were're supposed to have something like that:
    **
    **     source: "D:\WINAPPS\BUDGET.XLS'
    **     target: "/winapps/"
    **
    **   Now copy the rest of the source, converting slashes and
    **   changing to lowercase...
    */
    for (;  *s;  s++,t++) 
        {
        if (GetCompatibilityFlags(0) & WD_NOCHANGECASE)
	    *t = *s;
	else
	    *t = tolower(*s);
	if (*t == '\\')
	   *t = '/';
        }
    *t = 0;
    strcpy(target, buf);

#ifndef NETWARE /* don't do the rest for NetWare file specs... */
    if (fExpand) 
       {
       s = target;
       t = buf;

       if (*s != '/') 
          {	/* not a valid UNIX path */
	  getcwd(buf, _MAX_PATH);
	  t += strlen(buf);
	  *t++ = '/';
	  }
       while (*s) 
          {
	  if ((*s == '.') && (*(s+1) == '/')) 
             {
	     s += 2;
	     continue;
	     }
	  while ((*s == '.') && (*(s+1) == '.')) 
             {
	     if (*(t-1) == '/')
		*(t-1) = 0;
	     else *t = 0;
	     lpTemp = strrchr(buf, '/');
	     if (lpTemp)
                {
	        *(++lpTemp) = '\0';
		t = lpTemp;
		}
             else break;
	     if ( *(s+2) == '/' )
		s += 3;
             else s += 2;
	     continue;
	     } /* while ((*s... */

	  *t++ = *s++;
	  } /* while (*s) */

       *t = '\0';
       strcpy(target,buf);
       } /* if (fExpand) */
#endif /* NETWARE */

    /*
    ** MAKE SURE THERE ARE NO DOUBLE SLASHES...
    */
    for (t = target;  *t;  t++)
        if (*t == '/' && *(t+1) == '/') 
           if (*(t+2) != '\0')
              strcpy((char *)(t+1), (char *)(t+2));
           else *(t+1) = '\0';
    
    return drive;
}

/****************************************************************************
**
**   Convert 'source' from native to DOS file specification
**   and stores it in 'target'. 
**
*****************************************************************************/
static DWORD
mfs_dosname(BOOL Flags, LPSTR target, LPSTR source)
{
	int      drive;
	int      len = 0;
	int      match = 0;
	DEVICE  *dev;
	char    *p,*s;

	/* 
        **   Do we already have a DOS filename? 
        */
	if (source[1] == ':') /* do we have a drive spec?   */
           {
	   for (s = target, p = source;   *p;   p++,s++) 
               {
               if (GetCompatibilityFlags(0) & WD_NOCHANGECASE)
		   *s = *p;
	       else
	           *s = toupper(*p);

	       if (*s == '/' )
		  *s = '\\';
	       }
	   *s = '\0';
	   target[0] = toupper(target[0]); /* Do this even if NOCHANGECASE */
	   match = target[0] - 'A' + 1;
	   goto dosname_exit;
	   }
	/* 
        **   Go through each drive to figure out whether the given
        **   name maps to any drive in our list. If the match is
        **   found, then substitute the name with the drive letter.
        **
        **   Michael Dvorkin  10-JAN-1996  start from drive A:, which
        **                                 is 1 
        */
	for (drive = 1;   drive <= xdos.lastdrive;   drive++) 
            {
	    dev = xdos.drivemap[drive];	
	    if (dev) 
               {
	       if (strncmp(dev->name, source, strlen(dev->name)) == 0) 
                  {
		  if (strlen(dev->name) > len) 
                     {
		     match = drive;
		     len   = strlen(dev->name);
		     if (len == 1) /* root drive match */
		        len = 0;
		     }
		  }
	       }
	    }

	if (match == 0)
	   if (GetCompatibilityFlags(0) & WD_MAPCURDIR)
                match = xdos.bootdrive;
           else 
	      	match = xdos.currentdrive;
        /*
        */
	if (match) 
           {
           if (Flags) /* include drive letter */
              {
              if (strcmp(source, "/") == 0)
                 match = xdos.currentdrive;
 
              if (*(source+len) == '/' || *(source+len) == '\\' || 
                  (*(source+len) == '.' && *(source+len+1) == '.')) /* ../abc should be mapped to X:../ABC */
	         sprintf(target,"%c:%s", *xdos.drivemap[match]->device, source+len);
              /*
              **   Check for ./filename... Substitute with current dir, but
              **   do it carefully, since drive itself can be mapped to that dir.
              */
              else if (*(source) == '.' && (*(source+1) == '/' || *(source+1) == '\\'))
                      {
                      sprintf(target, "%c:", *xdos.drivemap[match]->device);

#ifdef LATER
	              if (GetCompatibilityFlags(0) & WD_NOMAPCURDIR)
                         {
                         char  CurDir[_MAX_PATH];
                         mfs_getcwd(0L, (DWORD)CurDir, (DWORD)_MAX_PATH, 0L);
                         strcat(target, CurDir);
                         }
#endif

                      if (LAST_CHAR(target) != '/' && LAST_CHAR(target) != '\\')
                        strcat(target, "\\");
                      	strcat(target, (char *)(source+2));
                      }
	              else 
			sprintf(target,"%c:\\%s", 
				*xdos.drivemap[match]->device, source+len);
	      } 
           else {
                strcpy(target, source);
                /*  Tarang's Excel
		*target = 0;
		if (len != 0 && *(source+len) == '/')
		   len++;
		sprintf(target,"%s", (char *)(source+len));
                */
		}
	   /* 
           **   Convert slashes and make uppercase... 
           */
	   for (p = target;   *p;   p++) 
               {
               if ((GetCompatibilityFlags(0) & WD_NOCHANGECASE) == 0)
	          *p = toupper(*p);
	       if (*p == '/')
		  *p = '\\';
               }
	   if (target[1] == ':')
	      target[0] = toupper(target[0]);  /* Ignore NOCHANGE for drive */
	   } 
        else {
	     *target = 0;	
	     }
dosname_exit:
    	/*
    	** MAKE SURE THERE ARE NO DOUBLE SLASHES...
	*/
    	for (p = target;  *p;  p++)
            if (*p == '\\' && *(p+1) == '\\') 
               if (*(p+2) != '\0')
                  strcpy((char *)(p+1), (char *)(p+2));
               else *(p+1) = '\0';
	
	return (DWORD)match;	
}

/****************************************************************************/
static int
drive_type(int drive) /* 0- A:, 1- B:, etc. */
{
	if (drive < 0 || drive > 31)
	   return 0;

	if (xdos.drivemap[drive] == NULL)
	   return 0;

	switch(xdos.drivemap[drive]->type)
	   {
	   case MFS_DRIVE_NATIVE:
		return DRIVE_FIXED;

	   case MFS_DRIVE_FAT:
	   case MFS_DRIVE_PSEUDO:
		return DRIVE_REMOVABLE;
	   }
	return 0;
}

/****************************************************************************/
int /* called from fn_36.c */
get_path(int drive, char *file, int len)
{
        if (xdos.drivemap[drive])
           {
           strncpy(file, xdos.drivemap[drive]->name, len);
           return 0;
           }
        return -1;
}
 




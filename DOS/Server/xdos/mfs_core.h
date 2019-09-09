/*************************************************************************

       	@(#)mfs_core.h	1.7
	multiple file system access definitions
      	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.

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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

**************************************************************************/


#ifndef mfs_core__h
#define mfs_core__h
/*
**   Utility functions and constants common to both native and FAT
**   filesystems...
*/
int       closefile(LPMFSFILE);
int       freeindex(int);
int       gethandle(void);
LPMFSFILE checkhandle(int, int);
char *    DriveMapping(int);

DWORD mfs_init(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_config(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_stat(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_access(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_open(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_close(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_read(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_write(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_seek(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_flush(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_create(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_delete(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_rename(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_mkdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_chdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_getcwd(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_regexp(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_opendir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_readdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_closedir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_fcntl(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_ioctl(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_findfirst(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_findnext(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_findfile(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_makepath(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_rmdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_touch(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_splitpath(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_makepath(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_finfo(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_copy(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_filedrive(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_diskfree(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD mfs_fileinfo(DWORD p1, DWORD p2, DWORD p3, DWORD p4);

DWORD fat_stat(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_access(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_open(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_close(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_read(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_write(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_seek(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_flush(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_create(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_delete(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_rename(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_mkdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_chdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_getcwd(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_regexp(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_opendir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_readdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_closedir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_fcntl(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_ioctl(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_findfirst(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_findnext(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_findfile(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_rmdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4);
DWORD fat_touch(DWORD p1, DWORD p2, DWORD p3, DWORD p4);

#endif


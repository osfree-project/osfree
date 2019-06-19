/*************************************************************************
*
*       @(#)dosdefn.h	1.9
*	definitions for standard dos file functions	
*       Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.

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

/*******************************************
**
**	dos definitions 
**	
**
********************************************/

#ifndef dosdefn__h
#define dosdefn__h

#include <stdio.h>			/* appropriate unix header files */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#define GETCWD	0x0000		/* drive is tacked on for getcwd */
#define CHDIR	0x0100
#define RMDIR	0x0200
#define MKDIR	0x0400

/* Definitions for handle_id function */

#define DOS_CREATE_FILE		0x3c
#define DOS_CREATE_TMPF		0x5a
#define DOS_CREATE_NEWF		0x5b

/* By coincidence, windows.h defines NO_ERROR as 0 */

#if 0
#define NO_ERROR			0x00
#endif

#define	INVALID_FUNCTION		0x01	/* dos error codes */
#define	FILE_NOT_FOUND			0x02
#define	PATH_NOT_FOUND			0x03
#define	NO_HANDLES			0x04
#define	ACCESS_DENIED			0x05
#define	INVALID_HANDLE			0x06
#define	MEMORY_DESTROYED		0x07
#define	INSUFFICIENT_MEMORY		0x08
#define	INVALID_ADDRESS			0x09
#define	INVALID_ENVIRONMENT		0x0a
#define	INVALID_FORMAT			0x0b
#define	INVALID_ACCESS			0x0c
#define	INVALID_DATA			0x0d
#define	INVALID_DRIVE			0x0f
#define	REDIR_OFF_SERVER		0x10
#define	NOT_SAME_DEVICE			0x11
#define	NO_MORE_FILES			0x12
#define SEEK_ERROR            		0x19
#define	SHARING_VIOLATION		0x20
#define	LOCK_VIOLATION			0x21
#define SHARE_BUF_EXCEED      		0x24
#define	NETWORK_REQUEST_UNSUPPORTED	0x32
#define	FILE_EXISTS			0x50
#define	CANNOT_MAKE_DIRECTORY_ENTRY	0x52
#define	FAIL_ON_INT24			0x53
#define	TOO_MANY_REDIRECTIONS		0x54
#define	DUPLICATE_REDIRECTION		0x55
#define	INVALID_PASSWORD		0x56
#define	INVALID_PARAMETER		0x57
#define NOV_TIMEOUT			0xfe
#define NOV_LOCK_FAILURE		0xff


#define	NORMAL_FILE	     0x00	/* file attribute bits */
#define	READONLY_FILE	     0x01
#define	HIDDEN_FILE	     0x02
#define	SYSTEM_FILE	     0x04
#define	VOLUME_LABEL	     0x08
#define	SUB_DIRECTORY	     0x10
#define	ARCHIVE_FLAG	     0x20
#define NOV_SHAREABLE		0x80	/* Novell shareable - dummy now */

#define	NO_INHERIT_FILE		128	/* Child does not inherit file handle */

#define	MAX_DRIVES		26	/* maximum number of disk drives */
#define	MAX_FILES		20	/* Default value for FILES statement
					   in SYSTEM.INI */
#endif /* dosdefn__h */

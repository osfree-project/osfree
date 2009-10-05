/*    
	dos_call.c	1.8
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

 */

#include "windows.h"
#include "BinTypes.h"
#include "dos.h"
#include "xdosproc.h"
#include "mfs_config.h"
#include "mfs_core.h"

void IVTInit(void);

DWORD
xdoscall(UINT cmd,UINT flag,LPVOID lpvParam1,LPVOID lpvParam2)
{
    ENV	*lp;	

    switch (cmd) {
	case XDOS_INIT:
		IVTInit();
		break;

	case XDOS_GETDOSNAME:
		mfs_config(XMFS_CFG_DOSNAME,0L,(DWORD)lpvParam1,(DWORD)lpvParam2);
		break;

	case XDOS_GETDOSPATH:
		mfs_config(XMFS_CFG_DOSPATH,0L,(DWORD)lpvParam1,(DWORD)lpvParam2);
		break;

	case XDOS_GETALTNAME:
		mfs_config(XMFS_CFG_ALTNAME,0L,(DWORD)lpvParam1,(DWORD)lpvParam2);
		break;

	case XDOS_GETALTPATH:
		mfs_config(XMFS_CFG_ALTPATH,0L,(DWORD)lpvParam1,(DWORD)lpvParam2);
		break;

	case XDOS_INT86:
		lp = (ENV *)lpvParam2;
		INT_handler(flag,lpvParam2);
		break;

	case XDOS_SETHANDLE:
		return mfs_config(XMFS_CFG_NFILES,1L,0L,(DWORD)(LPVOID)flag);

	default:
		return 0L;
    }
    return 0L;
}

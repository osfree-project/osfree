/*    
	dos_ioctl.c	1.6
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
#include "dos.h"
#include "mfs_config.h"
#include "Log.h"

void dos_ioctl()
{
	unsigned int info;
	func = envp->reg.ax & 0xff;

	switch (func) {

	case 0x00:		/* get device data */
		LOGSTR((LF_INT86, "INT 21/44/IOCTL: GETDEVINFO handle: %d\n",
			envp->reg.bx));

		result = mfs_config((DWORD)envp->reg.bx, MFS_FINFO, 0L, 0L);
		if(result == -1) {
			envp->reg.ax = result & 0xffff;
			envp->reg.flags = (result & CARRY_FLAG) >> 16;
		} else 
			envp->reg.dx = result & 0xffff;
		break;

	case 0x01:		/* set device data */
		LOGSTR((LF_INT86,"INT 21/44/IOCTL: SETDEVINFO handle: %x\n",
			envp->reg.bx));

		result = mfs_fcntl((DWORD)envp->reg.bx, MFS_SETINFO, 0L, 0L);
		if(result == -1) {
			envp->reg.ax = result & 0xffff;
			envp->reg.flags = (result & CARRY_FLAG) >> 16;
		} else
			envp->reg.dx = result & 0xffff;
		break;

}

/*    
	int_21.c	1.33
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

*   	DOS Interrupt 0x21

 */

#include <string.h>
#include "windows.h"
#include "kerndef.h"
#include "BinTypes.h"
#include "Kernel.h"
#include "xdos.h"
#include "dosdefn.h"
#include "xdosproc.h"
#include "xtime.h"
#include "Log.h"
#include "Endian.h"
#include "DPMI.h"
#include "dos.h"
#include "mfs_config.h"
#include "mfs_core.h"
#include "mfs_fileio.h"

static int 	disk_verify;
static DWORD 	dta_address;

void time_date(unsigned short, unsigned long *,
               unsigned long *,  unsigned long *);
unsigned int disk_free(int, unsigned int *, unsigned int *,
                       unsigned int *, unsigned int *);
unsigned int GetPSPSelector(void);
unsigned int country_info(int, char *);
unsigned long error_code(unsigned int);
void TWIN_SetPSPSelector(HTASK, WORD);


static WORD CodePage = 437;


void
int_21(int nNumber, register ENV *envp)
{
    DWORD  	  lpfnAddr;
    unsigned long other_flags;
    long 	  distance, location;
    int		  func, result=0, drive, mode, handle=0, count;
    unsigned int  attribute;
    char 	  *address, *address1;
    TASKINFO      *lpTaskInfo;
    HTASK 	  hTask;
    ERRORCODE  	  *err;
    char	  buffer[256];
    LPBYTE 	  dta;

    /*
    ** We need to preserve the flag bits. Many functions fiddle with
    ** CARRY. A very limited few fiddle with ZERO. We save all the flags
    ** other than CARRY in other_flags. A function that returns info
    ** in the flags register clears out all other flags. Then when
    ** interrupt processing is complete, we OR other_flags in with
    ** the resulting flags value.
    **
    ** There are four cases:
    ** 1) The function does not modify any flags. The OR at the end is
    **    then a no-op
    ** 2) The function modifies CARRY, but returns it clear. The OR at
    **    the end ORs the zero value in the flags register with the saved
    **    flags, therefore restoring all the flags other than CARRY.
    ** 3) The function returns CARRY set. The flags register contains
    **    only the CARRY, which is ORed with the saved flags.
    ** 4) The function fiddles with a flag other than CARRY. In this
    **    case, the code must manipulate regs->flags and other_flags
    **    directly. The easiest way to do this is to set other_flags
    **    to 0 so the OR at the end has no effect.
    */

	other_flags = envp->reg.flags & ~(CARRY_FLAG >> 16);
	func = HIBYTE(LOWORD(envp->reg.ax));

    	switch(func) {
	    case 0x0e:	/* Select Disk Drive */
		    result = envp->reg.dx & 0xff;
		    LOGSTR((LF_INT86,"INT21/0e/SelectDrive: %d (%c:)\n",
			    result, (char)(result & 0xff)+'A'));
		    result = mfs_config(XMFS_CFG_SETDRIVE,(DWORD)(result+1),0L,0L); /* XMFS_CFG_SETDRIVE accepts 1 for A:, etc. */
		    envp->reg.ax = (envp->reg.ax & 0xFF00) | (result & 0xFF); /* XMFS_CFG_SETDRIVE returns number of drives available */

		    break;
	
	    case 0x19: /* Get Current Disk */
		    result = mfs_config(XMFS_CFG_GETDRIVE,0L,0L,0L);
		    LOGSTR((LF_INT86,"DOS 21/19/GetCurrentDrive: %d (%c:)\n",
			    result-1, (char)(result-1+'A')));
		    envp->reg.ax = 0x1900 | ((result-1) & 0xFF);/* XMFS_CFG_GETDRIVE returns 1 for A:, etc. as _dos_getdrive */
		    break;

	    case 0x1a: /* Set DTA Address */
		    LOGSTR((LF_INT86,"DOS 21/1a/SetDTA: %x:%x\n",
				envp->reg.ds,envp->reg.dx));

		    dta_address = MAKELONG(LOWORD(envp->reg.dx),
						LOWORD(envp->reg.ds));
		    break;

	    case 0x1c: /* GetDriveData */
		    LOGSTR((LF_INT86,"DOS 21/1c/GetDriveData: drive=%x\n",
				LOBYTE(envp->reg.dx)));

		    envp->reg.ax = 0x1CFF;
/***
 OR:
		    envp->reg.ax = 0x1Cxx;	xx = sectors/cluster
		    envp->reg.ds = ssss;	ssss:oooo = DS:BX -> media byte
		    envp->reg.bx = oooo;
		    envp->reg.cx = yyyy;        yyyy = bytes/sector
		    envp->reg.dx = zzzz;        zzzz = clusters/drive

                    media byte = mm;            mm = 0xF8 (fixed disk)
***/
		    break;

	    case 0x25: /* Set Interrupt Vector */
		    LOGSTR((LF_INT86,"DOS 21/25/SetVector: %x\n", 
			    envp->reg.ax & 0xff));
		    SetVector( 0L, envp );
		    envp->reg.flags = 0;
		    break;

 	    case 0x2a:  /* Get Date */
		    time_date(GET_DATE,&(envp->reg.ax), &(envp->reg.cx),
			     &(envp->reg.dx));
		    LOGSTR((LF_INT86, "DOS 21/2a/GetDate: %d.%d.%d\n",
			(envp->reg.dx & 0xff00)>>8,
			envp->reg.dx & 0xff, envp->reg.cx));
		    break;

	    case 0x2b:  /* Set Date */
		    LOGSTR((LF_INT86, "DOS 21/2a/SetDate: %d.%d.%d\n",
			envp->reg.dx & 0xff00, envp->reg.dx & 0xff, envp->reg.cx));
		    time_date(SET_DATE,&(envp->reg.ax), &(envp->reg.cx),
			     &(envp->reg.dx));
		    break;

	    case 0x2c:  /* Get Time */
		    time_date(GET_TIME,&(envp->reg.ax), &(envp->reg.cx),
			     &(envp->reg.dx));
		    LOGSTR((LF_INT86, "DOS 21/2c/GetTime: %d:%d:%d\n",
			     envp->reg.cx & 0xff00, envp->reg.cx & 0xff, 
			     envp->reg.dx & 0xff00));
		    break;

	    case 0x2d:  /* Set Time */
		    LOGSTR((LF_INT86, "DOS 21/2c/SetTime: %d:%d:%d\n",
			     envp->reg.cx & 0xff00, envp->reg.cx & 0xff, 
			     envp->reg.dx & 0xff00));
		    time_date(SET_TIME,&(envp->reg.ax), &(envp->reg.cx),
			     &(envp->reg.dx));
		    break;

	    case 0x2e:  /* Set Verify Flag (???? b.p.) */
		    LOGSTR((LF_INT86,"DOS 21/2e/SetVerFlag: %s\n",
			(envp->reg.ax) ? "ON" : "OFF"));
		    result = ((envp->reg.ax & 1) ? 1 : 0);
		    envp->reg.ax = (envp->reg.ax & 0xff00) | (result & 0xff);
		    break;

	    case 0x2f:	/* Get DTA address */
		    if (dta_address == 0) {
			hTask = GetCurrentTask();
			lpTaskInfo = (TASKINFO *)GETTASKINFO(hTask);
			dta_address = MAKELONG(0x80,GetPSPSelector());
		    }
		    LOGSTR((LF_INT86,"DOS 21/2f/GetDTA: %x:%x\n",
			HIWORD(dta_address),LOWORD(dta_address)));
		    envp->reg.es = HIWORD(dta_address);
		    envp->reg.bx = LOWORD(dta_address);
		    break;

	    case 0x30:  /* Get DOS Version Number */
		{
		    int nMaj, nMin;
		    int dosversion;

		    dosversion = HIWORD(GetVersion());
		    nMaj = HIBYTE(dosversion);
		    nMin = LOBYTE(dosversion);
		    LOGSTR((LF_INT86,"DOS 21/30/GetDOSVer: %d.%d\n",nMaj, nMin));
		    envp->reg.ax = (nMin << 8) + nMaj;
		    envp->reg.bx = 0;
		    envp->reg.cx = 0;
		    break;
		}

	    case 0x33:	/* Get/Set Break Status; Get Boot Drive */
		    LOGSTR((LF_INT86,"DOS 21/33/GetSetBreak_GetBootDrive: al=%x\n",
			    envp->reg.ax & 0xff));
		    switch(envp->reg.ax & 0xff){
			  case 1:			/* Set break flag */
			  case 0:			/* Get break flag */
				break;
			  case 5:
				/* The drive number seems to have different	*/
				/* interpretations in dos depending on which	*/
				/* function is called - configurer sets drive	*/
				/* A to be 0. This guy wants drive A to be 1. 	*/

		    	        result = mfs_config(XMFS_CFG_GETBOOT,0L,0L,0L);
				envp->reg.dx = (envp->reg.dx & 0xff00) + result;
				break;
			  default: 
				logstr(LF_ERROR, 
					"DOS INT 21 invalid mode 33h\n");
				break;
		    }
		    break;


	    case 0x35:  /* Get Interrupt Vector */
		    LOGSTR((LF_INT86,"DOS 21/35/GetVector: %x\n",
			    envp->reg.ax & 0xff));
		    lpfnAddr = GetVector( LOWORD(LOBYTE(envp->reg.ax)) ); 
		    envp->reg.es = (REGISTER)HIWORD( lpfnAddr );
		    envp->reg.bx = (REGISTER)LOWORD( lpfnAddr );
		    envp->reg.flags  = 0;	/* ??? */
		    break;

	    case 0x36:  /* Get Disk Free Space */
		{
		    unsigned int freeclusters, clustperdrive;
		    unsigned int bytespersect, sectperclust;

		    drive = envp->reg.dx & 0xff;
		    result = disk_free(drive, &freeclusters, &clustperdrive,
				       &bytespersect, &sectperclust);
		    LOGSTR((LF_INT86,"DOS 21/35/GetDiskFreeSpace: ax: %x\n",
			     result & 0xffff));
		    if ((envp->reg.ax = result & 0xffff) != 0xffff) {
			envp->reg.bx = freeclusters & 0xffff;
			envp->reg.dx = clustperdrive & 0xffff;
			envp->reg.cx = bytespersect & 0xffff;
			envp->reg.ax = sectperclust & 0xffff;
		    }
		    break;
		}

	    case 0x38:  /* Get/Set Country-Dependent Information */
		    LOGSTR((LF_INT86,"DOS 21/38/GetSetCountryInfo\n"));
		    mode = envp->reg.ax & 0xff;
		    if (mode == 0xff) 
			mode = envp->reg.bx & 0xffff;
		    if (envp->reg.dx == 0xffff)
			address = (char *)NULL;
		    else
			address =(char *)GetAddress(envp->reg.ds, envp->reg.dx);
		    result = country_info(mode, address);
		    if (result & CARRY_FLAG) {
			envp->reg.ax = result & 0xffff;
			envp->reg.flags = CARRY_FLAG >> 16;
		    }
		    else
			envp->reg.flags = 0;
		    break;

	    case 0x39:  /* Create Directory */
		    address = (char *)GetAddress(envp->reg.ds, envp->reg.dx);
		    LOGSTR((LF_INT86,"DOS 21/39/mkdir: %s\n", (LPSTR)address));
		    result = MFS_MKDIR((DWORD)address, 0xFFFFFFFF);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x3a:  /* Remove Directory */
		    address = (char *)GetAddress(envp->reg.ds, envp->reg.dx);
		    LOGSTR((LF_INT86,"DOS 21/3a/rmdir: %s\n", (LPSTR)address));
		    result = MFS_RMDIR((DWORD)address);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x3b:  /* Change Current Directory */
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    LOGSTR((LF_INT86,"DOS 21/3b/chdir: %s\n", address));
		    result = MFS_CHDIR((DWORD)address);
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    envp->reg.ax = result & 0xffff;
		    break;

	    case 0x3c:  /* Create File */
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    attribute = envp->reg.cx & 0xffff;
		    result = MFS_CREATE ((DWORD)address, (DWORD)attribute);
		    LOGSTR((LF_INT86,"DOS 21/3c/CreateFile: %s attr: %x result: %x\n",
			address, attribute, result ));
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;
		    
	    case 0x3d:  /* Open Handle */
		    {
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    mode = envp->reg.ax & 0xff;
		    result = MFS_OPEN((DWORD)address, (DWORD)mode);
		    LOGSTR((LF_INT86,"DOS 21/3d/OpenFile: %s mode: %x result: %x\n",
			     address, mode, result));
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;
		    }

	    case 0x3e:  /* Close Handle */
		    LOGSTR((LF_INT86,"DOS 21/3e/CloseFile: handle: %x\n",
			     envp->reg.bx));
		    handle = envp->reg.bx & 0xffff;
		    result = MFS_CLOSE((DWORD)handle);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x3f:  /* Read from File or Device */
		    handle = envp->reg.bx & 0xffff;
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    count = envp->reg.cx & 0xffff;
		    LOGSTR((LF_INT86,"DOS 21/3f/ReadFile: handle: %x bytes: %d\n",
			     envp->reg.bx,envp->reg.cx));
		    result = MFS_READ((DWORD)handle, (DWORD)address, (DWORD)count);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    LOGSTR((LF_INT86,"DOS 21/3f/ReadFile: return %d\n", envp->reg.ax));
		    break;

	    case 0x40:  /* Write to File or Device */
		    handle = envp->reg.bx & 0xffff;
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    count = envp->reg.cx & 0xffff;
		    LOGSTR((LF_INT86,"DOS 21/40/WriteFile: handle: %x bytes: %x\n",
			     envp->reg.bx,envp->reg.cx));
		    result = MFS_WRITE((DWORD)handle, (DWORD)address, (DWORD)count);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    LOGSTR((LF_INT86,"DOS 21/3f/WriteFile: return %d\n", envp->reg.ax));
		    break;

	    case 0x41:  /* Delete File */
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    result = MFS_DELETE((DWORD)address);
		    LOGSTR((LF_INT86,"DOS 21/41/DeleteFile: %s  result: %x\n",
			     address, result));
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x42:  /* Move File Pointer */
		    handle = envp->reg.bx & 0xffff;
		    distance = (long)(((envp->reg.cx & 0xffff) << 16) +
					envp->reg.dx);
		    mode = envp->reg.ax & 0xff;

		    result = MFS_SEEK((DWORD)handle, (DWORD)distance, (DWORD)mode, (DWORD)&location);
	    	    LOGSTR((LF_INT86,"DOS 21/42/LseekFile: handle: %x mode %x loc %x\n",
			     envp->reg.bx,mode,location));
		    if (result & CARRY_FLAG) {
			envp->reg.ax = result & 0xffff;
			envp->reg.flags = CARRY_FLAG >> 16;
		    }
		    else {
			envp->reg.dx = (location >> 16) & 0xffff;
			envp->reg.ax = location & 0xffff;
			envp->reg.flags = NO_ERROR;
		    }
		    break;

	    case 0x43:  /* Get/Set File Attributes */
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    attribute = envp->reg.cx & 0xffff;
		    mode = envp->reg.ax & 0xff;
		    LOGSTR((LF_INT86,"DOS 21/43/GetSetAttr: %s\n", address));
		    result = MFS_ACCESS((DWORD)mode, (DWORD)address,(DWORD)&attribute);
		    envp->reg.cx = attribute;
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x44:  /* IOCTL - I/O Control for Devices */
		    {
		      DWORD *attribute = NULL;
		      /* Only partially implemented */
		      func = envp->reg.ax & 0xff;
		      LOGSTR((LF_INT86,"DOS 21/44/Ioctl (no FAT support): func=%x\n", func));
		      mode = 0;
		      
		      switch(func)	{
		      case 0:		/* get device info */
			func = 	MFS_IOCTL_GETINFO;
			break;
		      case 1:		/* set device info */ 
			func = 	MFS_IOCTL_SETINFO;
			break;
		      case 6:		/* get input status */
			func = MFS_IOCTL_STATUS;
			break;
		      case 7:		/* get output status */
			func = MFS_IOCTL_READY;
			break;
			
		      case 8:
			func = MFS_IOCTL_ISREMOVABLE;
			break;
		      case 9:
			func = MFS_IOCTL_ISREMOTE;
			attribute = &envp->reg.dx;
			break;
		      case 10:
			func = MFS_IOCTL_ISREMOTE;
			mode = 1;
			break;
		      }
		      result = mfs_ioctl((DWORD)(envp->reg.bx & 0xffff), (DWORD)func, (DWORD)mode, (DWORD)attribute);
		      
		      if(result < 0) {
			envp->reg.ax = result & 0xffff;
			envp->reg.flags = (result & CARRY_FLAG) >> 16;
		      } else
			envp->reg.dx = result & 0xffff;
		      break;
		    }
	    case 0x45:  /* Duplicate Handle */
		    handle = envp->reg.bx & 0xffff;
		    LOGSTR((LF_INT86,"DOS 21/45/DupHandle (no FAT support): handle: %x\n", handle));

		    /* duplicate to first available handle */
		    result = mfs_fcntl((DWORD)handle, MFS_FCNTL_DUPHANDLE, 0L, 0xFFFFFFFF);

		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x46:  /* Force Duplicate Handle */
		    handle = envp->reg.bx & 0xffff;  /* existing file handle */
		    LOGSTR((LF_INT86,"DOS 21/46/ForceDupHandle (no FAT support): handle: %x\n", handle));
		    result = envp->reg.cx;  /* handle to hold dup   */

		    /* duplicate to given handle */
		    result = mfs_fcntl((DWORD)handle, MFS_FCNTL_DUPHANDLE, 0L, (DWORD)result);

		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x47:  /* Get Current Directory */
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.si);
		    drive = envp->reg.dx & 0xff;
		    result = MFS_GETCWD((DWORD)drive, (DWORD)buffer, 256L);

		    if(result == 0)
		    	mfs_config(XMFS_CFG_DOSNAME,0L,(DWORD)address,(DWORD)buffer);
#ifdef LATER
                    /*
                    **   Strip leading backslash!!!
                    */
                    if (*address == '\\')
                       strcpy(address, (char *)(address+1));
#endif
		    LOGSTR((LF_INT86,"DOS 21/47/GetCWD: %s dos=%s\n", buffer,address));

		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x4c:  /* Terminate with Return Code */
		    LOGSTR((LF_INT86,"DOS 21/4c/Exit: code: %x\n", envp->reg.ax & 0xff));
		    /* FatalExit(LOBYTE(LOWORD(envp->reg.ax))); */
		    DeleteTask(0);
		    break;

	    case 0x4e:  /* Find First Matching Directory Entry */
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    attribute = envp->reg.cx & 0xffff;

		    LOGSTR((LF_INT86,"DOS 21/4e/FindFirst: %s  attr: %x\n",
			     address,attribute));

    		    dta = (LPBYTE)GetAddress(HIWORD(dta_address), LOWORD(dta_address));

		    result = MFS_FINDFILE((DWORD)address, (DWORD)attribute, (DWORD)dta);

		    envp->reg.ax = result & 0xffff;

		    /* Must pass back time here, somehow */
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x4f:  /* Find Next Matching Directory Entry */
		    LOGSTR((LF_INT86,"DOS 21/4f/FindNext:\n"));

    		    dta = (LPBYTE)GetAddress(HIWORD(dta_address), LOWORD(dta_address));
		    result = MFS_FINDFILE (0L, 0L, (DWORD)dta);

		    envp->reg.ax = result & 0xffff;
		    /* Must pass back time here, somehow */
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x50:
		    TWIN_SetPSPSelector(GetCurrentTask(), envp->reg.bx);
		    break;
	
	    case 0x54:  /* Get Verify Flag */
		    result = disk_verify;
		    LOGSTR((LF_INT86,"DOS 21/54/GetVerifyFlag: %s\n", 
			    (result) ? "ON" : "OFF"));
		    envp->reg.ax = (envp->reg.ax & 0xff00) | (result & 0xff);
		    break;

	    case 0x56:  /* Rename File */
		    address  = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    address1 = (char *)GetAddress(envp->reg.es,envp->reg.di);
		    LOGSTR((LF_INT86,"DOS 21/56/RenameFile: %s --> %s\n", 
			     address, address1));
		    result = MFS_RENAME((DWORD)address, (DWORD)address1, 0);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x57:  /* Get/Set File Date and Time */
		{
		    unsigned int access_time, access_date;

		    handle = envp->reg.bx & 0xffff;
		    access_time = envp->reg.cx;
		    access_date = envp->reg.dx;
		    func = envp->reg.ax & 0xff;

		    result = MFS_TOUCH((DWORD)handle, (DWORD)func, (DWORD)&access_time, (DWORD)&access_date);

		    LOGSTR((LF_INT86,"DOS 21/57/FileDateTime: f %x h %x time %x date %x res %x\n", 
			    func,handle,access_time,access_date,result));

		    if ((result & CARRY_FLAG) == 0) {
			envp->reg.cx = access_time;
			envp->reg.dx = access_date;
		    }
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;
		}

 	    case 0x59:  /* Get Extended Error Information */
		{
		    err = (ERRORCODE *) error_code(NO_ERROR); 
		    envp->reg.ax = err->ec_extended;
		    envp->reg.bx = err->ec_action + (err->ec_class << 8);
		    envp->reg.cx = (envp->reg.cx & 0xff) + (err->ec_locus << 8);
		    LOGSTR((LF_INT86,"DOS 21/59/GetExtErrorInfo %x %x %x %x\n",
				err->ec_extended,err->ec_action,
				err->ec_class,err->ec_locus));
		    break;
		}

	    case 0x5a:  /* Create Temporary File */
		    address  = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    LOGSTR((LF_INT86,"DOS 21/5a/CreateTempFile: %s\n", address));
		    attribute = envp->reg.cx & 0xffff;
		    result = MFS_CREATE((DWORD)address, (DWORD)attribute);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x5b:  /* Create New File */
		    address  = (char *)GetAddress(envp->reg.ds,envp->reg.dx);
		    LOGSTR((LF_INT86,"DOS 21/5b/CreateNewFile: %s\n", address));
		    attribute = envp->reg.cx & 0xffff;
		    result = MFS_CREATE((DWORD)address, (DWORD)attribute);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;
		    
	    case 0x5c:  /* Lock/Unlock File Region */
		    handle = envp->reg.bx & 0xffff;
		    location = envp->reg.dx + (envp->reg.cx << 16);
		    distance = envp->reg.di + (envp->reg.si << 16);
		    mode = envp->reg.ax & 0xff;
		    LOGSTR((LF_INT86,"DOS 21/5c/Lock-Unlock: %s %x bx: %x cx: %x dx: %x si: %x, di: %x\n",
			     (envp->reg.ax & 0xff) ? "UNLOCK" : "LOCK",envp->reg.bx,
			     envp->reg.cx,envp->reg.dx,
			     envp->reg.si,envp->reg.di));
#ifdef LATER
		    result = lock_file(handle, location, distance, mode);
#else
		    result = 1;
#endif
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x62:	/* Get PSP address */
		    envp->reg.bx = (REGISTER)GetPSPSelector();
		    envp->reg.flags = 0;
		    LOGSTR((LF_INT86,"DOS 21/62/GetPSPAddress selector=%x\n",
			envp->reg.bx));
		    break;


	    case 0x66: /* GLOBAL CODE PAGE TABLE */
		/* al = subfunc */
		func = envp->reg.ax & 0xff;
		switch (func)
		{
		case 0x01:
		    envp->reg.dx = CodePage;
		    envp->reg.bx = CodePage;
		    envp->reg.flags = 0;
		    break;
		case 0x02:
		    CodePage = envp->reg.bx;
		    envp->reg.flags = 0;
		    break;
		}
		LOGSTR((LF_INT86,"DOS 21/66/GLOBAL CODE PAGE=%d\n",envp->reg.bx));
		break;


	    case 0x67:  /* Set Maximum Handle Count */
		    handle = envp->reg.bx & 0xFFFF;
		    LOGSTR((LF_INT86, "DOS 21/67/SetMaxFileHandle: handle %x\n", handle));
		    result = mfs_config(XMFS_CFG_SETMAXFH,(DWORD)handle,0L,0L);
		    envp->reg.ax = result & 0xFFFF;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x68:	/* Commit File */
		    LOGSTR((LF_INT86,"DOS 21/68/CommitFile: handle %x\n",
				envp->reg.bx));
		    handle = envp->reg.bx & 0xffff;
		    result = MFS_FLUSH((DWORD)handle);
		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;

	    case 0x6c:	/* Extended Open/Create */
		    {
		    int action;

		    LOGSTR((LF_INT86,"DOS 21/6c/Extended Open/Create: (no FAT support)\n"));

		    mode = envp->reg.bx & 0xff;
		    address = (char *)GetAddress(envp->reg.ds,envp->reg.si);
		    attribute = envp->reg.cx & 0xffff;
		    action    = envp->reg.dx;

		    result = mfs_opencreate (address, mode, attribute, action);

		    envp->reg.ax = result & 0xffff;
		    envp->reg.flags = (result & CARRY_FLAG) >> 16;
		    break;
		    }

	    case 0xdc:	/* get station number */
		    envp->reg.ax = 0;
		    envp->reg.flags = 0;
		    break;

	    default:
		    logstr(LF_ERROR,"DOS INT21: Unsupported func %x\n",func);
		    envp->reg.ax = 0;
		    envp->reg.flags = CARRY_FLAG >> 16;
		    break;
	    }

    envp->reg.flags |= other_flags;
}


void 
CopyDTAData(LPBYTE dta,char *fname,WORD ftime,WORD fdate,DWORD fsize,BYTE fattr)
{
    *(dta + 0x15) = fattr;

    PUTWORD(dta+0x16, ftime);
    PUTWORD(dta+0x18, fdate);

    PUTDWORD(dta+0x1a, fsize);

    memcpy(dta+0x1e, fname, 12);

    *(dta + 0x1e + 12) = 0;	/* null-terminate filename string */
}

void
CopyDTAToC(LPBYTE dta, struct find_t *lpsft)
{
    lpsft->attrib = *(dta + 0x15);
    lpsft->wr_time = GETWORD(dta+0x16);
    lpsft->wr_date = GETWORD(dta+0x18);
    lpsft->size = GETDWORD(dta+0x1a);
    memcpy(&lpsft->name[0],dta+0x1e,13);
}

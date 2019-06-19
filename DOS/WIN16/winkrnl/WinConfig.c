/*    
	WinConfig.c	1.23
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
/*
 * Manage configuration parameters through a single configuration file.
 * 
 * These environment variables control parameters:
 *
 * TWINRC		location of configuration file if set
 * 			default, current working directory, home, willows
 * HOME			location of users home directory if set
 * 			default, value retrieved from pwnam
 * LOGNAME		user name if set
 *			default, use value from pwnam
 * DISPLAY		display server if set
 * 			default, value retrieved from twinrc file
 * PATH			path used to find and open files,
 *			default if not in TWINRC is to use PATH
 * LOGFILE		file to use for outputdebug string if set
 *			
 *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Driver.h"
#include "WinConfig.h"
#include "platform.h"

/**************************************************************************
 *
 * [boot]		startup configuration settings
 * [windows]		run-time configuration settings
 * [xdos]		mapping of DOS drives
 * [commondialogs]	how to drive common dialog fileopen dialog
 *
 * [fontalias]		what font mapping to use
 * [colors]		what colors to use
 * [trace]		what tracing options...
 *  
 **************************************************************************
 *
 *	parameter	what parameter is being addressed
 *	opcode	
 *		1	use environment as override if set
 *			use section, keyname, default to lookup string
 *		2	use section, keyname, return string
 *		3       return keynames for section
 *		4	lparam is data to convert...
 *		5	use section, keyname, environment to lookup string
 *			if no environment, use default
 *		6	if environment string exists, use getenv to
 *			retrieve it, otherwise return default
 *
 *	lpszsection	section to use
 *	lpszkeyname	keyname to use
 *	lpszdefault	default value
 *	lpszenviron	environment string
 *	lparam		optional long argument
 *	lpszstring	allocated buffer
 *
 *************************************************************************/


static char *sections[] = {
	"boot",
	"windows",
	"XDOS",
	"CommonDialogs"
};

static TWINRC  *wcpdata = NULL;


/* name of configuration file */
char *
GetTwinFilename(void)
{
	static char *twinrc;

	/* have we already set it */
	if(twinrc)
		return twinrc;

	twinrc = (char *) DRVCALL_CONFIG(PCFGH_GETFILENAME, 0, 0 , NULL);

	return twinrc;
	
}


char *
GetTwinBuffer(TWINRC *tp,char *wcpbuf)
{
	/* if we have a buffer, use it */
	if(wcpbuf)
		return wcpbuf;

	/* if we already allocated a buffer, return it */
	if(tp->lpszstring)
		return tp->lpszstring;

	/* allocate a permanent buffer */
	if(tp->lparam == 0)
		tp->lparam = 256;
	tp->lpszstring = (char *) WinMalloc(tp->lparam);

	/* return it */
	return tp->lpszstring;	
}

char *
GetTwinString(int wcp, char *wcpbuf,int nsize )
{
	TWINRC  *tp;
	char     *p = NULL;
	static char *twinrc;
	char	buffer[256];
	char	keynames[256];
	char  slash = TWIN_SLASHCHAR;

	if(twinrc == 0)
		twinrc = GetTwinFilename();
	
	if (wcpdata == NULL)
		wcpdata = (LPTWINRC)DRVCALL_CONFIG(PCFGH_GETDEFAULTS, 0, 0 , NULL);
	for(tp = wcpdata;tp->parameter > 0;tp++) {
		if(tp->parameter == wcp) {
		  switch(tp->opcode) {
			case 1:
				/* if we have an environment string, and it
 				 * is defined, return it, otherwise lookup
 				 * in the given section the given keyname and 
				 * return that. A default is provided for 
				 * the lookup, in the event it is not defined
				 * in the file.
				 */
				if(tp->lpszenviron && 
				   (p = (char *) getenv(tp->lpszenviron))) {
				       strcpy(GetTwinBuffer(tp,wcpbuf),p);
				} else {
					p = GetTwinBuffer(tp,wcpbuf);
					if(nsize == 0)
						nsize = tp->lparam;
					GetPrivateProfileString(
						sections[tp->section],
						tp->lpszkeyname,
						tp->lpszdefault,
						p,
						nsize, 
						twinrc);
				}
				return p;

			case 2:
				/* if an environment string is given, and has
				 * a defined value, use that as the default 
				 * when looking up the keyname, otherwise use 
				 * the default when looking up the keyname in 
				 * the given section.
				 */
				if(tp->lpszenviron)
				   p = (char *) getenv(tp->lpszenviron);
				if(p == 0)
				    p = tp->lpszdefault;
				       
				GetPrivateProfileString(
		  			sections[tp->section],
					tp->lpszkeyname,
					p,
					GetTwinBuffer(tp,wcpbuf),
	    				nsize, 
					twinrc);
				return GetTwinBuffer(tp,wcpbuf);
			case 3:
				/* return section,... */
				return 0;
			case 4:
				/* return section,... */
				sprintf(buffer,"%d",(int)tp->lparam);

				GetPrivateProfileString(
		  			sections[tp->section],
					tp->lpszkeyname,
					buffer,
					wcpbuf,
	    				nsize, 
					twinrc);
				break;
			case 5:
				/* section,parameter is keyname and default */
				strcpy(keynames,wcpbuf);
				GetPrivateProfileString(
		  			sections[tp->section],
					keynames,
					0,
					wcpbuf,
	    				nsize, 
					twinrc);
				break;

			 case 6:
				/* reading the windows path    */
				/* 1) use environment WINDOWS  */
				/* 2) use windows in twinrc    */
				/* 3) default of 0 implies     */
				/*    use same path as twinrc  */
				
				if(tp->lpszenviron && 
				   (p = (char *) getenv(tp->lpszenviron))) {
				       strcpy(GetTwinBuffer(tp,wcpbuf),p);

				       /* we got the WINDOWS variable */
				       return GetTwinBuffer(tp,wcpbuf);
				}
			
				/* get the value from the file */
				GetPrivateProfileString(
		  			sections[tp->section],
					tp->lpszkeyname,
					0,
					GetTwinBuffer(tp,wcpbuf),
	    				nsize, 
					twinrc);

				/* if we got something return it */
				if(strlen(GetTwinBuffer(tp,wcpbuf))) {
					return GetTwinBuffer(tp,wcpbuf);
				}
					
				/* use dirname(TwinFilename) */
				strcpy(GetTwinBuffer(tp,wcpbuf),
					GetTwinFilename());

				/* terminate */
				if((p = strrchr(wcpbuf,slash)))
					*p = 0;

				/* worst case same as TwinFilename directory */
				return GetTwinBuffer(tp,wcpbuf);
				
			default:
				break;
		  }
		  return wcpbuf;
		}
	}

	return 0;
}

unsigned long 
GetTwinInt(int wcp)
{
	char wcpbuffer[256];
	char *wcptr;

	wcptr = GetTwinString(wcp,wcpbuffer,256);
	
	return strtol(wcptr,0,0);
}


/********************************************************************
*	GetEnv
*
*	Our version of getenv that reads the [Environment] section of the twinrc file to obtain variables.
********************************************************************/
char *GetEnv(const char *env)
{
	char *twinrc = GetTwinFilename();
	static char buf[256];
	int count;

	/* Try and retrieve the environment variable */
	count = GetPrivateProfileString(sections[WCS_ENV], env, NULL, buf, 256,
		twinrc);

	if (count == 0)
		/* The default was used, meaning nothing was found */
		return(NULL);
	else
		/* Return the buffer containing the value found in the twinrc
		 * [Environment] section.
		 */
		return(buf);

}

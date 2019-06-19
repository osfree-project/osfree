/*    
	fn_38.c	1.8
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

#include "platform.h"

#include "xdos.h"
#include "dosdefn.h"

unsigned long error_code(unsigned int);

static char country_info_block[32] = { 
				0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x2c, 
				0x00, 0x2e, 0x00, 0x2d, 0x00, 0x3a, 0x00, 0x00, 
				0x02, 0x00, 0x6c, 0x17, 0x70, 0x02, 0x2c, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				};

unsigned int 
country_info( int code, char *info )
{
	int	i;
	unsigned int maj_ver;

	maj_ver = (GetVersion() >> 24) & 0xff;
	switch (maj_ver) {
	    case 2:
		if (code != 0)	/* code must be 0 for dos 2.10 */
			return (error_code (FILE_NOT_FOUND));
		if (info == NULL)
			return (error_code (FILE_NOT_FOUND));
		info[0] = 0;	/* use standard h:m:s m/d/y */
		info[1] = 0;
		info[2] = '$';	/* currency symbol */
		info[3] = 0;
		info[4] = ',';	/* thousands separator */
		info[5] = 0;
		info[4] = '.';	/* decimal separator */
		info[5] = 0;
		return NO_ERROR;

	    case 3:
	    case 5:	/* Need to see if DOS 5.0 has more functionality */

		if (info == NULL) {	/* this is set country code */
			if ((code == 0) || (code == 1)) 
				return NO_ERROR;
			return (error_code( INVALID_FUNCTION ));
		}
		else if (code == 0) {
			for (i =0; i < 32; i++)
				info[i] = country_info_block[i];
			return NO_ERROR;
		}
	}
	return (error_code( INVALID_FUNCTION ));
}


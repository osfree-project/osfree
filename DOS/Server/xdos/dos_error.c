/*    
	dos_error.c	1.8
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
#include "dos.h"

static ERRORCODE errorcode;

unsigned long
error_code( unsigned int dos_code )
{
	if (dos_code == NO_ERROR)
	    return (unsigned long) &errorcode;

	errorcode.ec_extended = dos_code;

	switch (dos_code) {

	case INVALID_FUNCTION:
		errorcode.ec_class = 0x07;
		errorcode.ec_action = 0x04;
		break;
	case FILE_NOT_FOUND:
		errorcode.ec_class = 0x08;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x02;
		break;
	case PATH_NOT_FOUND:
		errorcode.ec_class = 0x08;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x02;
		break;
	case NO_HANDLES:
		errorcode.ec_class = 0x01;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x01;
		break;
	case ACCESS_DENIED:
		errorcode.ec_class = 0x03;
		errorcode.ec_action = 0x03;
		break;
	case INVALID_HANDLE:
		errorcode.ec_class = 0x07;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x01;
		break;
	case MEMORY_DESTROYED:
		errorcode.ec_class = 0x07;
		errorcode.ec_action = 0x05;
		errorcode.ec_locus = 0x05;
		break;
	case INSUFFICIENT_MEMORY:
		errorcode.ec_class = 0x01;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x05;
		break;
	case INVALID_ADDRESS:
		errorcode.ec_class = 0x07;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x05;
		break;
	case INVALID_ENVIRONMENT:
		errorcode.ec_class = 0x07;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x05;
		break;
	case INVALID_FORMAT:
		errorcode.ec_class = 0x09;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x01;
		break;
	case INVALID_ACCESS:
		errorcode.ec_class = 0x07;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x01;
		break;
	case INVALID_DATA:
		errorcode.ec_class = 0x09;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x01;
		break;
	case INVALID_DRIVE:
		errorcode.ec_class = 0x08;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x02;
		break;
	case REDIR_OFF_SERVER:
		errorcode.ec_class = 0x03;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x02;
		break;
	case NOT_SAME_DEVICE:
		errorcode.ec_class = 0x0D;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x02;
		break;
	case NO_MORE_FILES:
		errorcode.ec_class = 0x08;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x02;
		break;
	case FILE_EXISTS:
		errorcode.ec_class = 0x0C;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x02;
		break;
	case SHARING_VIOLATION:
		errorcode.ec_class = 0x0A;
		errorcode.ec_action = 0x02;
		errorcode.ec_locus = 0x02;
		break;
	case LOCK_VIOLATION:
		errorcode.ec_class = 0x0A;
		errorcode.ec_action = 0x02;
		errorcode.ec_locus = 0x02;
		break;
	case TOO_MANY_REDIRECTIONS:
		errorcode.ec_class = 0x01;
		errorcode.ec_action = 0x04;
		break;
	case INVALID_PASSWORD:
		errorcode.ec_class = 0x03;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x01;
		break;
	case CANNOT_MAKE_DIRECTORY_ENTRY:
		errorcode.ec_class = 0x01;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x02;
		break;
	case NETWORK_REQUEST_UNSUPPORTED:
		errorcode.ec_class = 0x09;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x03;
		break;
	case DUPLICATE_REDIRECTION:
		errorcode.ec_class = 0x0C;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x03;
		break;
	case INVALID_PARAMETER:
		errorcode.ec_class = 0x09;
		errorcode.ec_action = 0x03;
		errorcode.ec_locus = 0x01;
		break;
	case FAIL_ON_INT24:
		errorcode.ec_class = 0x0D;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x01;
		break;
	case SHARE_BUF_EXCEED:
		errorcode.ec_class = 0x01;
		errorcode.ec_action = 0x04;
		errorcode.ec_locus = 0x05;
		break;
	default:
		break;
	}
	return (CARRY_FLAG | dos_code);
}


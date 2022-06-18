/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/* 
 * $Id$
 */

#include <rhbendia.h>
#include <stdio.h>

#ifdef __STDC__
int main(int argc,char **argv)
#else
int main(argc,argv) int argc; char **argv;
#endif
{
short s=*(short *)"\001\000\000\000";
#ifdef _PLATFORM_BIG_ENDIAN_
#	ifdef _PLATFORM_LITTLE_ENDIAN_
		#error endian ambiguous
#	else
		if (s < 256)
		{
			fprintf(stderr,"build(big) does not match exec(little)\n");
			return 1;
		}
		printf("_PLATFORM_BIG_ENDIAN_\n");
#	endif
#else
#	ifdef _PLATFORM_LITTLE_ENDIAN_
		if (s > 1)
		{
			fprintf(stderr,"build(little) does not match exec(big)\n");
			return 1;
		}
		printf("_PLATFORM_LITTLE_ENDIAN_\n");
#	else
		#error endian not known
#	endif
#endif
	return 0;
}

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

#include <rhbopt.h>
#include <stdio.h>

#ifdef HAVE_DLFCN_H
#	include <dlfcn.h>
#endif

int main(int argc,char **argv)
{
	int i=1;

	while (i < argc)
	{
#if defined(HAVE_DLOPEN) && defined(HAVE_DLFCN_H)
		char *p=argv[i];

		void *dl=dlopen(p,0
				#ifdef RTLD_GLOBAL
					|RTLD_GLOBAL
				#endif
				#ifdef RTLD_NOW
					|RTLD_NOW
				#endif
					);
		if (dl)
		{
			dlclose(dl);
		}
		else
		{
			fprintf(stderr,
				"%s failed with %s\n",
				p,
				dlerror());
			return 1;
		}
#else
		if (!argv[i]) return 1;
#endif

		i++;
	}

	return 0;
}

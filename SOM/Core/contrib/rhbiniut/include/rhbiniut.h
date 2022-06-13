/**************************************************************************
 *
 *  Copyright 2012, Roger Brown
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

#ifndef __rhbiniut_h__
#define __rhbiniut_h__

struct rhbinifile;

struct rhbinifileVtbl
{
	int (*QueryInterface)(struct rhbinifile *,const void *iid,void **ppv);
	int (*AddRef)(struct rhbinifile *);
	int (*Release)(struct rhbinifile *);
	const char *(*GetValue)(struct rhbinifile *,const char *name,const char *stanza);
};

struct rhbinifile
{
	struct rhbinifileVtbl *lpVtbl;
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct rhbinifile *rhbinifile(const char *path);

#ifdef __cplusplus
}
#endif

#endif

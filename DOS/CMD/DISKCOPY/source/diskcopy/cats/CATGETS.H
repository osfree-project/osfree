/* $Id: catgets.h,v 1.2 2000/06/09 03:12:01 jhall Exp $ */

/* Functions that emulate UNIX catgets */

/* Copyright (C) 1999,2000 Jim Hall <jhall1@isd.net> */

/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef _CATGETS_H
#define _CATGETS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Data types */

typedef int nl_catd;


/* Symbolic constants */

#define MCLoadBySet 0			/* not implemented */
#define MCLoadAll   0			/* not implemented */


/* Functions */

char *
catgets(nl_catd  cat,  int set_number, int message_number,
        char *message);

nl_catd
catopen(char *name, int flag);

void
catclose (nl_catd cat);

#ifdef __cplusplus
}
#endif

#endif /* _CATGETS_H */

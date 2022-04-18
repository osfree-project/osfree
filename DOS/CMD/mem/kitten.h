/* $Id: kitten.h,v 1.1 2004/09/19 10:01:26 bartoldeman Exp $ */

/* Functions that emulate UNIX catgets */

/* Copyright (C) 1999,2000 Jim Hall <jhall@freedos.org> */

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


#ifdef NOCATS

/* #define NOCATS to disable that stuff completely */
#define nl_catd int
#define catopen(x,y) 1
#define catclose(x) 
#define catgets(c,x,y,s) s

#else




#ifdef __cplusplus
extern "C" {
#endif

  /* Data types */

  typedef int nl_catd;

  /* Symbolic constants */

#define MCLoadBySet 0			/* not implemented */
#define MCLoadAll   0			/* not implemented */


  /* Functions */

#define catgets(catalog, set,message_number,message) kittengets(set,message_number,message)
#define catopen(name,flag) kittenopen(name)
#define catclose(catalog)  kittenclose()


  char *  pascal kittengets( int set_number, int message_number,char *message);
  nl_catd kittenopen(char *name);
  void    kittenclose (void);



#ifdef __cplusplus
}
#endif

#endif /* NOCATS */

#endif /* _CATGETS_H */

/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, 2001 Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be

 */

#ifndef INTERNATIONALIZATION_ASPECT_H_
#define INTERNATIONALIZATION_ASPECT_H_

/*
** Poor man's aspect oriented implementation of the internationalization
** aspect on diskcopy.
**
** Crosscuts are explicitelly named in the component code. I.e. no full
** seperation of concerns.
*/

#include "cats\catgets.h"

#define DISKCOPY_NLS_FILE_BASE "diskcopy"

/*
** Aspect definition to define the data used in the implementation
** of the aspect.
*/
#define CROSSCUT_NLS_DATA                                  \
 char *Cade[5];                                            \
 nl_catd catd;                                             \
                                                           \
 char CatYES='Y'; /* Default values for YES and NO, */     \
 char CatNO ='N'; /* can be overwritten */

/*
** This definition imports the NLS data objects in the
** current name space.
*/
#define CROSSCUT_NLS_DATA_IMPORT                \
 extern char* Cade[];                           \
 extern nl_catd catd;                           \
                                                \
 extern char CatYES;                            \
 extern char CatNO;

/*
** This definition opens the catalog and sets catYES and catNO,
** i.e. the NLS values for 'Y' and 'N'.
*/
#define CROSSCUT_NLS_OPEN                               \
{                                                       \
  /* Try opening NLS catalog*/                          \
  catd = catopen (DISKCOPY_NLS_FILE_BASE, 1);           \
                                                        \
  /* Get the definition for YES and NO chars */         \
  Cade[0] = catgets (catd, 0, 0, "Y");                  \
  CatYES=*(char *)Cade[0];                              \
  Cade[0] = catgets (catd, 0, 1, "N");                  \
  CatNO =*(char *)Cade[0];                              \
}

/*
** This definition closes the catalog.
*/
#define CROSSCUT_NLS_CLOSE                              \
{                                                       \
  /* Close the catalog */                               \
  catclose(catd);                                       \
}


/*
** This (generic) aspect gets the indicated message from the catalog
** and writes it on the screen.
*/
#define NLS_PRINTSTRING(setnum, msgnum, message)      \
{                                                     \
  Cade[0] = catgets (catd, setnum, msgnum, message);  \
  printf("%s", Cade[0]);                              \
}

/*
** This (generic) aspect gets the indicated message from the catalog
** and writes it on the screen, followed by a newline.
*/
#define NLS_PUTSTRING(setnum, msgnum, message)        \
{                                                     \
  Cade[0] = catgets (catd, setnum, msgnum, message);  \
  printf("%s", Cade[0]);                              \
  puts("");                                           \
}

/*
** This definition returns the indicated string.
*/
#define NLS_STRING(setnum, msgnum, message)     \
  catgets(catd, setnum, msgnum, message)


/*
** These definitions test wether the char is the NLS char for YES or NO.
*/

#define NLS_TEST_YES_NO(x)         \
  ((x == CatYES) || (x == CatNO))

#define NLS_TEST_YES(x)            \
  (x == CatYES)

#define NLS_TEST_NO(x)             \
  (x == CatNO)

#endif

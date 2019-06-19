/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, Imre Leber.

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

/*
   ** EMS.H
   **
   ** Header file Expanded Memory Routines
 */

#ifndef EMS_H_
#define EMS_H_

#define EMS_PAGE_SIZE   16384	/* Each page is this size */

unsigned int EMSbaseaddress (void);
int EMSversion (void);
int EMSstatus (void);
int EMSpages (void);
int EMSalloc (int pages);
int EMSfree (int handle);
int EMSmap (int bank, int handle, int page);

#endif

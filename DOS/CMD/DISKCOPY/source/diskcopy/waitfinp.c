/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MS-DOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, 2001, Imre Leber.

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
   email me at:  imre.leber@worlonline.be

   module: waitfinp.c - routine to wait for the user to press any key
   or press any button.
 */

#include <conio.h>

#include "mouse.h"
#include "waitfinp.h"

#define AMOFBUTTONS 3

/*
   **  Returns wether any button was pressed and returns that button or 
   **  0 if no button pressed.
 */

static int
AnyButtonPressed ()
{
  int presses, x, y;

  return CountButtonPresses (0, &presses, &x, &y) & 7;
}

/*
   ** Clears the keyboard buffer.
 */
void
ClrKbd ()
{
  while (kbhit ())
    getch ();
}

/*
   ** Clears the mouse from any presses.
 */
static void
ClearMouse ()
{
  int i, x, y, pressreleases;

  for (i = 0; i < AMOFBUTTONS; i++)
    {
      CountButtonPresses (i, &pressreleases, &x, &y);
    }
}

/*
   ** Real routine: waits until the user has either pressed a key or a button.
 */

int
WaitForInput ()
{
  ClearMouse ();
  ClrKbd ();

  for (;;)
    {
      if (AnyButtonPressed ())	/* Mouse button pressed. */
	return -1;
      if (kbhit ())		/* Key pressed.          */
	return getch ();
    }
}
